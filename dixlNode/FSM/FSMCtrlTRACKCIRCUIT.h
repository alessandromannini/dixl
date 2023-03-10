/**
 * FSMCtrlTRACKCIRCUIT.h
 * 
 * Finite State Machine controlling Ctrl task (Track Circuit node type)
 *
 * @author: Alessandro Mannini <alessandro.mannini@gmail.com>
 * @date: Jan 10, 2023
 */

#ifndef FSMCTRLTC_H_
#define FSMCTRLTC_H_
/* includes */
#include "../datatypes/messages.h"

/*
 * Public functions
 */
/**
 * FSM Initialization
 */
void FSMCtrlTRACKCIRCUIT();

/*
 * (Public) External Events
 */
/**
 * Notify a new message Event
 * @param message: pointer to the message received
 */
void FSMCtrlTRACKCIRCUITEvent_NewMessage(message *message);

/**
 * Notify the timer is expired
 * @param message: pointer to the message received
 */
void FSMCtrlTRACKCIRCUITEvent_TimerExpired();

#endif /* FSMCTRLTC_H_ */
