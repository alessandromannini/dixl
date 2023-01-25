/*
 * config.h
 * 
 * Configuration
 *
 *  Created on: Jan 10, 2023
 *  Author: Alessandro Mannini <alessandro.mannini@gmail.com>
 */
/* includes */

#ifndef CONFIG_H_
#define CONFIG_H_


/***************************************************
 *  tasks parameters
 ***************************************************/		
/* Task dixlInit */
#define TASKINITNAME  			"tDixlInit"		/* Task Init name */
#define	TASKINITPRIO 			100				/* Task Init prio */
#define	TASKINITSTACKSIZE 		20480			/* Task Init Stack Size */

/* Task dixlConnRx */
#define TASKCOMMRXNAME  		"tDixlConnRx"	/* Task ConnRx name */
#define	TASKCOMMRXPRIO 			100				/* Task ConnRx prio */
#define	TASKCOMMRXSTACKSIZE 	20480			/* Task ConnRx stack Size */

/* Task dixlConnTx */
#define TASKCOMMTXNAME  		"tDixlConnTx"	/* Task ConnTx name */
#define	TASKCOMMTXPRIO 			100				/* Task ConnTx prio */
#define	TASKCOMMTXSTACKSIZE 	20480			/* Task ConnTx stack Size */

/* Task dixlLog */
#define TASKLOGNAME  			"tDixlLog"	    /* Task ConnTx name */
#define	TASKLOGPRIO 			100				/* Task ConnTx prio */
#define	TASKLOGSTACKSIZE 		20480			/* Task ConnTx stack Size */

/* Task dixlCtrl */
#define TASKCTRLNAME  			"tDixlCtrl"		/* Task Ctrl name */
#define	TASKCTRLPRIO 			100				/* Task Ctrl prio */
#define	TASKCTRLSTACKSIZE 		20480			/* Task Ctrl stack Size */

/* Task dixlDiag */
#define TASKDIAGNAME  			"tDixlDiag"		/* Task Diag name */
#define	TASKDIAGPRIO 			100				/* Task Diag prio */
#define	TASKDIAGSTACKSIZE 		20480			/* Task Diag stack Size */

/* Task dixlSWITCH */
#define TASKSWITCHNAME 			"tDixlSwitch"	/* Task Switch name */
#define	TASKSWITCHPRIO 			100				/* Task Switch prio */
#define	TASKSWITCHSTACKSIZE		20480			/* Task Switch stack Size */

/***************************************************
 *  Messages queues specifications
 ***************************************************/
/* dxilInit task IN Queue */
#define MSGQINITMESSAGESMAX  		1024	/* Max number of the messages accepted */
#define MSGQINITMESSAGESLENGTH 		100		/* Max length of the messages */

/* dxilCommRx task IN Queue */
// TODO #define MSGQCOMMRXESSAGESMAX  		1024	/* Max number of the messages accepted */
// TODO #define MSGQCOMMRXESSAGESLENGTH 	100		/* Max length of the messages */

/* dxilCommTx task IN Queue */
#define MSGQCOMMTXMESSAGESMAX  		1024	/* Max number of the messages accepted */
#define MSGQCOMMTXMESSAGESLENGTH 	100		/* Max length of the messages */

/* dxilCtrl task IN Queue */
#define MSGQCTRLMESSAGESMAX  		1024	/* Max number of the messages accepted */
#define MSGQCTRLMESSAGESLENGTH 		100		/* Max length of the messages */

/* dxilLog task IN Queue */
#define MSGQLOGMESSAGESMAX  		1024	/* Max number of the messages accepted */
#define MSGQLOGMESSAGESLENGTH 		100		/* Max length of the messages */
		
/* dxilDiag task IN Queue */
#define MSGQDIAGMESSAGESMAX  		1024	/* Max number of the messages accepted */
#define MSGQDIAGMESSAGESLENGTH 		100		/* Max length of the messages */
		
/* TODO serve? dxilSwitch task IN Queue */
#define MSGQSWITCHMESSAGESMAX  		1024	/* Max number of the messages accepted */
#define MSGQSWITCHMESSAGESLENGTH 	100		/* Max length of the messages */
		
#endif /* CONFIG_H_ */
