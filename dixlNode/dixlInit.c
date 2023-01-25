/*
 * dxilInit.h
 *
 *  Created on: Jan 10, 2023
 *  Author: Alessandro Mannini <alessandro.mannini@gmail.com>
 *  Description: Initialization task
 */

/* includes */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <vxWorks.h>
#include <taskLib.h>
#include <logLib.h>
#include <msgQLib.h>
#include <sysLib.h>

#include "globals.h"
#include "config.h"
#include "utils.h"
#include "dixlComm.h"
#include "dixlDiag.h"
#include "dixlLog.h"
#include "dixlCtrl.h"
#include "dixlSwitch.h"

/* variables */
// Task
TASK_ID     taskInitId;

// Input message queue
MSG_Q_ID 	msgQInitId;
VX_MSG_Q(msgQInitName, MSGQINITMESSAGESMAX, MSGQINITMESSAGESLENGTH);

/* 
 * Print welcome message and Node ID to the logger
 */
void welcome_banner() {
	LOGMSG("\n");
	LOGMSG("******************************************************\n");
	LOGMSG("*     dixlNode - Distributed InterLocking system     *\n");
	LOGMSG("*     ------------------------------------------     *\n");
	LOGMSG("* author       :  Alessandro Mannini                 *\n");
	LOGMSG("* organization :  Universita' degli Studi di Firenze *\n");
	LOGMSG("* date         :  Jan 10, 2023                       *\n");
	LOGMSG("* contact      :  alessandro.mannini@gmail.com       *\n");
	LOGMSG("******************************************************\n");

	// Get Node informations
	// TODO char * IF = "eth0????";
	uint8_t  MAC[6] = { 00, 11, 22, 33, 44, 55};
	uint8_t IPv4[4] = { 255, 255, 255, 255 };
	// TODO uint8_t IPv6[6] = { 255, 255, 255, 255, 255, 255 };
	
	LOGMSG(" Node informations\n");
	LOGMSG(" -----------------\n");
	LOGMSG6(" ID            : %02d:%02d:%02d:%02d:%02d:%02d\n", MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5]);
	LOGMSG4(" IP address    : %03d.%03d.%03d.%03d\n\n", IPv4[0], IPv4[1], IPv4[2], IPv4[3]);
}

bool initializeInitMsgQueue() {
	// Initialize Message Queue
	if ((msgQInitId = msgQInitialize (msgQInitName, MSGQINITMESSAGESMAX, MSGQINITMESSAGESLENGTH, MSG_Q_FIFO)) == NULL) {
		/* initialization failed */
		LOGMSG1("Message queue initialization error. {%d{", rcINQUEUE_INITERR);
		exit(rcINQUEUE_INITERR);      
	}
	
	LOGMSG("Message queue initialized\n");
	
	return TRUE;
}

/*
 *  Initialization task
 */
void dixlInit() {
	// Delay until start task closes
	_Vx_freq_t tickRate = sysClkRateGet();
	while (taskIsReady(taskStartId)) {
		taskDelay(tickRate);
	};
	taskDelay(tickRate);
	
	// Print welcome banner
	welcome_banner();
		
	// Start
	taskInitId = taskIdSelf();
	LOGMSG("Task started\n");

	// Message queue initialization
	initializeInitMsgQueue();
	
	/**************************************************************
	 * WARNING DO NOT CHANGE SPAWNING ORDER, RESPECT DEPENDENCIES *
	 **************************************************************/	
	// Spawning dixlCommRx
	LOGMSG("Spawning Communcation Rx task...\n");
	taskCommRxId = taskSpawn(TASKCOMMRXNAME, TASKCOMMRXPRIO, 0, TASKCOMMRXSTACKSIZE, (FUNCPTR) dixlCommRx, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	
	// Spawning dixlLog
	LOGMSG("Spawning Logger task...\n");
	taskLogId = taskSpawn(TASKLOGNAME, TASKLOGPRIO, 0, TASKLOGSTACKSIZE, (FUNCPTR) dixlLog, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

	// Spawning dixlSwitch
	LOGMSG("Spawning Switch Simulator task...\n");
	taskSwitchId = taskSpawn(TASKSWITCHNAME, TASKSWITCHPRIO, 0, TASKSWITCHSTACKSIZE, (FUNCPTR) dixlSwitch, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

	// Spawning dixlCtrl
	LOGMSG("Spawning Control Logic task...\n");
	taskCtrlId = taskSpawn(TASKCTRLNAME, TASKCTRLPRIO, 0, TASKCTRLSTACKSIZE, (FUNCPTR) dixlCtrl, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

	// Spawning dixlDiag
	LOGMSG("Spawning Diagnostic task...\n");
	taskDiagId = taskSpawn(TASKDIAGNAME, TASKDIAGPRIO, 0, TASKDIAGSTACKSIZE, (FUNCPTR) dixlDiag, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

	// Spawning dixlCommTx
	LOGMSG("Spawning Communcation Tx task...\n");
	taskCommTxId = taskSpawn(TASKCOMMTXNAME, TASKCOMMTXPRIO, 0, TASKCOMMTXSTACKSIZE, (FUNCPTR) dixlCommTx, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	
	// TODO socket ssdp e reset	
	FOREVER {
		taskSuspend(0);
		break;
	}
}
