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
#include "../hw.h"
#include "../utils.h"
#include "dixlPoint.h"
#include "dixlComm.h"

/* variables */
// Task
TASK_ID     taskPointId;

// Input message queue
MSG_Q_ID 	msgQPointId;

// Semaphores
SEM_ID semPosition;							// Semaphore to access position

// Point State
static ePointPosition position = POINTPOS_STRAIGHT;
static ePointPosition requestedPosition = POINTPOS_STRAIGHT;
static struct timespec requestNonce;				// Nonce (timestamp) of the request (if 0 notification isn't sent)
static _Vx_freq_t stepTick;

/* Implementation functions */
static void initialize() {
	_Vx_freq_t tickRate = sysClkRateGet();
	
	// Copute floor approxmation of tick per step
	stepTick = math_ceil((TASKPOINTTRANSTIME * tickRate) , 1000*POINTPOS_DIVERGING);
	
	// Estimate real duration (due to tick resolution
	double realTransitionTime = (1000 * stepTick * POINTPOS_DIVERGING) / tickRate;
	double increment = (realTransitionTime - TASKPOINTTRANSTIME ) * 100 / TASKPOINTTRANSTIME;
	
	syslog(LOG_INFO, "Straight <-> Diverging switch specs:");
	syslog(LOG_INFO, "> Initial position         : %s", pointPosStr(position));
	syslog(LOG_INFO, "> Number of steps          : %i", POINTPOS_DIVERGING);
	syslog(LOG_INFO, "> Requested switch time    : %ims", TASKPOINTTRANSTIME);
	syslog(LOG_INFO, "> Tick rate (clock)        : %iHz", tickRate);
	syslog(LOG_INFO, "> Tick per step            : %i", stepTick);
	syslog(LOG_INFO, "> Real excepted switch time: %ims (+%0.f%)", realTransitionTime, increment);	
}

// Process a single message received
static void process_message(message message) {
	
	// If malfunction state, ignore all messages (physical reset needed)
	if (position != POINTPOS_UNDEFINED || message.header.type == IMSGTYPE_POINTRESET) {
		switch (message.header.type) {
			// Point RESET
			case IMSGTYPE_POINTRESET:
				requestedPosition = message.pointIReset.requestedPosition;
				position=requestedPosition;
				
				// Disable notify
				requestNonce.tv_nsec = 0;
				requestNonce.tv_sec = 0;
				
				// Log
				syslog(LOG_INFO, "Reset received with %s initial position", pointPosStr(requestedPosition));	
				
				break;
			
			// Positioning request
			case IMSGTYPE_POINTPOS:
				requestedPosition =  message.pointIPosition.requestedPosition;
				requestNonce = message.pointIPosition.requestTimestamp;
				
				// Log
				syslog(LOG_INFO, "Request for %s positioning received with nonce %i", pointPosStr(requestedPosition), requestNonce);	
				
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
	// Take sem
	semTake(semPosition, WAIT_FOREVER);			
	
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
		syslog(LOG_INFO, "Going to %s position (%i) current is %i", pointPosStr(requestedPosition), requestedPosition, position);			
	}

	// Give sem to caller
	semGive(semPosition);

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
	msgQPointId = msgQ_Initialize(MSGQPOINTMESSAGESMAX, MSGQPOINTMESSAGESLENGTH, MSG_Q_FIFO);
	
	// Create semaphore
	semPosition = semMCreate(SEM_Q_FIFO);

	// Wait for messages of positioning
	FOREVER {
		
		// Wait an activation message ... FOREVER
		message inMessage;
		msgQReceive(msgQPointId, (char *  ) &inMessage, sizeof(inMessage), WAIT_FOREVER);

		// Turn on LED
		pinMode(GPIO_PIN_LED, OUT);
		pinSet(GPIO_PIN_LED, HIGH);
		
		// Take sem
		semTake(semPosition, WAIT_FOREVER);
		
		// Process the message
		process_message(inMessage);
		
		// Need to move ?
		BOOL moved = FALSE;
		
		// Process spawning a worker while position!=requestedPosition AND requestedPosition!=UNDEFINED (Malfunction) OR new messages		
		while ((position != requestedPosition && position != POINTPOS_UNDEFINED) || msgQNumMsgs(msgQPointId)) {
			// Moved ate least one time
			moved = TRUE;
			
			// Give sem to worker
			semGive(semPosition);
			
			// Spawn a periodic worker
			taskSpawn(TASKPOINTWKRNAME, TASKPOINTWKRPRIO, 0, TASKPOINTWKRSTACKSIZE, (FUNCPTR) worker, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

			// Sleep for a period (number of tick between motor step)
			taskDelay(stepTick);
			
			// Take sem
			semTake(semPosition, WAIT_FOREVER);			
		}

		// Final give
		semGive(semPosition);

		// Turn OFF Led
		pinSet(GPIO_PIN_LED, LOW);
		
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
					syslog(LOG_INFO, "Position %s reached with nonce %i", pointPosStr(requestedPosition), requestNonce);	
				else
					syslog(LOG_INFO, "Already in %s position with nonce %i", pointPosStr(requestedPosition), requestNonce);
		}
	}		
}

