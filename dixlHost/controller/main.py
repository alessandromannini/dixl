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

import view.main as View

class   Main():
    # Constructor
    def __init__(self, model: Layout, view: View.Main) -> None:
        self.model: Layout = model
        self.view: View.Main = view

        # Subscribe view topics
        pub.subscribe(self.openLayout, 'view.layout.open')

    # Methods
    def openLayout(self, filename: str) -> bool:
        # Read layout from file
        if not filename: return False
        ll: Layout = Layout.ReadFromFile(filename=filename)

        # Set the view with the new Layout
        if ll: 
            self.view.setLayout(ll)
            for id,node in ll.nodes.items(): self.view.addNode(node)
            for id,route in ll.routes.items(): self.view.addRoute(route)

        return True

