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
#include "utils.h"
#include "tasks/dixlInit.h"
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
	if (taskCommTxId) 
		if (task_shutdown(taskCommTxId, TASKCOMMTXDESC)) taskCommTxId=0;
	if (taskDiagId) 
		if (task_shutdown(taskDiagId, TASKDIAGDESC)) taskDiagId=0;
	if (taskCtrlId) 
		if (task_shutdown(taskCtrlId, TASKCTRLDESC)) taskCtrlId=0;
	if (taskPointId) 
		if (task_shutdown(taskPointId, TASKPOINTDESC)) taskPointId=0;
	if (taskLogId) 
		if (task_shutdown(taskLogId, TASKLOGDESC)) taskLogId=0;
	if (taskCommRxId) 
		if (task_shutdown(taskCommRxId, TASKCOMMRXDESC)) taskCommRxId=0;
	if (taskInitId) 
		if (task_shutdown(taskInitId, TASKINITDESC)) taskInitId=0;
	
	// Node halted
	syslog(LOG_INFO, "dixlNode halted");
	
	return 0;
}
