"""
@author         : "Alessandro Mannini"
@organization   : "Università degli Studi di Firenze"
@contact        : "alessandro.mannini@gmail.com"
@date           : "Jan 10, 2023"
@version        : "1.0.0"
"""
from enum import Enum
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from node import Node
    from point import PointPosition

# Point types
class NodePosition(Enum):
	FIRST 				= -128	# First node of the route
	MIDDLE				= 0  	# Node between two other nodes of the route
	LAST				= 127   # Last node of the route   
                
class NodeConfigItem(object):

    # Constructor    
    def __init__(self, routeId: int,  prev: 'Node', next: 'Node' , position: NodePosition, requestedPos: 'PointPosition') -> None:
        self._routeId: int = routeId
        self._position: NodePosition = position
        self._prev: 'Node' = prev
        self._next: Node = next
        self._requestedPos: PointPosition

    # Properties
    @property
    def routeId(self) -> int:
        return self._routeId

    @property
    def position(self) -> NodePosition:
        return self._position
    
    @property
    def prev(self) -> 'Node':
        return self._prev

    @property
    def next(self) -> 'Node':
        return self._next

    @property
    def requestedPos(self) -> 'PointPosition':
        return self._requestedPos

    # Methods
    def to_json(self)->dict:
        # TODO
        dd: dict = super().to_json()
        return {"node": self.node }
