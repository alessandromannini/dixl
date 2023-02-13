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
#include "../utils.h"
#include "dixlCtrl.h"
#include "dixlComm.h"
#include "../FSM/FSMCtrlSW.h"
#include "../FSM/FSMCtrlTC.h"

/* types */
/* FSM generic manage functions  */
typedef void (*FSMCtrlFunc)(NodeState *pEventData);
typedef void (*FSMCtrlEventNewMessage)(message *message);
typedef void (*FSMCtrlEventTimeout)(message *message);

/* variables */
// Task
TASK_ID     taskCtrlId;

// Input message queue
MSG_Q_ID 	msgQCtrlId;
VX_MSG_Q(msgQCtrlName, MSGQCTRLMESSAGESMAX, MSGQCTRLMESSAGESLENGTH);


// Node state
NodeState nodeState;

FSMCtrlFunc FSMCtrl = NULL;						// Point to FSM initialization function
FSMCtrlEventNewMessage FSMNewMessage;			// Pointer to FSM New Message Event function
FSMCtrlEventTimeout    FSMTimeout = NULL; 		// Pointer to FSM Timeout Event function

void dixlCtrl() {
	
	// Start
	syslog(LOG_INFO, "Task started Id 0x%jx", taskCtrlId);	

	// Message queue initialization
	msgQCtrlId = msgQ_Initialize(msgQCtrlName, MSGQCTRLMESSAGESMAX, MSGQCTRLMESSAGESLENGTH, MSG_Q_FIFO);

	// Wait for messages and execute FSM
	FOREVER {
		message message;
		
		// Clean the 
		// Wait a message from the Queue ... FOREVER
		msgQReceive(msgQCtrlId, (char *  ) &message, sizeof(message), WAIT_FOREVER);
		
		// CONFIGRESET e CONFIGSET messages processed right here (Init can send them at any time), other passed to the current FSM
		switch (message.header.type) {
			// CONFIG RESET message
			case IMSGTYPE_NODECONFIGRESET:
				// TODO
				// -reelase resource of/or FSM
				// - put in fail-safe status
				
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
				nodeState.pRouteList = message.ctrlIConfigSet.pRoute;
				nodeState.numRoutes = message.ctrlIConfigSet.numRoutes;				
				nodeState.nodeType = message.ctrlIConfigSet.nodeType;
				
				// Set FSM function pointers and initialize it
				if (nodeState.nodeType == NODETYPE_TC) {
					// Track Circuit Node
					FSMCtrl = FSMCtrlTC;
					FSMNewMessage = FSMCtrlTCEvent_NewMessage;
					FSMTimeout = FSMCtrlTCEvent_TimerExpired;
					syslog(LOG_INFO, "Node configured for Track Circuit logic");
				} else {
					// Switch Node
					FSMCtrl = FSMCtrlSW;
					FSMNewMessage = FSMCtrlSWEvent_NewMessage;
					FSMTimeout = FSMCtrlSWEvent_TimerExpired;
					syslog(LOG_INFO, "Node configured for Switch logic");
				}
						
				// FSM Initialize function passing NodeState pointer
				FSMCtrl(&nodeState);
					
				break;
				
			// Other messages passed to the FSM (if configured)
			default:
				// If Event handler confgired, notify the message
				if (FSMNewMessage)
					// Notify the new message to the FSM
					FSMNewMessage(&message);
				else
					// Log and error and ignore the message
					syslog(LOG_ERR, "Node not configured: message discarted");

				break;
		}		
		
	}
}

/**
 * Shutdown function
 */
 void Shutdown() {
	 msgQDelete(msgQCtrlId);
	 
	 msgQCtrlId = NULL;
 }

