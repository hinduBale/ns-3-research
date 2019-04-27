#include "ns3/packet.h"
#include "ns3/log.h"
#include "ns3/callback.h"
#include "ns3/Radeep-address.h"
#include "ns3/Radeep-route.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/net-device.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/object-vector.h"
#include "ns3/Radeep-header.h"
#include "ns3/boolean.h"
#include "ns3/Radeep-routing-table-entry.h"
#include "ns3/traffic-control-layer.h"

#include "loopback-net-device.h"
#include "arp-l3-protocol.h"
#include "arp-cache.h"
#include "radeep-l3-protocol.h"
#include "icmpv4-l4-protocol.h"
#include "Radeep-interface.h"
#include "Radeep-raw-socket-impl.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RadeepL3Protocol");

const uint16_t RadeepL3Protocol::PROT_NUMBER = 0x63;   //Here the Protocol number has been set to 99.

NS_OBJECT_ENSURE_REGISTERED (RadeepL3Protocol);

TypeId 
RadeepL3Protocol::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RadeepL3Protocol")
    .SetParent<Radeep> ()
    .SetGroupName ("Internet")
    .AddConstructor<RadeepL3Protocol> ()
    .AddAttribute ("DefaultTtl",
                   "The TTL value set by default on "
                   "all outgoing packets generated on this node.",
                   UintegerValue (64),
                   MakeUintegerAccessor (&RadeepL3Protocol::m_defaultTtl),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("FragmentExpirationTimeout",
                   "When this timeout expires, the fragments "
                   "will be cleared from the buffer.",
                   TimeValue (Seconds (30)),
                   MakeTimeAccessor (&RadeepL3Protocol::m_fragmentExpirationTimeout),
                   MakeTimeChecker ())
    .AddTraceSource ("Tx",
                     "Send radeep packet to outgoing interface.",
                     MakeTraceSourceAccessor (&RadeepL3Protocol::m_txTrace),
                     "ns3::RadeepL3Protocol::TxRxTracedCallback")
    .AddTraceSource ("Rx",
                     "Receive radeep packet from incoming interface.",
                     MakeTraceSourceAccessor (&RadeepL3Protocol::m_rxTrace),
                     "ns3::RadeepL3Protocol::TxRxTracedCallback")
    .AddTraceSource ("Drop",
                     "Drop radeep packet",
                     MakeTraceSourceAccessor (&RadeepL3Protocol::m_dropTrace),
                     "ns3::RadeepL3Protocol::DropTracedCallback")
    .AddAttribute ("InterfaceList",
                   "The set of Radeep interfaces associated to this Radeep stack.",
                   ObjectVectorValue (),
                   MakeObjectVectorAccessor (&RadeepL3Protocol::m_interfaces),
                   MakeObjectVectorChecker<RadeepInterface> ())

    .AddTraceSource ("SendOutgoing",
                     "A newly-generated packet by this node is "
                     "about to be queued for transmission",
                     MakeTraceSourceAccessor (&RadeepL3Protocol::m_sendOutgoingTrace),
                     "ns3::RadeepL3Protocol::SentTracedCallback")
    .AddTraceSource ("UnicastForward",
                     "A unicast Radeep packet was received by this node "
                     "and is being forwarded to another node",
                     MakeTraceSourceAccessor (&RadeepL3Protocol::m_unicastForwardTrace),
                     "ns3::RadeepL3Protocol::SentTracedCallback")
    .AddTraceSource ("LocalDeliver",
                     "An Radeep packet was received by/for this node, "
                     "and it is being forward up the stack",
                     MakeTraceSourceAccessor (&RadeepL3Protocol::m_localDeliverTrace),
                     "ns3::RadeepL3Protocol::SentTracedCallback")

  ;
  return tid;
}

RadeepL3Protocol::RadeepL3Protocol()
{
  NS_LOG_FUNCTION (this);
}

RadeepL3Protocol::~RadeepL3Protocol ()
{
  NS_LOG_FUNCTION (this);
}

void
RadeepL3Protocol::Insert (Ptr<RadeepL4Protocol> protocol)
{
  NS_LOG_FUNCTION (this << protocol);
  L4ListKey_t key = std::make_pair (protocol->GetProtocolNumber (), -1);
  if (m_protocols.find (key) != m_protocols.end ())
    {
      NS_LOG_WARN ("Overwriting default protocol " << int(protocol->GetProtocolNumber ()));
    }
  m_protocols[key] = protocol;
}

void
RadeepL3Protocol::Insert (Ptr<RadeepL4Protocol> protocol, uint32_t interfaceIndex)
{
  NS_LOG_FUNCTION (this << protocol << interfaceIndex);

  L4ListKey_t key = std::make_pair (protocol->GetProtocolNumber (), interfaceIndex);
  if (m_protocols.find (key) != m_protocols.end ())
    {
      NS_LOG_WARN ("Overwriting protocol " << int(protocol->GetProtocolNumber ()) << " on interface " << int(interfaceIndex));
    }
  m_protocols[key] = protocol;
}

void
RadeepL3Protocol::Remove (Ptr<RadeepL4Protocol> protocol)
{
  NS_LOG_FUNCTION (this << protocol);

  L4ListKey_t key = std::make_pair (protocol->GetProtocolNumber (), -1);
  L4List_t::iterator iter = m_protocols.find (key);
  if (iter == m_protocols.end ())
    {
      NS_LOG_WARN ("Trying to remove an non-existent default protocol " << int(protocol->GetProtocolNumber ()));
    }
  else
    {
      m_protocols.erase (key);
    }
}

void
RadeepL3Protocol::Remove (Ptr<RadeepL4Protocol> protocol, uint32_t interfaceIndex)
{
  NS_LOG_FUNCTION (this << protocol << interfaceIndex);

  L4ListKey_t key = std::make_pair (protocol->GetProtocolNumber (), interfaceIndex);
  L4List_t::iterator iter = m_protocols.find (key);
  if (iter == m_protocols.end ())
    {
      NS_LOG_WARN ("Trying to remove an non-existent protocol " << int(protocol->GetProtocolNumber ()) << " on interface " << int(interfaceIndex));
    }
  else
    {
      m_protocols.erase (key);
    }
}

Ptr<RadeepL4Protocol>
RadeepL3Protocol::GetProtocol (int protocolNumber) const
{
  NS_LOG_FUNCTION (this << protocolNumber);

  return GetProtocol (protocolNumber, -1);
}

Ptr<RadeepL4Protocol>
RadeepL3Protocol::GetProtocol (int protocolNumber, int32_t interfaceIndex) const
{
  NS_LOG_FUNCTION (this << protocolNumber << interfaceIndex);

  L4ListKey_t key;
  L4List_t::const_iterator i;
  if (interfaceIndex >= 0)
    {
      // try the interface-specific protocol.
      key = std::make_pair (protocolNumber, interfaceIndex);
      i = m_protocols.find (key);
      if (i != m_protocols.end ())
        {
          return i->second;
        }
    }
  // try the generic protocol.
  key = std::make_pair (protocolNumber, -1);
  i = m_protocols.find (key);
  if (i != m_protocols.end ())
    {
      return i->second;
    }

  return 0;
}

void
RadeepL3Protocol::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  m_node = node;
  // Add a LoopbackNetDevice if needed, and an RadeepInterface on top of it
  SetupLoopback ();
}

Ptr<Socket> 
RadeepL3Protocol::CreateRawSocket (void)
{
  NS_LOG_FUNCTION (this);
  Ptr<RadeepRawSocketImpl> socket = CreateObject<RadeepRawSocketImpl> ();
  socket->SetNode (m_node);
  m_sockets.push_back (socket);
  return socket;
}
void 
RadeepL3Protocol::DeleteRawSocket (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  for (SocketList::iterator i = m_sockets.begin (); i != m_sockets.end (); ++i)
    {
      if ((*i) == socket)
        {
          m_sockets.erase (i);
          return;
        }
    }
  return;
}
/*
 * This method is called by AddAgregate and completes the aggregation
 * by setting the node in the Radeep stack
 */
void
RadeepL3Protocol::NotifyNewAggregate ()
{
  NS_LOG_FUNCTION (this);
  if (m_node == 0)
    {
      Ptr<Node>node = this->GetObject<Node>();
      // verify that it's a valid node and that
      // the node has not been set before
      if (node != 0)
        {
          this->SetNode (node);
        }
    }
  Radeep::NotifyNewAggregate ();
}

void 
RadeepL3Protocol::SetRoutingProtocol (Ptr<RadeepRoutingProtocol> routingProtocol)
{
  NS_LOG_FUNCTION (this << routingProtocol);
  m_routingProtocol = routingProtocol;
  m_routingProtocol->SetRadeep (this);
}


Ptr<RadeepRoutingProtocol> 
RadeepL3Protocol::GetRoutingProtocol (void) const
{
  NS_LOG_FUNCTION (this);
  return m_routingProtocol;
}

void 
RadeepL3Protocol::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  for (L4List_t::iterator i = m_protocols.begin (); i != m_protocols.end (); ++i)
    {
      i->second = 0;
    }
  m_protocols.clear ();

  for (RadeepInterfaceList::iterator i = m_interfaces.begin (); i != m_interfaces.end (); ++i)
    {
      *i = 0;
    }
  m_interfaces.clear ();
  m_reverseInterfacesContainer.clear ();

  m_sockets.clear ();
  m_node = 0;
  m_routingProtocol = 0;

  for (MapFragments_t::iterator it = m_fragments.begin (); it != m_fragments.end (); it++)
    {
      it->second = 0;
    }

  for (MapFragmentsTimers_t::iterator it = m_fragmentsTimers.begin (); it != m_fragmentsTimers.end (); it++)
    {
      if (it->second.IsRunning ())
        {
          it->second.Cancel ();
        }
    }

  m_fragments.clear ();
  m_fragmentsTimers.clear ();

  Object::DoDispose ();
}

void
RadeepL3Protocol::SetupLoopback (void)
{
  NS_LOG_FUNCTION (this);

  Ptr<RadeepInterface> interface = CreateObject<RadeepInterface> ();
  Ptr<LoopbackNetDevice> device = 0;
  // First check whether an existing LoopbackNetDevice exists on the node
  for (uint32_t i = 0; i < m_node->GetNDevices (); i++)
    {
      if ((device = DynamicCast<LoopbackNetDevice> (m_node->GetDevice (i))))
        {
          break;
        }
    }
  if (device == 0)
    {
      device = CreateObject<LoopbackNetDevice> (); 
      m_node->AddDevice (device);
    }
  interface->SetDevice (device);
  interface->SetNode (m_node);
  RadeepInterfaceAddress ifaceAddr = RadeepInterfaceAddress (RadeepAddress::GetLoopback (), RadeepMask::GetLoopback ());
  interface->AddAddress (ifaceAddr);
  uint32_t index = AddRadeepInterface (interface);
  Ptr<Node> node = GetObject<Node> ();
  node->RegisterProtocolHandler (MakeCallback (&RadeepL3Protocol::Receive, this), 
                                 RadeepL3Protocol::PROT_NUMBER, device);
  interface->SetUp ();
  if (m_routingProtocol != 0)
    {
      m_routingProtocol->NotifyInterfaceUp (index);
    }
}

void 
RadeepL3Protocol::SetDefaultTtl (uint8_t ttl)
{
  NS_LOG_FUNCTION (this << static_cast<uint32_t> (ttl));
  m_defaultTtl = ttl;
}

uint32_t 
RadeepL3Protocol::AddInterface (Ptr<NetDevice> device)
{
  NS_LOG_FUNCTION (this << device);
  NS_ASSERT (m_node != 0);

  Ptr<TrafficControlLayer> tc = m_node->GetObject<TrafficControlLayer> ();

  NS_ASSERT (tc != 0);

  m_node->RegisterProtocolHandler (MakeCallback (&TrafficControlLayer::Receive, tc),
                                   RadeepL3Protocol::PROT_NUMBER, device);
  m_node->RegisterProtocolHandler (MakeCallback (&TrafficControlLayer::Receive, tc),
                                   ArpL3Protocol::PROT_NUMBER, device);

  tc->RegisterProtocolHandler (MakeCallback (&RadeepL3Protocol::Receive, this),
                               RadeepL3Protocol::PROT_NUMBER, device);
  tc->RegisterProtocolHandler (MakeCallback (&ArpL3Protocol::Receive, PeekPointer (GetObject<ArpL3Protocol> ())),
                               ArpL3Protocol::PROT_NUMBER, device);

  Ptr<RadeepInterface> interface = CreateObject<RadeepInterface> ();
  interface->SetNode (m_node);
  interface->SetDevice (device);
  interface->SetTrafficControl (tc);
  interface->SetForwarding (m_radeepForward);
  tc->SetupDevice (device);
  return AddRadeepInterface (interface);
}

uint32_t 
RadeepL3Protocol::AddRadeepInterface (Ptr<RadeepInterface>interface)
{
  NS_LOG_FUNCTION (this << interface);
  uint32_t index = m_interfaces.size ();
  m_interfaces.push_back (interface);
  m_reverseInterfacesContainer[interface->GetDevice ()] = index;
  return index;
}

Ptr<RadeepInterface>
RadeepL3Protocol::GetInterface (uint32_t index) const
{
  NS_LOG_FUNCTION (this << index);
  if (index < m_interfaces.size ())
    {
      return m_interfaces[index];
    }
  return 0;
}

uint32_t 
RadeepL3Protocol::GetNInterfaces (void) const
{
  NS_LOG_FUNCTION (this);
  return m_interfaces.size ();
}

int32_t 
RadeepL3Protocol::GetInterfaceForAddress (
  RadeepAddress address) const
{
  NS_LOG_FUNCTION (this << address);
  int32_t interface = 0;
  for (RadeepInterfaceList::const_iterator i = m_interfaces.begin (); 
       i != m_interfaces.end (); 
       i++, interface++)
    {
      for (uint32_t j = 0; j < (*i)->GetNAddresses (); j++)
        {
          if ((*i)->GetAddress (j).GetLocal () == address)
            {
              return interface;
            }
        }
    }

  return -1;
}

int32_t 
RadeepL3Protocol::GetInterfaceForPrefix (
  RadeepAddress address, 
  RadeepMask mask) const
{
  NS_LOG_FUNCTION (this << address << mask);
  int32_t interface = 0;
  for (RadeepInterfaceList::const_iterator i = m_interfaces.begin (); 
       i != m_interfaces.end (); 
       i++, interface++)
    {
      for (uint32_t j = 0; j < (*i)->GetNAddresses (); j++)
        {
          if ((*i)->GetAddress (j).GetLocal ().CombineMask (mask) == address.CombineMask (mask))
            {
              return interface;
            }
        }
    }

  return -1;
}

int32_t 
RadeepL3Protocol::GetInterfaceForDevice (
  Ptr<const NetDevice> device) const
{
  NS_LOG_FUNCTION (this << device);

  RadeepInterfaceReverseContainer::const_iterator iter = m_reverseInterfacesContainer.find (device);
  if (iter != m_reverseInterfacesContainer.end ())
    {
      return (*iter).second;
    }

  return -1;
}

bool
RadeepL3Protocol::IsDestinationAddress (RadeepAddress address, uint32_t iif) const
{
  NS_LOG_FUNCTION (this << address << iif);
  // First check the incoming interface for a unicast address match
  for (uint32_t i = 0; i < GetNAddresses (iif); i++)
    {
      RadeepInterfaceAddress iaddr = GetAddress (iif, i);
      if (address == iaddr.GetLocal ())
        {
          NS_LOG_LOGIC ("For me (destination " << address << " match)");
          return true;
        }
      if (address == iaddr.GetBroadcast ())
        {
          NS_LOG_LOGIC ("For me (interface broadcast address)");
          return true;
        }
    }

  if (address.IsMulticast ())
    {
#ifdef NOTYET
      if (MulticastCheckGroup (iif, address ))
#endif
      if (true)
        {
          NS_LOG_LOGIC ("For me (RadeepAddr multicast address");
          return true;
        }
    }

  if (address.IsBroadcast ())
    {
      NS_LOG_LOGIC ("For me (RadeepAddr broadcast address)");
      return true;
    }

  if (GetWeakEsModel ())  // Check other interfaces
    { 
      for (uint32_t j = 0; j < GetNInterfaces (); j++)
        {
          if (j == uint32_t (iif)) continue;
          for (uint32_t i = 0; i < GetNAddresses (j); i++)
            {
              RadeepInterfaceAddress iaddr = GetAddress (j, i);
              if (address == iaddr.GetLocal ())
                {
                  NS_LOG_LOGIC ("For me (destination " << address << " match) on another interface");
                  return true;
                }
              //  This is a small corner case:  match another interface's broadcast address
              if (address == iaddr.GetBroadcast ())
                {
                  NS_LOG_LOGIC ("For me (interface broadcast address on another interface)");
                  return true;
                }
            }
        }
    }
  return false;
}

void 
RadeepL3Protocol::Receive ( Ptr<NetDevice> device, Ptr<const Packet> p, uint16_t protocol, const Address &from,
                          const Address &to, NetDevice::PacketType packetType)
{
  NS_LOG_FUNCTION (this << device << p << protocol << from << to << packetType);

  NS_LOG_LOGIC ("Packet from " << from << " received on node " << 
                m_node->GetId ());


  int32_t interface = GetInterfaceForDevice(device);
  NS_ASSERT_MSG (interface != -1, "Received a packet from an interface that is not known to Radeep");

  Ptr<Packet> packet = p->Copy ();

  Ptr<RadeepInterface> radeepInterface = m_interfaces[interface];

  if (radeepInterface->IsUp ())
    {
      m_rxTrace (packet, m_node->GetObject<Radeep> (), interface);
    }
  else
    {
      NS_LOG_LOGIC ("Dropping received packet -- interface is down");
      RadeepHeader radeepHeader;
      packet->RemoveHeader (radeepHeader);
      m_dropTrace (radeepHeader, packet, DROP_INTERFACE_DOWN, m_node->GetObject<Radeep> (), interface);
      return;
    }

  RadeepHeader radeepHeader;
  if (Node::ChecksumEnabled ())
    {
      radeepHeader.EnableChecksum ();
    }
  packet->RemoveHeader (radeepHeader);

  // Trim any residual frame padding from underlying devices
  if (radeepHeader.GetPayloadSize () < packet->GetSize ())
    {
      packet->RemoveAtEnd (packet->GetSize () - radeepHeader.GetPayloadSize ());
    }

  if (!radeepHeader.IsChecksumOk ()) 
    {
      NS_LOG_LOGIC ("Dropping received packet -- checksum not ok");
      m_dropTrace (radeepHeader, packet, DROP_BAD_CHECKSUM, m_node->GetObject<Radeepv4> (), interface);
      return;
    }

  // the packet is valid, we update the ARP cache entry (if present)
  Ptr<ArpCache> arpCache = radeepInterface->GetArpCache ();
  if (arpCache)
    {
      // case one, it's a a direct routing.
      ArpCache::Entry *entry = arpCache->Lookup (radeepHeader.GetSource ());
      if (entry)
        {
          if (entry->IsAlive ())
            {
              entry->UpdateSeen ();
            }
        }
      else
        {
          // It's not in the direct routing, so it's the router, and it could have multiple Radeep addresses.
          // In doubt, update all of them.
          // Note: it's a confirmed behavior for Linux routers.
          std::list<ArpCache::Entry *> entryList = arpCache->LookupInverse (from);
          std::list<ArpCache::Entry *>::iterator iter;
          for (iter = entryList.begin (); iter != entryList.end (); iter ++)
            {
              if ((*iter)->IsAlive ())
                {
                  (*iter)->UpdateSeen ();
                }
            }
        }
    }

  for (SocketList::iterator i = m_sockets.begin (); i != m_sockets.end (); ++i)
    {
      NS_LOG_LOGIC ("Forwarding to raw socket"); 
      Ptr<RadeepRawSocketImpl> socket = *i;
      socket->ForwardUp (packet, radeepHeader, radeepInterface);
    }

  NS_ASSERT_MSG (m_routingProtocol != 0, "Need a routing protocol object to process packets");
  if (!m_routingProtocol->RouteInput (packet, radeepHeader, device,
                                      MakeCallback (&RadeepL3Protocol::RadeepForward, this),
                                      MakeCallback (&RadeepL3Protocol::RadeepMulticastForward, this),
                                      MakeCallback (&RadeepL3Protocol::LocalDeliver, this),
                                      MakeCallback (&RadeepL3Protocol::RouteInputError, this)
                                      ))
    {
      NS_LOG_WARN ("No route found for forwarding packet.  Drop.");
      m_dropTrace (radeepHeader, packet, DROP_NO_ROUTE, m_node->GetObject<Radeep> (), interface);
    }
}

Ptr<Icmpv4L4Protocol> 
RadeepL3Protocol::GetIcmp (void) const
{
  NS_LOG_FUNCTION (this);
  Ptr<RadeepL4Protocol> prot = GetProtocol (Icmpv4L4Protocol::GetStaticProtocolNumber ());
  if (prot != 0)
    {
      return prot->GetObject<Icmpv4L4Protocol> ();
    }
  else
    {
      return 0;
    }
}

bool
RadeepL3Protocol::IsUnicast (RadeepAddress ad) const
{
  NS_LOG_FUNCTION (this << ad);

  if (ad.IsBroadcast () || ad.IsMulticast ())
    {
      return false;
    }
  else
    {
      // check for subnet-broadcast
      for (uint32_t ifaceIndex = 0; ifaceIndex < GetNInterfaces (); ifaceIndex++)
        {
          for (uint32_t j = 0; j < GetNAddresses (ifaceIndex); j++)
            {
              RadeepInterfaceAddress ifAddr = GetAddress (ifaceIndex, j);
              NS_LOG_LOGIC ("Testing address " << ad << " with subnet-directed broadcast " << ifAddr.GetBroadcast () );
              if (ad == ifAddr.GetBroadcast () )
                {
                  return false;
                }
            }
        }
    }

  return true;
}

bool
RadeepL3Protocol::IsUnicast (RadeepAddress ad, RadeepMask interfaceMask) const
{
  NS_LOG_FUNCTION (this << ad << interfaceMask);
  return !ad.IsMulticast () && !ad.IsSubnetDirectedBroadcast (interfaceMask);
}

void 
RadeepL3Protocol::SendWithHeader (Ptr<Packet> packet, 
                                RadeepHeader radeepHeader,
                                Ptr<RadeepRoute> route)
{
  NS_LOG_FUNCTION (this << packet << radeepHeader << route);
  if (Node::ChecksumEnabled ())
    {
      radeepHeader.EnableChecksum ();
    }
  SendRealOut (route, packet, radeepHeader);
}

void
RadeepL3Protocol::CallTxTrace (const RadeepHeader & radeepHeader, Ptr<Packet> packet,
                                    Ptr<Radeep> radeep, uint32_t interface)
{
  Ptr<Packet> packetCopy = packet->Copy ();
  packetCopy->AddHeader (radeepHeader);
  m_txTrace (packetCopy, radeep, interface);
}

void 
RadeepL3Protocol::Send (Ptr<Packet> packet, 
                      RadeepAddress source,
                      RadeepAddress destination,
                      uint8_t protocol,
                      Ptr<RadeepRoute> route)
{
  NS_LOG_FUNCTION (this << packet << source << destination << uint32_t (protocol) << route);

  RadeepHeader radeepHeader;
  bool mayFragment = true;
  uint8_t ttl = m_defaultTtl;
  SocketRadeepTtlTag tag;
  bool found = packet->RemovePacketTag (tag);
  if (found)
    {
      ttl = tag.GetTtl ();
    }

  uint8_t tos = 0;
  SocketRadeepTosTag radeepTosTag;
  found = packet->RemovePacketTag (radeepTosTag);
  if (found)
    {
      tos = radeepTosTag.GetTos ();
    }

  // Handle a few cases:
  // 1) packet is destined to limited broadcast address
  // 2) packet is destined to a subnet-directed broadcast address
  // 3) packet is not broadcast, and is passed in with a route entry
  // 4) packet is not broadcast, and is passed in with a route entry but route->GetGateway is not set (e.g., on-demand)
  // 5) packet is not broadcast, and route is NULL (e.g., a raw socket call, or ICMP)

  // 1) packet is destined to limited broadcast address or link-local multicast address
  if (destination.IsBroadcast () || destination.IsLocalMulticast ())
    {
      NS_LOG_LOGIC ("RadeepL3Protocol::Send case 1:  limited broadcast");
      radeepHeader = BuildHeader (source, destination, protocol, packet->GetSize (), ttl, tos, mayFragment);
      uint32_t ifaceIndex = 0;
      for (RadeepInterfaceList::iterator ifaceIter = m_interfaces.begin ();
           ifaceIter != m_interfaces.end (); ifaceIter++, ifaceIndex++)
        {
          Ptr<RadeepInterface> outInterface = *ifaceIter;
          bool sendIt = false;
          if (source == RadeepAddress::GetAny ())
            {
              sendIt = true;
            }
          for (uint32_t index = 0; index < outInterface->GetNAddresses (); index++)
            {
              if (outInterface->GetAddress (index).GetLocal () == source)
                {
                  sendIt = true;
                }
            }
          if (sendIt)
            {
              Ptr<Packet> packetCopy = packet->Copy ();

              NS_ASSERT (packetCopy->GetSize () <= outInterface->GetDevice ()->GetMtu ());

              m_sendOutgoingTrace (radeepHeader, packetCopy, ifaceIndex);
              CallTxTrace (radeepHeader, packetCopy, m_node->GetObject<Radeep> (), ifaceIndex);
              outInterface->Send (packetCopy, radeepHeader, destination);
            }
        }
      return;
    }

  // 2) check: packet is destined to a subnet-directed broadcast address
  uint32_t ifaceIndex = 0;
  for (RadeepInterfaceList::iterator ifaceIter = m_interfaces.begin ();
       ifaceIter != m_interfaces.end (); ifaceIter++, ifaceIndex++)
    {
      Ptr<RadeepInterface> outInterface = *ifaceIter;
      for (uint32_t j = 0; j < GetNAddresses (ifaceIndex); j++)
        {
          RadeepInterfaceAddress ifAddr = GetAddress (ifaceIndex, j);
          NS_LOG_LOGIC ("Testing address " << ifAddr.GetLocal () << " with mask " << ifAddr.GetMask ());
          if (destination.IsSubnetDirectedBroadcast (ifAddr.GetMask ()) && 
              destination.CombineMask (ifAddr.GetMask ()) == ifAddr.GetLocal ().CombineMask (ifAddr.GetMask ())   )
            {
              NS_LOG_LOGIC ("RadeepL3Protocol::Send case 2:  subnet directed bcast to " << ifAddr.GetLocal ());
              radeepHeader = BuildHeader (source, destination, protocol, packet->GetSize (), ttl, tos, mayFragment);
              Ptr<Packet> packetCopy = packet->Copy ();
              m_sendOutgoingTrace (radeepHeader, packetCopy, ifaceIndex);
              CallTxTrace (radeepHeader, packetCopy, m_node->GetObject<Radeep> (), ifaceIndex);
              outInterface->Send (packetCopy, radeepHeader, destination);
              return;
            }
        }
    }

  // 3) packet is not broadcast, and is passed in with a route entry
  //    with a valid RadeepAddress as the gateway
  if (route && route->GetGateway () != RadeepAddress ())
    {
      NS_LOG_LOGIC ("RadeepL3Protocol::Send case 3:  passed in with route");
      radeepHeader = BuildHeader (source, destination, protocol, packet->GetSize (), ttl, tos, mayFragment);
      int32_t interface = GetInterfaceForDevice (route->GetOutputDevice ());
      m_sendOutgoingTrace (radeepHeader, packet, interface);
      SendRealOut (route, packet->Copy (), radeepHeader);
      return; 
    } 
  // 4) packet is not broadcast, and is passed in with a route entry but route->GetGateway is not set (e.g., on-demand)
  if (route && route->GetGateway () == RadeepAddress ())
    {
      // This could arise because the synchronous RouteOutput() call
      // returned to the transport protocol with a source address but
      // there was no next hop available yet (since a route may need
      // to be queried).
      NS_FATAL_ERROR ("RadeepL3Protocol::Send case 4: This case not yet implemented");
    }
  // 5) packet is not broadcast, and route is NULL (e.g., a raw socket call)
  NS_LOG_LOGIC ("RadeepL3Protocol::Send case 5:  passed in with no route " << destination);
  Socket::SocketErrno errno_; 
  Ptr<NetDevice> oif (0); // unused for now
  radeepHeader = BuildHeader (source, destination, protocol, packet->GetSize (), ttl, tos, mayFragment);
  Ptr<RadeepRoute> newRoute;
  if (m_routingProtocol != 0)
    {
      newRoute = m_routingProtocol->RouteOutput (packet, radeepHeader, oif, errno_);
    }
  else
    {
      NS_LOG_ERROR ("RadeepL3Protocol::Send: m_routingProtocol == 0");
    }
  if (newRoute)
    {
      int32_t interface = GetInterfaceForDevice (newRoute->GetOutputDevice ());
      m_sendOutgoingTrace (radeepHeader, packet, interface);
      SendRealOut (newRoute, packet->Copy (), radeepHeader);
    }
  else
    {
      NS_LOG_WARN ("No route to host.  Drop.");
      m_dropTrace (radeepHeader, packet, DROP_NO_ROUTE, m_node->GetObject<Radeep> (), 0);
    }
}

// \todo when should we set Radeep_id?   check whether we are incrementing
// m_identification on packets that may later be dropped in this stack
// and whether that deviates from Linux
RadeepHeader
RadeepL3Protocol::BuildHeader (
  RadeepAddress source,
  RadeepAddress destination,
  uint8_t protocol,
  uint16_t payloadSize,
  uint8_t ttl,
  uint8_t tos,
  bool mayFragment)
{
  NS_LOG_FUNCTION (this << source << destination << (uint16_t)protocol << payloadSize << (uint16_t)ttl << (uint16_t)tos << mayFragment);
  RadeepHeader radeepHeader;
  radeepHeader.SetSource (source);
  radeepHeader.SetDestination (destination);
  radeepHeader.SetProtocol (protocol);
  radeepHeader.SetPayloadSize (payloadSize);
  radeepHeader.SetTtl (ttl);
  radeepHeader.SetTos (tos);

  uint64_t src = source.Get ();
  uint64_t dst = destination.Get ();
  uint64_t srcDst = dst | (src << 32);
  std::pair<uint64_t, uint8_t> key = std::make_pair (srcDst, protocol);

  if (mayFragment == true)
    {
      radeepHeader.SetMayFragment ();
      radeepHeader.SetIdentification (m_identification[key]);
      m_identification[key]++;
    }
  else
    {
      radeepHeader.SetDontFragment ();
      // RFC 6864 does not state anything about atomic datagrams
      // identification requirement:
      // >> Originating sources MAY set the Radeep ID field of atomic datagrams
      //    to any value.
      radeepHeader.SetIdentification (m_identification[key]);
      m_identification[key]++;
    }
  if (Node::ChecksumEnabled ())
    {
      radeepHeader.EnableChecksum ();
    }
  return radeepHeader;
}

void
RadeepL3Protocol::SendRealOut (Ptr<RadeepRoute> route,
                             Ptr<Packet> packet,
                             RadeepHeader const &radeepHeader)
{
  NS_LOG_FUNCTION (this << route << packet << &radeepHeader);
  if (route == 0)
    {
      NS_LOG_WARN ("No route to host.  Drop.");
      m_dropTrace (radeepHeader, packet, DROP_NO_ROUTE, m_node->GetObject<Radeep> (), 0);
      return;
    }
  Ptr<NetDevice> outDev = route->GetOutputDevice ();
  int32_t interface = GetInterfaceForDevice (outDev);
  NS_ASSERT (interface >= 0);
  Ptr<RadeepInterface> outInterface = GetInterface (interface);
  NS_LOG_LOGIC ("Send via NetDevice ifIndex " << outDev->GetIfIndex () << " radeepInterfaceIndex " << interface);

  if (!route->GetGateway ().IsEqual (RadeepAddress ("0.0.0.0")))
    {
      if (outInterface->IsUp ())
        {
          NS_LOG_LOGIC ("Send to gateway " << route->GetGateway ());
          if ( packet->GetSize () + radeepHeader.GetSerializedSize () > outInterface->GetDevice ()->GetMtu () )
            {
              std::list<RadeepPayloadHeaderPair> listFragments;
              DoFragmentation (packet, radeepHeader, outInterface->GetDevice ()->GetMtu (), listFragments);
              for ( std::list<RadeepPayloadHeaderPair>::iterator it = listFragments.begin (); it != listFragments.end (); it++ )
                {
                  CallTxTrace (it->second, it->first, m_node->GetObject<Radeep> (), interface);
                  outInterface->Send (it->first, it->second, route->GetGateway ());
                }
            }
          else
            {
              CallTxTrace (radeepHeader, packet, m_node->GetObject<Radeep> (), interface);
              outInterface->Send (packet, radeepHeader, route->GetGateway ());
            }
        }
      else
        {
          NS_LOG_LOGIC ("Dropping -- outgoing interface is down: " << route->GetGateway ());
          m_dropTrace (radeepHeader, packet, DROP_INTERFACE_DOWN, m_node->GetObject<Radeep> (), interface);
        }
    } 
  else 
    {
      if (outInterface->IsUp ())
        {
          NS_LOG_LOGIC ("Send to destination " << radeepHeader.GetDestination ());
          if ( packet->GetSize () + radeepHeader.GetSerializedSize () > outInterface->GetDevice ()->GetMtu () )
            {
              std::list<RadeepPayloadHeaderPair> listFragments;
              DoFragmentation (packet, radeepHeader, outInterface->GetDevice ()->GetMtu (), listFragments);
              for ( std::list<RadeepPayloadHeaderPair>::iterator it = listFragments.begin (); it != listFragments.end (); it++ )
                {
                  NS_LOG_LOGIC ("Sending fragment " << *(it->first) );
                  CallTxTrace (it->second, it->first, m_node->GetObject<Radeep> (), interface);
                  outInterface->Send (it->first, it->second, radeepHeader.GetDestination ());
                }
            }
          else
            {
              CallTxTrace (radeepHeader, packet, m_node->GetObject<Radeep> (), interface);
              outInterface->Send (packet, radeepHeader, radeepHeader.GetDestination ());
            }
        }
      else
        {
          NS_LOG_LOGIC ("Dropping -- outgoing interface is down: " << radeepHeader.GetDestination ());
          m_dropTrace (radeepHeader, packet, DROP_INTERFACE_DOWN, m_node->GetObject<Radeep> (), interface);
        }
    }
}

// This function analogous to Linux Radeep_mr_forward()
void
RadeepL3Protocol::RadeepMulticastForward (Ptr<RadeepMulticastRoute> mrtentry, Ptr<const Packet> p, const RadeepHeader &header)
{
  NS_LOG_FUNCTION (this << mrtentry << p << header);
  NS_LOG_LOGIC ("Multicast forwarding logic for node: " << m_node->GetId ());

  std::map<uint32_t, uint32_t> ttlMap = mrtentry->GetOutputTtlMap ();
  std::map<uint32_t, uint32_t>::iterator mapIter;

  for (mapIter = ttlMap.begin (); mapIter != ttlMap.end (); mapIter++)
    {
      uint32_t interfaceId = mapIter->first;
      //uint32_t outputTtl = mapIter->second;  // Unused for now

      Ptr<Packet> packet = p->Copy ();
      RadeepHeader h = header;
      h.SetTtl (header.GetTtl () - 1);
      if (h.GetTtl () == 0)
        {
          NS_LOG_WARN ("TTL exceeded.  Drop.");
          m_dropTrace (header, packet, DROP_TTL_EXPIRED, m_node->GetObject<Radeep> (), interfaceId);
          return;
        }
      NS_LOG_LOGIC ("Forward multicast via interface " << interfaceId);
      Ptr<RadeepRoute> rtentry = Create<RadeepRoute> ();
      rtentry->SetSource (h.GetSource ());
      rtentry->SetDestination (h.GetDestination ());
      rtentry->SetGateway (RadeepAddress::GetAny ());
      rtentry->SetOutputDevice (GetNetDevice (interfaceId));
      SendRealOut (rtentry, packet, h);
      continue;
    }
}

// This function analogous to Linux Radeep_forward()
void
RadeepL3Protocol::RadeepForward (Ptr<RadeepRoute> rtentry, Ptr<const Packet> p, const RadeepHeader &header)
{
  NS_LOG_FUNCTION (this << rtentry << p << header);
  NS_LOG_LOGIC ("Forwarding logic for node: " << m_node->GetId ());
  // Forwarding
  RadeepHeader radeepHeader = header;
  Ptr<Packet> packet = p->Copy ();
  int32_t interface = GetInterfaceForDevice (rtentry->GetOutputDevice ());
  radeepHeader.SetTtl (radeepHeader.GetTtl () - 1);
  if (radeepHeader.GetTtl () == 0)
    {
      // Do not reply to ICMP or to multicast/broadcast Radeep address 
      if (radeepHeader.GetProtocol () != Icmpv4L4Protocol::PROT_NUMBER && 
          radeepHeader.GetDestination ().IsBroadcast () == false &&
          radeepHeader.GetDestination ().IsMulticast () == false)
        {
          Ptr<Icmpv4L4Protocol> icmp = GetIcmp ();
          icmp->SendTimeExceededTtl (radeepHeader, packet, false);
        }
      NS_LOG_WARN ("TTL exceeded.  Drop.");
      m_dropTrace (header, packet, DROP_TTL_EXPIRED, m_node->GetObject<Radeep> (), interface);
      return;
    }
  // in case the packet still has a priority tag attached, remove it
  SocketPriorityTag priorityTag;
  packet->RemovePacketTag (priorityTag);
  uint8_t priority = Socket::RadeepTos2Priority (radeepHeader.GetTos ());
  // add a priority tag if the priority is not null
  if (priority)
    {
      priorityTag.SetPriority (priority);
      packet->AddPacketTag (priorityTag);
    }

  m_unicastForwardTrace (radeepHeader, packet, interface);
  SendRealOut (rtentry, packet, radeepHeader);
}

void
RadeepL3Protocol::LocalDeliver (Ptr<const Packet> packet, RadeepHeader const&radeep, uint32_t iif)
{
  NS_LOG_FUNCTION (this << packet << &radeep << iif);
  Ptr<Packet> p = packet->Copy (); // need to pass a non-const packet up
  RadeepHeader radeepHeader = radeep;

  if ( !radeepHeader.IsLastFragment () || radeepHeader.GetFragmentOffset () != 0 )
    {
      NS_LOG_LOGIC ("Received a fragment, processing " << *p );
      bool isPacketComplete;
      isPacketComplete = ProcessFragment (p, radeepHeader, iif);
      if ( isPacketComplete == false)
        {
          return;
        }
      NS_LOG_LOGIC ("Got last fragment, Packet is complete " << *p );
      radeepHeader.SetFragmentOffset (0);
      radeepHeader.SetPayloadSize (p->GetSize ());
    }

  m_localDeliverTrace (radeepHeader, p, iif);

  Ptr<RadeepL4Protocol> protocol = GetProtocol (radeepHeader.GetProtocol (), iif);
  if (protocol != 0)
    {
      // we need to make a copy in the unlikely event we hit the
      // RX_ENDPOINT_UNREACH codepath
      Ptr<Packet> copy = p->Copy ();
      enum RadeepL4Protocol::RxStatus status = 
        protocol->Receive (p, radeepHeader, GetInterface (iif));
      switch (status) {
        case RadeepL4Protocol::RX_OK:
        // fall through
        case RadeepL4Protocol::RX_ENDPOINT_CLOSED:
        // fall through
        case RadeepL4Protocol::RX_CSUM_FAILED:
          break;
        case RadeepL4Protocol::RX_ENDPOINT_UNREACH:
          if (radeepHeader.GetDestination ().IsBroadcast () == true ||
              radeepHeader.GetDestination ().IsMulticast () == true)
            {
              break; // Do not reply to broadcast or multicast
            }
          // Another case to suppress ICMP is a subnet-directed broadcast
          bool subnetDirected = false;
          for (uint32_t i = 0; i < GetNAddresses (iif); i++)
            {
              RadeepInterfaceAddress addr = GetAddress (iif, i);
              if (addr.GetLocal ().CombineMask (addr.GetMask ()) == radeepHeader.GetDestination ().CombineMask (addr.GetMask ()) &&
                  radeepHeader.GetDestination ().IsSubnetDirectedBroadcast (addr.GetMask ()))
                {
                  subnetDirected = true;
                }
            }
          if (subnetDirected == false)
            {
              GetIcmp ()->SendDestUnreachPort (radeepHeader, copy);
            }
        }
    }
}

bool
RadeepL3Protocol::AddAddress (uint32_t i, RadeepInterfaceAddress address)
{
  NS_LOG_FUNCTION (this << i << address);
  Ptr<RadeepInterface> interface = GetInterface (i);
  bool retVal = interface->AddAddress (address);
  if (m_routingProtocol != 0)
    {
      m_routingProtocol->NotifyAddAddress (i, address);
    }
  return retVal;
}

RadeepInterfaceAddress 
RadeepL3Protocol::GetAddress (uint32_t interfaceIndex, uint32_t addressIndex) const
{
  NS_LOG_FUNCTION (this << interfaceIndex << addressIndex);
  Ptr<RadeepInterface> interface = GetInterface (interfaceIndex);
  return interface->GetAddress (addressIndex);
}

uint32_t 
RadeepL3Protocol::GetNAddresses (uint32_t interface) const
{
  NS_LOG_FUNCTION (this << interface);
  Ptr<RadeepInterface> iface = GetInterface (interface);
  return iface->GetNAddresses ();
}

bool
RadeepL3Protocol::RemoveAddress (uint32_t i, uint32_t addressIndex)
{
  NS_LOG_FUNCTION (this << i << addressIndex);
  Ptr<RadeepInterface> interface = GetInterface (i);
  RadeepInterfaceAddress address = interface->RemoveAddress (addressIndex);
  if (address != RadeepInterfaceAddress ())
    {
      if (m_routingProtocol != 0)
        {
          m_routingProtocol->NotifyRemoveAddress (i, address);
        }
      return true;
    }
  return false;
}

bool
RadeepL3Protocol::RemoveAddress (uint32_t i, RadeepAddress address)
{
  NS_LOG_FUNCTION (this << i << address);

  if (address == RadeepAddress::GetLoopback())
    {
      NS_LOG_WARN ("Cannot remove loopback address.");
      return false;
    }
  Ptr<RadeepInterface> interface = GetInterface (i);
  RadeepInterfaceAddress ifAddr = interface->RemoveAddress (address);
  if (ifAddr != RadeepInterfaceAddress ())
    {
      if (m_routingProtocol != 0)
        {
          m_routingProtocol->NotifyRemoveAddress (i, ifAddr);
        }
      return true;
    }
  return false;
}

RadeepAddress
RadeepL3Protocol::SourceAddressSelection (uint32_t interfaceIdx, RadeepAddress dest)
{
  NS_LOG_FUNCTION (this << interfaceIdx << " " << dest);
  if (GetNAddresses (interfaceIdx) == 1)  // common case
    {
      return GetAddress (interfaceIdx, 0).GetLocal ();
    }
  // no way to determine the scope of the destination, so adopt the
  // following rule:  pick the first available address (index 0) unless
  // a subsequent address is on link (in which case, pick the primary
  // address if there are multiple)
  RadeepAddress candidate = GetAddress (interfaceIdx, 0).GetLocal ();
  for (uint32_t i = 0; i < GetNAddresses (interfaceIdx); i++)
    {
      RadeepInterfaceAddress test = GetAddress (interfaceIdx, i);
      if (test.GetLocal ().CombineMask (test.GetMask ()) == dest.CombineMask (test.GetMask ()))
        {
          if (test.IsSecondary () == false)
            {
              return test.GetLocal ();
            }
        }
    }
  return candidate;
}

RadeepAddress 
RadeepL3Protocol::SelectSourceAddress (Ptr<const NetDevice> device,
                                     RadeepAddress dst, RadeepInterfaceAddress::InterfaceAddressScope_e scope)
{
  NS_LOG_FUNCTION (this << device << dst << scope);
  RadeepAddress addr ("0.0.0.0");
  RadeepInterfaceAddress iaddr; 
  bool found = false;

  if (device != 0)
    {
      int32_t i = GetInterfaceForDevice (device);
      NS_ASSERT_MSG (i >= 0, "No device found on node");
      for (uint32_t j = 0; j < GetNAddresses (i); j++)
        {
          iaddr = GetAddress (i, j);
          if (iaddr.IsSecondary ()) continue;
          if (iaddr.GetScope () > scope) continue; 
          if (dst.CombineMask (iaddr.GetMask ())  == iaddr.GetLocal ().CombineMask (iaddr.GetMask ()) )
            {
              return iaddr.GetLocal ();
            }
          if (!found)
            {
              addr = iaddr.GetLocal ();
              found = true;
            }
        }
    }
  if (found)
    {
      return addr;
    }

  // Iterate among all interfaces
  for (uint32_t i = 0; i < GetNInterfaces (); i++)
    {
      for (uint32_t j = 0; j < GetNAddresses (i); j++)
        {
          iaddr = GetAddress (i, j);
          if (iaddr.IsSecondary ()) continue;
          if (iaddr.GetScope () != RadeepInterfaceAddress::LINK 
              && iaddr.GetScope () <= scope) 
            {
              return iaddr.GetLocal ();
            }
        }
    }
  NS_LOG_WARN ("Could not find source address for " << dst << " and scope " 
                                                    << scope << ", returning 0");
  return addr;
}

void 
RadeepL3Protocol::SetMetric (uint32_t i, uint16_t metric)
{
  NS_LOG_FUNCTION (this << i << metric);
  Ptr<RadeepInterface> interface = GetInterface (i);
  interface->SetMetric (metric);
}

uint16_t
RadeepL3Protocol::GetMetric (uint32_t i) const
{
  NS_LOG_FUNCTION (this << i);
  Ptr<RadeepInterface> interface = GetInterface (i);
  return interface->GetMetric ();
}

uint16_t 
RadeepL3Protocol::GetMtu (uint32_t i) const
{
  NS_LOG_FUNCTION (this << i);
  Ptr<RadeepInterface> interface = GetInterface (i);
  return interface->GetDevice ()->GetMtu ();
}

bool 
RadeepL3Protocol::IsUp (uint32_t i) const
{
  NS_LOG_FUNCTION (this << i);
  Ptr<RadeepInterface> interface = GetInterface (i);
  return interface->IsUp ();
}

void 
RadeepL3Protocol::SetUp (uint32_t i)
{
  NS_LOG_FUNCTION (this << i);
  Ptr<RadeepInterface> interface = GetInterface (i);

  // RFC 791, pg.25:
  //  Every internet module must be able to forward a datagram of 68
  //  octets without further fragmentation.  This is because an internet
  //  header may be up to 60 octets, and the minimum fragment is 8 octets.
  if (interface->GetDevice ()->GetMtu () >= 68)
    {
      interface->SetUp ();

      if (m_routingProtocol != 0)
        {
          m_routingProtocol->NotifyInterfaceUp (i);
        }
    }
  else
    {
      NS_LOG_LOGIC ("Interface " << int(i) << " is set to be down for Radeep. Reason: not respecting minimum Radeep MTU (68 octects)");
    }
}

void 
RadeepL3Protocol::SetDown (uint32_t ifaceIndex)
{
  NS_LOG_FUNCTION (this << ifaceIndex);
  Ptr<RadeepInterface> interface = GetInterface (ifaceIndex);
  interface->SetDown ();

  if (m_routingProtocol != 0)
    {
      m_routingProtocol->NotifyInterfaceDown (ifaceIndex);
    }
}

bool 
RadeepL3Protocol::IsForwarding (uint32_t i) const
{
  NS_LOG_FUNCTION (this << i);
  Ptr<RadeepInterface> interface = GetInterface (i);
  NS_LOG_LOGIC ("Forwarding state: " << interface->IsForwarding ());
  return interface->IsForwarding ();
}

void 
RadeepL3Protocol::SetForwarding (uint32_t i, bool val)
{
  NS_LOG_FUNCTION (this << i);
  Ptr<RadeepInterface> interface = GetInterface (i);
  interface->SetForwarding (val);
}

Ptr<NetDevice>
RadeepL3Protocol::GetNetDevice (uint32_t i)
{
  NS_LOG_FUNCTION (this << i);
  return GetInterface (i)->GetDevice ();
}

void 
RadeepL3Protocol::SetRadeepForward (bool forward) 
{
  NS_LOG_FUNCTION (this << forward);
  m_radeepForward = forward;
  for (RadeepInterfaceList::const_iterator i = m_interfaces.begin (); i != m_interfaces.end (); i++)
    {
      (*i)->SetForwarding (forward);
    }
}

bool 
RadeepL3Protocol::GetRadeepForward (void) const
{
  NS_LOG_FUNCTION (this);
  return m_radeepForward;
}

void 
RadeepL3Protocol::SetWeakEsModel (bool model)
{
  NS_LOG_FUNCTION (this << model);
  m_weakEsModel = model;
}

bool 
RadeepL3Protocol::GetWeakEsModel (void) const
{
  NS_LOG_FUNCTION (this);
  return m_weakEsModel;
}

void
RadeepL3Protocol::RouteInputError (Ptr<const Packet> p, const RadeepHeader & radeepHeader, Socket::SocketErrno sockErrno)
{
  NS_LOG_FUNCTION (this << p << radeepHeader << sockErrno);
  NS_LOG_LOGIC ("Route input failure-- dropping packet to " << radeepHeader << " with errno " << sockErrno); 
  m_dropTrace (radeepHeader, p, DROP_ROUTE_ERROR, m_node->GetObject<Radeep> (), 0);

  // \todo Send an ICMP no route.
}

void
RadeepL3Protocol::DoFragmentation (Ptr<Packet> packet, const RadeepHeader & radeepHeader, uint32_t outIfaceMtu, std::list<RadeepPayloadHeaderPair>& listFragments)
{
  // BEWARE: here we do assume that the header options are not present.
  // a much more complex handling is necessary in case there are options.
  // If (when) Radeep option headers will be implemented, the following code shall be changed.
  // Of course also the reassemby code shall be changed as well.

  NS_LOG_FUNCTION (this << *packet << outIfaceMtu << &listFragments);

  Ptr<Packet> p = packet->Copy ();

  NS_ASSERT_MSG( (radeepHeader.GetSerializedSize() == 5*4),
                 "Radeep fragmentation implementation only works without option headers." );

  uint16_t offset = 0;
  bool moreFragment = true;
  uint16_t originalOffset = radeepHeader.GetFragmentOffset();
  bool isLastFragment = radeepHeader.IsLastFragment();
  uint32_t currentFragmentablePartSize = 0;

  // Radeep fragments are all 8 bytes aligned but the last.
  // The Radeep payload size is:
  // floor( ( outIfaceMtu - radeepHeader.GetSerializedSize() ) /8 ) *8
  uint32_t fragmentSize = (outIfaceMtu - radeepHeader.GetSerializedSize () ) & ~uint32_t (0x7);

  NS_LOG_LOGIC ("Fragmenting - Target Size: " << fragmentSize );

  do
    {
      RadeepHeader fragmentHeader = radeepHeader;

      if (p->GetSize () > offset + fragmentSize )
        {
          moreFragment = true;
          currentFragmentablePartSize = fragmentSize;
          fragmentHeader.SetMoreFragments ();
        }
      else
        {
          moreFragment = false;
          currentFragmentablePartSize = p->GetSize () - offset;
          if (!isLastFragment)
            {
              fragmentHeader.SetMoreFragments ();
            }
          else
            {
              fragmentHeader.SetLastFragment ();
            }
        }

      NS_LOG_LOGIC ("Fragment creation - " << offset << ", " << currentFragmentablePartSize  );
      Ptr<Packet> fragment = p->CreateFragment (offset, currentFragmentablePartSize);
      NS_LOG_LOGIC ("Fragment created - " << offset << ", " << fragment->GetSize ()  );

      fragmentHeader.SetFragmentOffset (offset+originalOffset);
      fragmentHeader.SetPayloadSize (currentFragmentablePartSize);

      if (Node::ChecksumEnabled ())
        {
          fragmentHeader.EnableChecksum ();
        }

      NS_LOG_LOGIC ("Fragment check - " << fragmentHeader.GetFragmentOffset ()  );

      NS_LOG_LOGIC ("New fragment Header " << fragmentHeader);

      std::ostringstream oss;
      oss << fragmentHeader;
      fragment->Print (oss);

      NS_LOG_LOGIC ("New fragment " << *fragment);

      listFragments.push_back (RadeepPayloadHeaderPair (fragment, fragmentHeader));

      offset += currentFragmentablePartSize;

    }
  while (moreFragment);

  return;
}

bool
RadeepL3Protocol::ProcessFragment (Ptr<Packet>& packet, RadeepHeader& radeepHeader, uint32_t iif)
{
  NS_LOG_FUNCTION (this << packet << radeepHeader << iif);

  uint64_t addressCombination = uint64_t (radeepHeader.GetSource ().Get ()) << 32 | uint64_t (radeepHeader.GetDestination ().Get ());
  uint32_t idProto = uint32_t (radeepHeader.GetIdentification ()) << 16 | uint32_t (radeepHeader.GetProtocol ());
  std::pair<uint64_t, uint32_t> key;
  bool ret = false;
  Ptr<Packet> p = packet->Copy ();

  key.first = addressCombination;
  key.second = idProto;

  Ptr<Fragments> fragments;

  MapFragments_t::iterator it = m_fragments.find (key);
  if (it == m_fragments.end ())
    {
      fragments = Create<Fragments> ();
      m_fragments.insert (std::make_pair (key, fragments));
      m_fragmentsTimers[key] = Simulator::Schedule (m_fragmentExpirationTimeout,
                                                    &RadeepL3Protocol::HandleFragmentsTimeout, this,
                                                    key, radeepHeader, iif);
    }
  else
    {
      fragments = it->second;
    }

  NS_LOG_LOGIC ("Adding fragment - Size: " << packet->GetSize ( ) << " - Offset: " << (radeepHeader.GetFragmentOffset ()) );

  fragments->AddFragment (p, radeepHeader.GetFragmentOffset (), !radeepHeader.IsLastFragment () );

  if ( fragments->IsEntire () )
    {
      packet = fragments->GetPacket ();
      fragments = 0;
      m_fragments.erase (key);
      if (m_fragmentsTimers[key].IsRunning ())
        {
          NS_LOG_LOGIC ("Stopping WaitFragmentsTimer at " << Simulator::Now ().GetSeconds () << " due to complete packet");
          m_fragmentsTimers[key].Cancel ();
        }
      m_fragmentsTimers.erase (key);
      ret = true;
    }

  return ret;
}

RadeepL3Protocol::Fragments::Fragments ()
  : m_moreFragment (0)
{
  NS_LOG_FUNCTION (this);
}

RadeepL3Protocol::Fragments::~Fragments ()
{
  NS_LOG_FUNCTION (this);
}

void
RadeepL3Protocol::Fragments::AddFragment (Ptr<Packet> fragment, uint16_t fragmentOffset, bool moreFragment)
{
  NS_LOG_FUNCTION (this << fragment << fragmentOffset << moreFragment);

  std::list<std::pair<Ptr<Packet>, uint16_t> >::iterator it;

  for (it = m_fragments.begin (); it != m_fragments.end (); it++)
    {
      if (it->second > fragmentOffset)
        {
          break;
        }
    }

  if (it == m_fragments.end ())
    {
      m_moreFragment = moreFragment;
    }

  m_fragments.insert (it, std::pair<Ptr<Packet>, uint16_t> (fragment, fragmentOffset));
}

bool
RadeepL3Protocol::Fragments::IsEntire () const
{
  NS_LOG_FUNCTION (this);

  bool ret = !m_moreFragment && m_fragments.size () > 0;

  if (ret)
    {
      uint16_t lastEndOffset = 0;

      for (std::list<std::pair<Ptr<Packet>, uint16_t> >::const_iterator it = m_fragments.begin (); it != m_fragments.end (); it++)
        {
          // overlapping fragments do exist
          NS_LOG_LOGIC ("Checking overlaps " << lastEndOffset << " - " << it->second );

          if (lastEndOffset < it->second)
            {
              ret = false;
              break;
            }
          // fragments might overlap in strange ways
          uint16_t fragmentEnd = it->first->GetSize () + it->second;
          lastEndOffset = std::max ( lastEndOffset, fragmentEnd );
        }
    }

  return ret;
}

Ptr<Packet>
RadeepL3Protocol::Fragments::GetPacket () const
{
  NS_LOG_FUNCTION (this);

  std::list<std::pair<Ptr<Packet>, uint16_t> >::const_iterator it = m_fragments.begin ();

  Ptr<Packet> p = it->first->Copy ();
  uint16_t lastEndOffset = p->GetSize ();
  it++;

  for ( ; it != m_fragments.end (); it++)
    {
      if ( lastEndOffset > it->second )
        {
          // The fragments are overlapping.
          // We do not overwrite the "old" with the "new" because we do not know when each arrived.
          // This is different from what Linux does.
          // It is not possible to emulate a fragmentation attack.
          uint32_t newStart = lastEndOffset - it->second;
          if ( it->first->GetSize () > newStart )
            {
              uint32_t newSize = it->first->GetSize () - newStart;
              Ptr<Packet> tempFragment = it->first->CreateFragment (newStart, newSize);
              p->AddAtEnd (tempFragment);
            }
        }
      else
        {
          NS_LOG_LOGIC ("Adding: " << *(it->first) );
          p->AddAtEnd (it->first);
        }
      lastEndOffset = p->GetSize ();
    }

  return p;
}

Ptr<Packet>
RadeepL3Protocol::Fragments::GetPartialPacket () const
{
  NS_LOG_FUNCTION (this);
  
  std::list<std::pair<Ptr<Packet>, uint16_t> >::const_iterator it = m_fragments.begin ();

  Ptr<Packet> p = Create<Packet> ();
  uint16_t lastEndOffset = 0;

  if ( m_fragments.begin ()->second > 0 )
    {
      return p;
    }

  for ( it = m_fragments.begin (); it != m_fragments.end (); it++)
    {
      if ( lastEndOffset > it->second )
        {
          uint32_t newStart = lastEndOffset - it->second;
          uint32_t newSize = it->first->GetSize () - newStart;
          Ptr<Packet> tempFragment = it->first->CreateFragment (newStart, newSize);
          p->AddAtEnd (tempFragment);
        }
      else if ( lastEndOffset == it->second )
        {
          NS_LOG_LOGIC ("Adding: " << *(it->first) );
          p->AddAtEnd (it->first);
        }
      lastEndOffset = p->GetSize ();
    }

  return p;
}

void
RadeepL3Protocol::HandleFragmentsTimeout (std::pair<uint64_t, uint32_t> key, RadeepHeader & radeepHeader, uint32_t iif)
{
  NS_LOG_FUNCTION (this << &key << &radeepHeader << iif);

  MapFragments_t::iterator it = m_fragments.find (key);
  Ptr<Packet> packet = it->second->GetPartialPacket ();

  // if we have at least 8 bytes, we can send an ICMP.
  if ( packet->GetSize () > 8 )
    {
      Ptr<Icmpv4L4Protocol> icmp = GetIcmp ();
      icmp->SendTimeExceededTtl (radeepHeader, packet, true);
    }
  m_dropTrace (radeepHeader, packet, DROP_FRAGMENT_TIMEOUT, m_node->GetObject<Radeep> (), iif);

  // clear the buffers
  it->second = 0;

  m_fragments.erase (key);
  m_fragmentsTimers.erase (key);
}
} // namespace ns3
