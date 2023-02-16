/**
 * dxilSwitch.c
 * 
 * Switch simulator task
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
#include "dixlSwitch.h"
#include "dixlComm.h"

/* variables */
// Task
TASK_ID     taskSwitchId;

// Input message queue
MSG_Q_ID 	msgQSwitchId;
VX_MSG_Q(msgQSwitchName, MSGQSWITCHMESSAGESMAX, MSGQSWITCHMESSAGESLENGTH);


// Switch State
static int8_t position = SWITCHPOS_STRAIGHT;
static int8_t requestedPosition = SWITCHPOS_STRAIGHT;
static struct timespec requestTimestamp;
static _Vx_freq_t stepTick;

/* Implementation functions */
static void initialize() {
	_Vx_freq_t tickRate = sysClkRateGet();
	
	// Copute floor approxmation of tick per step
	stepTick = math_ceil((TASKSWITCHTRANSTIME * tickRate) , SWITCHPOS_DIVERGING);
	
	// Estimate real duration (due to tick resolution
	double realTransitionTime = (stepTick * SWITCHPOS_DIVERGING) / tickRate;
	double increment = (realTransitionTime - TASKSWITCHTRANSTIME ) *100 / TASKSWITCHTRANSTIME;
	
	syslog(LOG_INFO, "Straight <-> Diverging switch specs:");
	syslog(LOG_INFO, "> Number of steps          : %i", SWITCHPOS_DIVERGING);
	syslog(LOG_INFO, "> Requested switch time    : %is", TASKSWITCHTRANSTIME);
	syslog(LOG_INFO, "> Tick rate (clock)        : %iHz", tickRate);
	syslog(LOG_INFO, "> Tick per step            : %i", stepTick);
	syslog(LOG_INFO, "> Real excepted switch time: %.2fs (+%0.f%)", realTransitionTime, increment);	
}

// Process a single message received
static void process_message(message message) {
	
	// If malfunction state, ignore all messages (physical reset needed)
	if (position != SWITCHPOS_UNDEFINED) {
		switch (message.header.type) {
			// Positioning request
			case IMSGTYPE_SWITCHPOS:
				requestedPosition = (int8_t) message.switchIPosition.requestedPosition;
				requestTimestamp = message.switchIPosition.requestTimestamp;
				break;
				
			// Malfunction simulation requested by the host
			case MSGTYPE_SWITCHMALFUNC:
				position = SWITCHPOS_UNDEFINED;
				requestedPosition = SWITCHPOS_UNDEFINED;				
				break;
				
			// Other messages discarded
			default:
				syslog(LOG_ERR, "Unattended message type (%d). Should not be send to Switch task and will be ignored", message.iHeader.type);
				break;
		}
	}
}

static void worker() {
	
	// If present receive a message
	if (msgQNumMsgs(msgQSwitchId)) {
		// Receive the messsage
		message message;
		msgQReceive(msgQSwitchId, (char *  ) &message, sizeof(message), WAIT_FOREVER);
		
		// Process the message
		process_message(message);
	}
	
	// In any case modify the position (if necessary)
	if (position != requestedPosition && position != SWITCHPOS_UNDEFINED ) {
		if (position > requestedPosition)
			position -= 1;
		else
			position += 1;
	}
}

void dixlSwitch() {
	
	// Start
	syslog(LOG_INFO, "Task started Id 0x%jx", taskSwitchId);	

	// Initialize step variables
	initialize();
	
	// Message queue initialization
	msgQSwitchId = msgQ_Initialize(msgQSwitchName, MSGQSWITCHMESSAGESMAX, MSGQSWITCHMESSAGESLENGTH, MSG_Q_FIFO);

	// Wait for messages of positioning
	FOREVER {
		
		// Wait an activation message ... FOREVER
		message inMessage;
		msgQReceive(msgQSwitchId, (char *  ) &inMessage, sizeof(inMessage), WAIT_FOREVER);
		
		// Process the message
		process_message(inMessage);
		
		// Process spawning a worker while position!=requestedPosition AND requestedPosition!=UNDEFINED (Malfunction) OR new messages		
		while ((position != requestedPosition && position != SWITCHPOS_UNDEFINED) || msgQNumMsgs(msgQSwitchId)) {
			// Spawn a periodic worker
			taskSpawn(TASKSWITCHWKRNAME, TASKSWITCHWKRPRIO, 0, TASKSWITCHWKRSTACKSIZE, (FUNCPTR) worker, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

			// Sleep for a period (number of tick between motor step)
			taskDelay(stepTick);
		}
		
		// Prepare the notification message
		message outMessage;
		outMessage.iHeader.type = IMSGTYPE_SWITCHNOTIFY;
		outMessage.switchINotify.currentPosition = position;
		outMessage.switchINotify.requestTimestamp = requestTimestamp;

		// Notify all requests carried out to task Ctrl
		msgQ_Send(msgQCtrlId, (char *) &outMessage, sizeof(msgIHeader) + sizeof(msgISwitchNOTIFY));		
	}
}

