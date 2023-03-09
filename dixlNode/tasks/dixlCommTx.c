/**
 * dxilCommTx.c
 * 
 * TX Communication task
 *
 * @author: Alessandro Mannini <alessandro.mannini@gmail.com>
 * @date: Jan 10, 2023
 */

/* includes */
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <msgQLib.h>
#include <taskLib.h>
#include <syslog.h>

#include "dixlComm.h"
#include "../config.h"
#include "../datatypes/messages.h"
#include "../network.h"
#include "../utils.h"

/* variables */
// Task
TASK_ID     taskCommTxId;

// Input message queue
MSG_Q_ID 	msgQCommTxId;
VX_MSG_Q(msgQCommTxName, MSGQCOMMTXMESSAGESMAX, MSGQCOMMTXMESSAGESLENGTH);

// Host node address for direct communication
nodeId hostNode = {0, 0, 0, 0};

/* Implementation functions */
/** 
 * Acquire the configuration parameters
 * @param inMessage
 * @return
 */
static void set_config(const message *inMessage) {
	// Host node address
	hostNode = inMessage->commTxIConfigSet.hostNode;
	
	// Log
	syslog(LOG_INFO, "Host node address set to %d.%d.%d.%d", hostNode.bytes[0], hostNode.bytes[1], hostNode.bytes[2], hostNode.bytes[3]);
}
	
/** 
 * Reset the configuration parameters
 * @param inMessage
 * @return
 */
static void reset_config(const message *inMessage) {
	// Host node address
	memset(&hostNode, 0, sizeof(hostNode));
	
	// Log
	syslog(LOG_INFO, "Host node address resetted");	
}
	
	
/**
 * Process received message preparing the message to send
 * @param inMessage: received message
 * @param outMessage: messeage to send
 * @return
 */
static BOOL process_message(const message *inMessage, message *outMessage) {	
	// Common section
	outMessage->header.source = IPv4;
	uint8_t size = sizeof(msgHeader);
	
	
	// Type specific section
	switch (inMessage->iHeader.type) {
		case IMSGTYPE_ROUTEREQ:
			outMessage->header.type = MSGTYPE_ROUTEREQ;
			outMessage->header.destination = inMessage->routeIReq.destination;
			outMessage->routeReq.requestRouteId = inMessage->routeIReq.requestRouteId;
			size += sizeof(msgRouteREQ);
			break;
			
		case IMSGTYPE_ROUTEACK:
			outMessage->header.type = MSGTYPE_ROUTEACK;
			outMessage->header.destination = inMessage->routeIAck.destination;
			outMessage->routeAck.requestRouteId = inMessage->routeIAck.requestRouteId;
			size += sizeof(msgRouteACK);
			break;
			
		case IMSGTYPE_ROUTENACK:
			outMessage->header.type = MSGTYPE_ROUTENACK;
			outMessage->header.destination = inMessage->routeINAck.destination;
			outMessage->routeNAck.requestRouteId = inMessage->routeINAck.requestRouteId;
			size += sizeof(msgRouteNACK);
			break;
			
		case IMSGTYPE_ROUTECOMMIT:	
			outMessage->header.type = MSGTYPE_ROUTECOMMIT;			
			outMessage->header.destination = inMessage->routeICommit.destination;
			outMessage->routeCommit.requestRouteId = inMessage->routeICommit.requestRouteId;
			size += sizeof(msgRouteCOMMIT);
			break;
			
		case IMSGTYPE_ROUTEAGREE:
			outMessage->header.type = MSGTYPE_ROUTEAGREE;			
			outMessage->header.destination = inMessage->routeIAgree.destination;
			outMessage->routeAgree.requestRouteId = inMessage->routeIAgree.requestRouteId;
			size += sizeof(msgRouteAGREE);
			break;
			
		case IMSGTYPE_ROUTEDISAGREE:
			outMessage->header.type = MSGTYPE_ROUTEDISAGREE;			
			outMessage->header.destination = inMessage->routeIDisagree.destination;
			outMessage->routeDisagree.requestRouteId = inMessage->routeIDisagree.requestRouteId;
			size += sizeof(msgRouteDISAGREE);
			break;
			
		case IMSGTYPE_ROUTETRAINOK:
			outMessage->header.type = MSGTYPE_ROUTETRAINOK;			
			outMessage->header.destination = inMessage->routeITrainOk.destination;
			outMessage->routeTrainOk.requestRouteId = inMessage->routeITrainOk.requestRouteId;
			size += sizeof(msgRouteTRAINOK);
			break;
			
		case IMSGTYPE_ROUTETRAINNOK:
			outMessage->header.type = MSGTYPE_ROUTETRAINNOK;			
			outMessage->header.destination = inMessage->routeITrainNOk.destination;
			outMessage->routeTrainNOk.requestRouteId = inMessage->routeITrainNOk.requestRouteId;
			size += sizeof(msgRouteTRAINNOK);
			break;
			
		case IMSGTYPE_LOGSEND:
			outMessage->header.type = MSGTYPE_LOGSEND;			
			outMessage->header.destination = inMessage->routeITrainNOk.destination;
			outMessage->logISend.currentLine = inMessage->logISend.currentLine;
			outMessage->logISend.totalLines = inMessage->logISend.totalLines;
			outMessage->logISend.line = inMessage->logISend.line;
			size += sizeof(msgLogSEND);
			break;
			
		case IMSGTYPE_LOGDELACK:
			outMessage->header.type = MSGTYPE_LOGDELACK;			
			outMessage->header.destination = inMessage->routeITrainNOk.destination;
			size += sizeof(msgLogDELACK);
			break;
			
		default:			
			return FALSE;
			
	}
	outMessage->header.lentgh = size;
	
	return TRUE;
}

/**
 * Send the message to destination node using socket
 * @param message: message to send
 * @return
 */
static BOOL send_message(const message *message) {
	int fd;					// Send socket file descriptor
	IPv4String destAddr;	// Destination address of the node
	
	// Create the socket
	if ((fd = socket_create(COMMSOCKDOMAIN, COMMSOCKTYPE, COMMSOCKPROTOCOL)) == 0)
		exit(rcSOCKET_INITERR);
	
	// Get node destination address
	network_IPv4_to_str(&(message->header.destination), destAddr);
	
	// Connect to the server (destination node)
	if (socket_connect(fd, destAddr, COMMSOCKPORT) == SOCK_ERROR) {
		close(fd);
		// if connection fail, close it, return FALSE but don't exit the task
		return FALSE;
	}
		
	// Connection ok, send data
	if (socket_send(fd, (void *) message, message->header.lentgh) == SOCK_ERROR) {
		close(fd);
		// if send fail, close the socket and return FALSE but don't exit the task
		return FALSE;
	}
	
	// Close the socket
	socket_close(fd);

	return TRUE;
}

void dixlCommTx() {
	
	// Start
	syslog(LOG_INFO,"Task started Id 0x%jx", taskCommTxId);	

	// Message queue initialization
	msgQCommTxId = msgQ_Initialize(msgQCommTxName, MSGQCOMMTXMESSAGESMAX, MSGQCOMMTXMESSAGESLENGTH, MSG_Q_FIFO);

	// Wait for message, process and send it by socket to the destination
	FOREVER {
		message inMessage;
		message extMessage;
		
		// Clear memory
		memset(&inMessage, 0, sizeof(inMessage));
		memset(&extMessage, 0, sizeof(extMessage));
				
		// Wait a message from the Queue ... FOREVER
		msgQReceive(msgQCommTxId, (char *) &inMessage, sizeof(inMessage), WAIT_FOREVER);
		
		// Comm Tx Receive only Internal messages to deliver outside the node
		switch (inMessage.iHeader.type) {
			// Internal CONFIG message
			case IMSGTYPE_COMMTXCONFIGSET:
				set_config(&inMessage);
				break;
				
			case IMSGTYPE_COMMTXCONFIGRESET:
				reset_config(&inMessage);
				break;
		
			// Route messages internal request to send to
			case IMSGTYPE_ROUTEREQ:
			case IMSGTYPE_ROUTEACK:
			case IMSGTYPE_ROUTENACK:
			case IMSGTYPE_ROUTECOMMIT:	
			case IMSGTYPE_ROUTEAGREE:
			case IMSGTYPE_ROUTEDISAGREE:
			case IMSGTYPE_ROUTETRAINOK:
			case IMSGTYPE_ROUTETRAINNOK:
			case IMSGTYPE_LOGSEND:
				// Process the message preparing it for External delivery
				if (process_message(&inMessage, &extMessage))				    
					// Delivery it to the destination through the socket
					send_message(&extMessage);
				break;
				
			// TOOO Sensors messages
			case IMSGTYPE_SENSORON:
			case IMSGTYPE_SENSOROFF:

								
			// Other messages discarded
			default:
				syslog(LOG_ERR, "Unattended message type (%d). Should not be send to Comm TX and will be ignored", inMessage.iHeader.type);
				break;
		}
	}
}

