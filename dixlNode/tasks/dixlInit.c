/**
 * dxilInit.h
 * 
 * Initilization task
 *
 * @author: Alessandro Mannini <alessandro.mannini@gmail.com>
 * @date: Jan 10, 2023
 */

/* includes */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "net/if.h"

#include <taskLib.h>
#include <msgQLib.h>
#include <sysLib.h>
#include <syslog.h>

#include "../globals.h"
#include "../config.h"
#include "../utils.h"
#include "../network.h"
#include "dixlComm.h"
#include "../FSM/FSMInit.h"

/* variables */
// Node IDs
char ifName[IFNAMSIZ] = "";							// Network interface
MACAddress  MAC = {00, 00, 00, 00, 00, 00};		// MAC Address
IPv4Address IPv4 = {000, 000, 000, 000};		// IPv4 Address
IPv4String IPv4s;

// Task
TASK_ID     taskInitId = 0;

// Input message queue
MSG_Q_ID 	msgQInitId;

/** 
 * Get node informations
 */
void getNodeInformations(){

	// Get Node informations
	network_get_if_params(ifName, &IPv4, &MAC);
	network_IPv4_to_str(&IPv4, IPv4s);	
}

/** 
 * Print welcome message and Node ID to the logger
 */
void welcomeBanner() {
	syslog(LOG_INFO, "");
	syslog(LOG_INFO, "******************************************************");
	syslog(LOG_INFO, "*     dixlNode - Distributed InterLocking system     *");
	syslog(LOG_INFO, "*     ------------------------------------------     *");
	syslog(LOG_INFO, "* author       :  Alessandro Mannini                 *");
	syslog(LOG_INFO, "* organization :  Universita' degli Studi di Firenze *");
	syslog(LOG_INFO, "* date         :  Jan 10, 2023                       *");
	syslog(LOG_INFO, "* contact      :  alessandro.mannini@gmail.com       *");
	syslog(LOG_INFO, "******************************************************");
	
	syslog(LOG_INFO, "> Node informations");
	syslog(LOG_INFO, "> -----------------------------------------------");
	syslog(LOG_INFO, "> Clock tick rate (hz) : %d", sysClkRateGet());	
	syslog(LOG_INFO, "> Interface            : %s", ifName);
	syslog(LOG_INFO, "> MAC                  : %02x:%02x:%02x:%02x:%02x:%02x", MAC.bytes[0], MAC.bytes[1], MAC.bytes[2], MAC.bytes[3], MAC.bytes[4], MAC.bytes[5]);
	syslog(LOG_INFO, "> IP address           : %03d.%03d.%03d.%03d", IPv4.bytes[0], IPv4.bytes[1], IPv4.bytes[2], IPv4.bytes[3]);
	syslog(LOG_INFO, "> -----------------------------------------------");
}

/*
 *  Initialization task
 */
void dixlInit() {
	
	// Delay until start task closes
	task_wait4notReady(taskStartId, 1, 1);
	
	// Get node informations
	getNodeInformations();
		
	// Print welcome banner
	welcomeBanner();	
	
	// TODO Verifica already running
	syslog(LOG_DEBUG, "Task taskStartId 0x%jx", taskStartId);
		
	// Start
	taskInitId = taskIdSelf();
	syslog(LOG_INFO, "Task started Id 0x%jx", taskInitId);	

	// Message queue initialization
	msgQInitId = msgQ_Initialize(MSGQINITMESSAGESMAX, MSGQINITMESSAGESLENGTH, MSG_Q_FIFO);
	
	// FSM Initialize
	FSMInit();
	
	// Wait for messages and execute FSM
	FOREVER {
		// Wait a message ... FOREVER
		message message;
		msgQReceive(msgQInitId, (char *  ) &message, sizeof(message), WAIT_FOREVER);
		
		// Notify the new message to the FSM
		FSMInitEvent_NewMessage(&message);
	}
}
