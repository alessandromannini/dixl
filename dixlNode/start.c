/*
 * globals.h
 * 
 * Globals variable and constants
 *
 *  Created on: Jan 10, 2023
 *  Author: Alessandro Mannini <alessandro.mannini@gmail.com>
 */

#include <stdio.h>

#include "vxWorks.h"
#include <logLib.h>
#include <taskLib.h>

#include "globals.h"
#include "config.h"
#include "utils.h"
#include "dixlInit.h"
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
	LOGMSG("Spawning Initialization task...\n");
	
	// Spawning
	taskInitId = taskSpawn(TASKINITNAME, TASKINITPRIO, 0, TASKINITSTACKSIZE, (FUNCPTR) dixlInit, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

	return 0;
}

/*
 *  Description: Shutdown
 */
uint_t dixlShutdown(void) {
	
	// Shutting down
	LOGMSG("Shutting down dixlNode...\n");

	// Deleting task in reverse order
	if (taskCommTxId) if (taskShutdown(taskCommTxId, "Deleting Communication Tx task (0x%jx) ...\n")) taskCommTxId=0;
	if (taskDiagId) if (taskShutdown(taskDiagId, "Deleting Diagnostic task (0x%jx) ...\n")) taskDiagId=0;
	if (taskCtrlId) if (taskShutdown(taskCtrlId, "Deleting Control Logic task (0x%jx) ...\n")) taskCtrlId=0;
	if (taskSwitchId) if (taskShutdown(taskSwitchId, "Deleting Switch Simulator task (0x%jx) ...\n")) taskSwitchId=0;
	if (taskLogId) if (taskShutdown(taskLogId, "Deleting Logger task (0x%jx) ...\n")) taskLogId=0;
	if (taskCommRxId) if (taskShutdown(taskCommRxId, "Deleting Communication Tx task (0x%jx) ...\n")) taskCommRxId=0;
	if (taskInitId) if (taskShutdown(taskInitId, "Deleting Initialization task (0x%jx) ...\n")) taskInitId=0;
	
	// Node halted
	LOGMSG("dixlNode halted\n\n");
	
	return 0;
}
