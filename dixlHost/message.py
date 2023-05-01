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

import random
import socket
from pubsub import pub
import time

from config import *
from typing import TYPE_CHECKING
if TYPE_CHECKING:
	from model.node import Node
from model.node import NodeState
from model.node import NodeNull
from model.route import RouteState
from model.log_line import LogLine, LogType
from utility import *


# Message types
class MsgType(Enum):
	# Service messages - Init task
	NODERESET 					= 10	# Reset in the Init state
	NODECONFIG 					= 11	# Routes configuration sent by the host

	# Route messages - Ctrl task
	ROUTEREQ 					= 30	# Route request
	ROUTETRAINOK 				= 36	# 2PhaseCommit Train OK
	ROUTETTRAINOK 				= 37	# 2PhaseCommit Train NOK

	# Log messages - Log task
	LOGREQ						= 81	# Request current log messages
	LOGSEND						= 82	# Response current log messages
	LOGDEL						= 83	# Ack messages were received and ask to delete
	LOGDELACK					= 84	# Ack messages were deleted

	# Point requests - Point task
	POINTMALFUNC   				= 95   	# Point set malfunction state (error simulation request)


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
MsgLogREQ = namedtuple("MsgLogREQ", ["header"])
MsgLogLine = namedtuple("MsgLogLine", ["timestamp_s", "timestamp_ns", "type", "routeId", "nodeIP"])
MsgLogCurrentTotal = namedtuple("MsgLogCurrentTotal", ["currentLine", "totalLines"])
MsgLogSEND = namedtuple("MsgLogSEND", ["header", "currentTotal", "logline"])
MsgLogDEL = namedtuple("MsgLogDEL", ["header"])
MsgLogDELACK = namedtuple("MsgLogDELACK", ["header"])

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
MsgLogREQFormat = MsgHeaderFormat
MsgLogCurrentTotalFormat = "II"
MsgTimestampFormat = "qq"				# Python pack (signed) long long format (8 bytes)
MsgLogLineFormat = MsgTimestampFormat + "BxxxI4sxxxx"
MsgLogSENDFormat = MsgHeaderFormat + MsgLogCurrentTotalFormat + MsgLogLineFormat
MsgLogDELFormat = MsgHeaderFormat
MsgLogDELACKFormat = MsgHeaderFormat


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
	Create a client socket to node to send the RESET message
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

		# Reset request
		ret = client_socket.send(messageToSend)
			
		# Close the socket
		client_socket.shutdown(socket.SHUT_RDWR)

		node.resetRequest(NodeState.OK)
                
	except Exception as ex:
		node.resetRequest(NodeState.FAIL)
		print(f'Error resetting node {nodeIP}: {ex}')
		return False

def sendConfig(hostIP: bytes, node: 'Node'):
	"""
	Create a client socket to node to send the CONFIG message
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
		return False
	
def sendMalfunction(hostIP: bytes, node: 'Node', delay: int):
	"""
	Create a client socket to node to send the MALFUNCTION simulation message
	Parameters:
		- hostIP: IP of the sending host (bytes)
		- node: node object to send to
	"""
	try:
		# Prepare the message
		nodeIP: str = IP2str(node.IP)
		messageToSend = getMessageToSend( MsgPointMALFUNCTION( Header( 0, MsgType.POINTMALFUNC, hostIP, node.IP) ), MsgPointMALFUNCTIONFormat)
		
		# If delayed ... sleep
		if delay > 0: 
			value: float = delay / 1000
			print(f'Simulate Malfunction: delaying for {value}')
			time.sleep(delay / 1000)

		# Create the connection to the node
		client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		client_socket.connect((nodeIP, NodeCommPort))

		# Malfunction simulation request
		ret = client_socket.send(messageToSend)
			
		# Close the socket
		client_socket.shutdown(socket.SHUT_RDWR)

		node.resetRequest(NodeState.OK)
                
	except Exception as ex:
		node.resetRequest(NodeState.FAIL)
		print(f'Error requesting malfunction simulation to node {nodeIP}: {ex}')
		return False


def requestLog(hostIP: bytes, node: 'Node', IDDict: dict[bytes, str]):
	"""
	Create a client socket to the node, request the log, wait for response and confirm for deletion
	Parameters:
		- hostIP: IP of the sending host (bytes)
		- node: node object to request
	"""
	try:
		#
		# ### SEND REQUEST RESPONSE ###
		#
		# Get routeId and first node
		nodeIP: str = IP2str(node.IP)
		hostIPStr = IP2str(hostIP)

		# Prepare the message
		messageToSend = getMessageToSend( MsgLogREQ( Header( 0, MsgType.LOGREQ, hostIP, node.IP) ), MsgLogREQFormat)

		# Create the connection to the node
		client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		client_socket.connect((nodeIP, NodeCommPort))

		# Log request
		ret = client_socket.send(messageToSend)
			
		# Close the socket
		client_socket.shutdown(socket.SHUT_RDWR)
	
	except Exception as ex:
		node.resetRequest(NodeState.FAIL)
		print(f'Error requesting log to node {nodeIP}: {ex}')

	# Receive LOG lines in a temp list
	lines: list[dict[str, str]] = []			# reconstructed log lines

	try:
		#
		# ### WAIT FOR LOG LINES ###
		#
		# Create a server socket
		server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

		try:
			server_socket.settimeout(LogRequestResponseTimeout) # timeout for listening
			server_socket.bind((hostIPStr, NodeCommPort))
			server_socket.listen()

			# Receive data until socket disconnect or sequence complited or timeout
			data: bytearray = bytearray()				# socket buffer
			sequenceError: bool = False					# sequence error flag
			sequenceExcepted: int = 1					# excepted sequence number ( 0 => sequence end)

			while sequenceExcepted > 0:
				#Accept any incoming client connection or timeout
				client_socket, client_address = server_socket.accept()
				print(f"Connected with {client_address}...")

				while True:
					chunk: bytes = client_socket.recv(1024)

					if chunk: 
						data += chunk

						# process messages
						while sequenceExcepted > 0 and len(data) >= 56:
							# Message unpacking
							header = struct.unpack(MsgHeaderFormat,  data[0:16])
							header = Header._make(header)
							logCurrentTotal = struct.unpack(MsgLogCurrentTotalFormat,  data[16:24])
							logCurrentTotal = MsgLogCurrentTotal._make(logCurrentTotal)
							logLine = struct.unpack(MsgLogLineFormat,  data[24:56])
							logLine = MsgLogLine._make(logLine)

							# Remove used data
							data = data[56:]

							# Check message type
							if header.type != MsgType.LOGSEND.value:
								node.resetRequest(NodeState.FAIL)
								return False							
							# currentLine && totalLines == 00 => Log empty
							if logCurrentTotal.currentLine == 0 and logCurrentTotal.totalLines == 0:
								# reset request with OK and without log update notify
								node.resetRequest(NodeState.OK)
								return True
							
							# Check sequence (but continue receiving). Is the excepted one?
							if logCurrentTotal.currentLine != sequenceExcepted: sequenceError = True
							if logCurrentTotal.currentLine > logCurrentTotal.totalLines: sequenceError = True

							# Completed ?
							if logCurrentTotal.currentLine == logCurrentTotal.totalLines:
								sequenceExcepted = 0
							else:
								sequenceExcepted += 1

							# Log line appended
							if  logLine.nodeIP != NodeNull:
								ID: str = IDDict.get(logLine.nodeIP, None)
							else:
								ID = None
								
							lines.append(LogLine(logLine.timestamp_s, logLine.timestamp_ns, LogType(logLine.type), logLine.routeId, ID, logLine.nodeIP))

					else:
						# No more data
						break
			
				# Sequence error detected while receiving?
				if sequenceError:
					node.resetRequest(NodeState.FAIL)
					raise IndexError('Log sequence error')
				
		except Exception as ex:
			# rethrow the exception but in every way (finally) close the opened socket
			raise ex
		
		finally:
			server_socket.close()

	except Exception as ex:
		node.resetRequest(NodeState.FAIL)
		print(f'Error waiting response to log request to node {nodeIP}: {ex}')

		return False

	# LOG received but before to notify success have to send DEL requenst and wait ACK
	# Add lines to node log but keep a "bookmark" to rollback if needed
	numLines: int = len(node.log)
	node.log.extend(lines)

	try:
		#
		# ### SEND LOG DEL REQUEST ###
		#
		# Prepare the message
		messageToSend = getMessageToSend( MsgLogDEL( Header( 0, MsgType.LOGDEL, hostIP, node.IP) ), MsgLogDELFormat)

		# Create the connection to the node
		client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		client_socket.connect((nodeIP, NodeCommPort))

		# Log request
		ret = client_socket.send(messageToSend)
			
		# Close the socket
		client_socket.shutdown(socket.SHUT_RDWR)
	
	except Exception as ex:
		# Reset request
		node.resetRequest(NodeState.FAIL)

		# remove lines from log (because not deleted)
		while len(node.log) > numLines: node.log.pop()

		print(f'Error requesting log DEL to node {nodeIP}: {ex}')
		return False
	
	try:
		#
		# ### WAIT FOR A DELACK RESPONSE ###
		#
		# Create a server socket
		server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

		try:
			server_socket.settimeout(LogRequestResponseTimeout) 	# timeout for listening
			server_socket.bind((hostIPStr, NodeCommPort))
			server_socket.listen()

			# Accept any incoming client connection or timeout
			client_socket, client_address = server_socket.accept()
			print(f"Connected with {client_address}...")

			# Receive data
			data = client_socket.recv(1024)

			# Message unpacking
			header = struct.unpack(MsgHeaderFormat,  data[0:16])
			header = Header._make(header)

			# If data received
			if header.type != MsgType.LOGDELACK.value:
				# remove lines from log (because not deleted)
				while len(node.log) > numLines: node.log.pop()

				# reset request with FAIL
				node.resetRequest(NodeState.FAIL)
				return False

		except Exception as ex:
			# remove lines from log (because not deleted)
			while len(node.log) > numLines: node.log.pop()

			# rethrow the exception but in every way (finally) close the opened socket
			raise ex
		
		finally:
			server_socket.close()

		# DELACK Received with correct routeId, notify
		pub.sendMessage('node.update.log', node=node)

		# reset request with OK
		node.resetRequest(NodeState.OK)
		return True
                
	except Exception as ex:
		# Reset request
		node.resetRequest(NodeState.FAIL)
		print(f'Error waiting DEL ACK from node {nodeIP}: {ex}')

		return False

def sendRequest(hostIP: bytes, route: Route):
	"""
	Create a client socket to first node to send the ROUTEREQ message and wait for a reply (or timeout)
	Parameters:
		- hostIP: IP of the sending host (bytes)
		- route: route object to request
	"""
	try:
		#
		# ### SEND REQUEST RESPONSE ###
		#
		# Get routeId and first node
		routeId = route.id
		nodeFirst: Node = route[0].node
		hostIPStr: str = IP2str(hostIP)

		# Prepare the message
		nodeIP: str = IP2str(nodeFirst.IP)
		messageToSend = getMessageToSend( MsgRouteREQ( Header( 0, MsgType.ROUTEREQ, hostIP, nodeFirst.IP), routeId ), MsgRouteREQFormat)

		# Before sending request, spawn thread for malfunction simulation if present
		for nodeRef in route:
			if nodeRef.node.malfunction:
				# Request to send message of malfunction to node (in a new thread with a random delay enabled by default)
				nodeRef.node.simulateMalfunction(hostIP)
		# Create the connection to the node
		client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		client_socket.connect((nodeIP, NodeCommPort))

		# Route request
		ret = client_socket.send(messageToSend)
			
		# Close the socket
		client_socket.shutdown(socket.SHUT_RDWR)
	
	except Exception as ex:
		route.resetRequest(RouteState.FAIL)
		print(f'Error requesting route {routeId} to node {nodeIP}: {ex}')

	try:
		#
		# ### WAIT FOR A RESPONSE ###
		#
		# Create a server socket
		server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

		try:
			server_socket.settimeout(RouteRequestResponseTimeout) # timeout for listening
			server_socket.bind((hostIPStr, NodeCommPort))
			server_socket.listen()

			#Accept any incoming client connection or timeout
			client_socket, client_address = server_socket.accept()
			print(f"Connected with {client_address}...")

			# Receive data
			data = client_socket.recv(1024)

			# Message unpacking
			header = struct.unpack(MsgHeaderFormat,  data[0:16])
			header = Header._make(header)

			# If data received
			match header.type:
				case MsgType.ROUTETRAINOK.value:
					payload = struct.unpack(MsgRouteTRAINOKFormat, data )
					message = MsgRouteTRAINOK._make([header, payload[4]])

					# Check the response refere to excepted route id
					if message.requestRouteId != routeId:
						route.resetRequest(RouteState.FAIL)
						return False
				
				case MsgType.ROUTETTRAINOK.value:
					payload = struct.unpack(MsgRouteTRAINOKFormat, data )
					message = MsgRouteTRAINOK._make([header, payload[4]])

					# Check the response refere to excepted route id
					if message.requestRouteId != routeId:
						route.resetRequest(RouteState.FAIL)
						return False
					else:
						# Received TRAINNOK update state to FAIL
						route.resetRequest(RouteState.FAIL)
						return True

				case _:
						route.resetRequest(RouteState.FAIL)
						return False

		except Exception as ex:
			# rethrow the exception but in every way (finally) close the opened socket
			raise ex
		
		finally:
			server_socket.close()

		# TRAINOK Received with correct routeId
		route.resetRequest(RouteState.OK)
		return True
                
	except Exception as ex:
		route.resetRequest(RouteState.FAIL)
		print(f'Error waiting response to route request {routeId} from node {nodeIP}: {ex}')

		return False
