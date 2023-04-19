"""
@author         : "Alessandro Mannini"
@organization   : "Università degli Studi di Firenze"
@contact        : "alessandro.mannini@gmail.com"
@date           : "Jan 10, 2023"
@version        : "1.0.0"
"""
from model.node import Node

class NodeRef(object):
    # Constructor    
    def __init__(self, node: Node) -> None:
        self._node = node

    # Properties
    @property
    def node(self) -> Node:
        return self._node

    # Methods
    def to_json(self)->dict:
        return  {   "id": self.node.id
                }
