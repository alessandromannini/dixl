/**
 * messages.h
 * 
 * Internal and external messages definitions
 * 
 * @author: Alessandro Mannini <alessandro.mannini@gmail.com>
 * @date: Jan 10, 2023
 */

#ifndef MESSAGES_H_
#define MESSAGES_H_
#include "dataTypes.h"
#include "../tasks/dixlLog.h"
/**
 *  Defines
 */
#define MSG_MAXLENGTH			255		// Maximum message length
/**
 *  Enum
 */
/* Message type */
typedef enum {
	/** EXTERNAL TYPES*/
	// Service messages - Init task
	MSGTYPE_NODERESET 			= 10,	// Reset in the Init state
	MSGTYPE_NODECONFIG 			= 11,	// Routes configuration sent by the host
	MSGTYPE_NODEDISCOVERY 		= 20,	// TODO Nodes discovery from the host
	MSGTYPE_NODEADVERTISE 		= 21,	// TODO Node advertise reply to discovery
		
	// Route messages - Ctrl task
	MSGTYPE_ROUTEREQ 			= 30,	// Route request
	MSGTYPE_ROUTEACK 			= 31,	// 2PhaseCommit Ack
	MSGTYPE_ROUTENACK 			= 32,	// 2PhaseCommit NAck
	MSGTYPE_ROUTECOMMIT 		= 33,	// 2PhaseCommit Commit
	MSGTYPE_ROUTEAGREE 			= 34,	// 2PhaseCommit Agree
	MSGTYPE_ROUTEDISAGREE 		= 35,	// 2PhaseCommit Disagree
	MSGTYPE_ROUTETRAINOK 		= 36,	// 2PhaseCommit Train OK
	MSGTYPE_ROUTETRAINNOK 		= 37,	// 2PhaseCommit Train NOK

	// Log messages
	MSGTYPE_LOGREQ				= 81,	// Request current log messages
	MSGTYPE_LOGSEND				= 82,	// Response current log messages
	MSGTYPE_LOGDEL				= 83,	// Ack messages were received and ask to delete
	MSGTYPE_LOGDELACK			= 84,	// Ack messages were deleted

	// Diagnostic messages
	MSGTYPE_DIAGERRTASK			= 90,	// Diagnostic error on task
	MSGTYPE_DIAGERRCOMM 		= 91,	// Diagnostic communication error
	
	// Point requests  
	MSGTYPE_POINTMALFUNC   		= 95,   // Point set malfunction state	
	
	/** INTERNAL TYPES */
	// Config messages - ComTx task
	IMSGTYPE_COMMTXCONFIGSET    = 101,   // Set config in dixlCommTx task
	IMSGTYPE_COMMTXCONFIGRESET  = 102,   // Reset config in dixlCommTx task	

	// Service messages - Init task
	IMSGTYPE_NODECONFIGSET      = 111,   // Set config in dixlCtrl and dixlDiag task
	IMSGTYPE_NODECONFIGRESET    = 112,   // Reset config in dixlCtrl and dixlDiag task	
	
	// Route messages internal request to send to
	IMSGTYPE_ROUTEREQ 			= 130,	// Route request
	IMSGTYPE_ROUTEACK 			= 131,	// 2PhaseCommit Ack
	IMSGTYPE_ROUTENACK 			= 132,	// 2PhaseCommit NAck
	IMSGTYPE_ROUTECOMMIT 		= 133,	// 2PhaseCommit Commit	
	IMSGTYPE_ROUTEAGREE 		= 134,	// 2PhaseCommit Agree
	IMSGTYPE_ROUTEDISAGREE 		= 135,	// 2PhaseCommit Disagree
	IMSGTYPE_ROUTETRAINOK 		= 136,	// 2PhaseCommit Train OK
	IMSGTYPE_ROUTETRAINNOK 		= 137,	// 2PhaseCommit Train NOK

	// Sensors messages
	IMSGTYPE_SENSORSTATE		= 150,   // Track Circuit Sensor value request
	IMSGTYPE_SENSORNOTIFY		= 155,   // Track Circuit Sensor value notify
	
	// Log messages
	IMSGTYPE_LOG 				= 180,	// Log a message
	IMSGTYPE_LOGSEND			= 182,	// Send current  log lines to the host
	IMSGTYPE_LOGDELACK		    = 184,	// Ack log lines deletion to the host

	// Diagnostic messages
	IMSGTYPE_DIAGERRTASK		= 190,	// Diagnostic error on task
	IMSGTYPE_DIAGERRCOMM 		= 191,	// Diagnostic communication error

	// Point requests  
	IMSGTYPE_POINTRESET			= 195,   // Point position reset
	IMSGTYPE_POINTPOS			= 196,   // Point position request
	IMSGTYPE_POINTNOTIFY		= 197,   // Point position or malfunction notify

	// Timeout messate
	IMSGTYPE_TIMEOUTNOTIFY		= 199    // Point position or malfunction notify
} eMsgType;

/***************************************
 * MESSAGES and TYPES
 ***************************************/
/**
 *  EXT message HEADER
 */
typedef struct msgHeader {
	uint8_t lentgh;					// Message length
	uint8_t type;					// Type of message (eMsgType)
	uint8_t paddin[2];				// Padding to allign to 32bit
	nodeId source;					// Source node Id
	nodeId destination;				// Destination node Id
	uint8_t padding[4];				// Padding to 64bit	
} msgHeader;
/**
 *  INT message HEADER
 */
typedef struct msgIHeader {
	uint8_t paddingLeft[1];			// Padding to allign to EXT header
	uint8_t type;					// Type of message (eIMsgType)
	uint8_t padding[14];			// Padding to 64bit	
} msgIHeader;

/**
 *  EXTERNAL MESSAGES 
 */
/** message INIT types */
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

/**  message ROUTE types  */
typedef struct msgRouteREQ {
	routeId requestRouteId;			// Requested route Id
} msgRouteREQ;
typedef struct msgRouteACK {
	routeId requestRouteId;			// Requested route Id
} msgRouteACK;
typedef struct msgRouteNACK {
	routeId requestRouteId;			// Requested route Id
} msgRouteNACK;
typedef struct msgRouteCOMMIT {
	routeId requestRouteId;			// Requested route Id
} msgRouteCOMMIT;
typedef struct msgRouteAGREE {
	routeId requestRouteId;			// Requested route Id
} msgRouteAGREE;
typedef struct msgRouteDISAGREE {
	routeId requestRouteId;			// Requested route Id
} msgRouteDISAGREE;
typedef struct msgRouteTRAINOK {
	routeId requestRouteId;			// Requested route Id
} msgRouteTRAINOK;
typedef struct msgRouteTRAINNOK {
	routeId requestRouteId;			// Requested route Id
} msgRouteTRAINNOK;

/**  message POINT types */
typedef struct msgPOINTMALFUNC {
} msgPointMalfunc;

/**  message LOG types */
typedef struct msgLOGREQ {
} msgLogREQ;
typedef struct msgLOGSEND {
	uint32_t currentLine;			// Current line of the log
	uint32_t totalLines;			// Total number of lines
	logMessage line;
} msgLogSEND;
typedef struct msgLOGDEL {
} msgLogDEL;
typedef struct msgLOGDELACK {
} msgLogDELACK;

/** message DIAG types */
typedef struct msgDIAGERRTASK {
} msgDiagErrTask;
typedef struct msgDIAGERRCOMM {
	nodeId node;
} msgDiagErrComm;

/***************************************
 * INTERNAL MESSAGES and TYPES
 ***************************************/
/** message COMMTX types */ 
typedef struct msgICommTxCONFIGSET {
	nodeId hostNode;				// Address of the host
} msgICommTxCONFIGSET;
typedef struct msgICommTxCONFIGRESET {
} msgICommTxCONFIGRESET;

/** message NDDE types */ 
typedef struct msgINodeCONFIGSET {
	uint8_t nodeType;				// Type of the node ( => behaviour)
	uint8_t padding[3];	
	uint32_t numRoutes;				// Total number (N) of segments in the configuration
	route *pRoute;					// Pointer to array of routes
} msgINodeCONFIGSET;
typedef struct msgICtrlCONFIGRESET {
} msgINodeCONFIGRESET;

/** message ROUTE types */
typedef struct msgIRouteREQ {
	nodeId destination;				// Node destination
	routeId requestRouteId;			// Requested route Id
} msgIRouteREQ;
typedef struct msgIRouteACK {
	nodeId destination;				// Node destination
	routeId requestRouteId;			// Requested route Id
} msgIRouteACK;
typedef struct msgIRouteNACK {
	nodeId destination;				// Node destination
	routeId requestRouteId;			// Requested route Id
} msgIRouteNACK;
typedef struct msgIRouteCOMMIT {
	nodeId destination;				// Node destination
	routeId requestRouteId;			// Requested route Id
} msgIRouteCOMMIT;
typedef struct msgIRouteAGREE {
	nodeId destination;				// Node destination
	routeId requestRouteId;			// Requested route Id
} msgIRouteAGREE;
typedef struct msgIRouteDISAGREE {
	nodeId destination;				// Node destination
	routeId requestRouteId;			// Requested route Id
} msgIRouteDISAGREE;
typedef struct msgIRouteTRAINOK {
	nodeId destination;				// Node destination
	routeId requestRouteId;			// Requested route Id
} msgIRouteTRAINOK;
typedef struct msgIRouteTRAINNOK {
	nodeId destination;				// Node destination
	routeId requestRouteId;			// Requested route Id
} msgIRouteTRAINNOK;

/** message SENSOR types */
typedef struct msgISENSORSTATE {
	struct timespec requestTimestamp;	// Timestamp of the request as nonce	
	eSensorState requestedState;
} msgISensorSTATE;
typedef struct msgISENSORNOTIFY {
	struct timespec requestTimestamp;	// Timestamp of the request as nonce
	eSensorState currentState;
} msgISensorNOTIFY;

/** message POINT types */
typedef struct msgIPOINTRESET {
	ePointPosition requestedPosition;	// Requested point position
} msgIPointRESET;
typedef struct msgIPOINTPOS {
	struct timespec requestTimestamp;	// Timestamp of the request as nonce
	ePointPosition requestedPosition;	// Requested point position
} msgIPointPOS;
typedef struct msgIPOINTNOTIFY {
	struct timespec requestTimestamp;	// Timestamp of the request as nonce
	ePointPosition currentPosition;		// Current point position
} msgIPointNOTIFY;

/** message LOG types */
typedef struct msgILOG {
	logMessage message;
} msgILog;
typedef struct msgILOGSEND {
	nodeId destination;
	uint32_t currentLine;			// Current line of the log
	uint32_t totalLines;			// Total number of lines
	logMessage line;
} msgILogSEND;
typedef struct msgILOGDELACK {
	nodeId destination;
} msgILogDELACK;

/** message DIAG types */
typedef struct msgIDIAGERRTASK {
} msgIDiagErrTask;
typedef struct msgIDIAGERRCOMM {
	nodeId node;
} msgIDiagErrComm;

/** message TIMEOUT types */
typedef struct msgITIMEOUTNOTIFY {
} msgITimeoutNotify;


/**
 *  message GENERIC 
 */
typedef struct message {
	union {
		struct {
			msgHeader header;				// Common EXT message header
			union {
				// INIT
				msgInitRESET 		initReset;
				msgInitCONFIG 		initConfig;
				msgInitCONFIGTYPE 	initConfigType;
				
				// ROUTE
				msgRouteREQ         routeReq;
				msgRouteACK        	routeAck;
				msgRouteNACK        routeNAck;
				msgRouteCOMMIT     	routeCommit;
				msgRouteAGREE      	routeAgree;
				msgRouteDISAGREE    routeDisagree;
				msgRouteTRAINOK     routeTrainOk;
				msgRouteTRAINNOK    routeTrainNOk;	
				
				// POINT MALFUNCTION
				msgPointMalfunc    	pointMalfunc;

				//LOG
				msgLogSEND 			logSend;
				msgLogDELACK		logDelAck;

				// DIAG
				msgDiagErrTask		diagErrTask;
				msgDiagErrComm		diagErrComm;
			};
		};
		struct {
			msgIHeader iHeader;				// Common INT message header
			union {
				// COMMTX
				msgICommTxCONFIGSET   	commTxIConfigSet;
				msgICommTxCONFIGRESET 	commTxIConfigReset;

				// NODE (CTRL + DIAG)
				msgINodeCONFIGSET   	nodeIConfigSet;
				msgINodeCONFIGRESET 	nodeIConfigReset;
				
				// ROUTE
				msgIRouteREQ        	routeIReq;
				msgIRouteACK        	routeIAck;
				msgIRouteNACK       	routeINAck;
				msgIRouteCOMMIT     	routeICommit;
				msgIRouteAGREE      	routeIAgree;
				msgIRouteDISAGREE   	routeIDisagree;
				msgIRouteTRAINOK    	routeITrainOk;
				msgIRouteTRAINNOK   	routeITrainNOk;

				// SENSOR
				msgISensorSTATE        	sensorIPOS;			
				msgISensorNOTIFY       	sensorINOTIFY;		
				
				// POINT
				msgIPointRESET			pointIReset;
				msgIPointPOS			pointIPosition;
				msgIPointNOTIFY			pointINotify;
				
				// LOG
				msgILog 				logILog;
				msgILogSEND 			logISend;
				msgILogDELACK 			logIDelAck;

				// DIAG
				msgIDiagErrTask			diagIErrTask;
				msgIDiagErrComm			diagIErrComm;

				// TIMEOUT
				msgITimeoutNotify		timeoutNotify;
			};
		};
	};
} message;

#endif /* MESSAGES_H_ */
