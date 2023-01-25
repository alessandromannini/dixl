/*
 * utils.c
 * 
 * Globals utlities and macro
 *
 *  Created on: Jan 10, 2023
 *  Author: Alessandro Mannini <alessandro.mannini@gmail.com>
 */

#include <stdbool.h>

#include <logLib.h>
#include <taskLib.h>
#include <intLib.h>
#include <objLibCommon.h>

#include "utils.h"

/* FUNCTIONS helpers */
/*
 * Shutdown a task logging messages
 */
bool taskShutdown(TASK_ID tId, char *message) {	
	// Shutting down
	LOGMSG1(message, tId);
	
	// Delete task
	STATUS rc = taskDelete(tId);
	
	switch (rc) {
		case 0:	// OK
			LOGMSG1("(0x%jx) Task deleted\n", tId);	
			break;
			
		case S_intLib_NOT_ISR_CALLABLE:
			LOGMSG1("(0x%jx) error: Delete task must not be called from an ISR\n", tId);
			return FALSE;
			break;			
		case S_objLib_OBJ_DELETED:
			LOGMSG1("(0x%jx) warning: The specified task was already deleted\n", tId);
			return FALSE;
			break;			
			
		case S_objLib_OBJ_UNAVAILABLE:
			LOGMSG1("(0x%jx) error: The specified task is unavailable\n", tId);
			return FALSE;
			break;			
			
		case S_objLib_OBJ_ID_ERROR:
			LOGMSG1("(0x%jx) error: The task ID parameter is invalid\n", tId);
			return FALSE;
			break;			
			
		default:
			LOGMSG2("(0x%jx) error: (%d)\n", tId, rc);
			return FALSE;
			break;
	}
	
	return TRUE;
}


