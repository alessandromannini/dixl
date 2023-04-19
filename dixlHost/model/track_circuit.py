"""
@author         : "Alessandro Mannini"
@organization   : "Università degli Studi di Firenze"
@contact        : "alessandro.mannini@gmail.com"
@date           : "Jan 10, 2023"
@version        : "1.0.0"
"""
# imports
from model.node import Node, NodeType

class TrackCircuit(Node):
    # Constructor    
    def __init__(self, id: int, MAC: bytes) -> None:
        super().__init__(id, MAC)

    @property
    def type(self) -> NodeType:
        return NodeType.TRACKCIRCUIT

    def to_json(self)->dict:
        dd: dict = super().to_json()
        dd["type"] = self.type.name
        return dd

    # Properties

    # Methods