/*
 * utils.h
 * 
 * Globals utlities and macro
 *
 *  Created on: Jan 10, 2023
 *  Author: Alessandro Mannini <alessandro.mannini@gmail.com>
 */
#include <stdio.h>
#include <stdbool.h>
#include <logLib.h>
#include <taskLib.h>

/* MACROs */
#define 	LOGMSG(fmt)									NOWAIT_LOG6(fmt, 0, 0 ,0, 0, 0, 0)
#define     LOGMSG1(fmt,arg1)       					NOWAIT_LOG6(fmt, arg1, 0, 0, 0 , 0, 0 )
#define     LOGMSG2(fmt,arg1,arg2)       				NOWAIT_LOG6(fmt, arg1, arg2, 0, 0 , 0, 0 )
#define     LOGMSG3(fmt,arg1,arg2,arg3)       			NOWAIT_LOG6(fmt, arg1, arg2, arg3, 0 , 0, 0)
#define     LOGMSG4(fmt,arg1,arg2,arg3,arg4)       		NOWAIT_LOG6(fmt, arg1, arg2, arg3, arg4, 0, 0)
#define     LOGMSG5(fmt,arg1,arg2,arg3,arg4,arg5)       NOWAIT_LOG6(fmt, arg1, arg2, arg3, arg4 , arg5, 0)
#define     LOGMSG6(fmt,arg1,arg2,arg3,arg4,arg5,arg6) 	NOWAIT_LOG6(fmt, arg1, arg2, arg3, arg4 , arg5, arg6)

/* FUNCTIONS helpers */
bool taskShutdown(TASK_ID tId, char *message);
