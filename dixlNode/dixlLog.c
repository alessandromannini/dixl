/*
 * dxilLog.c
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

#include "dixlLog.h"
#include "globals.h"
#include "config.h"
#include "utils.h"

/* variables */
// Task
TASK_ID     taskLogId;

// Input message queue
MSG_Q_ID 	msgQLogId;
VX_MSG_Q(msgQLogName, MSGQLOGMESSAGESMAX, MSGQLOGMESSAGESLENGTH);

bool initializeLogMsgQueue() {
	// Initialize Message Queue
	if ((msgQLogId = msgQInitialize (msgQLogName, MSGQLOGMESSAGESMAX, MSGQLOGMESSAGESLENGTH, MSG_Q_FIFO)) == NULL) {
		/* initialization failed */
		LOGMSG1("Message queue initialization error. {%d{", rcINQUEUE_INITERR);
		exit(rcINQUEUE_INITERR);      
	}
	
	LOGMSG("Message queue initialized\n");
	
	return TRUE;
}

void dixlLog() {
	
	// Start
	LOGMSG("Task started\n");	

	// Message queue initialization
	initializeLogMsgQueue();

	// TODO 
	FOREVER {
		taskSuspend(0);
		break;
	}
}

