/**
 * ntp.h
 * 
 * (Basic) NTP client adapted to VxWorks environment
 *
 * @author: Tim Hogard (thogard@abnormal.com)
 *          Alessandro Mannini <alessandro.mannini@gmail.com> (port to VxWorks)
 * @date: Jan 10, 2023
 */

#ifndef INCLUDES_NTP_H_
#define INCLUDES_NTP_H_
#include <stdio.h>

/**
 * NTP client
 * 
 * @param ntpServer: IP address of the NTP server
 * @param timezone: +/- seconds of the timezone (default = 0)
 * @return time_t received from the server
 */
struct timespec  ntpc(char *ntpServer, int timezoneOffset);

#endif /* INCLUDES_NTP_H_ */
