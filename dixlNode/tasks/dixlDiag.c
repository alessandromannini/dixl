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
#include <string.h>

#include <vxWorks.h>
#include <msgQLib.h>
#include <pingLib.h>
#include <sysLib.h>
#include <syslog.h>
#include <taskLib.h>

#include "../config.h"
#include "../datatypes/messages.h"
#include "../globals.h"
#include "dixlDiag.h"

#include "../includes/network.h"
#include "../includes/utils.h"
#include "dixlComm.h"

/* variables */
// Client to check info
typedef struct client {
	nodeId id;						// Node id
	ulong_t numChecks;				// Num checks done from task start
	uint_t numFails;				// NUm failed checkes from last ok
	struct timespec lastCheck;			// timestamp of the last check
} client;
static client clients[CONFIGMAXROUTES];
static int currentChecked = -1;
static bool task_error = false;
static bool client_error = false;

// Task
TASK_ID     taskDiagId;

// Input message queue
MSG_Q_ID 	msgQDiagId;

// Semaphores
SEM_ID semDiag;							// Semaphore to access clients config

// Sensor State
static _Vx_freq_t periodTick = 0;

/* Implementation functions */
static void initialize() {
	
	// Clean configuration
	memset(clients, 0, sizeof(clients));
	
	// Diagnostic specs
	syslog(LOG_INFO, "Diagnostic check specs:");
	if (TASKDIAGCHECKPERIOD > 0) {

		_Vx_freq_t tickRate = sysClkRateGet();
	
		// Compute floor approxmation of tick per step
		periodTick = math_ceil((TASKDIAGCHECKPERIOD * tickRate) , 1000);
		
		// Estimate real duration (due to tick resolution)
		double realPeriodTime = 1000 * periodTick  / tickRate;
		double increment = (realPeriodTime - TASKDIAGCHECKPERIOD ) * 100 / TASKDIAGCHECKPERIOD;
	
		syslog(LOG_INFO, "> Requested check period   : %ims", TASKDIAGCHECKPERIOD);
		syslog(LOG_INFO, "> Tick rate (clock)        : %iHz", tickRate);
		syslog(LOG_INFO, "> Tick per period          : %i", periodTick);
		syslog(LOG_INFO, "> Real excepted period time: %.0fms (+%0.f%)", realPeriodTime, increment);
	} else
		syslog(LOG_INFO, "> Continuos running mode");
}

// Analyze all the routes and extract unique clients ids
static void config_pack(message message, client *clients) {
	// Clean configuration
	memset(clients, 0, sizeof(client) * CONFIGMAXROUTES);

	// Loop to extract all unique prev nodes
	uint32_t numRoutes = message.nodeIConfigSet.numRoutes;
	route *pRoute = message.nodeIConfigSet.pRoute;
	for(int idxRoute=0; idxRoute < numRoutes; idxRoute++) {
		
		// Search if current prev is already in
		int idxClient=0;
		while (!nodeIsNull(clients[idxClient].id)) {
			// Found?
			if (nodecmp(clients[idxClient].id , pRoute[idxRoute].prev) == 0)
				break;
			
			idxClient++;
		}
		
		// if zero => not found
		if (nodeIsNull(clients[idxClient].id)) {
			clients[idxClient].id = pRoute[idxRoute].prev;
		}		
	}
}

// Process a single message received
static void process_message(message message) {
	
	switch (message.header.type) {
		// Configuration RESET
		case IMSGTYPE_NODECONFIGRESET:
			// Clean configuration
			memset(clients, 0, sizeof(clients));
			currentChecked = -1;		
			task_error = false;
			client_error = false;

			// Log
			syslog(LOG_INFO,"Configuration RESET. Clients list cleaned");
			
			break;
			
		// Configuration SET
		case IMSGTYPE_NODECONFIGSET:
			// Pack routes extracting clients to monitor
			config_pack(message, clients);
			currentChecked = 0;

			// Log
			syslog(LOG_INFO,"Configuration SET. Clients list created.");
			
			break;
			
		// Other messages discarded
		default:
			syslog(LOG_ERR, "Unattended message type (%d). Should not be send to Sensor task and will be ignored", message.iHeader.type);
			break;
	}
}
// Send Diagnostic error notification to dixlCrl
static void sendToCtrl() {
	// Prepare  message to request position to Point task	
	message message;
	size_t size = sizeof(msgIHeader);
	if (task_error) {
		message.iHeader.type = IMSGTYPE_DIAGERRTASK;
		size += sizeof(msgIDiagErrTask);
	} else {
		message.iHeader.type = IMSGTYPE_DIAGERRCOMM;
		message.diagErrComm.node = clients[currentChecked].id;
		size += sizeof(msgIDiagErrComm);
	}
	
	// Log
	syslog(LOG_INFO, "Notifyning dixlCtrl task");				

	//Send to dixlPoint task queue
	msgQ_Send(msgQCtrlId, (char *) &message, size);	
}

// Send Diagnostic error notification to Host (throught dixlCommTx)
static void sendToHost() {
	// Prepare  message to request position to Point task	
	message message;
	size_t size = sizeof(msgIHeader);
	if (task_error) {
		message.iHeader.type = IMSGTYPE_DIAGERRTASK;
		size += sizeof(msgIDiagErrTask);
	} else {
		message.iHeader.type = IMSGTYPE_DIAGERRCOMM;
		message.diagErrComm.node = clients[currentChecked].id;
		size += sizeof(msgIDiagErrComm);
	}
	
	// Log
	syslog(LOG_INFO, "Notifyning to Host through dxilCommTx ");				

	//Send to dixlPoint task queue
	msgQ_Send(msgQCommTxId, (char *) &message, size);		
}

// Check if all task are alive and take different decision depending on missing task
static bool task_check() {
	TASK_DESC taskInfo;
	bool task_ctrl = true;
	bool task_commTx = true;
	
	// Check if dixlCtrl exists by id and name
	// If it doesn't exist (worst case) the node is automatically in fail-safe state, it can't reply to requests
	if (taskInfoGet(taskCtrlId, &taskInfo) == ERROR || strcmp(taskInfo.td_name, TASKCTRLNAME) != 0) {
		// Log
		syslog(LOG_ERR,"%s task is dead. Node is going into fail-safe mode", TASKCTRLNAME);
		
		// Turn on error flags
		task_error = true;		
		task_ctrl = false;
	}
	
	// Check if dixlCommTx exists by id and name
	// If it doesn't exist check if ctrl is available for notification
	if (taskInfoGet(taskCommTxId, &taskInfo) == ERROR || strcmp(taskInfo.td_name, TASKCOMMTXNAME) != 0) {
		// Log
		syslog(LOG_ERR,"%s task is dead. Node is going into fail-safe mode", TASKCOMMTXNAME);

		// Turn on error flags
		task_error = true;
		task_commTx = false;

		// If ctrl task is alive send ERROR message
		if (task_ctrl) sendToCtrl();		
	// If it exists check if ctrl was or need to be notified
	} else 
		if (!task_ctrl) sendToHost();
		
	// Check if dixlInit exists by id and name
	// If it doesn't exist notify ctrl and commTx if present
	if (taskInfoGet(taskInitId, &taskInfo) == ERROR || strcmp(taskInfo.td_name, TASKINITNAME) != 0) {
		// Log
		syslog(LOG_ERR,"%s task is dead. Node is going into fail-safe mode", TASKINITNAME);

		// Turn on error flag
		task_error = true;

		// If ctrl task is alive send ERROR message
		if (task_ctrl) sendToCtrl();		

		// If commTx task is alive send ERROR message to Host
		if (task_commTx) sendToHost();		
	}
	
	// Check if dixlCommRx exists by id and name
	// If it doesn't exist notify ctrl and commTx if present
	if (taskInfoGet(taskCommRxId, &taskInfo) == ERROR || strcmp(taskInfo.td_name, TASKCOMMRXNAME) != 0) {
		// Log
		syslog(LOG_ERR,"%s task is dead. Node is going into fail-safe mode", TASKCOMMRXNAME);

		// Turn on error flag
		task_error = true;

		// If ctrl task is alive send ERROR message
		if (task_ctrl) sendToCtrl();		

		// If commTx task is alive send ERROR message to Host
		if (task_commTx) sendToHost();		
	}

	// Check if dixlDiag exists by id and name
	// If it doesn't exist notify ctrl and commTx if present
	if (taskInfoGet(taskDiagId, &taskInfo) == ERROR || strcmp(taskInfo.td_name, TASKDIAGNAME) != 0) {
		// Log
		syslog(LOG_ERR,"%s task is dead. Node is going into fail-safe mode", TASKDIAGNAME);

		// Turn on error flag
		task_error = true;

		// If ctrl task is alive send ERROR message
		if (task_ctrl) sendToCtrl();		

		// If commTx task is alive send ERROR message to Host
		if (task_commTx) sendToHost();		
	}
	
	// Check if dixlLog exists by id and name
	// If it doesn't exist notify ctrl and commTx if present
	if (taskInfoGet(taskLogId, &taskInfo) == ERROR || strcmp(taskInfo.td_name, TASKLOGNAME) != 0) {
		// Log
		syslog(LOG_ERR,"%s task is dead. Node is going into fail-safe mode", TASKLOGNAME);

		// Turn on error flag
		task_error = true;

		// If ctrl task is alive send ERROR message
		if (task_ctrl) sendToCtrl();		

		// If commTx task is alive send ERROR message to Host
		if (task_commTx) sendToHost();		
	}
	
	// Check if dixlPoint exists by id and name
	// If it doesn't exist notify ctrl and commTx if present
	if (taskInfoGet(taskPointId, &taskInfo) == ERROR || strcmp(taskInfo.td_name, TASKPOINTNAME)) {
		// Log
		syslog(LOG_ERR,"%s task is dead. Node is going into fail-safe mode", TASKPOINTNAME);

		// Turn on error flag
		task_error = true;

		// If ctrl task is alive send ERROR message
		if (task_ctrl) sendToCtrl();		

		// If commTx task is alive send ERROR message to Host
		if (task_commTx) sendToHost();		
	}
	
	// Check if dixlSensor exists by id and name
	// If it doesn't exist notify ctrl and commTx if present
	if (taskInfoGet(taskSensorId, &taskInfo) == ERROR || strcmp(taskInfo.td_name, TASKSENSORNAME)) {
		// Log
		syslog(LOG_ERR,"%s task is dead. Node is going into fail-safe mode", TASKSENSORNAME);

		// Turn on error flag
		task_error = true;

		// If ctrl task is alive send ERROR message
		if (task_ctrl) sendToCtrl();		

		// If commTx task is alive send ERROR message to Host
		if (task_commTx) sendToHost();		
	}
	
	return !task_error;
}

// ping a client
static bool client_check(client *client) {
	// Get the IP
	IPv4String clientAddress = "\0";
	network_IPv4_to_str(&client->id, clientAddress);
	
	// Update timestamp
	clock_gettime(CLOCK_REALTIME, &client->lastCheck);
	
	// Try to ping
	STATUS ret = ping(clientAddress, TASKDIAGPINGPKTS, PING_OPT_SILENT | PING_OPT_NOHOST);
	
	// Update statistics and attributes
	if (ret == OK) {
		client->numChecks++;
		client->numFails = 0;
	} else {
		// Log
		syslog(LOG_ERR,"Unable to communicate with prev node %s. Node is going into fail-safe mode", clientAddress);

		// Update variables
		client->numChecks++;
		client->numFails++;
		client_error = true;
		sendToHost();
	}
	
	return (ret == OK);
}

static int worker() {
	// If present receive a message
	if (msgQNumMsgs(msgQDiagId)) {
		// Receive the messsage
		message message;
		msgQReceive(msgQDiagId, (char *  ) &message, sizeof(message), WAIT_FOREVER);
		
		// Process the message
		process_message(message);
	}

	// Checking tasks presence
	if (task_check())
		if (currentChecked >=0)
		client_check(&clients[currentChecked]);

	// Give sem to caller
	semGive(semDiag);
	
	// Self delete task
	taskDelete(taskIdSelf());
	
	return 0;
}

void dixlDiag() {
	// Initialize step variables
	initialize();
	
	// Create semaphore
	semDiag = semBCreate(SEM_Q_FIFO, SEM_FULL);	
		
	// Start
	syslog(LOG_INFO, "Task started Id 0x%jx", taskDiagId);	

	// Message queue initialization
	msgQDiagId = msgQ_Initialize(MSGQDIAGMESSAGESMAX, MSGQDIAGMESSAGESLENGTH, MSG_Q_FIFO);

	FOREVER {
			
		// Wait a configuration message ... FOREVER
		message inMessage;
		msgQReceive(msgQDiagId, (char *  ) &inMessage, sizeof(inMessage), WAIT_FOREVER);

		// Take sem
		semTake(semDiag, WAIT_FOREVER);		
		
		// Process the message
		process_message(inMessage);

		// Log
		syslog(LOG_INFO,"Starting to monitor");
		
		// Start diagnostic worker if:
		// - no task errors
		// - no client errors
		while (!task_error && !client_error) {		

			// Spawn a periodic worker
			taskSpawn(TASKDIAGWKRNAME, TASKDIAGWKRPRIO, 0, TASKDIAGWKRSTACKSIZE, (FUNCPTR) worker, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			
			// Sleep for a period if configured
			if (periodTick) taskDelay(periodTick);
			
			// Go to next client
			if (currentChecked >= 0)
				if (nodeIsNull(clients[++currentChecked].id)) currentChecked = 0;

			// Take sem (ensure previous Worker is finished)
			// Worker will Give
			semTake(semDiag, WAIT_FOREVER);					
		}

		// Final give
		semGive(semDiag);

		// Log
		syslog(LOG_INFO,"Monitoring stopped");
		
	}
}
