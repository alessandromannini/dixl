/**
 * dixlNode - Distributed Interlocking Node
 * 
 * Node implementation of the DIXL
 *
 * @author: Alessandro Mannini <alessandro.mannini@gmail.com>
 * @date: Jan 10, 2023
 */

#include <stdio.h>

#include "vxWorks.h"
#include <syslog.h>
#include <taskLib.h>

#include "globals.h"
#include "config.h"
#include "hw.h"
#include "utils.h"
#include "tasks/dixlInit.h"
#include "version.h"

/*
 * Task infos
 */
TASK_ID	taskStartId;
char	*taskStartName;

/*
 *  Description: Initialization task
 */
uint_t start(void) {
	
	// Get task infos
	taskStartId = taskIdSelf();
	taskStartName = taskName(0);
	
	// Check if already running
	if (taskInitId) {
		syslog(LOG_WARNING, "dixl Node already running ...");
		return 0;
	}
	
	// Spawn the Initialization task
	syslog(LOG_INFO, "Spawning Initialization task...");
	
	// Spawning
	taskInitId = taskSpawn(TASKINITNAME, TASKINITPRIO, 0, TASKINITSTACKSIZE, (FUNCPTR) dixlInit, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

	return 0;
}

/*
 *  Description: Shutdown
 */
uint_t stop(void) {
	
	// Shutting down
	syslog(LOG_INFO, "Shutting down dixlNode...");

	// Deleting tasks in reverse order
	task_shutdown(&taskDiagId, TASKDIAGDESC, NULL, NULL, &semDiag);
	task_shutdown(&taskCommTxId, TASKCOMMTXDESC, &msgQCommTxId, NULL, NULL);
	task_shutdown(&taskCtrlId, TASKCTRLDESC, &msgQCtrlId, NULL, NULL);
	task_shutdown(&taskPointId, TASKPOINTDESC, &msgQPointId, NULL, &semPosition);
	task_shutdown(&taskSensorId, TASKSENSORDESC, &msgQSensorId, NULL, &semSensor);
	task_shutdown(&taskLogId, TASKLOGDESC, &msgQLogId, NULL, NULL);
	task_shutdown(&taskCommRxId, TASKCOMMRXDESC, NULL, &dixlCommRxSocket, NULL);
	task_shutdown(&taskInitId, TASKINITDESC, &msgQInitId, NULL, NULL);
	
	// GPIO freeing
#if CPU !=_VX_SIMNT
	pinFree(GPIO_PIN_BUTTON);
	pinFree(GPIO_PIN_LED);
	syslog(LOG_INFO, "GPIO pin freed");
#endif
	// Node halted
	syslog(LOG_INFO, "dixlNode halted");
	
	return 0;
}
