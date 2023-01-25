
/*
 * globals.h
 * 
 * Globals variable and constants
 *
 *  Created on: Jan 10, 2023
 *  Author: Alessandro Mannini <alessandro.mannini@gmail.com>
 *
 */
/* includes */
#include <msgQLib.h>

#ifndef GLOBALS_H_
#define GLOBALS_H_


/***************************************************
 *  Return codes
 ***************************************************/
enum returnCode {
	rcOK					= 0,		// OK, no errors
	rcINQUEUE_INITERR		= 101,		// IN queue initialization error
};
typedef enum returnCode returnCode;


/***************************************************
 *  Task IDs
 ***************************************************/
/* task Ids */
extern  TASK_ID     taskStartId;		// Start task ID
extern  char       *taskStartName;		// Start task Name 
extern 	TASK_ID 	taskInitId;			// Init task ID
extern 	TASK_ID 	taskCommRxId;		// Comm Rx task ID
extern 	TASK_ID 	taskCommTxId;		// Comm Tx task ID
extern 	TASK_ID 	taskCtrlId;			// Ctrl task ID
extern 	TASK_ID 	taskLogId;			// Log  task ID
extern 	TASK_ID 	taskDiagId;			// Diag  task ID
extern 	TASK_ID 	taskSwitchId;		// Switch  task ID

/***************************************************
 *  Messages queues
 ***************************************************/
/* IN queues Ids */
extern 	MSG_Q_ID 	msgQInitId;			// Init task IN queue Id
extern 	MSG_Q_ID 	msgQCommTxId;		// Comm Tx task IN Queue Id
extern 	MSG_Q_ID 	msgQCtrlId;			// Ctrl task IN Queue Id
extern 	MSG_Q_ID 	msgQLogId;			// Log  task IN Queue Id

		
#endif /* GLOBALS_H_ */
 
