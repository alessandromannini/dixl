"""
@author         : "Alessandro Mannini"
@organization   : "Università degli Studi di Firenze"
@contact        : "alessandro.mannini@gmail.com"
@date           : "Jan 10, 2023"
@version        : "1.0.0"
"""
import PySimpleGUI as sg
import os
from pubsub import pub

from  model.layout import Layout
from  model.route import Route
from  model.node import Node
from model.node import NodeState
from  utility import *

class Main(sg.Window):
    """ 
    View Main window

    Notify events using Pypubsub library with these topics:
        LAYOUT OPERATIONS:
        - view.layout.open
# TODO
        - view.layout.refreship
        - view.layout.reset
        - view.layout.config
        
        ROUTE OPERATIONS:
        - view.route.request

        NODE OPERATIONS:
        - view.node.refreship
        - view.node.reset
        - view.node.config
        - view.node.log
        - view.node.malfunction
    """
    # Constructor
    def __init__(self):
        # Initialize layout
        layout = self.__layout_base()
        self.__layout: Layout = layout
        self.__nodeId: list[int] = list()
        self.__routeId: list[str] = list()
        super().__init__('dixlHost',layout=layout, margins=(400,300), finalize=True)

    # Properties
    @property
    def layout(self) -> Layout:
        return self.__layout

    # Statics
    def absolutePath(relative: str) -> str:
        script_dir: str = os.path.dirname(__file__) #<-- absolute dir the script is in
        return os.path.join(script_dir, relative)
    
    # Methods pprivate
    def __layout_base(self):

        working_dir = os.getcwd()

        host_row = [ [
                sg.Text('Host IP', font="bold"),
                sg.Text('', key='HOST.IP', size=(15,1), pad=(7,50), text_color='black', background_color='#C9E4E7', justification='center')
            ]
        ]

        layout_row = [ [
                sg.Text('Layout', font="bold"),
                sg.Text('', expand_x=True, key='LAYOUT.ID', size=(10,1), text_color='black', background_color='#C9E4E7'),
                sg.Text('', expand_x=True, key='LAYOUT.DESCRIPTION', size=(50,1), text_color='black', background_color='#C9E4E7'),
            ],
            [
                sg.Text('', expand_x=True, key='LAYOUT.FILE', text_color='black', background_color='#C9E4E7'),
                sg.Input(visible=False, enable_events=True, key='LAYOUT.FILE.IN'),
                sg.FileBrowse('Open', initial_folder=working_dir, target='LAYOUT.FILE.IN', file_types=(('DIXL layout', '*.dixl'),), key='LAYOUT.FILE.OPEN')
            ]
        ]


        nodes_column = [ [ 
                sg.Frame('Nodes', expand_x=True, key='FRAME.NODES', font="bold", layout=
                            # header
                            [[
                                sg.Text("ID", key=f"NODE.LBL.IP", size=(5,1), pad=((36,0),(7,7)),text_color='black', background_color='#C9E4E7'),
                                sg.Text("MAC", key=f"NODE.LBL.MAC", size=(15,1), pad=((1,0),(7,7)),text_color='black', background_color='#C9E4E7'),
                                sg.Text("IP", key=f"NODE.LBL.IP", size=(15,1), pad=((1,0),(7,7)),text_color='black', background_color='#C9E4E7'),
                                sg.Text("MAL", key=f"NODE.LBL.MALFUNC", size=(4,1), pad=((1,7),(7,7)),text_color='black', background_color='#C9E4E7')
                            ]]                                     
                        )]]    

        routes_column = [ [
                sg.Frame('Routes', expand_x=True, key='FRAME.ROUTES', font="bold", layout=
                            # header
                            [[
                                sg.Text("ID", key=f"ROUTE.LBL.IP", size=(5,1), pad=((36,0),(7,7)), text_color='black', background_color='#C9E4E7'),
                                sg.Text("DESCRIPTION", key=f"NODE.LBL.DESC", size=(30,1), pad=((1,7),(7,7)),text_color='black', background_color='#C9E4E7')                            ]]                                     
                        )]]    

        log_row = [ [
                sg.Frame('Log',layout=[[]], expand_x=True, key='FRAME.LOG'),
            ]
        ]

        layout =    [   host_row,
                        layout_row,
                        [
                            sg.Column(nodes_column),
                            sg.VSeperator(),
                            sg.Column(routes_column),
                        ],
                        log_row
                    ]
    
        return layout
    
    # Methods public
    def eventLoop(self) -> None:
        while True:
            # Wait for events
            event, values = self.read()

            # Loop until Exit or Close            
            if event == 'WIN.BTN.EXIT' or event == sg.WIN_CLOSED: break

            # Event analysis
            match event:
                case 'LAYOUT.FILE.IN':
                    filename: str = values['LAYOUT.FILE.OPEN']
                    self['LAYOUT.FILE'].update(filename)
                    pub.sendMessage('view.layout.open', filename=filename)

                case 'NODE.UPDATE.STATE':
                    self.setNodeStatus(values[event])

                case 'NODE.UPDATE.IP':
                    self.setNodeIP(values[event])

                case _:
                    # NODE Reset
                    if event.startswith('NODE.BTN.RESET.'):
                        pub.sendMessage('view.node.reset', nodeId=event.rsplit(".", 1)[1])
                    # NODE Config
                    elif event.startswith('NODE.BTN.CONFIG.'):
                        pub.sendMessage('view.node.config', nodeId=event.rsplit(".", 1)[1])
                    # NODE Refresh IP
                    elif event.startswith('NODE.BTN.IP.'):
                        pub.sendMessage('view.node.refreship', nodeId=event.rsplit(".", 1)[1])
                    # NODE Log
                    elif event.startswith('NODE.BTN.LOG.'):
                        pub.sendMessage('view.node.log', nodeId=event.rsplit(".", 1)[1])
                    # NODE Malfunction
                    elif event.startswith('NODE.BTN.MALFUNC.'):
                        pub.sendMessage('view.node.malfunction', nodeId=event.rsplit(".", 1)[1])
                    # ROUTE Request
                    elif event.startswith('ROUTE.BTN.REQUEST.'):
                        pub.sendMessage('view.route.request', nodeId=event.rsplit(".", 1)[1])
                    pass

        # Close the window
        self.close()

    def setLayout(self, layout: Layout) -> None:
        # Head - Layout
        self['LAYOUT.ID'].update(layout.id)
        self['LAYOUT.DESCRIPTION'].update(layout.description)

        # Nodes clean (skip first)
        for widget in self['FRAME.NODES'].widget.winfo_children()[1:]:
            widget.pack_forget()
            widget.destroy()

        # Routes clean (skip first)
        for widget in self['FRAME.ROUTES'].widget.winfo_children()[1:]:
            widget.pack_forget()
            widget.destroy()

    def addRoute(self, route: Route) -> None:
        # Prepare the row
        route_id = route.id
        route_row = [   
                         sg.Image(filename=Main.absolutePath('../images/status_led_inactive.png'), size=(20,20), subsample=2, key=f"ROUTE.{route_id}"),
                        sg.Text(route_id, key=f"ROUTE.TXT.ID.{route_id}", size=(5,1), pad=((6,0),(3,1)), text_color='black', background_color='#EEF292'),
                        sg.Text(route.description, key=f"ROUTE.TXT.DESC.{route_id}", size=(30,1), pad=((1,7),(3,1)), text_color='black', background_color='#EEF292'),
                        sg.Button("REQUEST", key=f"ROUTE.BTN.REQUEST.{route_id}")
                    ]
        self.extend_layout(self['FRAME.ROUTES'], [ route_row ])

        # Store the id
        self.__routeId.append(route_id)


    def addNode(self, node: Node) -> None:
        # Prepare the row
        node_id = node.id
        node_row =  [   
                        sg.Image(filename=Main.absolutePath('../images/status_led_inactive.png'), key=f'NODE.IMG.{node_id}', size=(20,20), subsample=2),
                        sg.Text(node_id, key=f"NODE.TXT.ID.{node_id}", size=(5,1), pad=((6,0),(3,1)), text_color='black', background_color='#34D1BF'),
                        sg.Text(MAC2str(node.MAC), key=f"NODE.TXT.MAC.{node_id}", size=(15,1), pad=((1,0),(3,1)), text_color='black', background_color='#34D1BF'),
                        sg.Text(IP2str(node.IP), key=f"NODE.TXT.IP.{node_id}", size=(15,1), pad=((1,0),(3,1)), text_color='black', background_color='#34D1BF'),
                        sg.Checkbox('',key=f"NODE.CHK.MALFUNC.{node_id}", pad=((10,7),(3,1)), text_color='black'),
                        sg.Button("IP", key=f"NODE.BTN.IP.{node_id}"),
                        sg.Button("RESET", key=f"NODE.BTN.RESET.{node_id}"),
                        sg.Button("CONFIG", key=f"NODE.BTN.CONFIG.{node_id}"),
                        sg.Button("LOG", key=f"NODE.BTN.LOG.{node_id}")
                    ]
        self.extend_layout(self['FRAME.NODES'], [ node_row ])

        # Store the id
        self.__nodeId.append(node_id)

    def hostIP(self, hostIP: str) -> None:
        # Set host IP
        self['HOST.IP'].update(hostIP)

    def setNodeStatus(self, node: Node) -> None:
        match node.state:
            case NodeState.PENDING:
                self[f'NODE.IMG.{node.id}'].update(filename=Main.absolutePath('../images/status_led_yellow.png'), size=(20,20), subsample=2)

            case NodeState.FAIL:
                self[f'NODE.IMG.{node.id}'].update(filename=Main.absolutePath('../images/status_led_red.png'), size=(20,20), subsample=2)

            case NodeState.OK:
                self[f'NODE.IMG.{node.id}'].update(filename=Main.absolutePath('../images/status_led_green.png'), size=(20,20), subsample=2)                

            case _:
                self[f'NODE.IMG.{node.id}'].update(filename=Main.absolutePath('../images/status_led_inactive.png'), size=(20,20), subsample=2)

    def setNodeIP(self, node: Node) -> None:
        self[f'NODE.TXT.IP.{node.id}'].update(IP2str(node.IP))
