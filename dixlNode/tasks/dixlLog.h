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
	// TODO Cancellare se confermato che non servono
	// LOGTYPE_REQACK 				= 11,	// Request ACKed
	// LOGTYPE_REQNACK				= 12,	// Request NACKed
	LOGTYPE_RESERVED			= 13,	// Request AGREEed
	LOGTYPE_DISAGREE			= 14,	// Request DISAGREEed
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
