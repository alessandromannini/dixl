"""
@author         : "Alessandro Mannini"
@organization   : "Università degli Studi di Firenze"
@contact        : "alessandro.mannini@gmail.com"
@date           : "Jan 10, 2023"
@version        : "1.0.0"
"""
# imports
from enum import Enum
from model.node import Node
from model.node_config_item import NodePosition
from model.node import Node
from model.node_ref import NodeRef
from model.point_ref import PointRef

import threading
from pubsub import pub
from typing import Iterable
from collections.abc import MutableSequence

# Route types
class RouteState(Enum):
    """Route State values:"""
    UNKNOWUN			= -1	# No operations performed
    OK 		            = 1	    # Last operation OK
    PENDING             = 99    # Last operation is pending
    FAIL                = 0     # Last operation FAILED       

class ReservationState(Enum):
    """Reservation State values:"""
    UNKNOWN			    = -1	# No reservation request performed
    TRAINOK 		    = 1	    # Last reservation request was TRAINOK
    TRAINNOK 		    = 10    # Last reservation request was TRAINNOK
    PENDING 		    = 99    # Last reservation request is pending
    FAIL                = 0     # Last operation FAILED       

class Route(MutableSequence[NodeRef]):
    """
    Represent a route that the train can request    
    """
    # Constructor    
    def __init__(self, id: int, description = None, iterable: Iterable[NodeRef] = None) -> None:
        self.__id: int = id
        self.description: str = description
        self.__nodes: list[NodeRef] = list()
        self.state: RouteState = RouteState.UNKNOWUN
        self.reservation: ReservationState = ReservationState.UNKNOWN
        self.__lock: threading.Lock = threading.Lock()

        # If an iterable is passed create the List
        if iterable is not None:
            # Add all nodeReg
            for nodeRef in iterable:
                self.__nodes.append(nodeRef)
            
            # Generate the config of the nodes
            self.__generateNodesConfig()        

    # Properties
    @property
    def id(self) -> int:
        return self.__id
    
    # Methods    
    def typecheck(self, v: NodeRef):
        if not isinstance(v, NodeRef):
            raise TypeError(v)

    def __getNodeConfigAttributes(self, index: int) -> tuple[Node, Node, NodePosition]:
        # Position
        position: NodePosition = NodePosition.MIDDLE

        # Prev
        if index == 0:
            position = NodePosition.FIRST
            prev: Node = None
        else:
            prev: Node = self.__nodes[index - 1].node

        # Next
        if index == (len(self.__nodes) - 1) :
            position = NodePosition.LAST
            next: Node = None
        else:
            next: Node = self.__nodes[index + 1].node

        return prev, next, position
        
    def __setNodeConfig(self, index: int, nodeRef: NodeRef) -> None:
        prev, next, position = self.__getNodeConfigAttributes(index)

        # Is a reference to Point?
        if isinstance(nodeRef, PointRef):
            nodeRef.node.Config.RouteSet(self.id, prev, next, position, nodeRef.requestedPos )
        else:
            nodeRef.node.Config.RouteSet(self.id, prev, next, position )                


    def __generateNodesConfig(self) -> None:
        count: int = 0
        for index, nodeRef in enumerate(self.__nodes):
            self.__setNodeConfig(index, nodeRef)

    def to_json(self)->dict:
        # Id
        dd: dict = {    "id" : self.id,
                        "description": self.description}

        # List of nodeRef
        nodeRefs = []
        for nodeRef in self.__nodes:
            nodeRefs.append(nodeRef.to_json())
        dd['nodeRefs'] = nodeRefs

        return dd    

    # Abstract methods
    def __len__(self): return len(self.__nodes)

    def __getitem__(self, index: int) -> NodeRef: return self.__nodes[index]

    def __delitem__(self, index: int):
        # Save reference to node to update
        node: Node = self.__nodes[index].node
        
        # Delete the route from the node config
        node.Config.RouteDel(self.id)
        
        # Delete the route from the list
        del self.__nodes[index]

        # Update previous (if present)
        if index > 0:
            self.__setNodeConfig((index - 1), self.__nodes[index -1])

        # Update next (if present): index point node next to the node deleted
        if index < len(self.__nodes):
            self.__setNodeConfig((index), self.__nodes[index])


    def __setitem__(self, index: int, v: NodeRef):
        self.typecheck(v)

        # In __setitem__ the item exist (or I get and exception)
        # Save reference to node to update
        node: Node = self.__nodes[index].node
        
        # Delete the route from the node config
        node.Config.RouteDel(self.id)

        # Update the position with the new node
        self.__nodes[index] = v

        # Update the node config
        self.__setNodeConfig(index, self.__nodes[index])

        # Update previous (if present)
        if index > 0:
            self.__setNodeConfig((index - 1), self.__nodes[index -1])

        # Update next (if present): 
        index += 1
        if index < len(self.__nodes):
            self.__setNodeConfig((index), self.__nodes[index])


    def insert(self, index: int, v: NodeRef):
        self.typecheck(v)
        self.__nodes.insert(index, v)

        # Update inserted node config
        self.__setNodeConfig(index, self.__nodes[index])

        # Update previous (if present)
        if index > 0:
            self.__setNodeConfig((index - 1), self.__nodes[index -1])

        # Update next (if present): 
        index += 1
        if index < len(self.__nodes):
            self.__setNodeConfig((index), self.__nodes[index])


    def __str__(self):
        return str(self.__nodes)

    def __setRequest(self) -> bool:
        """
        Check if a previous request is pending. If not, set it
        """
        # Acquire exclusive access to state
        self.__lock.acquire()
        try:
            # Operations pendind ?
            if self.state == RouteState.PENDING: return False

            # If not set it
            self.state = RouteState.PENDING
            pub.sendMessage('route.update.state', route=self)

            return True
        
        finally:
            self.__lock.release()

    def resetRequest(self, state: RouteState) -> None:
        """
        Set the new state, reset current pending operation and notify 
        """
        # Acquire exclusive access to state
        self.__lock.acquire()
        try:
            # Set new state
            self.state = state
            pub.sendMessage('route.update.state', route=self)

        finally:
            self.__lock.release()    
    
    def request(self, hostIP: bytes) -> bool:
        """
        Send route request to First node and wait for a reply (TRAINOK/TRAINNOK)
        """
        import message
        # Other operations pending ?
        if not self.__setRequest(): return False
        
        # Start the request in a new thread
        try:
            t = threading.Thread(target=message.sendRequest, kwargs={'hostIP':hostIP,'route': self}) 
            t.start()

        except Exception as ex:
            # Reset current operation if FAIL to create the thread
            self.resetRequest(RouteState.FAIL)

            # Rethrow the exception    
            raise ex    
