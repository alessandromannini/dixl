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

#include <netinet/in.h>
#include "globals.h"

/* DEFINEs */
#define 	SOCK_OK			0
#define 	SOCK_ERROR		-1

/* FUNCTIONS helpers */
/**
 *  bind socket to addr:port 
 *  @param domain: address family of the socket
 *  @param type: type of the protocol
 *  @param proto: protocol of the socket
 */
int socket_create(int domain, int type, int proto);

/**
 *  bind socket to addr:port 
 *  @param fd: file descriptor of a opened socket
 *  @param address: interface address to bind the socket to
 *  @param port: port to bind the socket to
 */
int socket_bind(int fd, char *address, int port);

/**
 *  enable connects to the socket
 *  @param fd: file descriptor of the socket
 */
int socket_listen(int fd);

/**
 * connect the socket to a server
 *  @param fd: file descriptor of a opened socket
 *  @param address: server address to connect the socket to
 *  @param port: port to connect the socket to
 */
int socket_connect(int fd, char *bind_address, int port);

/**
 *  accept a connection on the socket and return a new socket to manage it
 *  @param fd: file descriptor of the socket
 */
int socket_accept(int fd);

/**
 *  close the socket
 *  @param fd: file descriptor of the socket
 */
int socket_close(int fd);

/**
 *  receive a message from the socket
 *  @param fd: file descriptor of the socket
 *  @param buffer: char buffer where to store received data
 *  @param buffer_size: size of the buffer
 */
ssize_t socket_recv(int fd, void *buffer, size_t buffer_size);

/**
 *  send a message into the socket
 *  @param fd: file descriptor of the socket
 *  @param buffer: char buffer where data to send is stored
 *  @param buffer_size: size of the buffer
 */
size_t socket_send(int fd, void *buffer, size_t buffer_size);

/**
 *  get the name of the first interface excluding loopback
 *  @param ifname: buffer to store interface name
 */
STATUS network_get_if_params(char *ifname, IPv4Address *IPv4, MACAddress *MAC);

/**
 *  convert IPv4 from array of char to string
 */
void network_IPv4_to_str(const IPv4Address *IPv4, char *str);

/**
 *  convert IP address from in_addr to uint8_t array
 */
void network_n_to_IPv4(struct sockaddr_in *in_addr, IPv4Address *IPv4);
