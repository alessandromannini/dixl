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
 */
void FSMInitEvent_NewMessage(message *message);

/**
 * Notify the timer is expired
 * @param message: pointer to the message received
 */
void FSMInitEvent_TimerExpired();

#endif /* FSMINIT_H_ */
