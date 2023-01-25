/*
 * dxilCommTx.c
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

#include "dixlComm.h"
#include "globals.h"
#include "config.h"
#include "utils.h"

/* variables */
// Task
TASK_ID     taskCommTxId;

// Input message queue
MSG_Q_ID 	msgQCommTxId;
VX_MSG_Q(msgQCommTxName, MSGQCOMMTXMESSAGESMAX, MSGQCOMMTXMESSAGESLENGTH);

bool initializeCommTxMsgQueue() {
	// Initialize Message Queue
	if ((msgQCommTxId = msgQInitialize (msgQCommTxName, MSGQCOMMTXMESSAGESMAX, MSGQCOMMTXMESSAGESLENGTH, MSG_Q_FIFO)) == NULL) {
		/* initialization failed */
		LOGMSG1("Message queue initialization error. {%d{", rcINQUEUE_INITERR);
		exit(rcINQUEUE_INITERR);      
	}
	
	LOGMSG("Message queue initialized\n");
	
	return TRUE;
}

void dixlCommTx() {
	
	// Start
	LOGMSG("Task started\n");	

	// Message queue initialization
	initializeCommTxMsgQueue();

	// TODO 
	FOREVER {
		taskSuspend(0);
		break;
	}
}

