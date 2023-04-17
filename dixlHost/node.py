"""
@author         : "Alessandro Mannini"
@organization   : "Università degli Studi di Firenze"
@contact        : "alessandro.mannini@gmail.com"
@date           : "Jan 10, 2023"
@version        : "1.0.0"
"""
# imports
from abc import ABC, abstractproperty
from enum import Enum
from node_config import NodeConfig

# Node type 
class NodeType(Enum):
    POINT 				= 10	# Point (Deviatoio)
    TRACKCIRCUIT 		= 20	# Track Circuit (Cdb o Circuito di Binario)
        
class NodeState(Enum):
    UNKNOWUN			= -1	# No operations performed
    OK 		            = 1	    # Last operation OK
    FAIL                = 0     # Last operation FAILED       
        
class Node(object):

    # Constructor    
    def __init__(self, id: str, MAC: bytes) -> None:

        # Check params
        self._id: str = id
        self._MAC: bytes = MAC
        self.state: NodeState = NodeState.UNKNOWUN
        self.IP: bytearray = [0, 0, 0, 0]
        self.__config: NodeConfig = NodeConfig()

    # Properties
    @property
    def id(self) -> str:
        return self._id

    @property
    def MAC(self) -> bytes:
        return self._MAC

    @property
    def Config(self) -> 'NodeConfig':
        return self.__config

    @abstractproperty
    def type(self) -> NodeType:
        pass

    # Methods
    def to_json(self)->dict:
        return {    "id": self.id, 
                    "MAC": ":".join([ "{:02x}".format(v) for v in self.MAC]) 
                }

# TODO
#def Reset
#Config
#Malfunction
#request