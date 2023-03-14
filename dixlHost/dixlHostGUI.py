"""
@author         : "Alessandro Mannini"
@organization   : "Università degli Studi di Firenze"
@contact        : "alessandro.mannini@gmail.com"
@date           : "Jan 10, 2023"
@version        : "1.0.0"
"""
import tkinter as tk
from typing_extensions import IntVar
from dixlHostTest import *
from threading import *
import time
import random


blinking = Event()

def request(routeId: int):
    # Wait for a response from the first node (while) blinking

    try:
        res = False
        res = requestRouteWaitForResponse(routeId)

    except:
        print("Wait Request Response FAILED!!!")

    finally:
        blinking.clear()

        if res:
            lblRouteStatus.config(bg= "green")
        else:
            lblRouteStatus.config(bg= "red")
        btnRTRequest.config(state="normal")     

def malfunction(delay: int):
    # Send a malfunction message after delay ms

    try:
        time.sleep(delay / 1000)
        
        sendMalfunction2PT()

    except Exception as e:
        print("Malfunction send error" + repr(e))

def buttonPressed(action: str):

    global chkMalfunction

    match action:
        case "TCSendConfig":
            sendConfig2TC()

        case "TCReset":
            sendReset2TC()

        case "PTSendConfig":
            sendConfig2PT()

        case "PTReset":
            sendReset2PT()

        case "RequestRoute":
            idxs = lstRoute.curselection()
            if len(idxs)==1:
                # Get the route ID
                routeId = int(idxs[0]) + 1

                # Send route request
                requestRoute(routeId)

                # Disable button
                btnRTRequest.config(state="disabled")                     

                # Start blink 
                blinking.set()
                lblRouteStatus.after(500, blinkTRAINStatus)         
                
                # Start request (wait) in a second thread
                Thread(target=request, args=[routeId]).start()

                # Get Malfunction checkbox
                if chkMalfunction.get():
                    # Random delay (ms)
                    delay=random.randint(0, 2000)
                    Thread(target=malfunction, args=[delay]).start()
        case _:
            print("Unexcepted command")

def blinkTRAINStatus():

    if not blinking.is_set():
        return
    
    col = lblRouteStatus.cget("background")
    if col == "yellow":
        lblRouteStatus.config(bg= "white")
    else:
        lblRouteStatus.config(bg= "yellow")

    lblRouteStatus.after(500, blinkTRAINStatus)
        

# Create the wndMain
wndMain = tk.Tk()
wndMain.title("dixlHostGUI - Distributed Interlocking Host")
wndMain.rowconfigure([1, 3, 5], weight = 1)
wndMain.columnconfigure([3, 4], weight = 1)
wndMain.resizable(False, False)

# TRACK CIRCUIT NODE
# Title
frmTC = tk.Frame(master=wndMain, width=200, height=50, relief=tk.FLAT, bg="yellow")
frmTC.grid(row=0, column=0)
tk.Label(master=frmTC, text="Track Circuit Node", bg="yellow").grid(row=0, column=0)

# Commands row
frmTCCmds = tk.Frame(master=frmTC, relief=tk.SUNKEN, borderwidth=5, bg="yellow")
frmTCCmds.grid(row=1, column=0)
# IP
tk.Label(master=frmTCCmds, text="IP:", bg="yellow").grid(row=1, column=0, sticky="e")
tk.Label(master=frmTCCmds, text=NodeTrackCircuit["IP"], bg="yellow").grid(row=1, column=1, sticky="e")
# Reset
btnTCReset = tk.Button(
    master=frmTCCmds,
    text="Reset",
    width=15,
    command=lambda: buttonPressed("TCReset")
)
btnTCReset.grid(row=1, column=2, padx=5, pady=5)
# Send Config
btnTCSendConfig = tk.Button(
    master=frmTCCmds,
    text="Send config",
    width=15,
    command=lambda: buttonPressed("TCSendConfig")
)
btnTCSendConfig.grid(row=1, column=3, padx=5, pady=5)

# POINT NODE
frmPT = tk.Frame(master=wndMain, width=200, height=20,relief=tk.FLAT,  bg="orange")
frmPT.grid(row=2, column=0)
tk.Label(master=frmPT, text="Point Node", bg="orange").grid(row=2, column=0)

# Commands row
frmPTCmds = tk.Frame(master=frmPT, relief=tk.SUNKEN, borderwidth=5, bg="orange")
frmPTCmds.grid(row=3, column=0)
# IP
tk.Label(master=frmPTCmds, text="IP:", bg="orange").grid(row=3, column=0, sticky="e")
tk.Label(master=frmPTCmds, text=NodePoint["IP"], bg="orange").grid(row=3, column=1, sticky="e")
# Reset
btnPTReset = tk.Button(
    master=frmPTCmds,
    text="Reset",
    width=15,
    command=lambda: buttonPressed("PTReset")
)
btnPTReset.grid(row=3, column=2, padx=5, pady=5)
# Send Config
btnPTSendConfig = tk.Button(
    master=frmPTCmds,
    text="Send config",
    width=15,
    command=lambda: buttonPressed("PTSendConfig")
)
btnPTSendConfig.grid(row=3, column=3, padx=5, pady=5)


# ROUTE
frmRT = tk.Frame(master=wndMain, width=200, height= 30, relief=tk.FLAT,  bg="lightgray")
frmRT.grid(row=4, column=0)
tk.Label(master=frmRT, text="Routes", bg="lightgray").grid(row=4, column=0)
# Commands row
frmRTCmds = tk.Frame(master=frmRT, relief=tk.SUNKEN, borderwidth=5, bg="lightgray")
frmRTCmds.grid(row=5, column=0)
# Route
tk.Label(master=frmRTCmds, text="Route:", bg="lightgray").grid(row=5, column=0, sticky="e")
values = ["Route 1", "Route 2"]
valuesVar = tk.StringVar(value=values)
lstRoute = tk.Listbox(master=frmRTCmds, listvariable=valuesVar, height=2)
lstRoute.grid(row=5, column=1, sticky="n")

# Malfunction checkbox
chkMalfunction = tk.IntVar()
tk.Checkbutton(master=frmRTCmds, text="Malfunction:", bg="lightgray", variable=chkMalfunction).grid(row=5, column=2, padx=5)

# Request route
btnRTRequest = tk.Button(
    master=frmRTCmds,
    text="Request route",
    width=15,
    command=lambda: buttonPressed("RequestRoute")
)
btnRTRequest.grid(row=5, column=3, padx=5, pady=5)

# Route Status
lblRouteStatus=tk.Label(master=frmRTCmds, text="", width=1, height=1, bg="white")
lblRouteStatus.grid(row=5, column=4, padx=5, pady=5, sticky="nesw")

# Wait for events
wndMain.mainloop()
#wndMain.destroy()
