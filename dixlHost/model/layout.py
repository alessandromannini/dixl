"""
@author         : "Alessandro Mannini"
@organization   : "Università degli Studi di Firenze"
@contact        : "alessandro.mannini@gmail.com"
@date           : "Jan 10, 2023"
@version        : "1.0.0"
"""
# imports
from model.node import Node, NodeType
from model.point import Point, PointPosition
from model.track_circuit import TrackCircuit
from model.node_ref import NodeRef
from model.route import Route
from model.point_ref import PointRef
from model.track_circuit_ref import TrackCircuitRef
import os
import json
from json_encoder import JSONEncoderCustom
import utility


class Layout(object):
    # Constructor    
    def __init__(self, id: str, description: str) -> None:
        self.id = id
        self.description = description
        self.nodes = dict[str, Node]()
        self.routes = dict[int, Route]()
        
    # Methods
    def NodeAdd(self, node: Node) -> None:
        self.nodes[node.id] = node

    def RouteAdd(self, route: Route) -> None:
        self.routes[route.id] = route

    def to_json(self)->dict:
        # Id
        dd: dict = {"id" : self.id}

        # Description
        dd["description"] = self.description

        # List of nodes
        nodes = []
        for key, node in self.nodes.items():
            nodes.append(node.to_json())
        dd['nodes'] = nodes

        # List of routes
        routes = []
        for key, route in self.routes.items():
            routes.append(route.to_json())
        dd['routes'] = routes

        return dd
    
    @staticmethod
    def from_json(json_dict: dict)  -> 'Layout':
        # Id
        if 'id' in json_dict.keys():
            id: str = json_dict['id']
        else:
            raise KeyError()
        
        # Description
        if 'description' in json_dict.keys():
            description: str = json_dict['description']
        else:
            raise KeyError()

        # Layout obj
        ll: Layout = Layout(id, description)
        
        # Nodes
        if 'nodes' in json_dict.keys():
            nodes: list() = json_dict['nodes']
        else:
            raise KeyError()

        for node in nodes:
            id: str = node['id']    
            MAC: bytes = utility.str2MAC(node['MAC'])
            type: str = node['type']
            if type == NodeType.POINT.name:
                ll.NodeAdd(Point(id, MAC))
            else:
                ll.NodeAdd(TrackCircuit(id, MAC))

        # Routes
        if 'routes' in json_dict.keys():
            routes: list() = json_dict['routes']
        else:
            raise KeyError()
        
        for route in routes:            
            id: int = route['id']
            nodeRefs: list() = route['nodeRefs']
            nodeRefs_list = list[NodeRef]() 
            for nodeRef in nodeRefs:
                node: Node = ll.nodes[nodeRef['id']]
                if isinstance(node, Point):
                    if nodeRef['requestedPosition'] == PointPosition.STRAIGHT.name:
                        requestedPos = PointPosition.STRAIGHT
                    else:
                        requestedPos = PointPosition.DIVERGING
                    nodeRefs_list.append(PointRef(node, requestedPos))
                else:
                    nodeRefs_list.append(TrackCircuitRef(node))    
            
            route_obj = Route(id, nodeRefs_list)
            ll.RouteAdd(route_obj)
                        
        return ll

    def WriteToFile(self, pathname: str = None, filename: str = None) -> bool:
        # Check if filename was passed and extension
        if filename is None: filename = utility.filename_sanitize(self.id)
        if os.path.splitext(filename)[1] == "": filename += ".dixl"

        # Check if path was given
        if pathname is not None: filename = pathname + os.path.sep + filename

        # JSON coded
        coded: str = json.dumps(self, cls=JSONEncoderCustom, indent=4)
            
        with open(filename, "w") as file:
            file.write(coded)
    
    @staticmethod
    def ReadFromFile(pathname: str = None, filename: str = None) -> 'Layout':
        # Check if filename was passed and extension
        if filename is None: raise ValueError("Filename excepted")
        if os.path.splitext(filename)[1] == "": filename += ".dixl"

        # Check if path was given
        if pathname is not None: filename = pathname + os.path.sep + filename
           
        coded: str = ''
        with open(filename, "r") as file:
            coded = "".join(file.readlines())
        
        coded = json.loads(coded)

        return Layout.from_json(coded)

