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

#include <clockLib.h>
#include <taskLib.h>
#include <msgQLib.h>
#include <sysLib.h>
#include <syslog.h>
#include <objLibCommon.h>

#include "../globals.h"
#include "network.h"
#include "ntp.h"

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

bool msgQ_Receive(MSG_Q_ID msgQId, char *buffer, size_t  maxNBytes, int32_t msTimeout) {
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

bool msgQ_Send(MSG_Q_ID msgQId, char *buffer, size_t  nBytes) {
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


bool msgQ_Delete(MSG_Q_ID msgQId) {
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

double time_timespecdiff(const struct timespec *time1, const struct timespec *time0) {
  return (time1->tv_sec - time0->tv_sec)
      + (time1->tv_nsec - time0->tv_nsec) / 1000000000.0;
}

void time_timespecadd(struct timespec *time1, const struct timespec *time2) {
	time1->tv_sec +=  time2->tv_sec;
	time1->tv_nsec +=  time2->tv_nsec;
}

void time_timespectimeout(struct timespec *time1, const int seconds) {
	time1->tv_sec +=  seconds;
}


_Vx_ticks_t time_ticksToDeadline(const struct timespec deadline) {
	// if deadline not valued, return WAIT_FOREVER
	if (deadline.tv_sec == 0 && deadline.tv_nsec == 0) return 0;
	
	// Get current timestamp
	struct timespec current;
	clock_gettime(CLOCK_REALTIME, &current);
	
	// Compute difference
	double period = time_timespecdiff(&deadline, &current);
	
	// If zero or negative return WAIT_FOREVER
	if (period <=0) return 0;
	
	// Transform period in ticks
	_Vx_freq_t tickRate = sysClkRateGet();
	
	// Copute floor approxmation of tick per step
	_Vx_ticks_t ticks = math_ceil((unsigned int)(period * tickRate) , 1);
	
	// If zero or negative return WAIT_FOREVER
	if (ticks <=0) return 0;
	
	return ticks;
}

bool time_set(char *ntpServer, int timezoneOffset) {
	// Call NTP client
	struct timespec updated = ntpc(ntpServer, timezoneOffset);
	
	if (updated.tv_sec == 0) return false;
	
	// Get current timestamp
	struct timespec current;
	clock_gettime(CLOCK_REALTIME, &current);	
	
	// Set current timestamp
	clock_settime(CLOCK_REALTIME, &updated);
	
	
	//#compare to system time
	syslog(LOG_INFO, "System time is now %s", ctime(&updated.tv_sec));
	syslog(LOG_INFO, "System time was %0.2f seconds off\n", time_timespecdiff(&updated, &current));	
	
	return true;
}
