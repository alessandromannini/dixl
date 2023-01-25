/*
 * dxilSwitch.c
 *
 *  Created on: Jan 10, 2023
 *  Author: Alessandro Mannini <alessandro.mannini@gmail.com>
 */

/* includes */
#include <stdlib.h>
#include <stdbool.h>

#include <vxWorks.h>
#include <msgQLib.h>
#include <taskLib.h>

#include "dixlSwitch.h"
#include "globals.h"
#include "config.h"
#include "utils.h"

/* variables */
// Task
TASK_ID     taskSwitchId;

// Input message queue
MSG_Q_ID 	msgQSwitchId;
VX_MSG_Q(msgQSwitchName, MSGQSWITCHMESSAGESMAX, MSGQSWITCHMESSAGESLENGTH);

bool initializeSwitchMsgQueue() {
	// Initialize Message Queue
	if ((msgQSwitchId = msgQInitialize (msgQSwitchName, MSGQSWITCHMESSAGESMAX, MSGQSWITCHMESSAGESLENGTH, MSG_Q_FIFO)) == NULL) {
		/* initialization failed */
		LOGMSG1("Message queue initialization error. {%d{", rcINQUEUE_INITERR);
		exit(rcINQUEUE_INITERR);      
	}
	
	LOGMSG("Message queue initialized\n");
	
	return TRUE;
}

void dixlSwitch() {
	
	// Start
	LOGMSG("Task started\n");	

	// Message queue initialization
	initializeSwitchMsgQueue();

	// TODO 
	FOREVER {
		taskSuspend(0);
		break;
	}
}

