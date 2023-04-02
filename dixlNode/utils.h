/**
 * utils.h
 * 
 * Globals utlities and macros
 *
 * @author: Alessandro Mannini <alessandro.mannini@gmail.com>
 * @date: Jan 10, 2023
 */

#ifndef UTILS_H_
#define UTILS_H_
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

#endif /* UTILS_H_ */
