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
from collections import Iterable
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

#Enums
class MsgType(Enum):
	# Service messages - Init task
	NODERESET 					= 10	# Reset in the Init state
	NODECONFIG 					= 11	# Routes configuration sent by the host
	IMSGTYPE_NODECONFIGSET      = 12,   # (Internal) Set config in dixlCtrl task
	IMSGTYPE_NODECONFIGRESET    = 13,   # (Internal) Reset config in dixlCtrl task
	NODEDISCOVERY 				= 20	# TODO Nodes discovery from the host
	NODEADVERTISE 				= 21	# TODO Node advertise reply to discovery
        
	# Route messages - Ctrl task
	ROUTEREQ 			= 50	# Route request
	ROUTEACK 			= 51	# 2PhaseCommit Ack
	ROUTEAGREE 			= 52	# 2PhaseCommit Agree
	ROUTEDISAGREE 		= 53	# 2PhaseCommit Disagree
	ROUTECOMMIT 		= 54	# 2PhaseCommit Commit
	
	# Diagnostic messages - TODO
	NODEDIAG 			= 90	# Communication diagnostic request
	NODEDIAGACK 		= 91	# Communication diagnostic request Ack
	
	# Log messages - TODO
	LOG 				= 120	# Log a message
	LOGREQ				= 121	# Request current log messages
	LOGSEND				= 122	# Response current log messages
	LOGDEL				= 123	# Ack messages were received and ask to delete
	LOGDELACK			= 124	# Ack messages were deleted


# Node type 
class NodeType(Enum):
	SWITCH 				= 10	# Switch (Deviatoio)
	TRACKCIRCUIT 		= 20	# Track Circuit (Cdb o Circuito di Binario)

# Node position */
class NodePosition(Enum):
	FIRST 				= -128	# First node of the route
	MIDDLE				= 0  	# Node between two other nodes of the route
	LAST				= 127   # Last node of the route

# Switch position */
class SwitchPosition(Enum):
    UNDEFINED			= -1    # Undefined (for Track Circuits)
    STRAIGHT 			= 0	    # Stright direction switch
    DIVERGING			= 1 	# DIverging direction switch

# Messages defs
Header = namedtuple("Header", [ "length", "type" , "source", "destination"])
Route = namedtuple("Route", ["IP", "prev", "next", "position", "requestedPosition"])
MsgInitCONFIGTYPE = namedtuple("MsgHeaderCONFIG", ["header", "sequence", "totalSegments", "nodeType"])
MsgInitCONFIG = namedtuple("MsgHeaderCONFIG", ["header", "sequence", "totalSegments", "route"])

# Packed messages formats
MsgHeaderFormat = "BB4s4s"
MsgRouteFormat = "I4s4sbb"
MsgSequenceTotalFormat = "II"
MsgNodeTypeFormat = "B"
MsgInitCONFIGTYPEFormat = MsgHeaderFormat + MsgSequenceTotalFormat + MsgNodeTypeFormat
MsgInitCONFIGFormat = MsgHeaderFormat + MsgSequenceTotalFormat + MsgRouteFormat

# Host and Nodes IDs and IPs
# TODO Detect MAC and IP of the host
Host 		 	 = { "Id": bytes([192, 168, 173, 121]), "MAC": b"\x7C\x76\x35\xF0\x99\xC0", "IP": "192.168.173.121" }
NodeNull	 	 = { "Id": bytes([0, 0, 0, 0]), "MAC": b"\x00\x00\x00\x00\x00\x00", "IP": "192.168.173.121"  }
NodeTrackCircuit = { "Id": bytes([192, 168, 173, 80]), "MAC": b"\x7a\x7a\xc0\xa8\xad\x50", "IP": "192.168.173.80"  }
NodeSwitch 		 = { "Id": bytes([192, 168, 173, 90]), "MAC": b"\x7a\x7a\xc0\xa8\xad\x5A", "IP": "192.168.173.90"  }
NodeCommPort = 256

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
messageToSend = getMessageToSend( MsgInitCONFIG( Header( 0, MsgType.NODECONFIG, Host["Id"], NodeTrackCircuit["Id"]), 1, 2, Route( 1, Host["Id"], NodeSwitch["Id"], NodePosition.FIRST, SwitchPosition.UNDEFINED ) ), MsgInitCONFIGFormat)
client_socket.send(messageToSend)

# Track 2
messageToSend = getMessageToSend( MsgInitCONFIG( Header( 0, MsgType.NODECONFIG, Host["Id"], NodeTrackCircuit["Id"]), 2, 2, Route( 2, Host["Id"], NodeSwitch["Id"], NodePosition.FIRST, SwitchPosition.UNDEFINED ) ), MsgInitCONFIGFormat)
client_socket.send(messageToSend)

# Close the socket
client_socket.shutdown(socket.SHUT_RDWR)
#client_socket.close()

##############################################
# Create a client socket to NodeSwitch       #
##############################################
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect((NodeSwitch["IP"], NodeCommPort))

# Send config
# Node type
messageToSend = getMessageToSend( MsgInitCONFIGTYPE( Header( 0, MsgType.NODECONFIG, Host["Id"], NodeSwitch["Id"]), 0, 2, NodeType.SWITCH ), MsgInitCONFIGTYPEFormat)
client_socket.send(messageToSend)

# Track 1
messageToSend = getMessageToSend( MsgInitCONFIG( Header( 0, MsgType.NODECONFIG, Host["Id"], NodeSwitch["Id"]), 1, 2, Route( 1, NodeTrackCircuit["Id"], NodeNull["Id"], NodePosition.LAST, SwitchPosition.DIVERGING ) ), MsgInitCONFIGFormat)
client_socket.send(messageToSend)

# Track 2
messageToSend = getMessageToSend( MsgInitCONFIG( Header( 0, MsgType.NODECONFIG, Host["Id"], NodeSwitch["Id"]), 2, 2, Route( 2, NodeTrackCircuit["Id"], NodeNull["Id"], NodePosition.LAST, SwitchPosition.STRAIGHT ) ), MsgInitCONFIGFormat)
client_socket.send(messageToSend)

# Close the socket
client_socket.shutdown(socket.SHUT_RDWR)
#client_socket.close()
