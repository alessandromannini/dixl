/**
 * dxilCtrl.c
 *
 * Control logic task
 * 
 * @author: Alessandro Mannini <alessandro.mannini@gmail.com>
 * @date: Jan 10, 2023
 */

/* includes */
#include <stdlib.h>
#include <stdbool.h>

#include <vxWorks.h>
#include <msgQLib.h>
#include <taskLib.h>
#include <syslog.h>

#include "../globals.h"
#include "../config.h"
#include "../includes/utils.h"
#include "dixlCtrl.h"
#include "dixlComm.h"
#include "../FSM/FSMCtrlPOINT.h"
#include "../FSM/FSMCtrlTRACKCIRCUIT.h"
#include "../includes/utils.h"

/* types */
/* FSM generic manage functions  */
typedef void (*FSMCtrlFunc)(NodeState *pEventData);
typedef void (*FSMCtrlEventNewMessage)(message *message, struct timespec *deadline);
typedef void (*FSMCtrlEventTimeout)(struct timespec *deadline);

/* variables */
// Task
TASK_ID     taskCtrlId;

// Input message queue
MSG_Q_ID 	msgQCtrlId;

// Node state
NodeState nodeState;

struct timespec deadline;						// Next deadline for message timeout or zero
FSMCtrlFunc FSMCtrl = NULL;						// Point to FSM initialization function
FSMCtrlEventNewMessage FSMNewMessage;			// Pointer to FSM New Message Event function
FSMCtrlEventTimeout    FSMTimeout = NULL; 		// Pointer to FSM Timeout Event function

void dixlCtrl() {
	
	// Start
	syslog(LOG_INFO, "Task started Id 0x%jx", taskCtrlId);	

	// Message queue initialization
	msgQCtrlId = msgQ_Initialize(MSGQCTRLMESSAGESMAX, MSGQCTRLMESSAGESLENGTH, MSG_Q_FIFO);
	
	// Reset deadline
	deadline.tv_sec = 0;
	deadline.tv_nsec = 0;

	// Wait for messages and execute FSM
	FOREVER {
		message message, messagePoint;
		
		// Get timeout in ticks
		_Vx_ticks_t timeout = time_ticksToDeadline(deadline);
		ssize_t ret = 0;

		// Wait a message from the Queue ... FOREVER or till timeout
		if (timeout > 0)
			ret = msgQReceive(msgQCtrlId, (char *  ) &message, sizeof(message), timeout);
		else
			ret = msgQReceive(msgQCtrlId, (char *  ) &message, sizeof(message), WAIT_FOREVER);
		
		// Check if timedout
		if (ret == ERROR && errno == S_objLib_OBJ_TIMEOUT) {
			// Log
			syslog(LOG_INFO, "Timeout reacted");
			
			// If Event handler configured, notify the message
			if (FSMTimeout)
				// Notify the new message to the FSM
				FSMTimeout(&deadline);
			else
				// Log and error and ignore the message
				syslog(LOG_ERR, "Node not configured: timeout discarted");
			
			// skip the loop
			continue;
		}
		
		// CONFIGRESET e CONFIGSET messages processed right here (Init can send them at any time), other passed to the current FSM
		switch (message.header.type) {
			// CONFIG RESET message
			case IMSGTYPE_NODECONFIGRESET:
				// TODO
				// -reelase resource of/or FSM
				// - put in fail-safe status
				
				// If was configured as POINT propage reset to dixlPoint
				// Send to dixlPoint task queue (only if previously configured as POINT)
				if (nodeState.nodeType == NODETYPE_POINT && FSMCtrl) {
					messagePoint.iHeader.type = IMSGTYPE_POINTRESET;
					messagePoint.pointIReset.requestedPosition = POINTPOS_STRAIGHT;
					msgQ_Send(msgQPointId, (char *) &messagePoint, sizeof(msgIHeader) + sizeof(msgIPointRESET));		
				}
				
				// Clean FSM function pointers
				FSMCtrl = NULL;
				FSMNewMessage = NULL;
				FSMTimeout = NULL;
				
				// Reset Node State
				nodeState.pCurrentRoute = NULL;
				nodeState.pRouteList = NULL;
				nodeState.numRoutes = 0;
				break;
				
			// CONFIG SET message
			case IMSGTYPE_NODECONFIGSET:
				// TODO (come RESET, fare unica function)
				// -reelase resource of/or FSM
				// - put in fail-safe status
				
				// Get CONFIG
				nodeState.pRouteList = message.nodeIConfigSet.pRoute;
				nodeState.numRoutes = message.nodeIConfigSet.numRoutes;				
				nodeState.nodeType = message.nodeIConfigSet.nodeType;
				
				// Set FSM function pointers and initialize it
				if (nodeState.nodeType == NODETYPE_TRACKCIRCUIT) {
					// Track Circuit Node
					FSMCtrl = FSMCtrlTRACKCIRCUIT;
					FSMNewMessage = FSMCtrlTRACKCIRCUITEvent_NewMessage;
					FSMTimeout = FSMCtrlTRACKCIRCUITEvent_TimerExpired;
					syslog(LOG_INFO, "Node configured for Track Circuit logic");
				} else {
					// Point Node
					FSMCtrl = FSMCtrlPOINT;
					FSMNewMessage = FSMCtrlPOINTEvent_NewMessage;
					FSMTimeout = FSMCtrlPOINTEvent_TimerExpired;
					syslog(LOG_INFO, "Node configured for Point logic");
				}
						
				// FSM Initialize function passing NodeState pointer
				FSMCtrl(&nodeState);
					
				break;
				
			// Other messages passed to the FSM (if configured)
			default:
				// If Event handler configured, notify the message
				if (FSMNewMessage)
					// Notify the new message to the FSM
					FSMNewMessage(&message, &deadline);
				else
					// Log and error and ignore the message
					syslog(LOG_ERR, "Node not configured: message discarted");

				break;
		}		
		
	}
}

