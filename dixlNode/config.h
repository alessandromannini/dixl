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

/**
 *  tasks parameters
 *		
 */ 
/* Task dixlInit */
#define TASKINITNAME  			"tDixlInit"			/* Task Init name */
#define TASKINITDESC  			"Initialization"	/* Task Init description */
#define	TASKINITPRIO 			100					/* Task Init prio */
#define	TASKINITSTACKSIZE 		20480				/* Task Init Stack Size */

/* Task dixlConnRx */
#define TASKCOMMRXNAME  		"tDixlConnRx"		/* Task ConnRx name */
#define TASKCOMMRXDESC  		"Communication RX"	/* Task ConnRx description */
#define	TASKCOMMRXPRIO 			100					/* Task ConnRx prio */
#define	TASKCOMMRXSTACKSIZE 	20480				/* Task ConnRx stack Size */

/* Task dixlConnTx */
#define TASKCOMMTXNAME  		"tDixlConnTx"		/* Task ConnTx name */
#define TASKCOMMTXDESC  		"Communication TX"	/* Task ConnTx description */
#define	TASKCOMMTXPRIO 			100					/* Task ConnTx prio */
#define	TASKCOMMTXSTACKSIZE 	20480				/* Task ConnTx stack Size */

/* Task dixlLog */
#define TASKLOGNAME  			"tDixlLog"	    	/* Task Logger name */
#define TASKLOGDESC  			"Logger"			/* Task Logger description */
#define	TASKLOGPRIO 			100					/* Task Logger prio */
#define	TASKLOGSTACKSIZE 		20480				/* Task Logger stack Size */

/* Task dixlCtrl */
#define TASKCTRLNAME  			"tDixlCtrl"			/* Task Ctrl name */
#define TASKCTRLDESC  			"Control Logic"		/* Task Ctrl description */
#define	TASKCTRLPRIO 			100					/* Task Ctrl prio */
#define	TASKCTRLSTACKSIZE 		20480				/* Task Ctrl stack Size */

/* Task dixlDiag */
#define TASKDIAGNAME  			"tDixlDiag"			/* Task Diag name */
#define TASKDIAGDESC  			"Diagnostic"		/* Task Diag description */
#define	TASKDIAGPRIO 			100					/* Task Diag prio */
#define	TASKDIAGSTACKSIZE 		20480				/* Task Diag stack Size */

/* Task dixlSWITCH */
#define TASKSWITCHNAME 			"tDixlSwitch"		/* Task Switch name */
#define TASKSWITCHDESC  		"Switch Simulator"	/* Task Switch  description */
#define	TASKSWITCHPRIO 			100					/* Task Switch prio */
#define	TASKSWITCHSTACKSIZE		20480				/* Task Switch stack Size */

/**
 *  Messages queues specifications
 *
 */
/* dxilInit task IN Queue */
#define MSGQINITMESSAGESMAX  		1024			/* Max number of the messages accepted */
#define MSGQINITMESSAGESLENGTH 		MSG_MAXLENGTH	/* Max length of the messages */

/* dxilCommRx task IN Queue */
// TODO #define MSGQCOMMRXESSAGESMAX  		1024			/* Max number of the messages accepted */
// TODO #define MSGQCOMMRXESSAGESLENGTH 	MSG_MAXLENGTH	/* Max length of the messages */

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
		
/* TODO serve? dxilSwitch task IN Queue */
#define MSGQSWITCHMESSAGESMAX  		1024					/* Max number of the messages accepted */
#define MSGQSWITCHMESSAGESLENGTH 	MSG_MAXLENGTH			/* Max length of the messages */
		

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
