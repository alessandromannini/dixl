"""
@author         : "Alessandro Mannini"
@organization   : "Università degli Studi di Firenze"
@contact        : "alessandro.mannini@gmail.com"
@date           : "Jan 10, 2023"
@version        : "1.0.0"
"""
# imports
from node import Node
from route import Route
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

    def WriteToDisk(self, pathname: str = None, filename: str = None) -> bool:
        # Check if filename was passed and extension
        if filename is None: filename = utility.filename_sanitize(self.id)
        if os.path.splitext(filename)[1] == "": filename += ".dixl"

        # Check if path was given
        if pathname is not None: filename = pathname + os.path.sep + filename

        #
        coded: str = json.dumps(self, cls=JSONEncoderCustom, indent=4)
            
        with open(filename, "w") as file:
            file.write(coded)

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
