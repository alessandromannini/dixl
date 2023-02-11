/**
 * FSMCtrlSW.c
 * 
 * Finite State Machine controlling Ctrl task (Switch node type)
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


#include "FSMCtrlSW.h"
#include "../config.h"
#include "../datatypes/messages.h"
#include "../utils.h"


/* Private definitions */
/* Enums and types */
typedef enum eStates {
	StateDummy,
	StateNotReserved,
	StateWaitAck,
	StateWaitCommit,
	StateWaitAgree,
	StatePositioning,
	StateMalfunction,
	StateReserved,
	StateTrainInTransition,
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
// Node State

static NodeState *pCurrentNodeState = NULL;

/**
 *  Functions implementation 
 */
static BOOL setRoute(routeId requestedRouteId) {
	// Search for the requested route
	pCurrentNodeState->pCurrentRoute = NULL;
	
	for (uint32_t i=0; i < pCurrentNodeState->numRoutes ; i++ ) {
		 // If found set it and return TRUE
		if (pCurrentNodeState->pRouteList[i].id == requestedRouteId) {
			pCurrentNodeState->pCurrentRoute = &(pCurrentNodeState->pRouteList[i]);
			return TRUE;
		}
	}
		
	// Not found, return FALSE
	syslog(LOG_ERR, "Requested route id (%i) not found", requestedRouteId);
	return FALSE;
}

/**
 *  Forward functions definitions 
 */
static void FSMEvent_Internal(eStates newState, eventData *pEventData);
static void StateEngine();

/**
 * STATEDUMMY
 */
void FSMCtrlSW(NodeState *pState) {
	// Get pointer to NodeState
	pCurrentNodeState = pState;
	
	// Force first (Init) State
	FSMEvent_Internal(StateNotReserved, NULL);
	
	// and process it
	StateEngine();

	// Log
	syslog(LOG_INFO, "FSM initialized");
}

/**
 * STATENOTRESERVED
 */
static void NotReservedState(eventData *pEventData) {
	// Clean current request
	pCurrentNodeState->pCurrentRoute = NULL;
	
	// Log
	syslog(LOG_INFO, "Request cleaned");	
	
};

/**
 * STATEWAITACK
 */
static void WaitAckState(eventData *pEventData) {
};

/**
 * STATEWAISWOMMIT
 */
static void WaitCommitState(eventData *pEventData) {
};

/**
 * STATEWAITAGREE
 */
static void WaitAgreeState(eventData *pEventData) {
};

/**
 * STATEPOSITIONING
 */
static void PositioningState(eventData *pEventData) {
};

/**
 * STATEMALFUNCTION
 */
static void MalfunctionState(eventData *pEventData) {
};

/**
 * STATERESERVED
 */
static void ReservedState(eventData *pEventData) {
};

/**
 * STATEINTRANSITION
 */
static void TrainInTransitionState(eventData *pEventData) {
};

static StateMapItem StateMap[] = {
		// StateDummy
		{ NULL,						NULL, 				NULL},
		// StateNotReserved
		{ NotReservedState, 		NULL, 				NULL},
		// StateWaitAck
		{ WaitAckState,				NULL,				NULL},
		// StateWaitCommit
		{ WaitCommitState, 			NULL, 				NULL},
		// StateWaitAgree
		{ WaitAgreeState,			NULL,				NULL},
		// StatePositioning
		{ PositioningState,			NULL,				NULL},
		// StateMalfunction
		{ MalfunctionState,			NULL,				NULL},
		// StateReserved
		{ ReservedState,			NULL,				NULL},
		// StateTrainInTransition
		{ TrainInTransitionState,	NULL,				NULL},
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
static void FSMEvent_Internal(eStates newState, eventData *pEventData) {
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
void FSMCtrlTCEvent_NewMessage(message *pMessage) {	
	
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
			
		case StateNotReserved:
			break;
			
		case StateWaitAck:
			break;
			
		case StateWaitCommit:
			break;
			
		case StateWaitAgree:
			break;
			
		case StatePositioning:
			break;
			
		case StateMalfunction:
			break;
			
		case StateReserved:
			break;
			
		case StateTrainInTransition:
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

