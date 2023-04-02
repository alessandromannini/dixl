/**
 * dataHelper.c
 * 
 * Datatypes helper functions
 *
 * @author: Alessandro Mannini <alessandro.mannini@gmail.com>
 * @date: Jan 10, 2023
 */
#include "dataTypes.h"

const char *pointPosStr(ePointPosition pos) {
	switch (pos) {
	case POINTPOS_DIVERGING:
		return "diverging";
		
	case POINTPOS_STRAIGHT:
		return "straight";
	default:
		return "undefined";
	}	
}

const char *sensorStateStr(eSensorState state) {
	switch (state) {
		case SENSORSTATE_ON:
			return "ON";
			
		case SENSORSTATE_OFF:
			return "OFF";
			
		default:
			return "UNDEFINED";
	}	
}

bool nodeIsNull(nodeId node) {
	return (!(node.bytes[0] && node.bytes[1] && node.bytes[2] && node.bytes[3]));
}
 
bool nodecmp(const nodeId node1, const nodeId node2) {
	return (!(node1.bytes[0] == node2.bytes[0] && node1.bytes[1] == node2.bytes[1] && node1.bytes[2] == node2.bytes[2] && node1.bytes[3] == node2.bytes[3]));
}
