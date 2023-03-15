# dixl 
## _Experimental implementation through a network of Raspberry controllers of a distributed railway interlocking system_

### Main folders structure

<pre>
<b>dixl</b>
├───configs
│     vxsim_network.conf        # Configuration for Wind River Network Daemon for VxWorks Simulator (vxsimnetds)
│       
├───<b>dixlHost                    # HOST APPPLICATION EXPERIMENTS</b>
│     dixlHostGUI.py            # (minimal) _Desktop GUI_
│     dixlHostTest.py           # Base communication function and test data
│     requirements.txt          # Web interface app python packages requirements
│     run.py                    # Web interface app start script
│           
└───<b>dixlNode                    # NODE SOFTWARE</b>
  │   config.h                  # Configurable parameters
  │   dkm.c                     # Start module                    
  │   globals.h                 # Shared variables
  │   network.h                 # Networks utilities
  │   utils.h                   # General purpose utilities
  │   
  ├───datatypes
  │   dataHelpers.c             # Data helper functions
  │   dataTypes.h               # Data types definitions
  │   messages.h                # External and internal messages definitions
  │       
  ├───FSM
  │   FSMCtrlPOINT.h            # FSM that define tCtrl task behaviour when operating as a Point controller
  │   FSMCtrlTRACKCIRCUIT.h     # FSM that define tCtrl task behaviour when operating as a Track Circuit controller
  │   FSMInit.h                 # FSM that define tInit task behaviour
  │       
  └───tasks                     # Task set
      dixlComm.h                # Communication Tx and Rx tasks common definitions
      dixlCtrl.h                # Control logic task
      dixlDiag.h                # Diagnostic task
      dixlInit.h                # Initialization task
      dixlLog.h                 # Logger task
      dixlPoint.h               # Point simulation task
</pre>

### dixlNode
Contains the Wind River Workbench - VxWoks project of the software for the nodes.

### dixlHost
Contains two sub-projects:

- #### Desktop GUI
The GUI minimal and contains the basic commands to RESET, CONFIG and REQUEST a route to two preconfigured nodes.
The GUI can be started running the Python file dixlHostGUI.py

- #### Web Application prototype
The folders contains a (static) prototype of a possibile single-page responsive Web interface for the Host.
The web application is written with HTML, CSS, js and Bootstrap for the frontend and python with Flask for the backend.
It can be started running the Python script run.py.
