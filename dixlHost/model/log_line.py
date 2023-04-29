"""
@author         : "Alessandro Mannini"
@organization   : "Università degli Studi di Firenze"
@contact        : "alessandro.mannini@gmail.com"
@date           : "Jan 10, 2023"
@version        : "1.0.0"
"""
# Imports
from enum import Enum
import utility

# Message types
class LogType(Enum):
	"""
	Type of messages logged in the nodes
	"""
	REQ 				= 10	# Request received
	OCCUPIED			= 11	# Track occupied
	REQNACK				= 12	# Request NACKed
	DISAGREE			= 13	# Request DISAGREEed
	RESERVED			= 14	# Request AGREEed
	NOTRESERVED			= 99	# Not Reserved

class LogLine():
	"""
	One line of log
	"""
	def __init__(self, timestamp_sec: int, timestamp_nsec: int, type: LogType, routeId: int = 0, nodeId: str = None, nodeIP: bytes = b'\x00\x00\x00\x00') -> None:
		self._timestamp: dict() = { 'sec' : timestamp_sec, 'nsec' : timestamp_nsec }
		self.type: LogType = type
		self.routeId: int = routeId
		self.nodeId: str = nodeId
		self.nodeIP: bytes = nodeIP

	@property
	def timestamp(self) -> dict():
		return self._timestamp

	@timestamp.setter
	def timestamp(self, sec: int, nsec: int):
		self._timestamp['sec'] = sec
		self._timestamp['nsec'] = nsec

	def __str__(self) -> str:
		out: str = f"{(self.timestamp['sec'] + self.timestamp['nsec'] / 1.0e+09):.4f} - {self.type.name}:"

		match self.type:
			case LogType.REQ:
				out += f' route {self.routeId} requested from node'
				if self.nodeId:
					out += f' {self.nodeId} ({utility.IP2str(self.nodeIP)})'
				else:
					out += f' {utility.IP2str(self.nodeIP)}'
					
			case LogType.OCCUPIED:
				out += f' track segment occupied by the train'					

			case LogType.REQNACK:
				out += f' route {self.routeId} request from node'
				if self.nodeId:
					out += f' {self.nodeId} ({utility.IP2str(self.nodeIP)})'
				else:
					out += f' {utility.IP2str(self.nodeIP)}'
				out += ' not ACKnowledge'

			case LogType.DISAGREE:
				out += f' route {self.routeId} request from node'
				if self.nodeId:
					out += f' {self.nodeId} ({utility.IP2str(self.nodeIP)})'
				else:
					out += f' {utility.IP2str(self.nodeIP)}'
				out += ' DISagreed'

			case LogType.RESERVED:
				out += f' node reserved for route {self.routeId}'

			case LogType.NOTRESERVED:
				out += f' node in NOT reserved state'

		return out	



