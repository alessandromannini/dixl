"""
@author         : "Alessandro Mannini"
@organization   : "Università degli Studi di Firenze"
@contact        : "alessandro.mannini@gmail.com"
@date           : "Jan 10, 2023"
@version        : "1.0.0"
"""
from node_config_item import NodeConfigItem, NodePosition
from collections.abc import Sequence

from typing import TYPE_CHECKING
if TYPE_CHECKING:
    from node import Node
    from point import PointPosition

class NodeConfig(Sequence[NodeConfigItem]):

    # Constructor    
    def __init__(self):
        self._dict: dict[NodeConfigItem] = {}

    # Properties

    # Methods        
    def __getitem__(self, index: int) -> NodeConfigItem:
        return self._dict[index]
    
    def __len__(self):
        return len(self._dict)
    
    def clear(self) -> None:
        self._dict.clear()

    # Add or Update a NodeConfigItem
    def RouteSet(self, id: int, prev: 'Node', next: 'Node', position: NodePosition, requestedPos: 'PointPosition' = 'PointPosition.UNDEFINED') -> None:
        try:
            item: NodeConfigItem = self._dict[id]
            item.prev = prev
            item.next = next
            item.position = position
            item.requestedPos = requestedPos

        except:
            self._dict[id] = NodeConfigItem(id, prev, next, position, requestedPos)

    def RouteDel(self, id: int) -> None:
        if id in self._dict:
            raise KeyError()
        
        del(self._dict[id])