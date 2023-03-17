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
