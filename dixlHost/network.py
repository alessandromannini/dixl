"""
@author         : "Alessandro Mannini"
@organization   : "Università degli Studi di Firenze"
@contact        : "alessandro.mannini@gmail.com"
@date           : "Jan 10, 2023"
@version        : "1.0.0"   
"""
from scapy.all import Ether, ARP, srp1, conf
import ipaddress
import netifaces
from utility import *
from typing import TYPE_CHECKING
if TYPE_CHECKING:
	from model.node import Node
from model.node import NodeState

def getIPfromMAC(node: 'Node') -> bool:
    """
    Retrive the IP address from the MAC
    Parameters:
    - MAC: MAC address of the device to find the IP of (format xx:xx:xx:xx:xx:xx)

    Return:
    - IP address of the device or None
    """
    try:

        # Get the interface
        iface=netifaces.ifaddresses(conf.iface.guid)
        MAC: str = MAC2str(node.MAC)
        # Retrieve subnet (xxx.xxx.xxx.xxx/bits)
        for i,v in iface.items():
            if i>0:
                add = str(ipaddress.ip_interface(v[0]['addr'].rsplit(".",1)[0] + '.0/' + v[0]['netmask']).network)
                break

        # Send an ARP request to all subnet IP with target MAC address set to get the IP address associated with
        arp_request = Ether(dst=MAC)/ARP(pdst=f'{add}')
        arp_response = srp1(arp_request, timeout=2, verbose=0)

        if not arp_response or arp_response.hwsrc != MAC: 
            node.resetRequest(NodeState.FAIL)
            return False

        # Update node IP
        node.IP = str2IP(arp_response.psrc)
        node.resetRequest(NodeState.OK)
                
        return True
    
    except Exception as ex:
        node.resetRequest(NodeState.FAIL)
        print(f'Error retrieving IP of node {MAC}: {ex}')