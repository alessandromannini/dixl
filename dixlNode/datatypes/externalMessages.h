/**
 * dxilComm.h
 * 
 * Communication task
 * 
 * @author: Alessandro Mannini <alessandro.mannini@gmail.com>
 * @date: Jan 10, 2023
 */

#ifndef DXILCOMM_H_
#define DXILCOMM_H_
#include "../globals.h"
/**
 *  Defines
 */
#define MSG_MAXLENGTH			255		// Maximum message length
/**
 *  Enum
 */
/* Message type */
typedef enum {
	// Service messages - Init task
	MSGTYPE_NODERESET 			= 10,	// Reset in the Init state
	MSGTYPE_NODECONFIG 			= 11,	// Routes configuration sent by the host
	MSGTYPE_NODECONFIGSET       = 12,   // Set config in dixlCtrl task
	MSGTYPE_NODECONFIGRESET     = 13,   // Reset config in dixlCtrl task
	MSGTYPE_NODEDISCOVERY 		= 20,	// TODO Nodes discovery from the host
	MSGTYPE_NODEADVERTISE 		= 21,	// TODO Node advertise reply to discovery
	
	
	// Route messages - Ctrl task
	MSGTYPE_ROUTEREQ 			= 50,	// Route request
	MSGTYPE_ROUTEACK 			= 51,	// 2PhaseCommit Ack
	MSGTYPE_ROUTEAGREE 			= 52,	// 2PhaseCommit Agree
	MSGTYPE_ROUTEDISAGREE 		= 53,	// 2PhaseCommit Disagree
	MSGTYPE_ROUTECOMMIT 		= 54,	// 2PhaseCommit Commit
	
	// Diagnostic messages - TODO
	MSGTYPE_NODEDIAG 			= 90,	// Communication diagnostic request
	MSGTYPE_NODEDIAGACK 		= 91,	// Communication diagnostic request Ack
	
	// Log messages - TODO
	MSGTYPE_LOG 				= 120,	// Log a message
	MSGTYPE_LOGREQ				= 121,	// Request current log messages
	MSGTYPE_LOGSEND				= 122,	// Response current log messages
	MSGTYPE_LOGDEL				= 123,	// Ack messages were received and ask to delete
	MSGTYPE_LOGDELACK			= 124	// Ack messages were deleted
} eMsgType;



/***************************************
 * EXTERNAL MESSAGES and TYPES
 ***************************************/
/**
 *  message HEADER 
 */
typedef struct msgHeader {
	uint8_t lentgh;					// Message length
	uint8_t type;					// Type of message (eMsgType)
	nodeId source;					// Source node Id
	nodeId destination;				// Destination node Id
	uint8_t paddin[2];				// Padding to allign to 32bit
} msgHeader;

/**
 *  message INIT types 
 */
typedef struct msgInitRESET {
} msgInitRESET;

typedef struct msgInitCONFIGTYPE {
	uint32_t sequence;				// Sequence number of the message in respect to total configuration (0 for TYPE)
	uint32_t totalSegments;			// Total number (N) of segments (routes) in the configuration
	uint8_t nodeType;				// Type of the node ( => behaviour)
} msgInitCONFIGTYPE;

typedef struct msgInitCONFIG {
	uint32_t sequence;				// Sequence number of the message in respect to total configuration (1..N)
	uint32_t totalSegments;			// Total number (N) of segments (routes) in the configuration
	route route;					// Single route
} msgInitCONFIG;

/**
 *  message ROUTE types 
 */
typedef struct msgRouteREQ {
	routeId requestRouteId;			// Requested route Id
} msgRouteREQ;



/***************************************
 * INTERNAL MESSAGES and TYPES
 ***************************************/
/**
 *  message CTRL types 
 */
typedef struct msgICtrlCONFIG {
	uint8_t nodeType;				// Type of the node ( => behaviour)
	uint32_t numRoutes;				// Total number (N) of segments in the configuration
	route *pRoute;					// Pointer to array of routes
} msgICtrlCONFIG;

/**
 *  message GENERIC 
 */
typedef struct message {
	msgHeader header;				// Common message header
	union {
		// INIT
		msgInitRESET 		initReset;
		msgInitCONFIG 		initConfig;
		msgInitCONFIGTYPE 	initConfigType;
		
		// CTRL
		msgICtrlCONFIG   	ctrlConfig;
		
		// ROUTE
		msgRouteREQ         routeReq;
	};
} message;

/*
 * Comm Rx task function
 */
void dixlCommRx();

/*
 * Comm Tx task function
 */
void dixlCommTx();

#endif /* DXILCOMM_H_ */
