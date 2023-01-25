/*
 * dxilCommRx.c
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
// Input message queue
TASK_ID     taskCommRxId;

void dixlCommRx() {
	
	// Start
	LOGMSG("Task started\n");

	// TODO 	
	FOREVER {
		taskSuspend(0);
		break;
	}
}

