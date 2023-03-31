/**
 * utils.c
 * 
 * Globals utlities and macro
 *
 * @author: Alessandro Mannini <alessandro.mannini@gmail.com>
 * @date: Jan 10, 2023
 */

#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include <taskLib.h>
#include <msgQLib.h>
#include <sysLib.h>
#include <syslog.h>
#include <objLibCommon.h>

#include "globals.h"
#include "network.h"

/* FUNCTIONS helpers */

MSG_Q_ID msgQ_Initialize(size_t maxMsgs, size_t maxMsgLength, int options) {
	MSG_Q_ID msgQId = 0;
	
	// Initialize Message Queue
	if ((msgQId = msgQCreate(maxMsgs, maxMsgLength, options)) == NULL) {
		int err = errno;
		/* initialization failed */
		syslog(LOG_ERR, "Message queue initialization error %d: %s", err, strerror(err));
		taskExit(rcINQUEUE_INITERR);      
	}
	
	syslog(LOG_INFO, "Message queue initialized");
	
	return msgQId;
}

BOOL msgQ_Receive(MSG_Q_ID msgQId, char *buffer, size_t  maxNBytes, int32_t msTimeout) {
	// Set the timeout
	_Vx_ticks_t timeout;
	if (msTimeout != WAIT_FOREVER && msTimeout != NO_WAIT) {
		_Vx_freq_t tickRate = sysClkRateGet();				// Ticks/s
		timeout = msTimeout * tickRate / 1000;				// ms/1000 * ticks/s
	} else 
		timeout = msTimeout;


	// Wait for a message ... 
	ssize_t rc = msgQReceive(msgQId, buffer, maxNBytes, timeout);
	
	// Error check
	if (rc == ERROR) {
		int err = errno;
		if (err == S_objLib_OBJ_TIMEOUT && timeout != WAIT_FOREVER) {
			buffer[0]='\000';
			return FALSE;
		} else {
			syslog(LOG_ERR, "Message queue receive error %d: %s", err, strerror(err));
			taskExit(rcINQUEUE_RECEIVEERR);
		}
	}
	
	return TRUE;
}

BOOL msgQ_Send(MSG_Q_ID msgQId, char *buffer, size_t  nBytes) {
	// Send the message
	STATUS rc = msgQSend(msgQId, buffer, nBytes, WAIT_FOREVER, MSG_PRI_NORMAL);
	
	// Error check
	if (rc == ERROR) {
		int err = errno;
		syslog(LOG_ERR, "Message queue receive error %d: %s", err, strerror(err));
		taskExit(rcINQUEUE_SENDERR);
	}
	
	return TRUE;	
}


BOOL msgQ_Delete(MSG_Q_ID msgQId) {
	// Delete the queue
	STATUS rc = msgQDelete(msgQId);
	
	if (rc == ERROR) {
		int err = errno;
		/* initialization failed */
		syslog(LOG_ERR, "Message queue delete error %d: %s", err, strerror(err));
		return FALSE;      
	}
	
	return TRUE;	
}

void task_wait4notReady(TASK_ID taskId, int retryDelay, int finalDelay) {
	_Vx_freq_t tickRate = sysClkRateGet();
	retryDelay *= tickRate;
	finalDelay *= tickRate;

	// Delay while task is Ready
	while (taskIsReady(taskId)) {
		taskDelay(retryDelay);
	}
	taskDelay(finalDelay);
}

void task_shutdown(TASK_ID *tId, char *tName, MSG_Q_ID *msgQId, int *socket, SEM_ID *semaphoreId) {	
	// Shutting down
	syslog(LOG_INFO, "Deleting %s task (0x%jx) ...,", tName, *tId);
	
	// Delete task
	if (tId)
		if (*tId) {
			STATUS rc = taskDelete(*tId);
			
			if (rc == OK) {
				syslog(LOG_INFO, "%s task deleted", tName);
				(*tId) = NULL;
			} else {
				syslog(LOG_ERR, "(0x%jx) error: %s", tId, strerror(errno));
			}	
		};
	
	// Delete message Queue (if present)
	if (msgQId)
		if (*msgQId)
			if (msgQ_Delete(*msgQId)) {
				syslog(LOG_INFO, "%s task Message queue deleted", tName);				
				(*msgQId) = NULL;
			}
	
	// Close socket (if present)
	if (socket) 
		if (*socket)
			if (socket_close(*socket) == SOCK_OK) {
				syslog(LOG_INFO, "%s task Socket closed", tName);
				(*socket) = NULL;
			};

	// Delete semaphone (if present)
	if (semaphoreId) 
		if (*semaphoreId) {
			
			// Acquire the semaphore
			syslog(LOG_INFO, "Taking %s task semaphore", tName);
			semMTake(*semaphoreId, WAIT_FOREVER);
	
			if (semDelete(*semaphoreId) == OK) {
				syslog(LOG_INFO, "%s task semaphore deleted", tName);
				(*semaphoreId) = NULL;
			};
		}

}

int math_ceil(int num, int den) {
 	 return ((num / den) + ((num % den)==0 ?  0 : 1));
}

