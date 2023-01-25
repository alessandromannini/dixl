/*
 * dxilCtrl.c
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

#include "dixlCtrl.h"
#include "globals.h"
#include "config.h"
#include "utils.h"

/* variables */
// Task
TASK_ID     taskCtrlId;

// Input message queue
MSG_Q_ID 	msgQCtrlId;
VX_MSG_Q(msgQCtrlName, MSGQCTRLMESSAGESMAX, MSGQCTRLMESSAGESLENGTH);

bool initializeCtrlMsgQueue() {
	// Initialize Message Queue
	if ((msgQCtrlId = msgQInitialize (msgQCtrlName, MSGQCTRLMESSAGESMAX, MSGQCTRLMESSAGESLENGTH, MSG_Q_FIFO)) == NULL) {
		/* initialization failed */
		LOGMSG1("Message queue initialization error. {%d{", rcINQUEUE_INITERR);
		exit(rcINQUEUE_INITERR);      
	}
	
	LOGMSG("Message queue initialized\n");
	
	return TRUE;
}

void dixlCtrl() {
	
	// Start
	LOGMSG("Task started\n");	

	// Message queue initialization
	initializeCtrlMsgQueue();

	// TODO 
	FOREVER {
		taskSuspend(0);
		break;
	}
}

