/**
 * dxilCommTx.c
 * 
 * TX Communication task
 *
 * @author: Alessandro Mannini <alessandro.mannini@gmail.com>
 * @date: Jan 10, 2023
 */

/* includes */
#include <stdlib.h>
#include <stdbool.h>

#include <vxWorks.h>
#include <msgQLib.h>
#include <taskLib.h>
#include <syslog.h>

#include "../config.h"
#include "../datatypes/messages.h"
#include "../utils.h"
#include "dixlComm.h"

/* variables */
// Task
TASK_ID     taskCommTxId;

// Input message queue
MSG_Q_ID 	msgQCommTxId;
VX_MSG_Q(msgQCommTxName, MSGQCOMMTXMESSAGESMAX, MSGQCOMMTXMESSAGESLENGTH);

void dixlCommTx() {
	
	// Start
	syslog(LOG_INFO,"Task started Id 0x%jx", taskCommTxId);	

	// Message queue initialization
	msgQCommTxId = msgQ_Initialize(msgQCommTxName, MSGQCOMMTXMESSAGESMAX, MSGQCOMMTXMESSAGESLENGTH, MSG_Q_FIFO);

	// TODO 
	FOREVER {
		taskSuspend(0);
		break;
	}
}

