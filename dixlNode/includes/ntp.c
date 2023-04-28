 /* This code will query a ntp server for the local time and display

 * it.  it is intended to show how to use a NTP server as a time
 * source for a simple network connected device.
 * This is the C version.  The orignal was in Perl
 *
 * For better clock management see the offical NTP info at:
 * http://www.eecis.udel.edu/~ntp/
 *
 * written by Tim Hogard (thogard@abnormal.com)
 * Thu Sep 26 13:35:41 EAST 2002
 * Converted to C Fri Feb 21 21:42:49 EAST 2003
 * this code is in the public domain.
 * it can be found here http://www.abnormal.com/~thogard/ntp/
 *
 * ADAPTED TO VXWORKS by Alessandro Mannini <alessandro.mannini@mail.com>
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <syslog.h>
#include <time.h>
#include <string.h>

#include "network.h"


struct timespec ntpc(char *ntpServer, int timezoneOffset) {
	int portno=123;     			//NTP is port 123
	int maxlen=1024;    			//check our buffers
	ssize_t ret;          			// return values
	unsigned char msg[48];    		// the packet we send
	uint32_t buf[maxlen]; 			// the buffer we get back
	
	//struct in_addr ipaddr;         
	struct sockaddr_in server_addr;
	int s;  						// socket
	struct timespec tmit = { .tv_sec =0, .tv_nsec = 0 };   		// updated timespec

	//use Socket;
	//
	//#we use the system call to open a UDP socket
	//socket(SOCKET, PF_INET, SOCK_DGRAM, getprotobyname("udp")) or die "socket: $!";
	// Create the listening socket
	if ((s = socket_create(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == 0)
		return tmit;
	syslog(LOG_INFO, "NTPC: socket created");
	//
	memset( &server_addr, 0, sizeof( server_addr ));
	server_addr.sin_family=AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ntpServer);
	server_addr.sin_port=htons(portno);
	//printf("ipaddr (in hex): %x\n",server_addr.sin_addr);

	/*
	 * build a message.  Our message is all zeros except for a one in the
	 * protocol version field
	 * msg[] should be x1b and 47 byte to zero 
	 * it should be a total of 48 bytes long
	*/
	memset(&msg, 0, 48);
	msg[0] =0x1b;
	
	// send the data
	syslog(LOG_INFO, "NTPC: sending data ...");
	if ((ret = socket_sendto(s, msg, sizeof(msg), (struct sockaddr *) &server_addr, sizeof(server_addr))) == SOCK_ERROR) {
		socket_close(s);
		return tmit;
	}
	syslog(LOG_INFO, "NTPC: data sent OK");
	
	// get the data back
	struct sockaddr saddr;
	socklen_t saddr_l = sizeof (saddr);
	syslog(LOG_INFO, "NTPC: waiting for a response ...");
	if ((ret = socket_recvfrom(s, buf, sizeof(buf), &saddr,&saddr_l)) == SOCK_ERROR) {
		socket_close(s);
		return tmit;
	}
	syslog(LOG_INFO, "NTPC: data received");

	//We get 12 long words back in Network order
	/*
	 * The high word of transmit time is the 10th word we get back
	 * tmit is the time in seconds not accounting for network delays which
	 * should be way less than a second if this is a local NTP server
	 */

	tmit.tv_sec=ntohl((time_t)buf[10]);    //# get transmit time (sec)
	//tmit.tv_nsec=ntohl((time_t)buf[11]);   //# get transmit time (fraction of seconds converted to nsec)
	tmit.tv_nsec=(time_t)buf[11];   //# get transmit time (fraction of seconds converted to nsec)
	tmit.tv_nsec = (uint32_t)((double)tmit.tv_nsec * 1.0e6 / (double)(1LL << 32));
	//printf("tmit=%d\n",tmit);

	/*
	 * Convert time to unix standard time NTP is number of seconds since 0000
	 * UT on 1 January 1900 unix time is seconds since 0000 UT on 1 January
	 * 1970 There has been a trend to add a 2 leap seconds every 3 years.
	 * Leap seconds are only an issue the last second of the month in June and
	 * December if you don't try to set the clock then it can be ignored but
	 * this is importaint to people who coordinate times with GPS clock sources.
	 */
	tmit.tv_sec-= 2208988800U; 

	// Add timezone
	tmit.tv_sec += timezoneOffset;
	
	return tmit;
}


