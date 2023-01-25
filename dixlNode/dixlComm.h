/*
 * dxilComm.h
 *
 *  Created on: Jan 10, 2023
 *  Author: Alessandro Mannini <alessandro.mannini@gmail.com>
 */

#ifndef DXILCOMM_H_
#define DXILCOMM_H_

/* enum */
/* Message type */
enum msgType {
	
	// Route messages - Ctrl task
	MSG_ROUTEREQ,			// Route request
	MSG_ROUTEACK,			// 2PhaseCommit Ack
	MSG_ROUTEAGREE,			// 2PhaseCommit Agree
	MSG_ROUTEDISAGREE,		// 2PhaseCommit Disagree
	MSG_ROUTECOMMIT,		// 2PhaseCommit Commit
	
	// Service messages - Init task
	MSG_NODERESET,			// Reset in the Init state
	MSG_NODECONFIG,			// Routes configuration sent by the host
	MSG_NODEDISCOVERY,		// Nodes discovery from the host
	MSG_NODEADVERTISE,		// Node advertise reply to discovery
	
	// Diagnostic messages - TODO
	MSG_NODEDIAG,			// Communication diagnostic request
	MSG_NODEDIAGACK,		// Communication diagnostic request Ack
	
	// Log messages - TODO
	MSG_LOG,				// Log a message
	MSG_LOGREQ,				// Request current log messages
	MSG_LOGSEND,			// Response current log messages
	MSG_LOGDEL,				// Ack messages were received and ask to delete
	MSG_LOGDELACK			// Ack messages were deleted
};
typedef enum msgType msgType;

/* data types */
typedef int itineraryId;	// ID of the itinerary
typedef char MACAddress[16];
// TODO

struct msgBase {
	msgType type;
};
struct msgCtrl {
	msgType type;
	itineraryId id;
	//source
	
};
struct message {
	msgType type;
	union {
		struct msgBase base;
		struct msgCtrl ctrl;
	};
};

typedef struct message msg;

/*
 * Comm Rx task function
 */
void dixlCommRx();

/*
 * Comm Tx task function
 */
void dixlCommTx();

#endif /* DXILCOMM_H_ */
