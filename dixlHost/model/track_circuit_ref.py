from model.node_ref import NodeRef

class TrackCircuitRef(NodeRef):
    from model.node import Node
    
    # Constructor    
    def __init__(self, node: Node) -> None:
        super().__init__(node)

    # Properties

    # Methods
       