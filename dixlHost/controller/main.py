"""
@author         : "Alessandro Mannini"
@organization   : "Università degli Studi di Firenze"
@contact        : "alessandro.mannini@gmail.com"
@date           : "Jan 10, 2023"
@version        : "1.0.0"
"""
from pubsub import pub
from model.layout import Layout
from model.node import Node
from model.route import Route

import message
import utility

import view.main as View

class   Main():
    """ 
    Controller Main

    Subscribe events using Pypubsub library with these topics:
        LAYOUT OPERATIONS:
        - view.layout.open
        - view.node.reset
        - view.node.config
        - view.node.refreship
        - view.node.log
        - view.node.malfunction
        - view.route.request
        - node.update.state
        - node.update.log
    """         
    # Constructor
    def __init__(self, model: Layout, view: View.Main) -> None:
        self.model: Layout = model
        self.view: View.Main = view

        # Getting local host IP
        print("Trying to acquire local IP...")
        self.hostIPstr = message.getLocaIP()
        self.hostIP = utility.str2IP(self.hostIPstr)
        self.view.hostIP(self.hostIPstr)

        # Subscribe view topics
        pub.subscribe(self.viewOpenLayout, 'view.layout.open')
        pub.subscribe(self.viewNodeReset, 'view.node.reset')
        pub.subscribe(self.viewNodeConfigure, 'view.node.config')
        pub.subscribe(self.viewNodeRefreshIP, 'view.node.refreship')
        pub.subscribe(self.viewNodeLog, 'view.node.log')
        pub.subscribe(self.viewNodeClearLog, 'view.node.clearlog')
        pub.subscribe(self.viewNodeMalfunction, 'view.node.malfunction')
        pub.subscribe(self.viewRouteRequest, 'view.route.request')
        pub.subscribe(self.nodeUpdateState, 'node.update.state')
        pub.subscribe(self.nodeUpdateIP, 'node.update.IP')
        pub.subscribe(self.nodeUpdateLog, 'node.update.log')
        pub.subscribe(self.routeUpdateState, 'route.update.state')

    # Methods
    # LAYOUT hooks
    def viewOpenLayout(self, filename: str) -> bool:
        # Read layout from file
        if not filename: return False
        ll: Layout = Layout.ReadFromFile(filename=filename)

        # Set the view with the new Layout
        if ll: 
            self.model = ll
            self.view.setLayout(ll)
            for id,node in ll.nodes.items(): self.view.addNode(node)
            for id,route in ll.routes.items(): self.view.addRoute(route)

        return True

    # NODE hooks
    def nodeUpdateState(self, node: Node) -> None:
        # Notify node state update to view
        if node: self.view.write_event_value('NODE.UPDATE.STATE', node)

    def nodeUpdateIP(self, node: Node) -> None:
        # Notify node IP update to view
        if node: self.view.write_event_value('NODE.UPDATE.IP', node)

    def nodeUpdateLog(self, node: Node) -> None:
        # Notify node Log update to view
        if node: self.view.write_event_value('NODE.UPDATE.LOG', node)

    def viewNodeReset(self, nodeId: str) -> bool:
        # Check node ID
        if not nodeId: return False

        # Find node instance
        node: Node = self.model.nodes.get(nodeId, None)
        if not node: return False

        # Call the function
        return node.reset(self.hostIP)

    def viewNodeConfigure(self, nodeId: str) -> bool:
        # Check node ID
        if not nodeId: return False

        # Find node instance
        node: Node = self.model.nodes.get(nodeId, None)
        if not node: return False

        # Call the function
        return node.configure(self.hostIP)

    def viewNodeRefreshIP(self, nodeId: str) -> bool:
        # Check node ID
        if not nodeId: return False

        # Find node instance
        node: Node = self.model.nodes.get(nodeId, None)
        if not node: return False

        # Call the function
        return node.refreshIP()
    
    def viewNodeLog(self, nodeId: str) -> bool:
        # Check node ID
        if not nodeId: return False

        # Find node instance
        node: Node = self.model.nodes.get(nodeId, None)
        if not node: return False

        # Visualize current Log on the view before request for update
        self.nodeUpdateLog(node)

        # Create a dictionary to bind node IP to node ID
        nodes = self.model.nodes
        IDDict = {node.IP:k for k, node in nodes.items()}
        # Add host
        IDDict[self.hostIP] = 'Host'

        # Call the function
        return node.requestLog(self.hostIP, IDDict)

    def viewNodeClearLog(self, nodeId: str) -> bool:
        # Check node ID
        if not nodeId: return False

        # Find node instance
        node: Node = self.model.nodes.get(nodeId, None)
        if not node: return False

        # Call the function
        return node.clearLog()

    def viewNodeMalfunction(self, nodeId: str, state: bool) -> bool:
        # Check node ID
        if not nodeId: return False

        # Find node instance
        node: Node = self.model.nodes.get(nodeId, None)
        if not node: return False
        
        # Set new malfunction simulation state
        node.malfunction = state

        return True
    
    # ROUTE hooks    
    def routeUpdateState(self, route: Route) -> None:
        # Notify route state update to view
        if route: self.view.write_event_value('ROUTE.UPDATE.STATE', route)

    def viewRouteRequest(self, routeId: int) -> bool:
        # Check route ID
        if not routeId: return False

        # Find route instance
        route: Route = self.model.routes.get(routeId, None)
        if not route: return False

        # Call the function
        return route.request(self.hostIP)
