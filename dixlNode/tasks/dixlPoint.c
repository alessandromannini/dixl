/**
 * dxilPoint.c
 * 
 * Point simulator task
 *
 * @author: Alessandro Mannini <alessandro.mannini@gmail.com>
 * @date: Jan 10, 2023
 */

/* includes */
#include <stdlib.h>
#include <stdbool.h>

#include <msgQLib.h>
#include <taskLib.h>
#include <sysLib.h>
#include <syslog.h>

#include "../config.h"
#include "../datatypes/messages.h"
#include "../globals.h"
#include "../utils.h"
#include "dixlPoint.h"
#include "dixlComm.h"

/* variables */
// Task
TASK_ID     taskPointId;

// Input message queue
MSG_Q_ID 	msgQPointId;
VX_MSG_Q(msgQPointName, MSGQPOINTMESSAGESMAX, MSGQPOINTMESSAGESLENGTH);


// Point State
static int8_t position = POINTPOS_STRAIGHT;
static int8_t requestedPosition = POINTPOS_STRAIGHT;
static struct timespec requestNonce;				// Nonce (timestamp) of the request (if 0 notification isn't sent)
static _Vx_freq_t stepTick;

/* Implementation functions */
static void initialize() {
	_Vx_freq_t tickRate = sysClkRateGet();
	
	// Copute floor approxmation of tick per step
	stepTick = math_ceil((TASKPOINTTRANSTIME * tickRate) , POINTPOS_DIVERGING);
	
	// Estimate real duration (due to tick resolution
	double realTransitionTime = (stepTick * POINTPOS_DIVERGING) / tickRate;
	double increment = (realTransitionTime - TASKPOINTTRANSTIME ) *100 / TASKPOINTTRANSTIME;
	
	syslog(LOG_INFO, "Straight <-> Diverging switch specs:");
	syslog(LOG_INFO, "> Initial position         : %s", pointpos_str(position));
	syslog(LOG_INFO, "> Number of steps          : %i", POINTPOS_DIVERGING);
	syslog(LOG_INFO, "> Requested switch time    : %is", TASKPOINTTRANSTIME);
	syslog(LOG_INFO, "> Tick rate (clock)        : %iHz", tickRate);
	syslog(LOG_INFO, "> Tick per step            : %i", stepTick);
	syslog(LOG_INFO, "> Real excepted switch time: %.2fs (+%0.f%)", realTransitionTime, increment);	
}

// Process a single message received
static void process_message(message message) {
	
	// If malfunction state, ignore all messages (physical reset needed)
	if (position != POINTPOS_UNDEFINED || message.header.type == IMSGTYPE_POINTRESET) {
		switch (message.header.type) {
			// Point RESET
			case IMSGTYPE_POINTRESET:
				requestedPosition = (int8_t) message.pointIReset.requestedPosition;
				position=requestedPosition;
				
				// Disable notify
				requestNonce.tv_nsec = 0;
				requestNonce.tv_sec = 0;
				
				// Log
				syslog(LOG_INFO, "Reset received with %s initial position", pointpos_str(requestedPosition));	
				
				break;
			
			// Positioning request
			case IMSGTYPE_POINTPOS:
				requestedPosition = (int8_t) message.pointIPosition.requestedPosition;
				requestNonce = message.pointIPosition.requestTimestamp;
				
				// Log
				syslog(LOG_INFO, "Request for %s positioning received with nonce %i", pointpos_str(requestedPosition), requestNonce);	
				
				break;
				
			// Malfunction simulation requested by the host
			case MSGTYPE_POINTMALFUNC:
				position = POINTPOS_UNDEFINED;
				requestedPosition = POINTPOS_UNDEFINED;				

				// Log
				syslog(LOG_INFO, "Request for MALFUNCTION simulation received");	
				
				break;
				
			// Other messages discarded
			default:
				syslog(LOG_ERR, "Unattended message type (%d). Should not be send to Point task and will be ignored", message.iHeader.type);
				break;
		}
	}
}

static int worker() {
	
	// If present receive a message
	if (msgQNumMsgs(msgQPointId)) {
		// Receive the messsage
		message message;
		msgQReceive(msgQPointId, (char *  ) &message, sizeof(message), WAIT_FOREVER);
		
		// Process the message
		process_message(message);
	}
	
	// In any case modify the position (if necessary)
	if (position != requestedPosition && position != POINTPOS_UNDEFINED ) {
		if (position > requestedPosition)
			position -= 1;
		else
			position += 1;

		// Log
		syslog(LOG_INFO, "Going to %s position (%i) current is %i", pointpos_str(requestedPosition), requestedPosition, position);			
	}
	
	// Self delete task
	taskDelete(taskIdSelf());
	
	return 0;
}

void dixlPoint() {
	
	// Start
	syslog(LOG_INFO, "Task started Id 0x%jx", taskPointId);	

	// Initialize step variables
	initialize();
	
	// Message queue initialization
	msgQPointId = msgQ_Initialize(msgQPointName, MSGQPOINTMESSAGESMAX, MSGQPOINTMESSAGESLENGTH, MSG_Q_FIFO);

	// Wait for messages of positioning
	FOREVER {
		
		// Wait an activation message ... FOREVER
		message inMessage;
		msgQReceive(msgQPointId, (char *  ) &inMessage, sizeof(inMessage), WAIT_FOREVER);

		// TODO Turn on LED
		
		// Process the message
		process_message(inMessage);
		
		// Need to move ?
		BOOL moved = FALSE;
		
		// Process spawning a worker while position!=requestedPosition AND requestedPosition!=UNDEFINED (Malfunction) OR new messages		
		while ((position != requestedPosition && position != POINTPOS_UNDEFINED) || msgQNumMsgs(msgQPointId)) {
			// Moved ate least one time
			moved = TRUE;
			
			// Spawn a periodic worker
			taskSpawn(TASKPOINTWKRNAME, TASKPOINTWKRPRIO, 0, TASKPOINTWKRSTACKSIZE, (FUNCPTR) worker, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

			// Sleep for a period (number of tick between motor step)
			taskDelay(stepTick);
		}
		
		// TODO Turn OFF Led
		
		// IF a Nonce request is acrive
		if (requestNonce.tv_sec && requestNonce.tv_nsec) {
			// Prepare the notification message
			message outMessage;
			outMessage.iHeader.type = IMSGTYPE_POINTNOTIFY;
			outMessage.pointINotify.currentPosition = position;
			outMessage.pointINotify.requestTimestamp = requestNonce;

			// Notify all requests carried out to task Ctrl
			msgQ_Send(msgQCtrlId, (char *) &outMessage, sizeof(msgIHeader) + sizeof(msgIPointNOTIFY));					
			// Log
			if (position == POINTPOS_UNDEFINED)
				syslog(LOG_ERR, "Point is in Malfunction state");	
			else 
				if (moved)
					syslog(LOG_INFO, "Position %s reached with nonce %i", pointpos_str(requestedPosition), requestNonce);	
				else
					syslog(LOG_INFO, "Already in %s position with nonce %i", pointpos_str(requestedPosition), requestNonce);
		}
	}		
}

