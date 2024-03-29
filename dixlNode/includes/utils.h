/**
 * utils.h
 * 
 * Globals utlities and macros
 *
 * @author: Alessandro Mannini <alessandro.mannini@gmail.com>
 * @date: Jan 10, 2023
 */

#ifndef INCLUDES_UTILS_H_
#define INCLUDES_UTILS_H_
#include <stdio.h>
#include <stdbool.h>
#include <taskLib.h>

/* MACROs */

/* FUNCTIONS helpers */

/**
 * Inizialize the message Queue of the task
 * @return TRUE if queue initialization is OK, else FALSE
 */
MSG_Q_ID msgQ_Initialize(size_t maxMsgs, size_t maxMsgLength, int options);

/**
 * Receive a message from a Queue
 * 
 * @param msgQId
 * @param buffer
 * @param maxNBytes
 * @param msTimeout
 * @return
 */
bool msgQ_Receive(MSG_Q_ID msgQId, char *buffer, size_t  maxNBytes, int32_t msTimeout);

/**
 * Send a message to a Queue
 * @param msgQId
 * @param buffer
 * @param nBytes
 * @return
 */
bool msgQ_Send(MSG_Q_ID msgQId, char *buffer, size_t  nBytes);

/**
 * Wait the task with taskId ID become not ready, each test is made after a delay
 * @param taskId	: ID of the task to wait notReady  statefor
 * @param retryDelay: delay between each retry/test 
 * @param finalDelay: delay after the closing is detected
 * @return
 */
void task_wait4notReady(TASK_ID taskId, int retryDelay, int finalDelay);

/**
 * Shutdown a task logging messages
 * @param tId		: TASK_ID of the task to shutDwon
 * @param tName		: task name
 * @return			: TRUE if OK, else FALSE
 */
void task_shutdown(TASK_ID *tId, char *tName, MSG_Q_ID *msgQId, int *socket, SEM_ID *semaphoreId);

/**
 * Compute ceil of num / den
 */
int math_ceil(int num, int den);

/**
 * Compute difference between two timespecs
 * @param time1 greater timespec
 * @param time0 smaller timespec
 * @return
 */
double time_timespecdiff(const struct timespec *time1, const struct timespec *time0);


/**
 * Add time0 to time1
 * @param time1 timespec
 * @param time0 timespec
 * @return
 */
void time_timespecadd(struct timespec *time1, const struct timespec *time0);

/**
 * Compute deadline adding seconds to time1
 * @param time1 starting time
 * @param seconds unmber of seconds to add
 * @return
 */
void time_timespectimeout(struct timespec *time1, const int seconds);

/**
 * Get the period of time between a timespec and the current timestamp, in ticks
 * @param deadline timespec
 * @return number of ticks 
 */
_Vx_ticks_t time_ticksToDeadline(const struct timespec deadline);

/**
 * Get the period of time between a timespec and the current timestamp, in ticks
 * @param ntpServer: 		IP address of the NTP server
 * @param timezoneOffset:	offset of the time zone in seconds
 * @return number of ticks 
 */
bool time_set(char *ntpServer, int timezoneOffset);
#endif /* INCLUDES_UTILS_H_ */
