/**
 * dxilCommRx.c
 * 
 * RX Communication task
 *
 * @author: Alessandro Mannini <alessandro.mannini@gmail.com>
 * @date: Jan 10, 2023
 */

/* includes */
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <vxWorks.h>
#include <msgQLib.h>
#include <sockLib.h>
#include <taskLib.h>
#include <syslog.h>

#include "dixlComm.h"
#include "dixlLog.h"
#include "../config.h"
#include "../datatypes/messages.h"
#include "../network.h"
#include "../utils.h"

/* variables */
int dixlCommRxSocket = 0;

// Input message queue
TASK_ID     taskCommRxId;

// Received data buffer
char buffer[COMMBUFFERSIZE];
int bufferLen = 0;
/**
 * Process data received from the stream:
 * @param stream: chunk of stream data received
 * @param streamLen: data chunk length
 * 
 * - store data in the buffer updating bufferLen
 * - get message length (msgLen) from first byte
 * - if msgLen <= bufferLen then move (removing from buffer) data in msg struct and process it
 */
static bool process_message(char *stream, ssize_t streamLen) {
	
	// If chunk is empty, nothing to do
	if (streamLen <=0) return FALSE;

	// Copy received chunck to the and of the buffer
	// Overflow isn't possibile because of these lengths:
	// - streamLen <= MSG_MAXLENGTH
	// - buffer = 2 * MSG_MAXLENGTH
	// and when complete a message is removed
	memcpy(&buffer[bufferLen], stream, streamLen);
	bufferLen += streamLen;
	
	// While the buffer containts a complete message, process it
	uint8_t messageLen = 0;
	while (( messageLen = buffer[0] ) <= bufferLen ) {
	 	
		// The message is complete: copy data to message struct removing from buffer
		message message;
		memcpy(&message, buffer, messageLen);					// Copy data in message struct
		bufferLen -= messageLen;								// Remaining data in the buffer
		memcpy(buffer, &buffer[messageLen], bufferLen);			// Move remaining data to the start of the buffer		
		// Get message data
		eMsgType messageType = message.header.type;
		
		// Process the message (EXTERNAL TYPES)
		switch (messageType) {
			// INIT messages
			case MSGTYPE_NODECONFIG:
			case MSGTYPE_NODERESET:
				// Send to dixlInit task queue
				msgQ_Send(msgQInitId, (char *) &message, messageLen);	
				break;
				
			// LOG Messages
			case MSGTYPE_LOGREQ:
			case MSGTYPE_LOGSEND:			
			case MSGTYPE_LOGDEL:
				// Send to dixlLog task queue
				msgQ_Send(msgQLogId, (char *) &message, messageLen);	
				break;
	
			// ROUTE Messages
			case MSGTYPE_ROUTEREQ:
			case MSGTYPE_ROUTEACK:
			case MSGTYPE_ROUTENACK:
			case MSGTYPE_ROUTECOMMIT:
			case MSGTYPE_ROUTEAGREE:
			case MSGTYPE_ROUTEDISAGREE:				
				// Send to dixlCtrl task queue
				msgQ_Send(msgQCtrlId, (char *) &message, messageLen);					
				break;
				
			// UNKNOWN Messages	
			default:
				return FALSE;
		}	
	}
	
	return true;
}

void dixlCommRx() {
	
	// Start
	syslog(LOG_INFO, "Task started Id 0x%jx", taskCommRxId);
	
	// Create the listening socket
	if ((dixlCommRxSocket = socket_create(COMMSOCKDOMAIN, COMMSOCKTYPE, COMMSOCKPROTOCOL)) == 0)
		exit(rcSOCKET_INITERR);
	
	// Bind the socket to network interface and port
	if ( socket_bind(dixlCommRxSocket, IPv4s, COMMSOCKPORT) != SOCK_OK)
		exit(rcSOCKET_BINDERR);	 
	syslog(LOG_INFO, "RX socket created");
	
	// Listen from the socket
	if ( socket_listen(dixlCommRxSocket) != SOCK_OK)
		exit(rcSOCKET_LISTENERR);	
	syslog(LOG_INFO, "Listening on %03d.%03d.%03d.%03d:%d ...",IPv4.bytes[0], IPv4.bytes[1], IPv4.bytes[2], IPv4.bytes[3], COMMSOCKPORT);

	FOREVER {		
		// Waiting for a connection
		int receiveSocket = 0;			/* The new socket created to receive data for each connection */			
		if (( receiveSocket = socket_accept(dixlCommRxSocket)) == SOCK_ERROR) {
			socket_close(dixlCommRxSocket);
			exit(rcSOCKET_ACCEPTERR);		
		}

		// Initialize buffer length
		bufferLen = 0;
		
		// Receive and process messages until connection close
		FOREVER {
			// Receive data from TCP stream: max message length bytes
			// Max a message longest message at time, data gathered in the buffer area until at lesat a complete message is received
			// Multiple shorter messages can be received and processed at each socket_recv
			char stream[MSG_MAXLENGTH];
			ssize_t streamLen = 0;

			if (( streamLen = socket_recv(receiveSocket, (char *) &stream, sizeof(stream))) == SOCK_ERROR || streamLen == 0) {
				// if ERROR or NO DATA don't exit the task:
				// - close the client socket
				// - wait for a new connection
				break;
			}		

			// Process the message
			process_message(stream, streamLen);
		}
		
		// Close the receive socket anyway
		socket_close(receiveSocket);

	}
}

