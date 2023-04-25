"""
@author         : "Alessandro Mannini"
@organization   : "Università degli Studi di Firenze"
@contact        : "alessandro.mannini@gmail.com"
@date           : "Jan 10, 2023"
@version        : "1.0.0"
"""
# imports
from enum import Enum
from model.node import Node, NodeType

# Point position */
class PointPosition(Enum):
    UNDEFINED			= -1    # Undefined (for Track Circuits or Malfunction)
    STRAIGHT 			= 0	    # Stright direction
    DIVERGING			= 50 	# DIverging direction

class Point(Node):
    # Constructor    
    def __init__(self, id: int, MAC: bytes, IP: bytes = None) -> None:
        super().__init__(id, MAC, IP)
        
    # Properties
    @property
    def type(self) -> NodeType:
        return NodeType.POINT

    # Methods
    def to_json(self)->dict:
        dd: dict = super().to_json()
        dd["type"] = self.type.name
        return dd
