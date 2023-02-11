/**
 * FSMInit.h
 * 
 * Finite State Machine controlling Initilization task
 *
 * @author: Alessandro Mannini <alessandro.mannini@gmail.com>
 * @date: Jan 10, 2023
 */

/* includes */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <vxWorks.h>
#include <syslog.h>


#include "FSMInit.h"
#include "../config.h"
#include "../datatypes/messages.h"
#include "../globals.h"
#include "../utils.h"
#include "../tasks/dixlComm.h"
#include "../tasks/dixlDiag.h"
#include "../tasks/dixlLog.h"
#include "../tasks/dixlCtrl.h"
#include "../tasks/dixlSwitch.h"

/* Private definitions */
/* Enums and types */
typedef enum eStates {
	StateDummy,
	StateInit,
	StateIdle,
	StateConfiguring,
	StateConfigured,
} eStates;

typedef struct eventData {
	struct message *pMessage;
} eventData;


typedef void (*StateFunc)(eventData *pEventData);
typedef void (*EntryFunc)(eventData *pEventData);
typedef void (*ExitFunc)(eventData *pEventData);

typedef struct StateMapItem {
	StateFunc pStateFunc;
	EntryFunc pEntryFunc;
	ExitFunc pExitFunc;
} StateMapItem;

/* Machine Instance */
typedef struct {
	// TODO timer	
	eStates newState;				// New state to pass to
	eStates currentState;			// Current state
    StateMapItem *stateMap;			// Map to function for each state
	BOOL eventGenerated;			// An event occured and hasn't been served
	eventData *pEventData;			// Point to current event Data
} FiniteStateMachine;



/**
 *  variables 
 */
// Configurations receive area
static nodeId source = {0, 0, 0, 0};
static nodeId destination = {0, 0, 0, 0};
static int configPreviousSegment = -1;
static int configTotalSegments = -1;
static eNodeType configNodeType;
static route *configuration[CONFIGMAXROUTES];

/**
 *  Functions implementation 
 */
static void spawnCoreTasks() {
	/**************************************************************
	 * WARNING DO NOT CHANGE SPAWNING ORDER, RESPECT DEPENDENCIES *
	 **************************************************************/	
	// Spawning dixlCommRx
	syslog(LOG_INFO, "Spawning Communcation Rx task...");
	taskCommRxId = taskSpawn(TASKCOMMRXNAME, TASKCOMMRXPRIO, 0, TASKCOMMRXSTACKSIZE, (FUNCPTR) dixlCommRx, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	
	// Spawning dixlLog
	syslog(LOG_INFO, "Spawning Logger task...");
	taskLogId = taskSpawn(TASKLOGNAME, TASKLOGPRIO, 0, TASKLOGSTACKSIZE, (FUNCPTR) dixlLog, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

	// Spawning dixlSwitch
	syslog(LOG_INFO, "Spawning Switch Simulator task...");
	taskSwitchId = taskSpawn(TASKSWITCHNAME, TASKSWITCHPRIO, 0, TASKSWITCHSTACKSIZE, (FUNCPTR) dixlSwitch, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

	// Spawning dixlCtrl
	syslog(LOG_INFO, "Spawning Control Logic task...");
	taskCtrlId = taskSpawn(TASKCTRLNAME, TASKCTRLPRIO, 0, TASKCTRLSTACKSIZE, (FUNCPTR) dixlCtrl, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

	// Spawning dixlDiag
	syslog(LOG_INFO, "Spawning Diagnostic task...");
	taskDiagId = taskSpawn(TASKDIAGNAME, TASKDIAGPRIO, 0, TASKDIAGSTACKSIZE, (FUNCPTR) dixlDiag, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

	// Spawning dixlCommTx
	syslog(LOG_INFO, "Spawning Communcation Tx task...");
	taskCommTxId = taskSpawn(TASKCOMMTXNAME, TASKCOMMTXPRIO, 0, TASKCOMMTXSTACKSIZE, (FUNCPTR) dixlCommTx, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

/**
 *  Forward functions definitions 
 */
static void FSMEvent_Internal(eStates newState, eventData *pEventData);
static void StateEngine();

/**
 * STATEDUMMY
 */
void FSMInit() {
	// Force first (Init) State
	FSMEvent_Internal(StateInit, NULL);
	
	// and process it
	StateEngine();
	
	syslog(LOG_INFO, "FSM initialized");
}

/**
 * STATEINIT
 */
static void InitState(eventData *pEventData) {
	// Spawn core tasks
	spawnCoreTasks();
	
	// Pass to next (Idle) state
	FSMEvent_Internal(StateIdle, pEventData);
};

/**
 * STATEIDLE
 */
static void IdleState(eventData *pEventData) {
	// Clean CONFIG
	configPreviousSegment = -1;
	configTotalSegments = -1;

	// Log cleaned CONFIG
	syslog(LOG_INFO, "CONFIG cleaned");
	
	// TODO Spawn SSDP client task
};

static void IdleExit(eventData *pEventData) {
	// TODO Delete SSDP client task
};


/**
 * STATECONFIGURING
 */
static void ConfiguringEntry(eventData *pEventData) {
	// Get total number of segments in the CONFIG
	message *pMessage = pEventData->pMessage;
	source = pMessage->header.source;
	destination = pMessage->header.destination;
	configTotalSegments = pMessage->initConfigType.totalSegments;
	configNodeType = pMessage->initConfigType.nodeType;
}
static void ConfiguringState(eventData *pEventData) {
	// Get message pointer
	message *pMessage = pEventData->pMessage;
	
	// Get current sequence	
	uint32_t configCurrentSequence = pMessage->initConfig.sequence;
	
	// Check if CONFIG is correct
	// If sequence or total segment error, discard the message and the sequence and go back to StateIdle
	if ( (configCurrentSequence - configPreviousSegment != 1 || pMessage->initConfig.totalSegments != configTotalSegments )) { 
		syslog(LOG_INFO, "Wrong CONFIG sequence going back to idle state");
		FSMEvent_Internal(StateIdle, pEventData);
	} else {
		// Sequence OK
		// Store current sequence 
		configuration[configCurrentSequence] = &(pMessage->initConfig.route);
		configPreviousSegment = configCurrentSequence;
	
		// Log received CONFIG
		if (configCurrentSequence == 0)	
			syslog(LOG_INFO, "Received CONFIG NodeType %s, Total routes %i", ( configNodeType == NODETYPE_TC ) ? "TrackCircuit" : "Switch", configTotalSegments);
		else
			syslog(LOG_INFO, "Received CONFIG route %i of %i", configCurrentSequence, configTotalSegments);
		
		//If last go to next State (Configured) without need for events
		if (configCurrentSequence == configTotalSegments)
			FSMEvent_Internal(StateConfigured, pEventData);	
	}

};

/**
 * STATECONFIGURED
 */
static void ConfiguredState(eventData *pEventData) {
	// Prepare CONFIG SET message for dixlCtrl
	message message;
	message.iHeader.type = IMSGTYPE_NODECONFIGSET;
	message.ctrlIConfigSet.pRoute = configuration[0];
	message.ctrlIConfigSet.numRoutes = configTotalSegments;
	message.ctrlIConfigSet.nodeType = (uint8_t) configNodeType;
	// Check CONFIG
	if (configTotalSegments <= 0) {
		syslog(LOG_ERR, "Wrong CONFIG number of segments (%i): going back to Idle state", configTotalSegments);
		FSMEvent_Internal(StateIdle, pEventData);
	} else if ( configNodeType != NODETYPE_TC && configNodeType != NODETYPE_SW) {
		syslog(LOG_ERR, "Wrong CONFIG node type: going back to Idle state");
		FSMEvent_Internal(StateIdle, pEventData);
	} else {		
		// CONFIG ok, Send to dixlCtrl task queue
		msgQ_Send(msgQCtrlId, (char *) &message, sizeof(msgIHeader) + sizeof(msgICtrlCONFIGSET));
	}
};
static void ConfiguredExit(eventData *pEventData) {
	// Prepare CONFIG RESET message for dixlCtrl
	message message;
	message.iHeader.type = IMSGTYPE_NODECONFIGRESET;
	
	// Send to dixlCtrl task queue
	msgQ_Send(msgQCtrlId, (char *) &message, sizeof(msgIHeader) + sizeof(msgICtrlCONFIGSET));		
};



static StateMapItem StateMap[] = {
		// StateDummy
		{ NULL,				NULL, 				NULL},
		// StateInit
		{ InitState, 		NULL, 				NULL},
		// StateIdle
		{ IdleState,		NULL,				IdleExit},
		// StateConfiguring
		{ ConfiguringState, ConfiguringEntry, 	NULL},
		// StateConfigured
		{ ConfiguredState,	NULL,				ConfiguredExit},
};

/* FiniteStateMachine istance object */
static FiniteStateMachine FSM =  {
	StateDummy,
	StateDummy,
	StateMap,
	FALSE,
	NULL
};

/**
 * The state engine execute until events are generated
 */
static void StateEngine() {
    // Temporary pointer to current event data
	eventData *pDataTemp = NULL;

    // While events are being generated keep executing states
    while (FSM.eventGenerated) {

        // Get the pointers to function
        StateFunc state = FSM.stateMap[FSM.newState].pStateFunc;
        EntryFunc entry = FSM.stateMap[FSM.newState].pEntryFunc;
        ExitFunc exit = FSM.stateMap[FSM.currentState].pExitFunc;

        // Copy of event data pointer
        pDataTemp = FSM.pEventData;

        // Event data used and resetted
        FSM.pEventData = NULL;

        // Event served and resetted
        FSM.eventGenerated = FALSE;

		// Transitioning to a new state?
		if (FSM.newState != FSM.currentState)
		{
			// Execute the state exit action on current state before switching to new state
			if (exit != NULL)
				exit(pDataTemp);

			// Execute the state entry action on the new state
			if (entry != NULL)
				entry(pDataTemp);

			// Ensure exit/entry actions didn't call FSMEvent_Internal by accident 
			assert(FSM.eventGenerated == FALSE);
		}

		// Switch to the new current state
		FSM.currentState = FSM.newState;

		// Execute the state action passing in event data
		assert(state != NULL);
		state(pDataTemp);
    }
}


/**
 * Generates an internal event
 * @param newState: the new state to pass to
 * @param pEventData: pointer to the event data
 */ 
void FSMEvent_Internal(eStates newState, eventData *pEventData) {
    FSM.pEventData = pEventData;
    FSM.eventGenerated = TRUE;
    FSM.newState = newState;
}

/**
 * Event Functions
 * Only conditions to go to next state are tested and newstate decided then the StateEngine is executed, that is:
 * - currentStateExit
 * - newStateEntry
 * - newState
 * @param message: message received
 */
void FSMInitEvent_NewMessage(message *pMessage) {	
	
	// Result of the precondition evaluation to pass to new state
	BOOL condition = TRUE;
	// Event data
	eventData eventData;
	eventData.pMessage = pMessage;
	// New state
	eStates newState=NULL;
	
	switch (FSM.currentState) {
		case StateDummy:
			// Should not happen
			syslog(LOG_ERR, "Wrong state Dummy: message received");
			exit(rcFSM_WRONGSTATE);
			break;
			
		case StateInit:
			// Should not happen
			syslog(LOG_ERR, "Wrong state Init: message received");
			exit(rcFSM_WRONGSTATE);
			break;

		case StateIdle:
			// Accept only CFG messages discard others			
			if (pMessage->header.type == MSGTYPE_NODECONFIG) {
				// Get current sequence
				int configCurrentSequence = pMessage->initConfig.sequence;
				
				// First in sequence? OK to next state (Configuring)			
				if (configCurrentSequence == 0) {
					newState = StateConfiguring;
					condition = TRUE;
				} else
					condition = FALSE;			
			} else {
				// Discard other messages
				condition = FALSE;
			}
			break;
			
		case StateConfiguring:		
			// Accept CONFIG message					
			if (pMessage->header.type == MSGTYPE_NODERESET) {
				// continue checking (if ok and last) and configuring 
				newState = StateConfiguring;
				condition = TRUE;
			} else {				
				// Discard other message type
				condition = FALSE;
			};
			break;
			
		case StateConfigured:
			// Accept only RESET message
			if (pMessage->header.type == MSGTYPE_NODERESET) {
				// Reset config going back to Idle
				newState = StateIdle;
				condition = TRUE;
			}
			break;
	};
	
	// If the condition is passed, the new state is served
	if (condition) {
		// Generate the internal event
		// New state, event data and event flag set only if used
		FSMEvent_Internal(newState, &eventData);
		
		// Process the state change
		StateEngine();
	}
}
