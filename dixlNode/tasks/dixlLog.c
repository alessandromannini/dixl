/**
 * dxilLog.c
 * 
 * Logger task
 *
 * @author: Alessandro Mannini <alessandro.mannini@gmail.com>
 * @date: Jan 10, 2023
 */

/* includes */
#include <stdlib.h>
#include <stdbool.h>

#include <msgQLib.h>
#include <taskLib.h>
#include <syslog.h>
#include <string.h>

#include "../config.h"
#include "../datatypes/messages.h"
#include "../globals.h"
#include "dixlLog.h"

#include "../includes/utils.h"
#include "dixlComm.h"

/* variables */
// Task
TASK_ID     taskLogId;

// Input message queue
MSG_Q_ID 	msgQLogId;
nodeId NodeNULL = {0 ,0 ,0 ,0};

// Log data
static logMessage logLines[TASKLOGMAXLINES];		// Storage area for log lines
static int head;									// Log line head index
static int numLines = 0;							// Number of lines currently in the storage
static int newestSent = -1;							// Index of the last line sent in the last request (from the host)

/* Helpers functions */
/* Log a new line */
void logger_log(eLogType type, routeId requestedRouteId, nodeId source) {
	logMessage logMessage; 		// Log line
	message message;			// Task queue message
	
	// Construct the log line
	clock_gettime(CLOCK_REALTIME, &logMessage.timestamp);
	logMessage.type = type;
	logMessage.requestedRouteId = requestedRouteId;
	logMessage.source = source;

	// Incapsulate in queue message
	message.iHeader.type = IMSGTYPE_LOG;
	message.logILog.message = logMessage;	
	
	// Enqueue to Log tak queue (logger_log caller it's a different task)
	msgQ_Send(msgQLogId, (char *) &message, sizeof(msgIHeader) + sizeof(msgILog));
}


/* Implementation functions */
/* Store a new line */
static void logEnqueue(logMessage line) {
		
	// If number of lines reach the max, the latest is overwritten (in head position)
	if (numLines < TASKLOGMAXLINES)
	    numLines += 1;
	else {
		// head position re-used
		head = (head + 1) % TASKLOGMAXLINES;

		// If head reach newestSent, unset it
		if (newestSent >= 0 && newestSent == head) newestSent = -1;
	}
	
	// Compute insert index
	int insertIndex = ( head + numLines - 1 ) % TASKLOGMAXLINES;

	logLines[insertIndex] = line;
}

/* Get head line without dequeueing */
static bool logHead(logMessage line) {
	// Check at least one line present
	if (numLines == 0) return FALSE;
	
	// Return current head line
	line = logLines[head];
	
	return TRUE;
}

/* Get head line and dequeue it */
static bool logDequeue(logMessage line) {
	// Check at least one line present
	if (numLines == 0) return FALSE;
	
	// Decrease number of lines
	numLines -= 1;
	
	// Return current head line
	line = logLines[head];
	
	// Increase head
	head = (head + 1) % TASKLOGMAXLINES;
	
	// If head reach newestSent, unset it
	if (newestSent >= 0 && newestSent == head) newestSent = -1;
	
	return TRUE;
}

/* Send all current lines to CommTx to be sent to destination */
static bool logSendCurrent(nodeId destination) {
	// Something to send ?
	if (numLines == 0) {
		newestSent = -1;
		
		// Send a empty log sequence
		// Prepare the message for dixlCommTx
		message message;
		memset(&message, 0, sizeof(message));
		
		message.iHeader.type = IMSGTYPE_LOGSEND;
		message.logISend.destination = destination;
		message.logISend.currentLine = 0;
		message.logISend.totalLines = 0;
		
		// Send to dilCommTx
		msgQ_Send(msgQCommTxId, (char *) &message, sizeof(msgIHeader) + sizeof(msgILogSEND));		
		return FALSE;
	}
		
	// Current stored lines loop
	for (int i=0; i<numLines; i++) {
		// Prepare the message for dixlCommTx
		message message;
		memset(&message, 0, sizeof(message));
		
		message.iHeader.type = IMSGTYPE_LOGSEND;
		message.logISend.destination = destination;
		message.logISend.currentLine = (i+1);
		message.logISend.totalLines = numLines;
		message.logISend.line = logLines[ ((head+i) % TASKLOGMAXLINES) ];	
		
		// Send to dilCommTx
		msgQ_Send(msgQCommTxId, (char *) &message, sizeof(msgIHeader) + sizeof(msgILogSEND));
	}
	
	// Store last sent index
	newestSent = (head + numLines -1) % TASKLOGMAXLINES;
	
	return TRUE;
}

/* Remove all lines trasmitted in the last request */
static void logPrune() {
	// Something to prune?
	if (newestSent < 0) return;
	
	// Remove from head to newestSent
	int numRemoved = 0;
	
	// Compute num of lines to remove
	if (newestSent >= head)
		numRemoved = newestSent - head +1;
	else
		numRemoved = TASKLOGMAXLINES - head + newestSent +1;


	// Remove from head
	head = (head + numRemoved) % TASKLOGMAXLINES;
	numLines -= numRemoved;
		
	// Reset last tramitted index
	newestSent = -1;
}

void dixlLog() {
	
	// Start
	syslog(LOG_INFO, "Task started Id 0x%jx", taskLogId);	

	// Message queue initialization
	msgQLogId = msgQ_Initialize(MSGQLOGMESSAGESMAX, MSGQLOGMESSAGESLENGTH, MSG_Q_FIFO);

	// Wait for messages, log and forward
	FOREVER {
		message inMessage;
		
		// Wait a message from the Queue ... FOREVER
		msgQReceive(msgQLogId, (char *  ) &inMessage, sizeof(inMessage), WAIT_FOREVER);
		
		// Process request
		switch (inMessage.header.type) {
			// Internal Log a message
			case IMSGTYPE_LOG:
				logEnqueue(inMessage.logILog.message);
				break;
				
			// External Log lines request
			case MSGTYPE_LOGREQ:
				// Log
				syslog(LOG_INFO, "Log REQ received from host node (%d.%d.%d.%d)", inMessage.header.source.bytes[0], inMessage.header.source.bytes[1], inMessage.header.source.bytes[2], inMessage.header.source.bytes[3]);			
				
				// Send current log to the requester
				logSendCurrent(inMessage.header.source);

				// Log
				syslog(LOG_INFO, "Log SENT to host node (%d.%d.%d.%d)", inMessage.header.source.bytes[0], inMessage.header.source.bytes[1], inMessage.header.source.bytes[2], inMessage.header.source.bytes[3]);			
				
				break;

			// External Log del request (after a logREQ)
			case MSGTYPE_LOGDEL:
				// Log
				syslog(LOG_INFO, "Log DEL received from host node (%d.%d.%d.%d)", inMessage.header.source.bytes[0], inMessage.header.source.bytes[1], inMessage.header.source.bytes[2], inMessage.header.source.bytes[3]);			
				
				// Prune log lines based on last request
				logPrune();

				// Log
				syslog(LOG_INFO, "Log PRUNED ...");			

				// ACK the deletion
				message outMessage;
				outMessage.iHeader.type = IMSGTYPE_LOGDELACK;
				outMessage.logIDelAck.destination = inMessage.header.source;

				// Send to CommTx task
				msgQ_Send(msgQCommTxId, (char *) &outMessage, sizeof(msgIHeader) + sizeof(msgILogDELACK));		
				
				// Log
				syslog(LOG_INFO, "Log DEL ACK sent to host node (%d.%d.%d.%d)", inMessage.header.source.bytes[0], inMessage.header.source.bytes[1], inMessage.header.source.bytes[2], inMessage.header.source.bytes[3]);			

				break;
		}
	}
		
	// Dummy call (to avoid compiler warnings ;)
	logMessage line;
	logHead(line);
	logDequeue(line);	
}

