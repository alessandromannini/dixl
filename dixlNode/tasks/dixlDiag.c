/**
 * dxilDiag.c
 * 
 * Diagnostic task
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
#include "dixlDiag.h"
#include "dixlComm.h"

/* variables */
// Task
TASK_ID     taskDiagId;

// Input message queue
MSG_Q_ID 	msgQDiagId;
VX_MSG_Q(msgQDiagName, MSGQDIAGMESSAGESMAX, MSGQDIAGMESSAGESLENGTH);


void dixlDiag() {
	
	// Start
	syslog(LOG_INFO, "Task started Id %d", taskDiagId);	

	// Message queue initialization
	msgQDiagId = msgQ_Initialize(msgQDiagName, MSGQDIAGMESSAGESMAX, MSGQDIAGMESSAGESLENGTH, MSG_Q_FIFO);

	// TODO 
	FOREVER {
		taskSuspend(0);
		break;
	}
}

