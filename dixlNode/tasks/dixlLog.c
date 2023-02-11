/**
 * dxilLog.c
 * 
 * Logger task
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
#include "dixlLog.h"
#include "dixlComm.h"

/* variables */
// Task
TASK_ID     taskLogId;

// Input message queue
MSG_Q_ID 	msgQLogId;
VX_MSG_Q(msgQLogName, MSGQLOGMESSAGESMAX, MSGQLOGMESSAGESLENGTH);

void dixlLog() {
	
	// Start
	syslog(LOG_INFO, "Task started Id 0x%jx", taskLogId);	

	// Message queue initialization
	msgQLogId = msgQ_Initialize(msgQLogName, MSGQLOGMESSAGESMAX, MSGQLOGMESSAGESLENGTH, MSG_Q_FIFO);

	// TODO 
	FOREVER {
		taskSuspend(0);
		break;
	}
}

