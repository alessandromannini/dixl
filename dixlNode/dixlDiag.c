/*
 * dxilDiag.c
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

#include "dixlDiag.h"
#include "globals.h"
#include "config.h"
#include "utils.h"

/* variables */
// Task
TASK_ID     taskDiagId;

// Input message queue
MSG_Q_ID 	msgQDiagId;
VX_MSG_Q(msgQDiagName, MSGQDIAGMESSAGESMAX, MSGQDIAGMESSAGESLENGTH);

bool initializeDiagMsgQueue() {
	// Initialize Message Queue
	if ((msgQDiagId = msgQInitialize (msgQDiagName, MSGQDIAGMESSAGESMAX, MSGQDIAGMESSAGESLENGTH, MSG_Q_FIFO)) == NULL) {
		/* initialization failed */
		LOGMSG1("Message queue initialization error. {%d{", rcINQUEUE_INITERR);
		exit(rcINQUEUE_INITERR);      
	}
	
	LOGMSG("Message queue initialized\n");
	
	return TRUE;
}

void dixlDiag() {
	
	// Start
	LOGMSG("Task started\n");	

	// Message queue initialization
	initializeDiagMsgQueue();

	// TODO 
	FOREVER {
		taskSuspend(0);
		break;
	}
}

