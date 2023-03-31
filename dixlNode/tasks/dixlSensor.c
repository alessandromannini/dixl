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
#include "../hw.h"
#include "../utils.h"
#include "dixlSensor.h"
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
			
			// Log
			syslog(LOG_INFO, "Waiting for %s state", sensorStateStr(requestedState));				
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
	// If is VxSim wait 5 seconds and return requested state
	if (isVxSim) {
		syslog(LOG_INFO,"Simulation is active: emulating SENSOR %s in 5 seconds",sensorStateStr(requestedState));
		taskDelay(sysClkRateGet() * 5);
		currentState = requestedState;
	} else {
		// Read sensor state from GPIO
		pinMode(GPIO_PIN_BUTTON, IN);
		if (pinGet(GPIO_PIN_BUTTON) == GPIO_VALUE_LOW) 
			currentState = SENSORSTATE_ON;
		else
			currentState = SENSORSTATE_OFF;
	}
	
	// Log
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

	// Wait for messages of positioning
	FOREVER {
		
		// Wait an activation message ... FOREVER
		message inMessage;
		msgQReceive(msgQSensorId, (char *  ) &inMessage, sizeof(inMessage), WAIT_FOREVER);

		// Take sem
		semTake(semSensor, WAIT_FOREVER);		
		
		// Process the message
		process_message(inMessage);
		
		// Need to run ?
		BOOL runned = FALSE;
		
		// Process spawning a worker while position!=requestedPosition AND requestedPosition!=UNDEFINED (Malfunction) OR new messages		
		while (currentState != requestedState) {
			// Moved ate least one time
			runned = TRUE;
			
			// Give sem to worker
			semGive(semSensor);			
			
			// Spawn a periodic worker
			taskSpawn(TASKSENSORWKRNAME, TASKSENSORWKRPRIO, 0, TASKSENSORWKRSTACKSIZE, (FUNCPTR) worker, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

			// Sleep for a period (number of tick between motor step)
			taskDelay(periodTick);
			
			// Take sem
			semTake(semSensor, WAIT_FOREVER);					
		}

		// Final give
		semGive(semSensor);
		
		// Prepare the notification message
		message outMessage;
		outMessage.iHeader.type = IMSGTYPE_SENSORNOTIFY;
		outMessage.sensorINOTIFY.currentState = currentState;

		// Notify to task Ctrl
		msgQ_Send(msgQCtrlId, (char *) &outMessage, sizeof(msgIHeader) + sizeof(msgISensorNOTIFY));					
			
		// Log
		if (runned)
			syslog(LOG_INFO, "State %s reached", sensorStateStr(requestedState));	
		else
			syslog(LOG_INFO, "Already in %s state", sensorStateStr(requestedState));
	}		
}

