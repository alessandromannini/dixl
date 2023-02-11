/**
 * FSMCtrlSW.h
 * 
 * Finite State Machine controlling Ctrl task (Switch node type)
 *
 * @author: Alessandro Mannini <alessandro.mannini@gmail.com>
 * @date: Jan 10, 2023
 */

#ifndef FSMCTRLSW_H_
#define FSMCTRLSW_H_
/* includes */
#include "../datatypes/messages.h"

/*
 * Public functions
 */
/**
 * FSM Initialization
 */
void FSMCtrlSW();

/*
 * (Public) External Events
 */
/**
 * Notify a new message Event
 * @param message: pointer to the message received
 */
void FSMCtrlSWEvent_NewMessage(message *message);

/**
 * Notify the timer is expired
 * @param message: pointer to the message received
 */
void FSMCtrlSWEvent_TimerExpired();

#endif /* FSMCTRLSW_H_ */
