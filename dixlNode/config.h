/**
 * config.h
 * 
 * Configuration
 *
 * @author: Alessandro Mannini <alessandro.mannini@gmail.com>
 * @date: Jan 10, 2023
 */

/* includes */
#include <sockLib.h>
#include <netinet/in.h>

#ifndef CONFIG_H_
#define CONFIG_H_

#define C

/**
 *  tasks parameters
 *		
 */ 
/* Task dixlInit */
#define TASKINITNAME  			"tDixlInit"			/* Task Init name */
#define TASKINITDESC  			"Initialization"	/* Task Init description */
#define	TASKINITPRIO 			80					/* Task Init prio */
#define	TASKINITSTACKSIZE 		20480				/* Task Init Stack Size */

/* Task dixlConnRx */
#define TASKCOMMRXNAME  		"tDixlConnRx"		/* Task ConnRx name */
#define TASKCOMMRXDESC  		"Communication RX"	/* Task ConnRx description */
#define	TASKCOMMRXPRIO 			90					/* Task ConnRx prio */
#define	TASKCOMMRXSTACKSIZE 	20480				/* Task ConnRx stack Size */

/* Task dixlConnTx */
#define TASKCOMMTXNAME  		"tDixlConnTx"		/* Task ConnTx name */
#define TASKCOMMTXDESC  		"Communication TX"	/* Task ConnTx description */
#define	TASKCOMMTXPRIO 			90					/* Task ConnTx prio */
#define	TASKCOMMTXSTACKSIZE 	20480				/* Task ConnTx stack Size */

/* Task dixlLog */
#define TASKLOGNAME  			"tDixlLog"	    	/* Task Logger name */
#define TASKLOGDESC  			"Logger"			/* Task Logger description */
#define	TASKLOGPRIO 			95					/* Task Logger prio */
#define	TASKLOGSTACKSIZE 		20480				/* Task Logger stack Size */
#define TASKLOGMAXLINES			1024				/* Task Logger: maximum lines that can be stored */

/* Task dixlCtrl */
#define TASKCTRLNAME  			"tDixlCtrl"			/* Task Ctrl name */
#define TASKCTRLDESC  			"Control Logic"		/* Task Ctrl description */
#define	TASKCTRLPRIO 			80					/* Task Ctrl prio */
#define	TASKCTRLSTACKSIZE 		20480				/* Task Ctrl stack Size */

/* Task dixlDiag */
#define TASKDIAGNAME  			"tDixlDiag"			/* Task Diag name */
#define TASKDIAGDESC  			"Diagnostic"		/* Task Diag description */
#define	TASKDIAGPRIO 			VX_TASK_PRIORITY_MAX/* Task Diag prio */
#define	TASKDIAGSTACKSIZE 		20480				/* Task Diag stack Size */
#define	TASKDIAGCHECKPERIOD		00000				/* Task Diag check period (ms), 0 = continuous */
#define	TASKDIAGPINGPKTS		3					/* Task Diag packets to send for each ping */

#define TASKDIAGWKRNAME  		"tDixlDiagWkr"			/* Task Diag name */
#define TASKDIAGWKRDESC  		"Diagnostic Worker"		/* Task Diag description */
#define	TASKDIAGWKRPRIO 		VX_TASK_PRIORITY_MAX -5 /* Task Diag prio */
#define	TASKDIAGWKRSTACKSIZE 	20480					/* Task Diag stack Size */

/* Task dixlPOINT */
#define TASKPOINTNAME 			"tDixlPoint"		/* Task Point name */
#define TASKPOINTDESC  			"Point Simulator"	/* Task Point description */
#define	TASKPOINTPRIO 			86					/* Task Point prio */
#define	TASKPOINTSTACKSIZE		20480				/* Task Point stack Size */
#define TASKPOINTTRANSTIME      3000                /* Task Point time (ms) to switch between straight and diverge position */

#define TASKPOINTWKRNAME 		"tDixlPointWkr"				/* Task Point worker name */
#define TASKPOINTWKRDESC  		"Point Simulator Worker"	/* Task Point worker description */
#define	TASKPOINTWKRPRIO 		85							/* Task Point worker prio */
#define	TASKPOINTWKRSTACKSIZE	20480						/* Task Point worker stack Size */

/* Task dixlSENSOR */
#define TASKSENSORNAME 			"tDixlSensor"		/* Task Sensor name */
#define TASKSENSORDESC  		"Sensor Checker"	/* Task Sensor description */
#define	TASKSENSORPRIO 			86					/* Task Sensor prio */
#define	TASKSENSORSTACKSIZE		20480				/* Task Sensor stack Size */
#define TASKSENSORCHECKPERIOD   1000                /* Task Sensor check period (ms) */

#define TASKSENSORWKRNAME 		"tDixlSensorWkr"			/* Task Sensor worker name */
#define TASKSENSORWKRDESC  		"Sensor Checker Worker"		/* Task Sensor worker description */
#define	TASKSENSORWKRPRIO 		85							/* Task Sensor worker prio */
#define	TASKSENSORWKRSTACKSIZE	20480						/* Task Sensor worker stack Size */

/**
 *  Messages queues specifications
 *
 */
/* dxilInit task IN Queue */
#define MSGQINITMESSAGESMAX  		1024			/* Max number of the messages accepted */
#define MSGQINITMESSAGESLENGTH 		MSG_MAXLENGTH	/* Max length of the messages */

/* dxilCommTx task IN Queue */
#define MSGQCOMMTXMESSAGESMAX  		1024					/* Max number of the messages accepted */
#define MSGQCOMMTXMESSAGESLENGTH 	MSG_MAXLENGTH			/* Max length of the messages */

/* dxilCtrl task IN Queue */
#define MSGQCTRLMESSAGESMAX  		1024					/* Max number of the messages accepted */
#define MSGQCTRLMESSAGESLENGTH 		MSG_MAXLENGTH			/* Max length of the messages */

/* dxilLog task IN Queue */
#define MSGQLOGMESSAGESMAX  		1024					/* Max number of the messages accepted */
#define MSGQLOGMESSAGESLENGTH 		MSG_MAXLENGTH			/* Max length of the messages */
		
/* dxilDiag task IN Queue */
#define MSGQDIAGMESSAGESMAX  		1024					/* Max number of the messages accepted */
#define MSGQDIAGMESSAGESLENGTH 		MSG_MAXLENGTH			/* Max length of the messages */
		
/* dxilPoint task IN Queue */
#define MSGQPOINTMESSAGESMAX  		1024					/* Max number of the messages accepted */
#define MSGQPOINTMESSAGESLENGTH 	MSG_MAXLENGTH			/* Max length of the messages */
		
/* dxilSensor task IN Queue */
#define MSGQSENSORMESSAGESMAX  		1024					/* Max number of the messages accepted */
#define MSGQSENSORMESSAGESLENGTH 	MSG_MAXLENGTH			/* Max length of the messages */
		
/**
 * GPIO PINs
 */
#define GPIO_PIN_LED				17						/* GPIO PIN number for led */
#define GPIO_PIN_BUTTON				27						/* GPIO PIN number for button */

/**
 * Communication parameters
 */
#define COMMSOCKDOMAIN      		AF_INET					/* IPv4 (use AF_INET6 for IPv6 */
#define COMMSOCKTYPE 				SOCK_STREAM				/* Connection-based (use SOCK-DGRAM for datagram) */
#define COMMSOCKPROTOCOL    		IPPROTO_TCP				/* TCP  /use IPPROTO_UDP for UDP */
#define COMMSOCKPORT        		256		        		/* port, IANA unassigned */
#define COMMBUFFERSIZE		        2 * MSG_MAXLENGTH		/* Comm buffer size to receive messages */

/**
 * Configurations parameters
 */
#define CONFIGMAXROUTES      		256						/* Max number of routes in node config */

#endif /* CONFIG_H_ */
