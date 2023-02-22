/**
 * datatypes.h
 * 
 * Datatypes definitions
 *
 * @author: Alessandro Mannini <alessandro.mannini@gmail.com>
 * @date: Jan 10, 2023
 */


#ifndef DATATYPES_H_
#define DATATYPES_H_

/* includes */

/***************************************************
 *  Return codes
 ***************************************************/
enum returnCode {
	rcOK					= 0,		// OK, no errors
	rcINQUEUE_INITERR		= 101,		// IN QUEUE initialization error
	rcINQUEUE_RECEIVEERR	= 102,		// IN QUEUE receive error
	rcINQUEUE_SENDERR		= 103,		// IN QUEUE send error
	
	rcSOCKET_INITERR		= 201,		// SOCKET initialization error
	rcSOCKET_BINDERR		= 202,		// SOCKET binding error
	rcSOCKET_LISTENERR		= 203,		// SOCKET listening error
	rcSOCKET_ACCEPTERR		= 204,		// SOCKET accept error
	rcSOCKET_RECEIVEERR		= 205,		// SOCKET receive error
	rcSOCKET_SENDERR		= 206,		// SOCKET send error
	
	rcNETWORK_GETIFADDRSERR = 301,		// NETWORK errore getting if addresses
	
	rcFSM_WRONGSTATE		= 400,		// FSM in a wrong state

};
typedef enum returnCode returnCode;



/***************************************************
 *  Node data types and parameters
 ***************************************************/
/* Node type */
typedef enum {
	NODETYPE_POINT 				= 10,	// Point (Deviatoio)
	NODETYPE_TRACKCIRCUIT 		= 20	// Track Circuit (Cdb o Circuito di Binario)
} eNodeType;

/* Node position */
typedef enum {
	NODEPOS_FIRST 				= -128,	// First node of the route
	NODEPOS_MIDDLE				= 0,  	// Node between two other nodes of the route
	NODEPOS_LAST				= 127,  // Last node of the route
} eNodePosition;

/* Point position */
typedef enum {
	POINTPOS_UNDEFINED			= -1,	// Undefined / Fault position
	POINTPOS_STRAIGHT 			= 0,	// Stright direction
	POINTPOS_DIVERGING			= 50 	// Diverging direction
} ePointPosition;

/**
 *  Data types
 */
typedef uint32_t routeId;			// ID of the route/itinerary
typedef struct MACAddress {
	unsigned char bytes[6];			// MAC Address
} MACAddress;
typedef struct IPv4Address {		// IPv4 packet	
	unsigned char bytes[4];			
} IPv4Address;
typedef char IPv4String[17];		// IPv4 string (16 characters + \0)
typedef IPv4Address nodeId;			// Node ID is the IP (packed)

// Route
typedef struct route {
	routeId id;						// ID of the route
	nodeId prev;					// Previous node in the route (can be the host)
	nodeId next;					// Next node in the route (can be NULL)
	int8_t position;				// Node position in the route
	uint8_t requestedPosition;		// Requested position (if type is Point)
	uint8_t padding[2];				// Padding to 32bit
} route;

typedef struct NodeState {
	uint8_t nodeType;				// Type of the node ( => behaviour)
	uint32_t numRoutes;				// Total number (N) of segments in the configuration
	route *pRouteList;				// Array of route in the configuration received
	route *pCurrentRoute;			// Current requested route
} NodeState;

#endif /* DATATYPES_H_ */
 
