/**
 * globals.h
 * 
 * Globals variable and constants
 *
 * @author: Alessandro Mannini <alessandro.mannini@gmail.com>
 * @date: Jan 10, 2023
 */

/* includes */

#ifndef GLOBALS_H_
#define GLOBALS_H_
#include <msgQLib.h>

#include "datatypes/dataTypes.h"
/**
 * Node parameters
 */
extern MACAddress MAC;				// MAC address of the node
extern IPv4Address IPv4;			// IPv4 of the node
extern IPv4String IPv4s;			// IPv4s (string) of the node
extern nodeId NodeNULL;				// NULL node
extern char *CPUName;				// CPU board name
extern bool isVxSim;				// VxSim simulator on?

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
extern 	TASK_ID 	taskLogId;			// Log task ID
extern 	TASK_ID 	taskDiagId;			// Diag task ID
extern 	TASK_ID 	taskPointId;		// Point task ID
extern 	TASK_ID 	taskSensorId;		// Sensor task ID

/***************************************************
 *  Messages queues
 ***************************************************/
/* IN queues Ids */
extern 	MSG_Q_ID 	msgQInitId;			// Init task IN queue Id
extern 	MSG_Q_ID 	msgQDiagId;			// Diag task IN queue Id
extern 	MSG_Q_ID 	msgQCommTxId;		// Comm Tx task IN Queue Id
extern 	MSG_Q_ID 	msgQCtrlId;			// Ctrl task IN Queue Id
extern 	MSG_Q_ID 	msgQLogId;			// Log  task IN Queue Id	
extern 	MSG_Q_ID 	msgQPointId;    	// Point task IN Queue Id	
extern 	MSG_Q_ID 	msgQSensorId;    	// Sensor task IN Queue Id	

/***************************************************
 *  Sockets
 ***************************************************/
extern 	int         dixlCommRxSocket;	// Comm Rx task IN Socket

/***************************************************
 *  Semaphores
 ***************************************************/
extern SEM_ID semPosition;							// Semaphore to access position
extern SEM_ID semSensor;							// Semaphore to access sensor
extern SEM_ID semDiag;							    // Semaphore to access internal variables

#endif /* GLOBALS_H_ */
 
