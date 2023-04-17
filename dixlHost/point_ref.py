"""
@author         : "Alessandro Mannini"
@organization   : "Università degli Studi di Firenze"
@contact        : "alessandro.mannini@gmail.com"
@date           : "Jan 10, 2023"
@version        : "1.0.0"
"""
from enum import Enum
from node_ref import NodeRef

# Point types
class PointPosition(Enum):
    UNDEFINED			= -1    # Undefined (for Track Circuits or Malfunction)
    STRAIGHT 			= 0	    # Stright direction
    DIVERGING			= 50 	# DIverging direction

class PointRef(NodeRef):
    from node import Node

    # Constructor    
    def __init__(self, node: Node, requestedPosition: PointPosition) -> None:
        super().__init__(node)
        self._requestedPos = requestedPosition

    # Properties
    @property
    def requestedPos(self) -> PointPosition:
        return self._requestedPos

    # Methods
    def to_json(self)->dict:
        dd: dict = super().to_json()
        dd[ "requestedPosition"] = self.requestedPos
        return dd            