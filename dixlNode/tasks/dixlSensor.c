/**
 * dxilSensor.c
 * 
 * Sensor simulator task
 *
 * @author: Alessandro Mannini <alessandro.mannini@gmail.com>
 * @date: Jan 10, 2023
 */

/* includes */
#include <stdlib.h>
#include <stdbool.h>

#include <msgQLib.h>
#include <taskLib.h>
#include <sysLib.h>
#include <syslog.h>

#include "../config.h"
#include "../datatypes/messages.h"
#include "../globals.h"
#include "dixlSensor.h"

#include "../includes/hw.h"
#include "../includes/utils.h"
#include "dixlComm.h"

/* variables */
// Task
TASK_ID     taskSensorId;

// Input message queue
MSG_Q_ID 	msgQSensorId;

// Semaphores
SEM_ID semSensor;							// Semaphore to access sensor

// Sensor State
static eSensorState currentState = -1;
static eSensorState requestedState = SENSORSTATE_ON;
static struct timespec requestNonce;				// Nonce (timestamp) of the request (if 0 notification isn't sent)
static _Vx_freq_t periodTick;

/* Implementation functions */
static void initialize() {
	_Vx_freq_t tickRate = sysClkRateGet();
	
	// Copute floor approxmation of tick per step
	periodTick = math_ceil((TASKSENSORCHECKPERIOD * tickRate) , 1000);
	
	// Estimate real duration (due to tick resolution)
	double realPeriodTime = 1000 * periodTick  / tickRate;
	double increment = (realPeriodTime - TASKSENSORCHECKPERIOD ) * 100 / TASKSENSORCHECKPERIOD;
	
	syslog(LOG_INFO, "Sensor check specs:");
	syslog(LOG_INFO, "> Requested check period   : %ims", TASKSENSORCHECKPERIOD);
	syslog(LOG_INFO, "> Tick rate (clock)        : %iHz", tickRate);
	syslog(LOG_INFO, "> Tick per period          : %i", periodTick);
	syslog(LOG_INFO, "> Real excepted period time: %.0fms (+%0.f%)", realPeriodTime, increment);	
}

// Process a single message received
static void process_message(message message) {
	
	switch (message.header.type) {
		// Positioning request
		case IMSGTYPE_SENSORSTATE:
			requestedState = message.sensorIPOS.requestedState;
			requestNonce = message.sensorIPOS.requestTimestamp;
			
			
			// Log
			syslog(LOG_INFO, "Waiting for %s state with nonce %i", sensorStateStr(requestedState), requestNonce);				
			break;
			
		// Other messages discarded
		default:
			syslog(LOG_ERR, "Unattended message type (%d). Should not be send to Sensor task and will be ignored", message.iHeader.type);
			break;
	}
}

static int worker() {
	// Take sem
	semTake(semSensor, WAIT_FOREVER);		
	
	// If present receive a message
	if (msgQNumMsgs(msgQSensorId)) {
		// Receive the messsage
		message message;
		msgQReceive(msgQSensorId, (char *  ) &message, sizeof(message), WAIT_FOREVER);
		
		// Process the message
		process_message(message);
	}
	// If VxSim compile Button emulate mode (only when needed)
#if CPU ==_VX_SIMNT
	if (currentState != requestedState) {
		syslog(LOG_INFO,"Simulation mode: emulating SENSOR %s in 5 seconds", sensorStateStr(requestedState));
		taskDelay(sysClkRateGet() * 5);
		// Log occupied if it wasn't
		if (currentState != SENSORSTATE_ON) logger_log(LOGTYPE_OCCUPIED, NULL, NodeNULL );
		currentState = requestedState;
	}
#else
	// Read sensor state from GPIO
	pinMode(GPIO_PIN_BUTTON, IN);
	if (pinGet(GPIO_PIN_BUTTON) == GPIO_VALUE_LOW) {
		// Log occupied if it wasn't
		if (currentState != SENSORSTATE_ON) logger_log(LOGTYPE_OCCUPIED, NULL, NodeNULL );
		currentState = SENSORSTATE_ON;		
	} else
		currentState = SENSORSTATE_OFF;
#endif

	// Log	
	if (requestNonce.tv_sec)
		syslog(LOG_INFO, "Read state %s from GPIO", sensorStateStr(currentState));

	// Give sem to caller
	semGive(semSensor);
	
	// Self delete task
	taskDelete(taskIdSelf());
	
	return 0;
}

void dixlSensor() {
	
	// Start
	syslog(LOG_INFO, "Task started Id 0x%jx", taskSensorId);	

	// Initialize step variables
	initialize();
	
	// Create semaphore
	semSensor = semMCreate(SEM_Q_FIFO);	
	
	// Message queue initialization
	msgQSensorId = msgQ_Initialize(MSGQSENSORMESSAGESMAX, MSGQSENSORMESSAGESLENGTH, MSG_Q_FIFO);
	
	// Take sem
	semTake(semSensor, WAIT_FOREVER);			
	
	// Spawn the worker ... FOREVER
	FOREVER {
		
		// Give sem to worker
		semGive(semSensor);			
		
		// Spawn a periodic worker to ready sensor state
		taskSpawn(TASKSENSORWKRNAME, TASKSENSORWKRPRIO, 0, TASKSENSORWKRSTACKSIZE, (FUNCPTR) worker, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

		// Sleep for a period (number of tick between checks)
		taskDelay(periodTick);
		
		// Take sem
		semTake(semSensor, WAIT_FOREVER);					
		
		// IF a Nonce request is active
		if (requestNonce.tv_sec || requestNonce.tv_nsec) {			
			// Prepare the notification message
			message outMessage;
			outMessage.iHeader.type = IMSGTYPE_SENSORNOTIFY;
			outMessage.sensorINOTIFY.currentState = currentState;
			outMessage.sensorINOTIFY.requestTimestamp = requestNonce;
	
			// Notify to task Ctrl
			msgQ_Send(msgQCtrlId, (char *) &outMessage, sizeof(msgIHeader) + sizeof(msgISensorNOTIFY));					
				
			// Reset request nonce
			requestNonce.tv_sec = 0;
			requestNonce.tv_nsec =0;			

			// Log
			syslog(LOG_INFO, "State %s reached", sensorStateStr(requestedState));	
		}
	}		
}

