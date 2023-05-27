# _(DIXL) Experimental implementation of a distributed railway interlocking system through a network of Raspberry controllers._
Author: Alessandro Mannini

## Abstract
A fundamental element of railway signaling is interlocking, which is the safety-critical system that controls train movement in stations and between adjacent stations, preventing conflicts between trains and avoiding the two main types of railway failure: collision and derailment.
The classical centralized implementation of interlocking has some limitations due to its global and monolithic nature.
In this thesis, to overcome these limitations, a new concept of distributed interlocking system is analyzed, in which processing is distributed among nodes connected in a network, each associated with a railway element (track circuit or point). The safety functions of the interlocking system emerge as a result of the collective cooperation of the nodes.
Through the formulation of a model that defines global and node-related specifications, a Proofof-Concept has been implemented to demonstrate the actual feasibility of the system.
Finally, the system has been verified on a series of test scenarios that both confirmed the basic safety properties of interlocking and highlighted some critical issues.

## Introduction
This is the code of my thesis that implements a prototype of a railway distributed interlocking system.
The repo includes two main applications:
- NODE, developed in C for the RTOS VxWorks is loaded on Raspberry Pi 4 and implement a node of the distributed interlocking system;
- HOST, developed in Python allow to send route requests to the nodes and other basic management functions.

### Main folders structure

<pre>
<b>dixl</b>
├─── configs
│     ├─── QEMU-dixlTc-tap.bat       # QEMU node 1 launch script (for Windows)
│     ├─── QEMU-dixlTc-tap2.bat      # QEMU node 2 launch script (for Windows)
│     └─── vxsim_network.conf        # Configuration for Wind River Network Daemon for VxWorks Simulator (vxsimnetds)
│       
├─── <b>dixlHost                        # HOST APPPLICATION</b>
│     ├─── controller
│     │  └─── main.py                # MVC - CONTROLLER
│     │
│     ├─── images                    # Images folder
│     │ 
│     ├─── model                     # MVC - MODEL
│     │  ├─── layout.py              # Layout
│     │  ├─── log_line.py            # Line of log 
│     │  ├─── node.py                # Node
│     │  ├─── node_config.py         # Node configuration set
│     │  ├─── node_config_item.py    # Node configuration item
│     │  ├─── node_ref.py            # Node reference
│     │  ├─── point.py               # Point
│     │  ├─── point_ref.py           # Point reference
│     │  ├─── route.py               # Route
│     │  ├─── track_circuit.py       # Track circuit
│     │  └─── track_circuit_ref.py   # Track circuit reference
│     │
│     ├─── view
│     │  └─── main.py                # MVC - VIEW
│     │
│     ├─── config.py                 # Application configuration file
│     ├─── dixlHostGUI.py            # Main appplication file (to run)
│     ├─── json_encoder.py           # JSON encoder helper
│     ├─── main_2nodes.dixl          # Two nodes dixl layouts
│     ├─── main_fill.dixl            # Complete dixl layout
│     ├─── message.py                # Message defs and helper functions
│     ├─── network.py                # Network helper functions
│     ├─── requirements.txt          # Application python packages requirements
│     └─── utility.py                # Utility functions
│           
├─── <b>dixlNode                        # NODE SOFTWARE</b>
│     ├─── datatypes
│     │  ├─── dataHelpers.c          # Data helper functions
│     │  ├─── dataTypes.h            # Data types definitions
│     │  └─── messages.h             # External and internal messages definitions
│     │       
│     ├─── FSM
│     │  ├─── FSMCtrlPOINT.h         # FSM that define tCtrl task behaviour when operating as a Point controller
│     │  ├─── FSMCtrlTRACKCIRCUIT.h  # FSM that define tCtrl task behaviour when operating as a Track Circuit controller
│     │  └─── FSMInit.h              # FSM that define tInit task behaviour
│     │       
│     ├─── includes
│     │  ├─── hw.h                   # GPIO management
│     │  ├─── network.h              # Network utilities
│     │  ├─── ntp.h                  # NTP basic client
│     │  └─── utils.h                # General purpose utilities
│     │       
│     └─── tasks                     # Task set
│       ├─── dixlComm.h              # Communication Tx and Rx tasks common definitions
│       ├─── dixlCtrl.h              # Control logic task
│       ├─── dixlDiag.h              # Diagnostic task
│       ├─── dixlInit.h              # Initialization task
│       ├─── dixlLog.h               # Logger task
│       ├─── dixlPoint.h             # Point simulation task
│       └─── dixlSensor.h            # Sensor checker task
│
├─── <b>miscStuff                      # HOST WEB APPPLICATION EXPERIMENTS</b>
│
├─── build.sh                        # CLI compiler build script
├─── config.h                        # Configurable parameters
├─── dkm.c                           # Main kernel module
├─── ftpd.sh                         # CLI ftp script
├─── globals.h                       # Shared variables 
├─── version.h                       # Versioning file
│    
</pre>

### dixlNode
Contains the Wind River Workbench - VxWoks project of the software for the <b>nodes</b>.
Run on Raspberry Pi 3/4 booted with the VxWorks SDK.

### dixlHost
Contains the <b>Host</b> application.
