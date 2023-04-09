/**
 * FSMInit.h
 * 
 * Finite State Machine controlling Initilization task
 *
 * @author: Alessandro Mannini <alessandro.mannini@gmail.com>
 * @date: Jan 10, 2023
 */

#ifndef FSMINIT_H_
#define FSMINIT_H_
/* includes */
#include "../datatypes/messages.h"

/*
 * Public functions
 */
/**
 * FSM Initialization
 */
void FSMInit();

/*
 * (Public) External Events
 */
/**
 * Notify a new message Event
 * @param message: pointer to the message received
 * @param deadline: next deadline or 0
 */
void FSMInitEvent_NewMessage(message *message, struct timespec *deadline);

/**
 * Notify the timer is expired
 * @param message: pointer to the message received
 * @param deadline: next deadline or 0
 */
void FSMInitEvent_TimerExpired(message *message, struct timespec *deadline);

#endif /* FSMINIT_H_ */
