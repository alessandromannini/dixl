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

#include <vxWorks.h>
#include <msgQLib.h>
#include <taskLib.h>
#include <syslog.h>

#include "../config.h"
#include "../datatypes/messages.h"
#include "../utils.h"
#include "dixlSwitch.h"
#include "dixlComm.h"

/* variables */
// Task
TASK_ID     taskSwitchId;

// Input message queue
MSG_Q_ID 	msgQSwitchId;
VX_MSG_Q(msgQSwitchName, MSGQSWITCHMESSAGESMAX, MSGQSWITCHMESSAGESLENGTH);

void dixlSwitch() {
	
	// Start
	syslog(LOG_INFO, "Task started Id 0x%jx", taskSwitchId);	

	// Message queue initialization
	msgQSwitchId = msgQ_Initialize(msgQSwitchName, MSGQSWITCHMESSAGESMAX, MSGQSWITCHMESSAGESLENGTH, MSG_Q_FIFO);

	// TODO 
	FOREVER {
		taskSuspend(0);
		break;
	}
}

