/**
 * network.h
 * 
 * Networking helpers
 *
 * @author: Alessandro Mannini <alessandro.mannini@gmail.com>
 * @date: Jan 10, 2023
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <netinet/in.h>

#include <syslog.h>
#include <inetLib.h>
#include <sockLib.h>
#include <net/if.h>
#include <net/if_dl.h>
#include "network.h"
#include "ifaddrs.h"

#include "utils.h" 
#include "globals.h" 

/* FUNCTIONS helpers */
int socket_create(int domain, int type, int proto) {

    int fd = socket(domain, type, proto); 
    if (fd == SOCK_ERROR) {
    	/* log the error information */
    	int err = errno;
        syslog(LOG_ERR, "Create socket error %i: %s", err, strerror(err));
    }
    return fd;
}

int socket_bind(int fd, char *bind_address, int port) {
    int ret;
    struct sockaddr_in addr;

    /* the addr specifies the address family , IP address and port
     for the socket that is being bound */
    memset (&addr, 0, sizeof(struct sockaddr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(bind_address); /* the local address */
    addr.sin_port = htons(port);
    addr.sin_len = sizeof(addr);

    /* bind the socket */
    if ((ret = bind(fd, (struct sockaddr*)&addr, sizeof(addr))) == SOCK_ERROR) {
    	int err = errno;
    	close(fd);
        syslog(LOG_ERR, "Bind socket error %i: %s", err, strerror(err));
    }
    return ret;
}

int socket_listen(int fd) {
    int ret;
    /* listen for incoming request on the specific socket */
    if ((ret = listen(fd, 1)) == SOCK_ERROR) {
    	int err = errno;
    	close(fd);
        syslog(LOG_ERR, "Listen socket error %i: %s", err, strerror(err));
    }
    return ret;
}

int socket_Connect(int fd, char *bind_address, int port) {
    int ret;
    struct sockaddr_in server;

    /* the addr specifies the address family , IP address and port
     for the socket that is being bound */
    memset (&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(bind_address); /* the local address */
    server.sin_port = htons(port);
    server.sin_len = sizeof(server);

    /* bind the socket */
    if ((ret = connect(fd, (struct sockaddr*)&server, sizeof(server))) == SOCK_ERROR) {
    	int err = errno;
    	close(fd);
        syslog(LOG_ERR, "Connect socket error %i: %s", err, strerror(err));
    }
    
    return ret;
}

int socket_accept(int fd) {
    int ret;
	struct sockaddr_in peer_addr;
	socklen_t len = sizeof(peer_addr);

	/* blocked, waiting for incoming requests */
	if ((ret = accept(fd, (struct sockaddr*)&peer_addr, &len)) == SOCK_ERROR) {
		// Error
    	int err = errno;
        syslog(LOG_ERR, "Accept socket error %i: %s", err, strerror(err));
	}
	
	//If no errors, return the new socket create for receiving data
	return ret;
}

int socket_close(int fd) {
    int ret;

	/* blocked, waiting for incoming requests */
	if ((ret = close(fd)) == SOCK_ERROR) {
		// Error
    	int err = errno;
        syslog(LOG_ERR, "Close socket error %i: %s", err, strerror(err));	
	}
	
	return ret;
}

ssize_t socket_recv(int fd, void *buffer, size_t buffer_size) {
	ssize_t ret;

	/* receive data into the buffer */
	if ((ret = recv(fd, buffer, buffer_size, 0)) == SOCK_ERROR) {
		// Error
		int err=errno;
        syslog(LOG_ERR, "Receive socket error %i: %s", err, strerror(err));
	}
	
	return ret;
}

size_t socket_send(int fd, void *buffer, size_t buffer_size) {
    int ret;

	if ((ret = send(fd, buffer, buffer_size, 0) == SOCK_ERROR)) {
	    // Error	  
		int err=errno;
		syslog(LOG_ERR, "Send socket error: %s", err, strerror(err));
	} else if (ret != buffer_size) {
		syslog (LOG_ERR, "Send socket error: %i bytes sent istead of %i", ret, buffer_size);
	}
	return ret;	
}

STATUS network_get_if_params(char *ifname, IPv4Address *IPv4, MACAddress *MAC) {
    struct ifaddrs *ifaddrs,*ifaddr;
    struct sockaddr_in *sa;

    if (getifaddrs(&ifaddrs) == -1) {
		// Error
		int err=errno;
        syslog(LOG_ERR, "getifaddrs error %i: %s", err, strerror(err));        
        exit(rcNETWORK_GETIFADDRSERR);
    }

    // Find the first interface excluding loopback
    ifname[0]='\000';    
    for (ifaddr = ifaddrs; ifaddr; ifaddr = ifaddr->ifa_next) {
        if (ifaddr->ifa_addr && ifaddr->ifa_addr->sa_family == AF_INET) {
        	// Skip local/loopbackinterfaces
        	if ( ifaddr->ifa_flags & IFF_LOOPBACK)
        		continue;
        	// name
            strncpy(ifname, ifaddr->ifa_name, IFNAMSIZ);

            // IP address
        	sa = (struct sockaddr_in *) ifaddr->ifa_addr;
        	network_n_to_IPv4(sa, IPv4);

        	break;
        }
    }

    // If interface name found then search the corrisponding MAC
    if (ifname[0]) {
    	
    	
        for (ifaddr = ifaddrs; ifaddr; ifaddr = ifaddr->ifa_next) {
            if (ifaddr->ifa_addr && ifaddr->ifa_addr->sa_family == AF_LINK) {
            	
            	// Search interface name
            	if ( strcmp(ifaddr->ifa_name, ifname) != 0 )
            		continue;

				unsigned char *ptr;
				ptr = (unsigned char *)LLADDR((struct sockaddr_dl *)(ifaddr)->ifa_addr);
				
				for(int i=0; i < 6; i++) 
					MAC->bytes[i]=*(ptr+i);

				freeifaddrs(ifaddrs);	
				return OK;
            }
        
        }
	}

    // free mem
    freeifaddrs(ifaddrs);	
	return ERROR;
}

void network_IPv4_to_str(const IPv4Address *IPv4, char *str) {
	int i=0;
	int r=0;
	char temp[4];
	
	for(i=0;i<4;i++) {
		snprintf(temp, 4, "%d", IPv4->bytes[i]);
		int j=0;
		while (temp[j]) 
			str[r++]=temp[j++];
		if (i<3) str[r++]='.';
	}
	
	str[r]='\00';
}

void network_n_to_IPv4(struct sockaddr_in *sa, IPv4Address *IPv4) {
    struct in_addr addr = sa->sin_addr;
    uint32_t value = addr.s_addr;
    uint32_t mod;
    
	for(int i=0; i<40;i++) {
		mod = value % 256;
    	IPv4->bytes[i] = (unsigned char) mod;
    	value /= 256;
    }
}
