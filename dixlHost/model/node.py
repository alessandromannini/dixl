"""
@author         : "Alessandro Mannini"
@organization   : "Università degli Studi di Firenze"
@contact        : "alessandro.mannini@gmail.com"
@date           : "Jan 10, 2023"
@version        : "1.0.0"
"""
# imports
from abc import abstractproperty
from enum import Enum
import random
import threading
from pubsub import pub

import config
from model.node_config import NodeConfig

# Node type 
class NodeType(Enum):
    POINT 				= 10	# Point (Deviatoio)
    TRACKCIRCUIT 		= 20	# Track Circuit (Cdb o Circuito di Binario)
        
class NodeState(Enum):
    UNKNOWUN			= -1	# No operations performed
    OK 		            = 1	    # Last operation OK
    PENDING 		    = 99    # Last request is pending
    FAIL                = 0     # Last operation FAILED       

NodeNull: bytes = b'\x00\x00\x00\x00'                

class Node(object):
    """ 
    Node object

    Notify events using Pypubsub library with these topics:
        NODE OPERATIONS:
        - node.update.status
    """
    # Constructor    
    def __init__(self, id: str, MAC: bytes, IP: bytes = None) -> None:

        # Check params
        self._id: str = id
        self._MAC: bytes = MAC
        self.state: NodeState = NodeState.UNKNOWUN
        if IP:
            self._IP = IP
        else:
            self._IP: bytes = bytes([0, 0, 0, 0])
        self.log: list[str] = []    
        self.malfunction: bool = False                          # Malfunction simulation enabled        
        self.__config: NodeConfig = NodeConfig()
        self.__lock: threading.Lock = threading.Lock()

    # Properties
    @property
    def id(self) -> str:
        return self._id

    @property
    def MAC(self) -> bytes:
        return self._MAC

    @property
    def IP(self) -> bytes:
        return self._IP
    @IP.setter
    def IP(self, v: bytes) -> None:
        self._IP = v
        pub.sendMessage('node.update.IP', node=self)

    @property
    def Config(self) -> 'NodeConfig':
        return self.__config

    @abstractproperty
    def type(self) -> NodeType:
        pass

    # Methods
    def to_json(self)->dict:
        return {    "id": self.id, 
                    "MAC": ":".join([ "{:02x}".format(v) for v in self.MAC]),
                    "IP": ".".join([ int(v) for v in self.IP]),
                }

    def __setRequest(self) -> bool:
        """
        Check if a previous request is pending. If not, set it
        """
        # Acquire exclusive access to state
        self.__lock.acquire()
        try:
            # Operations pendind ?
            if self.state == NodeState.PENDING: return False

            # If not set it
            self.state = NodeState.PENDING
            pub.sendMessage('node.update.state', node=self)

            return True
        
        finally:
            self.__lock.release()

    def resetRequest(self, state: NodeState) -> None:
        """
        Set the new state, reset current pending operation and notify 
        """
        # Acquire exclusive access to state
        self.__lock.acquire()
        try:
            # Set new state
            self.state = state
            pub.sendMessage('node.update.state', node=self)

        finally:
            self.__lock.release()

    def reset(self, hostIP: bytes) -> bool:
        """
        Send a reset message to the node (thread safe)
        Parameters:
            - hostIP: IP string of the host
        """
        import message
        # Other operations pending ?
        if not self.__setRequest(): return False
        
        # Start the request in a new thread
        try:
            t = threading.Thread(target=message.sendReset, kwargs={'hostIP':hostIP, 'node': self}) 
            t.start()

        except Exception as ex:
            # Reset current operation if FAIL to create the thread
            self.resetRequest(NodeState.FAIL)

            # Rethrow the exception    
            raise ex
        
    def configure(self, hostIP: bytes) -> bool:
        """
        Send the config sequence messages to the node (thread safe)
        Parameters:
            - hostIP: IP string of the host
        """
        import message
        # Other operations pending ?
        if not self.__setRequest(): return False
        
        # Start the request in a new thread
        try:
            t = threading.Thread(target=message.sendConfig, kwargs={'hostIP': hostIP, 'node': self}) 
            t.start()

        except Exception as ex:
            # Reset current operation if FAIL to create the thread
            self.resetRequest(NodeState.FAIL)

            # Rethrow the exception    
            raise ex
    
    def refreshIP(self) -> bool:
        """
        Send ARP request to get fresh IP from MAC address (thread safe)
        """
        import network
        # Other operations pending ?
        if not self.__setRequest(): return False
        
        # Start the request in a new thread
        try:
            t = threading.Thread(target=network.getIPfromMAC, kwargs={'node': self}) 
            t.start()

        except Exception as ex:
            # Reset current operation if FAIL to create the thread
            self.resetRequest(NodeState.FAIL)

            # Rethrow the exception    
            raise ex
        
    
    def requestLog(self, hostIP: bytes, IDDict: dict[bytes, str]) -> bool:
        """
        Send a Log request to the node and wait for reply (thread safe)
        """
        import message
        # Other operations pending ?
        if not self.__setRequest(): return False
        
        # Start the request in a new thread
        try:
            t = threading.Thread(target=message.requestLog, kwargs={'hostIP': hostIP, 'node': self, 'IDDict': IDDict}) 
            t.start()

        except Exception as ex:
            # Reset current operation if FAIL to create the thread
            self.resetRequest(NodeState.FAIL)

            # Rethrow the exception    
            raise ex        
    
    def clearLog(self) -> bool:
        """
        Clear Log and notify (thread safe)
        """
        # Other operations pending ?
        if not self.__setRequest(): return False
        
        # Start the request in a new thread
        try:
            # Clear log lines
            self.log.clear()

            # Notify
            pub.sendMessage("node.update.log", node=self)

            # Reset request
            self.resetRequest(NodeState.OK)

        except Exception as ex:
            # Reset current operation if FAIL to create the thread
            self.resetRequest(NodeState.FAIL)

            # Rethrow the exception    
            raise ex        
    
    def simulateMalfunction(self, hostIP: bytes, delayed: bool = True) -> bool:
        """
        Send a Malfunction simulation request to the node (thread safe)
        """
        import message
        # Other operations pending ?
        if not self.__setRequest(): return False
        
        # Start the request in a new thread
        try:
            # Random delay
            if delayed:
                delay: int = 0
            else:
                delay: int = random.randint(0, 2000)

            # Spawn the thread                
            t = threading.Thread(target=message.sendMalfunction, kwargs={'hostIP': hostIP, 'node': self, 'delay': delay}) 
            t.start()

        except Exception as ex:
            # Reset current operation if FAIL to create the thread
            self.resetRequest(NodeState.FAIL)

            # Rethrow the exception    
            raise ex  