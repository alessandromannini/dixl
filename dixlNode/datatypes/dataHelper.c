/**
 * dataHelper.c
 * 
 * Datatypes helper functions
 *
 * @author: Alessandro Mannini <alessandro.mannini@gmail.com>
 * @date: Jan 10, 2023
 */
#include "dataTypes.h"

const char *pointpos_str(ePointPosition pos) {
	switch (pos) {
	case POINTPOS_DIVERGING:
		return "diverging";
		
	case POINTPOS_STRAIGHT:
		return "straight";
	default:
		return "undefined";
	}	
}
