/**
 * FSMCtrlPOINT.c
 * 
 * Finite State Machine controlling Ctrl task (Point node type)
 *
 * @author: Alessandro Mannini <alessandro.mannini@gmail.com>
 * @date: Jan 10, 2023
 */

/* includes */
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <clockLib.h>
#include <syslog.h>

#include "FSMCtrlPOINT.h"
#include "../config.h"
#include "../datatypes/messages.h"
#include "../globals.h"
#include "../includes/utils.h"


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
	StateFailSafe
} eStates;

typedef struct eventData {
	struct message *pMessage;
    struct timespec *deadline;   	// Next timeout or 0 = no timeout	
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
	eStates newState;				// New state to pass to
	eStates currentState;			// Current state
    StateMapItem *stateMap;			// Map to function for each state
	bool eventGenerated;			// An event occured and hasn't been served
	eventData *pEventData;			// Point to current event Data
} FiniteStateMachine;

/**
 *  variables 
 */
// Node State
static NodeState *pCurrentNodeState = NULL;
static struct timespec lastPointNonce; 	// Nonce of the last Point position request (the excepted one) 
static struct timespec lastSensorNonce; // Nonce of the last Sensor request (the excepted one) 

/**
 *  Functions implementation 
 */
static bool setRoute(nodeId source, routeId requestedRouteId) {
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
	logger_log(LOGTYPE_REQ, requestedRouteId, source );			
	logger_log(LOGTYPE_DISAGREE, requestedRouteId, NodeNULL );
	return FALSE;
}

static eNodePosition findPosition(routeId requestedRouteId) {
	// Search for the requested route
	route *pCurrentRoute = pCurrentNodeState->pRouteList;
	
	for (uint32_t i=0; i < pCurrentNodeState->numRoutes ; i++ ) {
		 // If found return position
		if (pCurrentRoute[i].id == requestedRouteId) {
			return pCurrentRoute[i].position;
		}
	}
		
	// Not found, return NODEPOS_UNDEFINED
	syslog(LOG_ERR, "Requested route id (%i) not found", requestedRouteId);
	return NODEPOS_UNDEFINED;
}

/**
 *  Forward functions definitions 
 */
static void FSMEvent_Internal(eStates newState, eventData *pEventData);
static void StateEngine();

/**
 * Common functions
 * 
 * @param pInMessage: original message received
 */
static void rejectRouteRequest(message *pInMessage) {
	eNodePosition position;
	
	// Get original message
	nodeId *destNode = &(pInMessage->header.source);
	message message;
	size_t size = sizeof(msgIHeader);
	routeId requestedRouteId = pInMessage->routeReq.requestRouteId;

	// Reply DISAGREE only to route request
	switch (pInMessage->header.type) {
		case MSGTYPE_ROUTEREQ:
			
			// Log
			logger_log(LOGTYPE_REQ, requestedRouteId, *destNode );			
			
			// Prepare DISAGREE/NACK message for source node			
			position = findPosition(requestedRouteId);
			
			// First node? Send to host
			if (position == NODEPOS_FIRST) {
				message.header.type = IMSGTYPE_ROUTETRAINNOK;
				message.routeITrainNOk.destination = *destNode;
				message.routeITrainNOk.requestRouteId = pInMessage->routeReq.requestRouteId;
				size += sizeof(msgIRouteTRAINNOK);
				
				// Log
				logger_log(LOGTYPE_DISAGREE, requestedRouteId, *destNode );
				syslog(LOG_INFO, "Sending TRAINNOK for route (%i) to host node (%d.%d.%d.%d)", pInMessage->routeReq.requestRouteId, destNode->bytes[0], destNode->bytes[1], destNode->bytes[2], destNode->bytes[3]);			
			} else {
				message.iHeader.type = IMSGTYPE_ROUTENACK;
				message.routeINAck.destination = *destNode;
				message.routeINAck.requestRouteId = pInMessage->routeReq.requestRouteId;
				size += sizeof(msgIRouteNACK);
	
				// Log
				logger_log(LOGTYPE_REQNACK, requestedRouteId, *destNode );
				syslog(LOG_INFO, "Sending NACK for route (%i) to node (%d.%d.%d.%d)", pInMessage->routeReq.requestRouteId, destNode->bytes[0], destNode->bytes[1], destNode->bytes[2], destNode->bytes[3]);			
			}		
			
			//Send to dixlCommTx task queue
			msgQ_Send(msgQCommTxId, (char *) &message, size);
			break;
			
		default:
			syslog(LOG_INFO, "Received unexcepted message (%i) from node (%d.%d.%d.%d)", pInMessage->header.type, destNode->bytes[0], destNode->bytes[1], destNode->bytes[2], destNode->bytes[3]);
			break;
	}
}

/**
 * STATENOTRESERVED
 */
static void NotReservedEntry(eventData *pEventData) {
	// Clean current request
	pCurrentNodeState->pCurrentRoute = NULL;
	
	// Log
	syslog(LOG_INFO, "Request cleaned");	
	logger_log(LOGTYPE_NOTRESERVED, 0, NodeNULL);	
	
	// Reset timeout
	if (pEventData) {
		pEventData->deadline->tv_sec = 0;	
		pEventData->deadline->tv_nsec = 0;		
	}
}
static void NotReservedState(eventData *pEventData) {
}
static void NotReservedExit(eventData *pEventData) {
	// Get original message
	message *pInMessage = pEventData->pMessage;
	
	// If DIAGERR* do nothing
	if (pInMessage->header.type == IMSGTYPE_DIAGERRCOMM || pInMessage->header.type == IMSGTYPE_DIAGERRTASK)
		return;
	
	// Last node ?
	if (pCurrentNodeState->pCurrentRoute->position != NODEPOS_LAST) {
		// Log
		nodeId *destNode = &(pCurrentNodeState->pCurrentRoute->next);
		syslog(LOG_INFO, "Received route request (%i) propagating to next node (%d.%d.%d.%d)", pCurrentNodeState->pCurrentRoute->id, destNode->bytes[0], destNode->bytes[1], destNode->bytes[2], destNode->bytes[3]);
	} else 
		// Log
		syslog(LOG_INFO, "Received route request (%i) not propagating (last)", pCurrentNodeState->pCurrentRoute->id);

	// Log
	logger_log(LOGTYPE_REQ, pCurrentNodeState->pCurrentRoute->id, pInMessage->header.source );
	
}

/**
 * STATEWAITACK
 */
static void WaitAckEntry(eventData *pEventData) {
	// Prepare IROUTEREQ message for dixlCommTx
	message message;
	message.iHeader.type = IMSGTYPE_ROUTEREQ;
	
	// Send REQ to next node
	message.routeIReq.destination = pCurrentNodeState->pCurrentRoute->next;
	message.routeIReq.requestRouteId = pCurrentNodeState->pCurrentRoute->id;
		
	//Send to dixlCommTx task queue
	msgQ_Send(msgQCommTxId, (char *) &message, sizeof(msgIHeader) + sizeof(msgIRouteREQ));

	// Set timeout
	clock_gettime(CLOCK_REALTIME, pEventData->deadline);	
	time_timespectimeout(pEventData->deadline, COMMMSGTIMEOUT);	
}
static void WaitAckState(eventData *pEventData) {

}
static void WaitAckExit(eventData *pEventData) {
	// Get original message
	message *pInMessage = pEventData->pMessage;
	
	// If DIAGERR* do nothing
	if (pInMessage->header.type == IMSGTYPE_DIAGERRCOMM || pInMessage->header.type == IMSGTYPE_DIAGERRTASK)
		return;	
	
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
			nodeId *destNode = &(pCurrentNodeState->pCurrentRoute->prev);
			logger_log(LOGTYPE_DISAGREE, pCurrentNodeState->pCurrentRoute->id, NodeNULL );			
			syslog(LOG_INFO, "Received NACK for route (%i) sending back TRAINNOK to host node (%d.%d.%d.%d)", pCurrentNodeState->pCurrentRoute->id, destNode->bytes[0], destNode->bytes[1], destNode->bytes[2], destNode->bytes[3]);
		} else {
			message.iHeader.type = IMSGTYPE_ROUTENACK;
			message.routeINAck.destination = pCurrentNodeState->pCurrentRoute->prev;
			message.routeINAck.requestRouteId = pCurrentNodeState->pCurrentRoute->id;
			size += sizeof(msgIRouteNACK);

			// Log
			nodeId *destNode = &(pCurrentNodeState->pCurrentRoute->prev);
			logger_log(LOGTYPE_REQNACK, pCurrentNodeState->pCurrentRoute->id, NodeNULL );			
			syslog(LOG_INFO, "Received NACK for route (%i) sending back NACK to previous node (%d.%d.%d.%d)", pCurrentNodeState->pCurrentRoute->id, destNode->bytes[0], destNode->bytes[1], destNode->bytes[2], destNode->bytes[3]);
		}
		
		//Send to dixlCommTx task queue
		msgQ_Send(msgQCommTxId, (char *) &message, size);
	}
}

/**
 * STATEWAITCOMMIT
 */
static void WaitCommitEntry(eventData *pEventData) {
	// Prepare IROUTEACK message for dixlCommTx
	message message;
	message.iHeader.type = IMSGTYPE_ROUTEACK;
	
	// Send ACK to prev node
	message.routeIAck.destination = pCurrentNodeState->pCurrentRoute->prev;
	message.routeIAck.requestRouteId = pCurrentNodeState->pCurrentRoute->id;

	// Log
	nodeId *destNode = &(pCurrentNodeState->pCurrentRoute->prev);
	syslog(LOG_INFO, "Route request (%i) ACKed sending back ACK to previous node (%d.%d.%d.%d)", pCurrentNodeState->pCurrentRoute->id, destNode->bytes[0], destNode->bytes[1], destNode->bytes[2], destNode->bytes[3]);
	
	//Send to dixlCommTx task queue
	msgQ_Send(msgQCommTxId, (char *) &message, sizeof(msgIHeader) + sizeof(msgIRouteACK));
	
	// Set timeout
	clock_gettime(CLOCK_REALTIME, pEventData->deadline);	
	time_timespectimeout(pEventData->deadline, COMMMSGTIMEOUT);	
}
static void WaitCommitState(eventData *pEventData) {
}
static void WaitCommitExit(eventData *pEventData) {
	// Get original message
	message *pInMessage = pEventData->pMessage;

	// If DIAGERR* do nothing
	if (pInMessage->header.type == IMSGTYPE_DIAGERRCOMM || pInMessage->header.type == IMSGTYPE_DIAGERRTASK)
		return;
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
			nodeId *destNode = &(pCurrentNodeState->pCurrentRoute->next);
			syslog(LOG_INFO, "Received DISAGREE for route (%i) forwarding DISAGREE to next node (%d.%d.%d.%d)", pCurrentNodeState->pCurrentRoute->id, destNode->bytes[0], destNode->bytes[1], destNode->bytes[2], destNode->bytes[3]);
		
			//Send to dixlCommTx task queue
			msgQ_Send(msgQCommTxId, (char *) &message, size);
		} else 
			// Log
			syslog(LOG_INFO, "Received DISAGREE for route (%i) not forwarding (last)", pCurrentNodeState->pCurrentRoute->id);	
		
		// Log
		logger_log(LOGTYPE_DISAGREE, pCurrentNodeState->pCurrentRoute->id, NodeNULL );
	}
}


/**
 * STATEWAITAGREE
 */
static void WaitAgreeEntry(eventData *pEventData) {
	// Prepare IROUTECOMMIT message for dixlCommTx
	message message;
	message.iHeader.type = IMSGTYPE_ROUTECOMMIT;
	
	// Send COMMIT to next node
	message.routeICommit.destination = pCurrentNodeState->pCurrentRoute->next;
	message.routeICommit.requestRouteId = pCurrentNodeState->pCurrentRoute->id;

	// Log
	nodeId *destNode = &(pCurrentNodeState->pCurrentRoute->next);
	syslog(LOG_INFO, "Route request (%i) COMMITed forwarding COMMIT to next node (%d.%d.%d.%d)", pCurrentNodeState->pCurrentRoute->id, destNode->bytes[0], destNode->bytes[1], destNode->bytes[2], destNode->bytes[3]);				
	
	//Send to dixlCommTx task queue
	msgQ_Send(msgQCommTxId, (char *) &message, sizeof(msgIHeader) + sizeof(msgIRouteCOMMIT));
	
	// Set timeout
	clock_gettime(CLOCK_REALTIME, pEventData->deadline);	
	time_timespectimeout(pEventData->deadline, COMMMSGTIMEOUT);	
}
static void WaitAgreeState(eventData *pEventData) {
}
static void WaitAgreeExit(eventData *pEventData) {
	// Get original message
	message *pInMessage = pEventData->pMessage;

	// If DIAGERR* do nothing
	if (pInMessage->header.type == IMSGTYPE_DIAGERRCOMM || pInMessage->header.type == IMSGTYPE_DIAGERRTASK)
		return;

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
			nodeId *destNode = &(pCurrentNodeState->pCurrentRoute->prev);
			syslog(LOG_INFO, "Received DISAGREE for route (%i) sending back TRAINNOK to host node (%d.%d.%d.%d)", pCurrentNodeState->pCurrentRoute->id, destNode->bytes[0], destNode->bytes[1], destNode->bytes[2], destNode->bytes[3]);			
		} else {
			message.iHeader.type = IMSGTYPE_ROUTEDISAGREE;
			message.routeIDisagree.destination = pCurrentNodeState->pCurrentRoute->prev;
			message.routeIDisagree.requestRouteId = pCurrentNodeState->pCurrentRoute->id;
			size += sizeof(msgIRouteDISAGREE);

			// Log
			nodeId *destNode = &(pCurrentNodeState->pCurrentRoute->prev);
			syslog(LOG_INFO, "Received DISAGREE for route (%i) sending back DISAGREE to previous node (%d.%d.%d.%d)", pCurrentNodeState->pCurrentRoute->id, destNode->bytes[0], destNode->bytes[1], destNode->bytes[2], destNode->bytes[3]);			
		}		
		
		// Log
		logger_log(LOGTYPE_DISAGREE, pCurrentNodeState->pCurrentRoute->id, NodeNULL );
		
		//Send to dixlCommTx task queue
		msgQ_Send(msgQCommTxId, (char *) &message, size);
	}
}


/**
 * STATEPOSITIONING
 */
static void PositioningEntry(eventData *pEventData) {			
	// Prepare  message to request position to Point task	
	message message;
	size_t size = sizeof(msgIHeader);
	message.iHeader.type = IMSGTYPE_POINTPOS;
	message.pointIPosition.requestedPosition = pCurrentNodeState->pCurrentRoute->requestedPosition;
	clock_gettime(CLOCK_REALTIME, &lastPointNonce);
	message.pointIPosition.requestTimestamp = lastPointNonce;
	size += sizeof(msgIPointPOS);
	
	// Log
	syslog(LOG_INFO, "Route request (%i) AGREEed requesting %s positioning to Point with nonce %i ", pCurrentNodeState->pCurrentRoute->id, pointPosStr(pCurrentNodeState->pCurrentRoute->requestedPosition), lastPointNonce);				

	//Send to dixlPoint task queue
	msgQ_Send(msgQPointId, (char *) &message, size);	

	// Reset timeout
	pEventData->deadline->tv_sec = 0;	
	pEventData->deadline->tv_nsec = 0;	
}
static void PositioningState(eventData *pEventData) {	
}
static void PositioningExit(eventData *pEventData) {
	// Get original message
	message *pInMessage = pEventData->pMessage;

	// If DIAGERR* do nothing
	if (pInMessage->header.type == IMSGTYPE_DIAGERRCOMM || pInMessage->header.type == IMSGTYPE_DIAGERRTASK)
		return;

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
			nodeId *destNode = &(pCurrentNodeState->pCurrentRoute->next);
			syslog(LOG_INFO, "Received DISAGREE for route (%i) forwarding DISAGREE to next node (%d.%d.%d.%d)", pCurrentNodeState->pCurrentRoute->id, destNode->bytes[0], destNode->bytes[1], destNode->bytes[2], destNode->bytes[3]);			
		
			//Send to dixlCommTx task queue
			msgQ_Send(msgQCommTxId, (char *) &message, size);
		} else 
			// Log
			syslog(LOG_INFO, "Received DISAGREE for route (%i) not forwarding (last)", pCurrentNodeState->pCurrentRoute->id);	

		// Log
		logger_log(LOGTYPE_DISAGREE, pCurrentNodeState->pCurrentRoute->id, NodeNULL );
	}
}

/**
 * STATEMALFUNCTION
 */
static void MalfunctionStateEntry(eventData *pEventData) {
	// Prepare messagePrev (DISAGREE, TRAINNOK) and messageNext (DISAGREE) for dixlCommTx
	message messagePrev, messageNext;
	size_t size;
	
	// DISAGREE/TRAINNOK to prev node
	// If First send TRAINNOK to host otherwise DISAGREE to prev node
	if (pCurrentNodeState->pCurrentRoute->position == NODEPOS_FIRST) {
		// Prepare IROUTETRAINOK messagePrev for dixlCommTx
		messagePrev.iHeader.type = IMSGTYPE_ROUTETRAINNOK;
		size = sizeof(msgIHeader);
		
		// Send TRAINNOK to host node
		messagePrev.routeITrainNOk.destination = pCurrentNodeState->pCurrentRoute->prev;
		messagePrev.routeITrainNOk.requestRouteId = pCurrentNodeState->pCurrentRoute->id;
		size += sizeof(msgIRouteTRAINNOK);
	
		// Log
		nodeId *destNode = &(pCurrentNodeState->pCurrentRoute->prev);
		syslog(LOG_INFO, "Route request (%i) MALFUNCTION reached sending TRAINNOK to host node (%d.%d.%d.%d)", pCurrentNodeState->pCurrentRoute->id, destNode->bytes[0], destNode->bytes[1], destNode->bytes[2], destNode->bytes[3]);					
	} else {
		// Prepare IROUTEDISAGREE messagePrev for dixlCommTx
		messagePrev.iHeader.type = IMSGTYPE_ROUTEDISAGREE;
		size = sizeof(msgIHeader);
		
		// Send DISAGREE to prev node
		messagePrev.routeIDisagree.destination = pCurrentNodeState->pCurrentRoute->prev;
		messagePrev.routeIDisagree.requestRouteId = pCurrentNodeState->pCurrentRoute->id;
		size += sizeof(msgIRouteDISAGREE);
	
		// Log
		nodeId *destNode = &(pCurrentNodeState->pCurrentRoute->prev);
		syslog(LOG_INFO, "Route request (%i) MALFUNCTION reached sending back DISAGREE to prev node (%d.%d.%d.%d)", pCurrentNodeState->pCurrentRoute->id, destNode->bytes[0], destNode->bytes[1], destNode->bytes[2], destNode->bytes[3]);			
	}

	// Log	
	logger_log(LOGTYPE_MALFUNCTION, 0, NodeNULL );			
	logger_log(LOGTYPE_DISAGREE, pCurrentNodeState->pCurrentRoute->id, NodeNULL );			
	
	//Send to dixlCommTx task queue
	msgQ_Send(msgQCommTxId, (char *) &messagePrev, size);

	// DISAGREE to next node to abort the reservation
	// If not LAST send DISAGREE to next node
	if (pCurrentNodeState->pCurrentRoute->position != NODEPOS_LAST) {
		// Prepare IROUTETRAINOK messagePrev for dixlCommTx
		messageNext.iHeader.type = IMSGTYPE_ROUTEDISAGREE;
		size = sizeof(msgIHeader);
		
		// Send DISAGREE to prev node
		messageNext.routeIDisagree.destination = pCurrentNodeState->pCurrentRoute->next;
		messageNext.routeIDisagree.requestRouteId = pCurrentNodeState->pCurrentRoute->id;
		size += sizeof(msgIRouteDISAGREE);
	
		// Log
		nodeId *destNode = &(pCurrentNodeState->pCurrentRoute->next);
		syslog(LOG_INFO, "Route request (%i) MALFUNCTION reached sending DISAGREE to next node (%d.%d.%d.%d)", pCurrentNodeState->pCurrentRoute->id, destNode->bytes[0], destNode->bytes[1], destNode->bytes[2], destNode->bytes[3]);					

		//Send to dixlCommTx task queue
		msgQ_Send(msgQCommTxId, (char *) &messageNext, size);
	} else 
		// Log
		syslog(LOG_INFO, "Route request (%i) MALFUNCTION reached not propagating (last)", pCurrentNodeState->pCurrentRoute->id);
	
	// Reset timeout
	pEventData->deadline->tv_sec = 0;	
	pEventData->deadline->tv_nsec = 0;		
}
static void MalfunctionState(eventData *pEventData) {
	// Simply go to FailSafeState
	FSMEvent_Internal(StateFailSafe, pEventData);
}


/**
 * STATERESERVED
 */
static void ReservedStateEntry(eventData *pEventData) {
	// Prepara message for dixlCommTx
	message message;
	size_t size;
	
	// If Fist send TRAINOK to host otherwise AGREE to prev node
	if (pCurrentNodeState->pCurrentRoute->position == NODEPOS_FIRST) {
		// Prepare IROUTETRAINOK message for dixlCommTx
		message.iHeader.type = IMSGTYPE_ROUTETRAINOK;
		size = sizeof(msgIHeader);
		
		// Send TRAINOK to host node
		message.routeITrainOk.destination = pCurrentNodeState->pCurrentRoute->prev;
		message.routeITrainOk.requestRouteId = pCurrentNodeState->pCurrentRoute->id;
		size += sizeof(msgIRouteTRAINOK);
		
		// Log
		nodeId *destNode = &(pCurrentNodeState->pCurrentRoute->prev);
		syslog(LOG_INFO, "Route request (%i) TRAIN OK reached sending back to host node (%d.%d.%d.%d)", pCurrentNodeState->pCurrentRoute->id, destNode->bytes[0], destNode->bytes[1], destNode->bytes[2], destNode->bytes[3]);					
	} else {
		// Prepare IROUTEAGREE message for dixlCommTx
		message.iHeader.type = IMSGTYPE_ROUTEAGREE;
		size = sizeof(msgIHeader);
		
		// Send AGREE to prev node
		message.routeIAgree.destination = pCurrentNodeState->pCurrentRoute->prev;
		message.routeIAgree.requestRouteId = pCurrentNodeState->pCurrentRoute->id;
		size += sizeof(msgIRouteAGREE);
	
		// Log
		nodeId *destNode = &(pCurrentNodeState->pCurrentRoute->prev);
		syslog(LOG_INFO, "Route request (%i) AGREEed sending back AGREE to prev node (%d.%d.%d.%d)", pCurrentNodeState->pCurrentRoute->id, destNode->bytes[0], destNode->bytes[1], destNode->bytes[2], destNode->bytes[3]);			
	}
	
	// Log
	logger_log(LOGTYPE_RESERVED, pCurrentNodeState->pCurrentRoute->id, NodeNULL );			

	//Send to dixlCommTx task queue
	msgQ_Send(msgQCommTxId, (char *) &message, size);
	
	// Prepare  message to request state to Sensor task	
	memset(&message, 0,sizeof(message));
	size = sizeof(msgIHeader);
	message.iHeader.type = IMSGTYPE_SENSORSTATE;
	message.sensorIPOS.requestedState = SENSORSTATE_ON;
	clock_gettime(CLOCK_REALTIME, &lastSensorNonce);
	message.pointIPosition.requestTimestamp = lastSensorNonce;	
	size += sizeof(msgISensorSTATE);
	
	// Log
	syslog(LOG_INFO, "Route request (%i) waiting for SENSOR ON with nonce %i", pCurrentNodeState->pCurrentRoute->id, lastSensorNonce);				

	//Send to dixlSensor task queue
	msgQ_Send(msgQSensorId, (char *) &message, size);	

	// Reset timeout
	pEventData->deadline->tv_sec = 0;	
	pEventData->deadline->tv_nsec = 0;	
}
static void ReservedState(eventData *pEventData) {	
}
static void ReservedStateExit(eventData *pEventData) {
	// Get original message
	message *pInMessage = pEventData->pMessage;

	// If DIAGERR* do nothing
	if (pInMessage->header.type == IMSGTYPE_DIAGERRCOMM || pInMessage->header.type == IMSGTYPE_DIAGERRTASK)
		return;

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
			nodeId *destNode = &(pCurrentNodeState->pCurrentRoute->next);
			syslog(LOG_INFO, "Received DISAGREE for route (%i) forwarding DISAGREE to next node (%d.%d.%d.%d)", pCurrentNodeState->pCurrentRoute->id, destNode->bytes[0], destNode->bytes[1], destNode->bytes[2], destNode->bytes[3]);			

			//Send to dixlCommTx task queue
			msgQ_Send(msgQCommTxId, (char *) &message, size);
		} else 
			// Log
			syslog(LOG_INFO, "Received DISAGREE for route (%i) not forwarding (last)", pCurrentNodeState->pCurrentRoute->id);	
		
		// Log
		logger_log(LOGTYPE_DISAGREE, pCurrentNodeState->pCurrentRoute->id, NodeNULL );
	}
}

/**
 * STATEINTRANSITION
 */
static void TrainInTransitionEntry(eventData *pEventData) {
	// Log
	syslog(LOG_INFO, "Route request (%i) TRAIN IS GOING THROUGH", pCurrentNodeState->pCurrentRoute->id);
	
	// Prepare  message to request state to Sensor task	
	message message;
	size_t size = sizeof(msgIHeader);
	message.iHeader.type = IMSGTYPE_SENSORSTATE;
	message.sensorIPOS.requestedState = SENSORSTATE_OFF;
	clock_gettime(CLOCK_REALTIME, &lastSensorNonce);
	message.pointIPosition.requestTimestamp = lastSensorNonce;	
	size += sizeof(msgISensorSTATE);
	
	// Log
	syslog(LOG_INFO, "Route request (%i) waiting for SENSOR OFF with nonce %i", pCurrentNodeState->pCurrentRoute->id, lastSensorNonce);					

	//Send to dixlSensor task queue
	msgQ_Send(msgQSensorId, (char *) &message, size);			

	// Reset timeout
	pEventData->deadline->tv_sec = 0;	
	pEventData->deadline->tv_nsec = 0;	
}
static void TrainInTransitionState(eventData *pEventData) {
}
static void TrainInTransitionExit(eventData *pEventData) {
	// Get original message
	message *pInMessage = pEventData->pMessage;
	
	// If DIAGERR* do nothing
	if (pInMessage->header.type == IMSGTYPE_DIAGERRCOMM || pInMessage->header.type == IMSGTYPE_DIAGERRTASK)
		return;	
	
	// Log
	syslog(LOG_INFO, "Route request (%i) SENSOR OFF received", pCurrentNodeState->pCurrentRoute->id);
}

/**
 * STATEFAILSAFE
 */
static void FailSafeEntry(eventData *pEventData) {
	// Log
	syslog(LOG_ERR, "Node is going in fail-safe mode all subsequent requests will be rejected");

	// Reset timeout
	pEventData->deadline->tv_sec = 0;	
	pEventData->deadline->tv_nsec = 0;	
}
static void FailSafeState(eventData *pEventData) {
	// Reject All reuqests
	// Filter DIAGERR* initial messages and MALFUNCTION
	switch  (pEventData->pMessage->header.type) {
		case IMSGTYPE_DIAGERRCOMM:
		case IMSGTYPE_DIAGERRTASK:
		case IMSGTYPE_POINTPOS:			
			break;
		default:
			rejectRouteRequest(pEventData->pMessage);
	}
}

static StateMapItem StateMap[] = {
		// StateDummy
		{ NULL,						NULL, 					NULL},
		// StateNotReserved
		{ NotReservedState, 		NotReservedEntry, 		NotReservedExit},
		// StateWaitAck
		{ WaitAckState,				WaitAckEntry,			WaitAckExit},
		// StateWaitCommit
		{ WaitCommitState, 			WaitCommitEntry, 		WaitCommitExit},
		// StateWaitAgree
		{ WaitAgreeState,			WaitAgreeEntry,			WaitAgreeExit},
		// StatePositioning
		{ PositioningState,			PositioningEntry,		PositioningExit},
		// StateMalfunction
		{ MalfunctionState,			MalfunctionStateEntry,	NULL},
		// StateReserved
		{ ReservedState,			ReservedStateEntry,		ReservedStateExit},
		// StateTrainInTransition
		{ TrainInTransitionState,	TrainInTransitionEntry,	TrainInTransitionExit},
		// StateFailSafe
		{ FailSafeState,			FailSafeEntry,			NULL },
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
 * STATEDUMMY
 */
void FSMCtrlPOINT(NodeState *pState) {	
	// Get pointer to NodeState
	pCurrentNodeState = pState;
	
	// Force first (Init) State
	FSM.currentState = StateDummy;	
	FSMEvent_Internal(StateNotReserved, NULL);
	
	// and process it
	StateEngine();

	// Log
	syslog(LOG_INFO, "FSM initialized");
}

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
 * @param deadline: next deadline or 0
 */
void FSMCtrlPOINTEvent_NewMessage(message *pMessage, struct timespec *deadline) {	
	
	// Result of the precondition evaluation to pass to new state
	bool condition = FALSE;
	// Event data
	eventData eventData;
	eventData.pMessage = pMessage;
	eventData.deadline = deadline;
	
	// New state
	eStates newState=NULL;
	
	// 1) in every state DIAGERR* messagessend to StateFailSafe
	// 2) in every state except NOT_RESERVED REQ messages are discarded
	if (pMessage->header.type == IMSGTYPE_DIAGERRCOMM || pMessage->header.type == IMSGTYPE_DIAGERRTASK) {
		newState = StateFailSafe;
		condition = TRUE;				
	} else if (pMessage->header.type == MSGTYPE_ROUTEREQ && FSM.currentState != StateNotReserved) {
		// SImply reject request
		rejectRouteRequest(pMessage);
		condition = FALSE;
	} else {	
		switch (FSM.currentState) {
			case StateDummy:
				// Should not happen
				syslog(LOG_ERR, "Wrong state Dummy: message received");
				taskExit(rcFSM_WRONGSTATE);
				break;
				
			case StateNotReserved:
				// Accept only ROUTEREQ messages, discard others			
				if (pMessage->header.type == MSGTYPE_ROUTEREQ) {
					// Set information of the requested route or fail
					if (!setRoute(pMessage->header.source, pMessage->routeReq.requestRouteId))
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
				// Accept only ACK or NACK or TIMOUT messages, discard others
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
					case IMSGTYPE_TIMEOUTNOTIFY:
						if (pMessage->routeNAck.requestRouteId == pCurrentNodeState->pCurrentRoute->id || pMessage->header.type == IMSGTYPE_TIMEOUTNOTIFY) {
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
				// Accept only COMMIT or DISAGREE or TIMEOUT messages, discard others
				switch (pMessage->header.type) {
					case MSGTYPE_ROUTECOMMIT:
						if (pMessage->routeCommit.requestRouteId == pCurrentNodeState->pCurrentRoute->id) {
							// Go to next state depending on node position LAST or MIDDLE (in the current requested route)
							switch (pCurrentNodeState->pCurrentRoute->position) {
								case NODEPOS_MIDDLE:
									newState = StateWaitAgree;
									break;
									
								case NODEPOS_LAST:
									newState = StatePositioning;
									break;
							}
							condition = TRUE;
						} else
							// Discard
							condition = FALSE;
						break;
					
					case MSGTYPE_ROUTEDISAGREE:
					case IMSGTYPE_TIMEOUTNOTIFY:
						if (pMessage->routeDisagree.requestRouteId == pCurrentNodeState->pCurrentRoute->id  || pMessage->header.type == IMSGTYPE_TIMEOUTNOTIFY) {
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
				// Accept only AGREE or DISAGREE or TIMEOUT messages, discard others
				switch (pMessage->header.type) {
					case MSGTYPE_ROUTEAGREE:
						if (pMessage->routeCommit.requestRouteId == pCurrentNodeState->pCurrentRoute->id) {
							// Go to positionig state to send a message to tPoint task and receive a feedback (message)
							newState = StatePositioning;						
							condition = TRUE;
						} else
							// Discard
							condition = FALSE;
						break;
					
					case MSGTYPE_ROUTEDISAGREE:
					case IMSGTYPE_TIMEOUTNOTIFY:
						if (pMessage->routeDisagree.requestRouteId == pCurrentNodeState->pCurrentRoute->id  || pMessage->header.type == IMSGTYPE_TIMEOUTNOTIFY) {
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
				// Accept only POINTNOTIFY and DISAGREE or TIMEOUT, discard others
				switch (pMessage->header.type) {
					case IMSGTYPE_POINTNOTIFY:
						// Malfunction? 
						if (pMessage->pointINotify.currentPosition == POINTPOS_UNDEFINED) {
							newState = StateMalfunction;
							condition = TRUE;
						// Check request timestamp request match (if different, discard the message)
						} else if (pMessage->pointINotify.requestTimestamp.tv_sec == lastPointNonce.tv_sec && pMessage->pointINotify.requestTimestamp.tv_nsec == lastPointNonce.tv_nsec ) {
							// If the timestamp math but the position doesn't is considered a Malfunction
							if (pMessage->pointINotify.currentPosition != pCurrentNodeState->pCurrentRoute->requestedPosition) {
								// Go to Malfunction state
								newState = StateMalfunction;
								condition = TRUE;
							} else {
								// Go to reserved state to send a message to tPoint task and receive a feedback (message)
								newState = StateReserved;						
								condition = TRUE;
							}
						} else
							// Discard
							condition = FALSE;
						break;
					
					case MSGTYPE_ROUTEDISAGREE:
					case IMSGTYPE_TIMEOUTNOTIFY:
						if (pMessage->routeDisagree.requestRouteId == pCurrentNodeState->pCurrentRoute->id || pMessage->header.type == IMSGTYPE_TIMEOUTNOTIFY) {
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
				
			case StateMalfunction:
				// Keep this state (until general dixlCtrlReset)
				newState = StateMalfunction;
				condition= TRUE;
				break;
				
			case StateReserved:
				// Accept only current requested route id (for DISAGREE)
				// Accept only SENSORON or DISAGREE messages, discard others
				switch (pMessage->header.type) {
					// If SENSOR ON received go to state TrainInTransition
					case IMSGTYPE_SENSORNOTIFY:		
						if (pMessage->sensorINOTIFY.currentState == SENSORSTATE_ON && pMessage->sensorINOTIFY.requestTimestamp.tv_sec == lastSensorNonce.tv_sec && pMessage->sensorINOTIFY.requestTimestamp.tv_nsec == lastSensorNonce.tv_nsec ) {
							newState = StateTrainInTransition;
							condition = TRUE;
						} else 
							condition = FALSE;
						break;
					
					case MSGTYPE_ROUTEDISAGREE:
						if (pMessage->routeDisagree.requestRouteId == pCurrentNodeState->pCurrentRoute->id) {
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
				if (pMessage->iHeader.type == IMSGTYPE_SENSORNOTIFY && pMessage->sensorINOTIFY.currentState == SENSORSTATE_OFF && pMessage->sensorINOTIFY.requestTimestamp.tv_sec == lastSensorNonce.tv_sec && pMessage->sensorINOTIFY.requestTimestamp.tv_nsec == lastSensorNonce.tv_nsec ) {
					newState = StateNotReserved;
					condition = TRUE;
	
				} else {
					// Discard other messages
					condition = FALSE;
				}
				break;
			
			case StateFailSafe:
				// Keep the state
				newState = StateFailSafe;
				condition = TRUE;
				break;
				
		}
	}
	
	// If the condition is passed, the new state is served
	if (condition) {
		// Generate the internal event
		// New state, event data and event flag set only if used
		FSMEvent_Internal(newState, &eventData);
		
		// Process the state change
		StateEngine();
	}	
}

// Manage the timeout creating a dummy message
void FSMCtrlPOINTEvent_TimerExpired(struct timespec *deadline) {
	// Dummy message
	message message;
	
	// Prepare the message
	message.iHeader.type = IMSGTYPE_TIMEOUTNOTIFY;
	
	// Call the message handler
	FSMCtrlPOINTEvent_NewMessage(&message, deadline);	
}

