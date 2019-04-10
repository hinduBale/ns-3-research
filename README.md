# ns-3-research
Here I'll collect all the research I do on ns-3.


My primary objective is to complete the patch work present [here](https://www.nsnam.org/wiki/GSOC2018PatchRequirement).

## Brief Explanation of the Patch Work:

  We are tasked with creating a **custom L3 Protocol with the Protocol Number 99**. Examples of some [L3 protocols](http://www.cs.miami.edu/home/burt/learning/Csc688.012/lecture1/)  protocols are as follows:
1. IPv4/ IPv6
2. ICMP
3. ARP etcetera

  *The objective of this patch is to add a new, simple, basic, L3 protocol, with the protocol number 99. The new protocol implementation should be registered on the node, and it should be
  able to send a ping packet, and receive a response. That is, it does not need to do routing or introduce any new addressing scheme; it merely 
  needs to be able to send a 'ping-like' packet to the NetDevice, and receive a packet (destined to protocol 99) from a NetDevice. A simple
  print on the screen at the send and receive moment is enough to show a working implementation.*
  
### Concise Action Plan:
-----------------------
  
1. Study the IPv4 protocol implementation from the ns-3 codebase.
2. Try and understand the protocol, it's implementation and then make relevant changes.
  *Actual changes will be listed in the detailed description as and when they are made.
3. Create nodes using the *NodeContainer::Create()* and install a protocol stack on it using  
*InternetStackHelper::Install()*.
4. By default, [InternetStackHelper](https://www.nsnam.org/docs/release/3.10/manual/html/internet-stack.html) installs the following protocols on a NetDevice which itself is
installed on a node.
  *Arpv3Protocol
  *Ipv4L3Prtocol
  *Icmpv4L4protocol
  *UdpL4Protocol
5. We need to add/replace our custom protocol (**RadeepProtocol**) in the InternetStackHelper function to 
get our protocol installed on the node.
6. After the installation of **Radeep** on the node, we will use the *UdpEchoHelper* class to send recieve 
the ping messages.
  
