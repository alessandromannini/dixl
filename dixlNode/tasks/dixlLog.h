/**
 * dxilLog.h
 * 
 * Logger task
 *
 * @author: Alessandro Mannini <alessandro.mannini@gmail.com>
 * @date: Jan 10, 2023
 */

#ifndef DXILLOG_H_
#define DXILLOG_H_
#include <clockLib.h>

#include "../datatypes/dataTypes.h"

/**
 *  Defines
 */
#define LOG_MSGLENGTH			sizeof(logMessage)		// Length of a log message

/**
 *  Enum
 */
/* Message type */
typedef enum {
	/** TYPES*/
	// Service messages - Init task
	LOGTYPE_REQ 				= 10,	// Request received
	LOGTYPE_OCCUPIED			= 11,	// Track occupied
	LOGTYPE_REQNACK				= 12,	// Request NACKed
	LOGTYPE_DISAGREE			= 13,	// Request DISAGREEed
	LOGTYPE_RESERVED			= 14,	// Request AGREEed
	LOGTYPE_FREED				= 15,	// Track freed
	LOGTYPE_MALFUNCTION			= 90,	// Malfunction
	LOGTYPE_NOTRESERVED			= 99,	// Not reserved
} eLogType;

/*  Log message struct  */
typedef struct logMessage {
	struct timespec timestamp;			// Timestamp of the logged message
	eLogType type;					// Type of message (eMsgType)
	routeId requestedRouteId;		// Id of the requested route the message refer to
	nodeId source;					// Source node (only for REQ)
} logMessage;

/**
 * Log facility function
 * @param type: type of log row
 * @param requestedRouteId: id of the requested route the log refer to
 * @param souce: (optional) node source (only for REQ)
 */
void logger_log(eLogType type, routeId requestedRouteId, nodeId source);

/*
 * Log task function
 */
void dixlLog();

#endif /* DXILLOG_H_ */
