"""
@author         : "Alessandro Mannini"
@organization   : "Università degli Studi di Firenze"
@contact        : "alessandro.mannini@gmail.com"
@date           : "Jan 10, 2023"
@version        : "1.0.0"
"""
import struct
import socket
from collections import namedtuple
from collections.abc import Iterable
from enum import Enum

def flatten(x):
    if isinstance(x, bytes):
        return [x]
    elif isinstance(x, Iterable):
        return [a for i in x for a in flatten(i)]
    elif isinstance(x, Enum):
        return [x.value]
    else:
        return [x]

def getMessageToSend(data: namedtuple, format: str):
    dataToSend = bytearray(struct.pack(format, *flatten(data)))
    dataToSend[0]=len(dataToSend)
    return dataToSend

class MsgType(Enum):
	# Service messages - Init task
	NODERESET 					= 10	# Reset in the Init state
	NODECONFIG 					= 11	# Routes configuration sent by the host
	NODEDISCOVERY 				= 20	# TODO Nodes discovery from the host
	NODEADVERTISE 				= 21	# TODO Node advertise reply to discovery
        
	# Route messages - Ctrl task
	ROUTEREQ 					= 30	# Route request
	ROUTEACK 					= 31	# 2PhaseCommit Ack
	ROUTENACK 					= 32	# 2PhaseCommit NAck
	ROUTECOMMIT					= 33	# 2PhaseCommit Commit
	ROUTEAGREE 					= 34	# 2PhaseCommit Agree
	ROUTEDISAGREE 				= 35	# 2PhaseCommit Disagree
	ROUTETRAINOK 				= 36	# 2PhaseCommit Train OK
	ROUTETTRAINOK 				= 37	# 2PhaseCommit Train NOK

	# Log messages - Log task
	LOGREQ						= 81	# Request current log messages
	LOGSEND						= 82	# Response current log messages
	LOGDEL						= 83	# Ack messages were received and ask to delete
	LOGDELACK					= 84	# Ack messages were deleted

	# Diagnostic messages - TODO - Diag task
	NODEDIAG 					= 90	# Communication diagnostic request
	NODEDIAGACK 				= 91	# Communication diagnostic request Ack

	# Point requests - Point task
	MSGTYPE_POINTMALFUNC   		= 95,   # Point set malfunction state (error simulation request)

# Node type 
class NodeType(Enum):
	POINT 				= 10	# Point (Deviatoio)
	TRACKCIRCUIT 		= 20	# Track Circuit (Cdb o Circuito di Binario)

# Node position */
class NodePosition(Enum):
	FIRST 				= -128	# First node of the route
	MIDDLE				= 0  	# Node between two other nodes of the route
	LAST				= 127   # Last node of the route

# Point position */
class PointPosition(Enum):
    UNDEFINED			= -1    # Undefined (for Track Circuits or Malfunction)
    STRAIGHT 			= 0	    # Stright direction
    DIVERGING			= 50 	# DIverging direction

# Messages defs
Header = namedtuple("Header", [ "length", "type" , "source", "destination"])
Route = namedtuple("Route", ["IP", "prev", "next", "position", "requestedPosition"])
MsgInitCONFIGTYPE = namedtuple("MsgHeaderCONFIG", ["header", "sequence", "totalSegments", "nodeType"])
MsgInitCONFIG = namedtuple("MsgHeaderCONFIG", ["header", "sequence", "totalSegments", "route"])
MsgRouteREQ = namedtuple("MsgRouteREQ", ["header", "requestRouteId"])
MsgRouteTRAINOK = namedtuple("MsgRouteTRAINOK", ["header", "requestRouteId"])
MsgRouteTRAINNOK = namedtuple("MsgRouteTRAINNOK", ["header", "requestRouteId"])
MsgInitRESET = namedtuple("MsgInitRESET", ["headere"])

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

# Host and Nodes IDs and IPs
# TODO Detect MAC and IP of the host
Host 		 	 = { "Id": bytes([192, 168, 110, 254]), "MAC": b"\x7C\x76\x35\xF0\x99\xC0", "IP": "192.168.110.254" }
NodeNull	 	 = { "Id": bytes([0, 0, 0, 0]), "MAC": b"\x00\x00\x00\x00\x00\x00", "IP": "192.168.173.121"  }
NodeTrackCircuit = { "Id": bytes([192, 168, 110, 80]), "MAC": b"\x7a\x7a\xc0\xa8\xad\x50", "IP": "192.168.110.80"  }
NodePoint 		 = { "Id": bytes([192, 168, 110, 90]), "MAC": b"\x7a\x7a\xc0\xa8\xad\x5A", "IP": "192.168.110.90"  }
NodeCommPort 	 = 256

def sendReset2TC():
	##############################################
	# Create a client socket to NodeTrackCircuit #
	##############################################
	client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	client_socket.connect((NodeTrackCircuit["IP"], NodeCommPort))

	# Send config
	# Route request
	messageToSend = getMessageToSend( MsgInitRESET( Header( 0, MsgType.NODERESET, Host["Id"], NodeTrackCircuit["Id"]) ), MsgInitRESETFormat)
	ret = client_socket.send(messageToSend)
        
	# Close the socket
	client_socket.shutdown(socket.SHUT_RDWR)

def sendReset2PT():
	##############################################
	# Create a client socket to NodePoint        #
	##############################################
	client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	client_socket.connect((NodePoint["IP"], NodeCommPort))

	# Send config
	# Route request
	messageToSend = getMessageToSend( MsgInitRESET( Header( 0, MsgType.NODERESET, Host["Id"], NodeTrackCircuit["Id"]) ), MsgInitRESETFormat)
	client_socket.send(messageToSend)
        
	# Close the socket
	client_socket.shutdown(socket.SHUT_RDWR)

def sendConfig2TC():
	##############################################
	# Create a client socket to NodeTrackCircuit #
	##############################################
	client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	client_socket.connect((NodeTrackCircuit["IP"], NodeCommPort))

	# Send config
	# Node type
	messageToSend = getMessageToSend( MsgInitCONFIGTYPE( Header( 0, MsgType.NODECONFIG, Host["Id"], NodeTrackCircuit["Id"]), 0, 2, NodeType.TRACKCIRCUIT ), MsgInitCONFIGTYPEFormat)
	client_socket.send(messageToSend)

	# Track 1
	messageToSend = getMessageToSend( MsgInitCONFIG( Header( 0, MsgType.NODECONFIG, Host["Id"], NodeTrackCircuit["Id"]), 1, 2, Route( 1, Host["Id"], NodePoint["Id"], NodePosition.FIRST, PointPosition.UNDEFINED ) ), MsgInitCONFIGFormat)
	client_socket.send(messageToSend)

	# Track 2
	messageToSend = getMessageToSend( MsgInitCONFIG( Header( 0, MsgType.NODECONFIG, Host["Id"], NodeTrackCircuit["Id"]), 2, 2, Route( 2, Host["Id"], NodePoint["Id"], NodePosition.FIRST, PointPosition.UNDEFINED ) ), MsgInitCONFIGFormat)
	client_socket.send(messageToSend)

	# Close the socket
	client_socket.shutdown(socket.SHUT_RDWR)

def sendConfig2PT():
	##############################################
	# Create a client socket to NodePoint       #
	##############################################
	client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	client_socket.connect((NodePoint["IP"], NodeCommPort))

	# Send config
	# Node type
	messageToSend = getMessageToSend( MsgInitCONFIGTYPE( Header( 0, MsgType.NODECONFIG, Host["Id"], NodePoint["Id"]), 0, 2, NodeType.POINT ), MsgInitCONFIGTYPEFormat)
	client_socket.send(messageToSend)

	# Track 1
	messageToSend = getMessageToSend( MsgInitCONFIG( Header( 0, MsgType.NODECONFIG, Host["Id"], NodePoint["Id"]), 1, 2, Route( 1, NodeTrackCircuit["Id"], NodeNull["Id"], NodePosition.LAST, PointPosition.DIVERGING ) ), MsgInitCONFIGFormat)
	client_socket.send(messageToSend)

	# Track 2
	messageToSend = getMessageToSend( MsgInitCONFIG( Header( 0, MsgType.NODECONFIG, Host["Id"], NodePoint["Id"]), 2, 2, Route( 2, NodeTrackCircuit["Id"], NodeNull["Id"], NodePosition.LAST, PointPosition.STRAIGHT ) ), MsgInitCONFIGFormat)
	client_socket.send(messageToSend)

	# Close the socket
	client_socket.shutdown(socket.SHUT_RDWR)

def requestRoute(routeId: int):
	##############################################
	# Create a client socket to NodeTrackCircuit #
	##############################################
	client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	client_socket.connect((NodeTrackCircuit["IP"], NodeCommPort))

	# Send config
	# Route request
	messageToSend = getMessageToSend( MsgRouteREQ( Header( 0, MsgType.ROUTEREQ, Host["Id"], NodeTrackCircuit["Id"]), routeId ), MsgRouteREQFormat)
	client_socket.send(messageToSend)
        
	# Close the socket
	client_socket.shutdown(socket.SHUT_RDWR)
        
def requestRouteWaitForResponse(routeId: int) -> bool:
	# Create a server socket
	server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

	try:
		server_socket.settimeout(10) # timeout for listening
		server_socket.bind((Host["IP"], NodeCommPort))
		server_socket.listen()

		# Connect an incoming client to the server
		try:
			#Accept any incoming client connection or timeout
			client_socket, client_address = server_socket.accept()
			print(f"Connected with {client_address}...")
			# Receive data
			data = client_socket.recv(1024)

			# Message unpacking
			header = struct.unpack(MsgHeaderFormat,  data[0:16])
			# TODO Check il correct length and conversion ok
			header = Header._make(header)

			# If data received
			match header.type:
				case MsgType.ROUTETRAINOK.value:
					payload = struct.unpack(MsgRouteTRAINOKFormat, data )
					message = MsgRouteTRAINOK._make([header, payload[4]])

					# Check the response refere to excepted route id
					if message.requestRouteId != routeId:
						return False
				
				case MsgType.ROUTETTRAINOK.value:
					payload = struct.unpack(MsgRouteTRAINOKFormat, data )
					message = MsgRouteTRAINOK._make([header, payload[4]])

					# Check the response refere to excepted route id
					if message.requestRouteId != routeId:
						return False							
		except Exception as e: 
			print(e)
			return False
	finally:
		server_socket.close()

	return True
