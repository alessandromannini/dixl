/**
 * FSMCtrlSW.c
 * 
 * Finite State Machine controlling Ctrl task (Switch node type)
 *
 * @author: Alessandro Mannini <alessandro.mannini@gmail.com>
 * @date: Jan 10, 2023
 */

/* includes */
#include <stdio.h>
#include <stdbool.h>

#include <syslog.h>

#include "FSMCtrlSW.h"
#include "../config.h"
#include "../datatypes/messages.h"
#include "../globals.h"
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
	// Prepare IROUTEREQ message for dixlCommTx
	message message;
	message.iHeader.type = IMSGTYPE_ROUTEREQ;
	
	// Log
	syslog(LOG_INFO, "Received route request (%i) propagating to next node", pCurrentNodeState->pCurrentRoute->id);
	
	// Send REQ to next node
	message.routeIReq.destination = pCurrentNodeState->pCurrentRoute->next;
	message.routeIReq.requestRouteId = pCurrentNodeState->pCurrentRoute->id;
		
	//Send to dixlCommTx task queue
	msgQ_Send(msgQCommTxId, (char *) &message, sizeof(msgIHeader) + sizeof(msgIRouteREQ));
};
static void WaitAckExit(eventData *pEventData) {
	// Get original message
	message *pInMessage = pEventData->pMessage;
	
	// If exit due to NACK, send back to prev node
	if (pInMessage->header.type == MSGTYPE_ROUTENACK) {
		// Prepare  message for prev node
		message message;
		size_t size = sizeof(msgIHeader);
		
		// First node? Send to host
		if (pCurrentNodeState->pCurrentRoute->position == NODEPOS_FIRST) {
			message.iHeader.type = IMSGTYPE_ROUTETRAINNOK;
			message.routeITrainNOk.destination = pCurrentNodeState->pCurrentRoute->prev;
			message.routeITrainNOk.requestRouteId = pCurrentNodeState->pCurrentRoute->id;
			size += sizeof(msgIRouteTRAINNOK);
			
			// Log
			syslog(LOG_INFO, "Received NACK for route (%i) sending back TRAINNOK to host node", pCurrentNodeState->pCurrentRoute->id);			
		} else {
			message.iHeader.type = IMSGTYPE_ROUTENACK;
			message.routeINAck.destination = pCurrentNodeState->pCurrentRoute->prev;
			message.routeINAck.requestRouteId = pCurrentNodeState->pCurrentRoute->id;
			size += sizeof(msgIRouteNACK);

			// Log
			syslog(LOG_INFO, "Received NACK for route (%i) sending back NACK to previous node", pCurrentNodeState->pCurrentRoute->id);
		}
		
		//Send to dixlCommTx task queue
		msgQ_Send(msgQCommTxId, (char *) &message, size);
	}
};


/**
 * STATEWAITCOMMIT
 */
static void WaitCommitState(eventData *pEventData) {
	// Prepare IROUTEACK message for dixlCommTx
	message message;
	message.iHeader.type = IMSGTYPE_ROUTEACK;
	
	// Send ACK to prev node
	message.routeIAck.destination = pCurrentNodeState->pCurrentRoute->prev;
	message.routeIAck.requestRouteId = pCurrentNodeState->pCurrentRoute->id;

	// Log
	syslog(LOG_INFO, "Route request (%i) ACKed sending back ACK to previous node", pCurrentNodeState->pCurrentRoute->id);
	
	//Send to dixlCommTx task queue
	msgQ_Send(msgQCommTxId, (char *) &message, sizeof(msgIHeader) + sizeof(msgIRouteACK));
};
static void WaitCommitExit(eventData *pEventData) {
	// Get original message
	message *pInMessage = pEventData->pMessage;

	// If exit due to DISAGREE, forward to next node (if present)
	if (pInMessage->header.type == MSGTYPE_ROUTEDISAGREE) {
		
		// Not last node? Send to next node
		if (pCurrentNodeState->pCurrentRoute->position != NODEPOS_LAST) {
			// Prepare  message for next node
			message message;
			size_t size = sizeof(msgIHeader);
			message.iHeader.type = IMSGTYPE_ROUTEDISAGREE;
			message.routeIDisagree.destination = pCurrentNodeState->pCurrentRoute->next;
			message.routeIDisagree.requestRouteId = pCurrentNodeState->pCurrentRoute->id;
			size += sizeof(msgIRouteDISAGREE);
			
			// Log
			syslog(LOG_INFO, "Received DISAGREE for route (%i) forwarding DISAGREE to next node", pCurrentNodeState->pCurrentRoute->id);			
		
			//Send to dixlCommTx task queue
			msgQ_Send(msgQCommTxId, (char *) &message, size);
		} else 
			// Log
			syslog(LOG_INFO, "Received DISAGREE for route (%i) not forwarding (last)", pCurrentNodeState->pCurrentRoute->id);			
	}
};


/**
 * STATEWAITAGREE
 */
static void WaitAgreeState(eventData *pEventData) {
	// Prepare IROUTECOMMIT message for dixlCommTx
	message message;
	message.iHeader.type = IMSGTYPE_ROUTECOMMIT;
	
	// Send COMMIT to next node
	message.routeICommit.destination = pCurrentNodeState->pCurrentRoute->next;
	message.routeICommit.requestRouteId = pCurrentNodeState->pCurrentRoute->id;

	// Log
	syslog(LOG_INFO, "Route request (%i) COMMITed forwarding COMMIT to next node", pCurrentNodeState->pCurrentRoute->id);
	
	//Send to dixlCommTx task queue
	msgQ_Send(msgQCommTxId, (char *) &message, sizeof(msgIHeader) + sizeof(msgIRouteCOMMIT));
};
static void WaitAgreeExit(eventData *pEventData) {
	// Get original message
	message *pInMessage = pEventData->pMessage;

	// If exit due to DISAGREE, send back to prev node (if present)
	if (pInMessage->header.type == MSGTYPE_ROUTEDISAGREE) {
		// Prepare  message for prev node
		message message;
		size_t size = sizeof(msgIHeader);
		
		// First node? Send to host
		if (pCurrentNodeState->pCurrentRoute->position == NODEPOS_FIRST) {
			message.iHeader.type = IMSGTYPE_ROUTETRAINNOK;
			message.routeITrainNOk.destination = pCurrentNodeState->pCurrentRoute->prev;
			message.routeITrainNOk.requestRouteId = pCurrentNodeState->pCurrentRoute->id;
			size += sizeof(msgIRouteTRAINNOK);
			
			// Log
			syslog(LOG_INFO, "Received DISAGREE for route (%i) sending back TRAINNOK to host node", pCurrentNodeState->pCurrentRoute->id);			
		} else {
			message.iHeader.type = IMSGTYPE_ROUTEDISAGREE;
			message.routeIDisagree.destination = pCurrentNodeState->pCurrentRoute->prev;
			message.routeIDisagree.requestRouteId = pCurrentNodeState->pCurrentRoute->id;
			size += sizeof(msgIRouteDISAGREE);

			// Log
			syslog(LOG_INFO, "Received DISAGREE for route (%i) sending back DISAGREE to previous node", pCurrentNodeState->pCurrentRoute->id);
		}		
		
		//Send to dixlCommTx task queue
		msgQ_Send(msgQCommTxId, (char *) &message, size);
	}
};


/**
 * STATEPOSITIONING
 */
static void PositioningState(eventData *pEventData) {
	// TODO Positioning State
};
static void PositioningExit(eventData *pEventData) {
	// TODO Positioning Exit altre azioni?
	// Get original message
	message *pInMessage = pEventData->pMessage;

	// If exit due to DISAGREE, forward to next node (if present)
	if (pInMessage->header.type == MSGTYPE_ROUTEDISAGREE) {
		
		// Not last node? Send to next node
		if (pCurrentNodeState->pCurrentRoute->position != NODEPOS_LAST) {
			// Prepare  message for next node
			message message;
			size_t size = sizeof(msgIHeader);
			message.iHeader.type = IMSGTYPE_ROUTEDISAGREE;
			message.routeIDisagree.destination = pCurrentNodeState->pCurrentRoute->next;
			message.routeIDisagree.requestRouteId = pCurrentNodeState->pCurrentRoute->id;
			size += sizeof(msgIRouteDISAGREE);
			
			// Log
			syslog(LOG_INFO, "Received DISAGREE for route (%i) forwarding DISAGREE to next node", pCurrentNodeState->pCurrentRoute->id);			
		
			//Send to dixlCommTx task queue
			msgQ_Send(msgQCommTxId, (char *) &message, size);
		} else 
			// Log
			syslog(LOG_INFO, "Received DISAGREE for route (%i) not forwarding (last)", pCurrentNodeState->pCurrentRoute->id);			
	}
};

/**
 * STATEMALFUNCTION
 */
static void MalfunctionState(eventData *pEventData) {
	// TODO Malfunction State
};

/**
 * STATERESERVED
 */
static void ReservedState(eventData *pEventData) {
	// Prepare IROUTEAGREE message for dixlCommTx
	message message;
	message.iHeader.type = IMSGTYPE_ROUTEAGREE;
	
	// Send AGREE to prev node
	message.routeIAgree.destination = pCurrentNodeState->pCurrentRoute->prev;
	message.routeIAgree.requestRouteId = pCurrentNodeState->pCurrentRoute->id;

	// Log
	syslog(LOG_INFO, "Route request (%i) AGREEed sending back AGREE to prev node", pCurrentNodeState->pCurrentRoute->id);
	
	//Send to dixlCommTx task queue
	msgQ_Send(msgQCommTxId, (char *) &message, sizeof(msgIHeader) + sizeof(msgIRouteAGREE));	
};
static void ReservedExit(eventData *pEventData) {
	// Get original message
	message *pInMessage = pEventData->pMessage;

	// If exit due to DISAGREE, forward to next node (if present)
	if (pInMessage->header.type == MSGTYPE_ROUTEDISAGREE) {
		// Prepare  message for prev node
		message message;
		size_t size = sizeof(msgIHeader);
		
		// Not last node? Send to next node
		if (pCurrentNodeState->pCurrentRoute->position != NODEPOS_LAST) {
			message.iHeader.type = IMSGTYPE_ROUTEDISAGREE;
			message.routeIDisagree.destination = pCurrentNodeState->pCurrentRoute->next;
			message.routeIDisagree.requestRouteId = pCurrentNodeState->pCurrentRoute->id;
			size += sizeof(msgIRouteDISAGREE);

			// Log
			syslog(LOG_INFO, "Received DISAGREE for route (%i) forwarding DISAGREE to next node", pCurrentNodeState->pCurrentRoute->id);

			//Send to dixlCommTx task queue
			msgQ_Send(msgQCommTxId, (char *) &message, size);
		} else 
			// Log
			syslog(LOG_INFO, "Received DISAGREE for route (%i) not forwarding (last)", pCurrentNodeState->pCurrentRoute->id);					
	}
};

/**
 * STATEINTRANSITION
 */
static void TrainInTransitionState(eventData *pEventData) {
	// Only if FIRST node
	// Prepare IROUTETRAINOK message for dixlCommTx
	if (pCurrentNodeState->pCurrentRoute->position == NODEPOS_FIRST) {
		message message;
		message.iHeader.type = IMSGTYPE_ROUTETRAINOK;
		
		// Send AGREE to prev node
		message.routeITrainOk.destination = pCurrentNodeState->pCurrentRoute->prev;
		message.routeITrainOk.requestRouteId = pCurrentNodeState->pCurrentRoute->id;
	
		// Log
		syslog(LOG_INFO, "Route request (%i) TRAIN OK reached sending back to host node", pCurrentNodeState->pCurrentRoute->id);
		
		//Send to dixlCommTx task queue
		msgQ_Send(msgQCommTxId, (char *) &message, sizeof(msgIHeader) + sizeof(msgIRouteTRAINOK));
	} else
		// Log
		syslog(LOG_INFO, "Route request (%i) TRAIN OK reached not sending back (not first)", pCurrentNodeState->pCurrentRoute->id);
		
};
static void TrainInTransitionExit(eventData *pEventData) {
	// Log
	syslog(LOG_INFO, "Route request (%i) SENSOR OFF received", pCurrentNodeState->pCurrentRoute->id);
};

static StateMapItem StateMap[] = {
		// StateDummy
		{ NULL,						NULL, 				NULL},
		// StateNotReserved
		{ NotReservedState, 		NULL, 				NULL},
		// StateWaitAck
		{ WaitAckState,				NULL,				WaitAckExit},
		// StateWaitCommit
		{ WaitCommitState, 			NULL, 				WaitCommitExit},
		// StateWaitAgree
		{ WaitAgreeState,			NULL,				WaitAgreeExit},
		// StatePositioning
		{ PositioningState,			NULL,				PositioningExit},
		// StateMalfunction
		{ MalfunctionState,			NULL,				NULL},
		// StateReserved
		{ ReservedState,			NULL,				ReservedExit},
		// StateTrainInTransition
		{ TrainInTransitionState,	NULL,				TrainInTransitionExit},
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
			// Accept only ROUTEREQ messages, discard others			
			if (pMessage->header.type == MSGTYPE_ROUTEREQ) {
				// Set information of the requested route or fail
				if (!setRoute(pMessage->routeReq.requestRouteId))
					condition = FALSE;
				else {
					// Go to next state depending on node position (in the current requested route)
					switch (pCurrentNodeState->pCurrentRoute->position) {
						case NODEPOS_FIRST:
						case NODEPOS_MIDDLE:
							newState = StateWaitAck;
							break;
							
						case NODEPOS_LAST:
							newState = StateWaitCommit;
							break;
					}
					condition = TRUE;
				}
			} else {
				// Discard other messages
				condition = FALSE;
			}
			break;
			
		case StateWaitAck:
			// Accept only current requested route id
			// Accept only ACK or NACK messages, discard others
			switch (pMessage->header.type) {
				case MSGTYPE_ROUTEACK:
					if (pMessage->routeAck.requestRouteId == pCurrentNodeState->pCurrentRoute->id) {
						// Go to next state depending on node position FIRST or MIDDLE (in the current requested route)
						switch (pCurrentNodeState->pCurrentRoute->position) {
							case NODEPOS_FIRST:
								newState = StateWaitAgree;
								break;
								
							case NODEPOS_MIDDLE:
								newState = StateWaitAck;
								break;
						}
						condition = TRUE;
					} else
						// Discard
						condition = FALSE;
					break;
				
				case MSGTYPE_ROUTENACK:
					if (pMessage->routeINAck.requestRouteId == pCurrentNodeState->pCurrentRoute->id) {
						// Not necessary to test NodePosition
						// Exit send NACK to prev or TRAINNOK to host
						newState = StateNotReserved;
						condition = TRUE;
					} else
						// Discard
						condition = FALSE;
					break;
					
				default:
					condition = FALSE;
					break;
			}						
			break;
			
		case StateWaitCommit:
			// Accept only current requested route id
			// Accept only COMMIT or DISAGREE messages, discard others
			switch (pMessage->header.type) {
				case MSGTYPE_ROUTECOMMIT:
					if (pMessage->routeCommit.requestRouteId == pCurrentNodeState->pCurrentRoute->id) {
						// Go to next state depending on node position LAST or MIDDLE (in the current requested route)
						switch (pCurrentNodeState->pCurrentRoute->position) {
							case NODEPOS_MIDDLE:
								newState = StateWaitAgree;
								break;
								
							case NODEPOS_LAST:
								newState = StateReserved;
								break;
						}
						condition = TRUE;
					} else
						// Discard
						condition = FALSE;
					break;
				
				case MSGTYPE_ROUTEDISAGREE:
					if (pMessage->routeIDisagree.requestRouteId == pCurrentNodeState->pCurrentRoute->id) {
						// Not necessary to test NodePosition
						// Exit send DISAGREE to next node if is not last
						newState = StateNotReserved;
						condition = TRUE;
					} else
						// Discard
						condition = FALSE;
					break;

				default:
					condition = FALSE;
					break;
			}						
			break;
			
		case StateWaitAgree:
			// Accept only current requested route id
			// Accept only AGREE or DISAGREE messages, discard others
			switch (pMessage->header.type) {
				case MSGTYPE_ROUTEAGREE:
					if (pMessage->routeCommit.requestRouteId == pCurrentNodeState->pCurrentRoute->id) {
						// Go to next state depending Switch position, no need to check NodePosition(in the current requested route)
						// If Switch not in position, go to positioning
						// TODO Position check
						if (NULL) {
							newState = StatePositioning;						
						} else {
							// TODO Valutare se necessario passare direttamente a StateInTransition per il FIRST come è nel CtrlTC)
							newState = StateReserved;
						}
						condition = TRUE;
					} else
						// Discard
						condition = FALSE;
					break;
				
				case MSGTYPE_ROUTEDISAGREE:
					if (pMessage->routeIDisagree.requestRouteId == pCurrentNodeState->pCurrentRoute->id) {
						// Not necessary to test NodePosition
						// Exit send DISAGREE to prev node or TRAINNOK to host
						newState = StateNotReserved;
						condition = TRUE;
					} else
						// Discard
						condition = FALSE;
					break;

				default:
					condition = FALSE;
					break;
			}						
			break;
			
		case StatePositioning:
			// TODO Check position
			// Accept only 
			break;
			
		case StateMalfunction:
			// TODO Check Malfuncton
			break;
			
		case StateReserved:
			// Accept only current requested route id (for DISAGREE)
			// Accept only SENSORON or DISAGREE messages, discard others
			switch (pMessage->header.type) {
			// TODO SensorON check
				case IMSGTYPE_SENSORON:						
					newState = StateTrainInTransition;
					condition = TRUE;
					break;
				
				case MSGTYPE_ROUTEDISAGREE:
					if (pMessage->routeIDisagree.requestRouteId == pCurrentNodeState->pCurrentRoute->id) {
						// Not necessary to test NodePosition
						// Exit send DISAGREE to next node
						newState = StateNotReserved;
						condition = TRUE;
					} else
						// Discard
						condition = FALSE;
					break;

				default:
					condition = FALSE;
					break;
			}						
			break;
			break;
			
		case StateTrainInTransition:
			// Accept only SENSOROFF
			// TODO SensorOFF
			if (pMessage->iHeader.type == IMSGTYPE_SENSOROFF) {
				newState = StateNotReserved;
				condition = TRUE;
			} else {
				// Discard other messages
				condition = FALSE;
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

