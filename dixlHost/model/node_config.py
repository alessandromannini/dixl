"""
@author         : "Alessandro Mannini"
@organization   : "Università degli Studi di Firenze"
@contact        : "alessandro.mannini@gmail.com"
@date           : "Jan 10, 2023"
@version        : "1.0.0"
"""
from model.node_config_item import NodeConfigItem, NodePosition
from collections.abc import Sequence

from typing import TYPE_CHECKING
if TYPE_CHECKING:
    from node import Node
    from point import PointPosition

class NodeConfig(Sequence[NodeConfigItem]):

    # Constructor    
    def __init__(self):
        # Data list
        self._list: list[NodeConfigItem] = []
        # Dictionary for direct access 2*O(1)
        self._dict: dict[int, int] = {}

    # Properties

    # Methods        
    def __getitem__(self, index: int) -> NodeConfigItem:
        return self._list[index]
    def getByRoute(self, routeId: int) -> NodeConfigItem:
        return self._list[self._dict[routeId]]
    
    def __len__(self):
        return len(self._dict)
    
    def clear(self) -> None:
        self._list.clear()
        self._dict.clear()

    # Add or Update a NodeConfigItem
    def RouteSet(self, routeId: int, prev: 'Node', next: 'Node', position: NodePosition, requestedPos: 'PointPosition' = 'PointPosition.UNDEFINED') -> None:
        try:
            # Sarch in dict
            itemIdx: int = self._dict.get[routeId]
            item: NodeConfigItem = self._list[itemIdx]
            item.prev = prev
            item.next = next
            item.position = position
            item.requestedPos = requestedPos

        except:
            # Not present: create, add to list and add index to dict
            self._dict[routeId] = len(self._list)
            self._list.append(NodeConfigItem(routeId, prev, next, position, requestedPos))


    def RouteDel(self, routeId: int) -> None:
        if id not in self._dict:
            raise KeyError()
        
        itemIdx: int = self._dict[routeId]
        del(self._dict[routeId])
        del(self._list[itemIdx])