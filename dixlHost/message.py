"""
@author         : "Alessandro Mannini"
@organization   : "Università degli Studi di Firenze"
@contact        : "alessandro.mannini@gmail.com"
@date           : "Jan 10, 2023"
@version        : "1.0.0"
"""
import struct
from collections import namedtuple
from collections.abc import Iterable
from enum import Enum

import socket

from config import *
from typing import TYPE_CHECKING
if TYPE_CHECKING:
	from model.node import Node
from model.node import NodeState
from utility import *

# Message types
class MsgType(Enum):
	# Service messages - Init task
	NODERESET 					= 10	# Reset in the Init state
	NODECONFIG 					= 11	# Routes configuration sent by the host

# Messages defs
Header = namedtuple("Header", [ "length", "type" , "source", "destination"])
Route = namedtuple("Route", ["ID", "prev", "next", "position", "requestedPosition"])
MsgInitCONFIGTYPE = namedtuple("MsgHeaderCONFIG", ["header", "sequence", "totalSegments", "nodeType"])
MsgInitCONFIG = namedtuple("MsgHeaderCONFIG", ["header", "sequence", "totalSegments", "route"])
MsgRouteREQ = namedtuple("MsgRouteREQ", ["header", "requestRouteId"])
MsgRouteTRAINOK = namedtuple("MsgRouteTRAINOK", ["header", "requestRouteId"])
MsgRouteTRAINNOK = namedtuple("MsgRouteTRAINNOK", ["header", "requestRouteId"])
MsgInitRESET = namedtuple("MsgInitRESET", ["header"])
MsgPointMALFUNCTION = namedtuple("MsgPointMALFUNCTION", ["header"])

# Packed messages formats
MsgHeaderFormat = "BBxx4s4sxxxx"
MsgRouteFormat = "I4s4sbbxx"
MsgSequenceTotalFormat = "II"
MsgNodeTypeFormat = "Bxxx"
MsgInitCONFIGTYPEFormat = MsgHeaderFormat + MsgSequenceTotalFormat + MsgNodeTypeFormat
MsgInitCONFIGFormat = MsgHeaderFormat + MsgSequenceTotalFormat + MsgRouteFormat
MsgRouteRequestFormat = "I"
MsgRouteREQFormat = MsgHeaderFormat + MsgRouteRequestFormat
MsgRouteTRAINOKFormat = MsgHeaderFormat + MsgRouteRequestFormat
MsgRouteTRAINNOKFormat = MsgHeaderFormat + MsgRouteRequestFormat
MsgInitRESETFormat = MsgHeaderFormat
MsgPointMALFUNCTIONFormat = MsgHeaderFormat

def getLocaIP() -> str:
	"""
	Try to obtain local IP address
	"""
	s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	s.connect(("8.8.8.8", 80))
	IP: str = s.getsockname()[0]
	s.close()

	return IP

def flatten(x):
    """	
    Return a flat representation (list) of the named tuples messages.
	Parameters:
		x - object to be flatten
          
    Return:
		list with values of the object
    """ 
    if isinstance(x, bytes):
        return [x]
    elif isinstance(x, Iterable):
        return [a for i in x for a in flatten(i)]
    elif isinstance(x, Enum):
        return [x.value]
    else:
        return [x]

def getMessageToSend(data: namedtuple, format: str):
	""" 
    Given the named tuple with values and the format, return the formated bytes of the message.
    Parameters: 
		data - namedtuple with data to be sent
        format - string representing the packed format of the message
    """
	dataToSend = bytearray(struct.pack(format, *flatten(data)))
	dataToSend[0]=len(dataToSend)
	return dataToSend

def sendReset(hostIP: bytes, node: 'Node'):
	"""
	Create a client socket to NodeTrackCircuit to send the RESET messasge
	Parameters:
		- hostIP: IP of the sending host (bytes)
		- node: node object to send to
	"""
	try:
		# Prepare the message
		nodeIP: str = IP2str(node.IP)
		messageToSend = getMessageToSend( MsgInitRESET( Header( 0, MsgType.NODERESET, hostIP, node.IP) ), MsgInitRESETFormat)

		# Create the connection to the node
		client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		client_socket.connect((nodeIP, NodeCommPort))

		# Send config
		# Route request
		ret = client_socket.send(messageToSend)
			
		# Close the socket
		client_socket.shutdown(socket.SHUT_RDWR)

		node.resetRequest(NodeState.OK)
                
	except Exception as ex:
		node.resetRequest(NodeState.FAIL)
		print(f'Error resetting node {nodeIP}: {ex}')

def sendConfig(hostIP: bytes, node: 'Node'):
	"""
	Create a client socket to NodeTrackCircuit to send the CONFIG messasge
	Parameters:
		- hostIP: IP of the sending host (bytes)
		- node: node object to send to
	"""
	try:
		# Create the connection to the node
		nodeIP: str = IP2str(node.IP)
		client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		client_socket.connect((nodeIP, NodeCommPort))

		# Prepare the message header
		messageToSend = getMessageToSend( MsgInitCONFIGTYPE( Header( 0, MsgType.NODECONFIG, hostIP, node.IP), 0, len(node.Config), node.type ), MsgInitCONFIGTYPEFormat)
		# Send to node
		client_socket.send(messageToSend)

		# Cycle over config
		counter: int = 0
		for configItem in node.Config:
			# Count
			counter += 1

			# Get prev and next
			if configItem.prev is None:
				prev = hostIP
			else:
				prev = configItem.prev.IP
			if configItem.next is None:
				next = bytes([0, 0, 0, 0])
			else:
				next = configItem.next.IP				
			# Prepare the message of the current config item
			messageToSend = getMessageToSend( MsgInitCONFIG( Header( 0, MsgType.NODECONFIG, hostIP, node.IP), counter, len(node.Config), Route( configItem.routeId, prev, next, configItem.position, configItem.requestedPos ) ), MsgInitCONFIGFormat)
			# Send to node
			client_socket.send(messageToSend)

		# Close the socket
		client_socket.shutdown(socket.SHUT_RDWR)

		node.resetRequest(NodeState.OK)
                
	except Exception as ex:
		node.resetRequest(NodeState.FAIL)
		print(f'Error configuring node {nodeIP}: {ex}')