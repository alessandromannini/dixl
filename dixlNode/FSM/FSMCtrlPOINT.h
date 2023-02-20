/**
 * FSMCtrlPOINT.h
 * 
 * Finite State Machine controlling Ctrl task (Point node type)
 *
 * @author: Alessandro Mannini <alessandro.mannini@gmail.com>
 * @date: Jan 10, 2023
 */

#ifndef FSMCTRLPOINT_H_
#define FSMCTRLPOINT_H_
/* includes */
#include "../datatypes/messages.h"

/*
 * Public functions
 */
/**
 * FSM Initialization
 */
void FSMCtrlPOINT();

/*
 * (Public) External Events
 */
/**
 * Notify a new message Event
 * @param message: pointer to the message received
 */
void FSMCtrlPOINTEvent_NewMessage(message *message);

/**
 * Notify the timer is expired
 * @param message: pointer to the message received
 */
void FSMCtrlPOINTEvent_TimerExpired();

#endif /* FSMCTRLPOINT_H_ */
