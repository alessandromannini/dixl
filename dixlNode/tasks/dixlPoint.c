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
static struct timespec requestTimestamp;
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
	syslog(LOG_INFO, "> Number of steps          : %i", POINTPOS_DIVERGING);
	syslog(LOG_INFO, "> Requested switch time    : %is", TASKPOINTTRANSTIME);
	syslog(LOG_INFO, "> Tick rate (clock)        : %iHz", tickRate);
	syslog(LOG_INFO, "> Tick per step            : %i", stepTick);
	syslog(LOG_INFO, "> Real excepted switch time: %.2fs (+%0.f%)", realTransitionTime, increment);	
}

// Process a single message received
static void process_message(message message) {
	
	// If malfunction state, ignore all messages (physical reset needed)
	if (position != POINTPOS_UNDEFINED) {
		switch (message.header.type) {
			// Positioning request
			case IMSGTYPE_POINTPOS:
				requestedPosition = (int8_t) message.pointIPosition.requestedPosition;
				requestTimestamp = message.pointIPosition.requestTimestamp;
				break;
				
			// Malfunction simulation requested by the host
			case MSGTYPE_POINTMALFUNC:
				position = POINTPOS_UNDEFINED;
				requestedPosition = POINTPOS_UNDEFINED;				
				break;
				
			// Other messages discarded
			default:
				syslog(LOG_ERR, "Unattended message type (%d). Should not be send to Point task and will be ignored", message.iHeader.type);
				break;
		}
	}
}

static void worker() {
	
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
	}
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
		
		// Process the message
		process_message(inMessage);
		
		// Process spawning a worker while position!=requestedPosition AND requestedPosition!=UNDEFINED (Malfunction) OR new messages		
		while ((position != requestedPosition && position != POINTPOS_UNDEFINED) || msgQNumMsgs(msgQPointId)) {
			// Spawn a periodic worker
			taskSpawn(TASKPOINTWKRNAME, TASKPOINTWKRPRIO, 0, TASKPOINTWKRSTACKSIZE, (FUNCPTR) worker, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

			// Sleep for a period (number of tick between motor step)
			taskDelay(stepTick);
		}
		
		// Prepare the notification message
		message outMessage;
		outMessage.iHeader.type = IMSGTYPE_POINTNOTIFY;
		outMessage.pointINotify.currentPosition = position;
		outMessage.pointINotify.requestTimestamp = requestTimestamp;

		// Notify all requests carried out to task Ctrl
		msgQ_Send(msgQCtrlId, (char *) &outMessage, sizeof(msgIHeader) + sizeof(msgIPointNOTIFY));		
	}
}

