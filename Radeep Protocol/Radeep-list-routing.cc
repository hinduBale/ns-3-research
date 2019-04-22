#include "ns3/log.h"
#include "ns3/Radeep.h"
#include "ns3/Radeep-route.h"
#include "ns3/node.h"
#include "ns3/Radeep-static-routing.h"
#include "Radeep-list-routing.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RadeepListRouting");

NS_OBJECT_ENSURE_REGISTERED (RadeepListRouting);

TypeId
RadeepListRouting::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RadeepListRouting")
    .SetParent<RadeepRoutingProtocol> ()
    .SetGroupName ("Internet")
    .AddConstructor<RadeepListRouting> ()
  ;
  return tid;
}


RadeepListRouting::RadeepListRouting () 
  : m_radeep (0)
{
  NS_LOG_FUNCTION (this);
}

RadeepListRouting::~RadeepListRouting () 
{
  NS_LOG_FUNCTION (this);
}

void
RadeepListRouting::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  for (RadeepRoutingProtocolList::iterator rprotoIter = m_routingProtocols.begin ();
       rprotoIter != m_routingProtocols.end (); rprotoIter++)
    {
      // Note:  Calling dispose on these protocols causes memory leak
      //        The routing protocols should not maintain a pointer to
      //        this object, so Dispose() shouldn't be necessary.
      (*rprotoIter).second = 0;
    }
  m_routingProtocols.clear ();
  m_radeep = 0;
}

void
RadeepListRouting::PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit) const
{
  NS_LOG_FUNCTION (this << stream);
  *stream->GetStream () << "Node: " << m_radeep->GetObject<Node> ()->GetId () 
                        << ", Time: " << Now().As (unit)
                        << ", Local time: " << GetObject<Node> ()->GetLocalTime ().As (unit)
                        << ", RadeepListRouting table" << std::endl;
  for (RadeepRoutingProtocolList::const_iterator i = m_routingProtocols.begin ();
       i != m_routingProtocols.end (); i++)
    {
      *stream->GetStream () << "  Priority: " << (*i).first << " Protocol: " << (*i).second->GetInstanceTypeId () << std::endl;
      (*i).second->PrintRoutingTable (stream, unit);
    }
}

void
RadeepListRouting::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  for (RadeepRoutingProtocolList::iterator rprotoIter = m_routingProtocols.begin ();
       rprotoIter != m_routingProtocols.end (); rprotoIter++)
    {
      Ptr<RadeepRoutingProtocol> protocol = (*rprotoIter).second;
      protocol->Initialize ();
    }
  RadeepRoutingProtocol::DoInitialize ();
}


Ptr<RadeepRoute>
RadeepListRouting::RouteOutput (Ptr<Packet> p, const RadeepHeader &header, Ptr<NetDevice> oif, enum Socket::SocketErrno &sockerr)
{
  NS_LOG_FUNCTION (this << p << header.GetDestination () << header.GetSource () << oif << sockerr);
  Ptr<RadeepRoute> route;

  for (RadeepRoutingProtocolList::const_iterator i = m_routingProtocols.begin ();
       i != m_routingProtocols.end (); i++)
    {
      NS_LOG_LOGIC ("Checking protocol " << (*i).second->GetInstanceTypeId () << " with priority " << (*i).first);
      NS_LOG_LOGIC ("Requesting source address for destination " << header.GetDestination ());
      route = (*i).second->RouteOutput (p, header, oif, sockerr);
      if (route)
        {
          NS_LOG_LOGIC ("Found route " << route);
          sockerr = Socket::ERROR_NOTERROR;
          return route;
        }
    }
  NS_LOG_LOGIC ("Done checking " << GetTypeId ());
  NS_LOG_LOGIC ("");
  sockerr = Socket::ERROR_NOROUTETOHOST;
  return 0;
}

// Patterned after Linux Radeep_route_input and Radeep_route_input_slow
bool 
RadeepListRouting::RouteInput (Ptr<const Packet> p, const RadeepHeader &header, Ptr<const NetDevice> idev, 
                             UnicastForwardCallback ucb, MulticastForwardCallback mcb, 
                             LocalDeliverCallback lcb, ErrorCallback ecb)
{
  NS_LOG_FUNCTION (this << p << header << idev << &ucb << &mcb << &lcb << &ecb);
  bool retVal = false;
  NS_LOG_LOGIC ("RouteInput logic for node: " << m_radeep->GetObject<Node> ()->GetId ());

  NS_ASSERT (m_radeep != 0);
  // Check if input device supports Radeep 
  NS_ASSERT (m_radeep->GetInterfaceForDevice (idev) >= 0);
  uint32_t iif = m_radeep->GetInterfaceForDevice (idev); 

  retVal = m_radeep->IsDestinationAddress (header.GetDestination (), iif);
  if (retVal == true)
    {
      NS_LOG_LOGIC ("Address "<< header.GetDestination () << " is a match for local delivery");
      if (header.GetDestination ().IsMulticast ())
        {
          Ptr<Packet> packetCopy = p->Copy ();
          lcb (packetCopy, header, iif);
          retVal = true;
          // Fall through
        }
      else
        {
          lcb (p, header, iif);
          return true;
        }
    }
  // Check if input device supports Radeep forwarding
  if (m_radeep->IsForwarding (iif) == false)
    {
      NS_LOG_LOGIC ("Forwarding disabled for this interface");
      ecb (p, header, Socket::ERROR_NOROUTETOHOST);
      return true;
    }
  // Next, try to find a route
  // If we have already delivered a packet locally (e.g. multicast)
  // we suppress further downstream local delivery by nulling the callback
  LocalDeliverCallback downstreamLcb = lcb;
  if (retVal == true)
    {
      downstreamLcb = MakeNullCallback<void, Ptr<const Packet>, const RadeepHeader &, uint32_t > ();
    }
  for (RadeepRoutingProtocolList::const_iterator rprotoIter =
         m_routingProtocols.begin ();
       rprotoIter != m_routingProtocols.end ();
       rprotoIter++)
    {
      if ((*rprotoIter).second->RouteInput (p, header, idev, ucb, mcb, downstreamLcb, ecb))
        {
          NS_LOG_LOGIC ("Route found to forward packet in protocol " << (*rprotoIter).second->GetInstanceTypeId ().GetName ()); 
          return true;
        }
    }
  // No routing protocol has found a route.
  return retVal;
}

void 
RadeepListRouting::NotifyInterfaceUp (uint32_t interface)
{
  NS_LOG_FUNCTION (this << interface);
  for (RadeepRoutingProtocolList::const_iterator rprotoIter =
         m_routingProtocols.begin ();
       rprotoIter != m_routingProtocols.end ();
       rprotoIter++)
    {
      (*rprotoIter).second->NotifyInterfaceUp (interface);
    }
}
void 
RadeepListRouting::NotifyInterfaceDown (uint32_t interface)
{
  NS_LOG_FUNCTION (this << interface);
  for (RadeepRoutingProtocolList::const_iterator rprotoIter =
         m_routingProtocols.begin ();
       rprotoIter != m_routingProtocols.end ();
       rprotoIter++)
    {
      (*rprotoIter).second->NotifyInterfaceDown (interface);
    }
}
void 
RadeepListRouting::NotifyAddAddress (uint32_t interface, RadeepInterfaceAddress address)
{
  NS_LOG_FUNCTION (this << interface << address);
  for (RadeepRoutingProtocolList::const_iterator rprotoIter =
         m_routingProtocols.begin ();
       rprotoIter != m_routingProtocols.end ();
       rprotoIter++)
    {
      (*rprotoIter).second->NotifyAddAddress (interface, address);
    }
}
void 
RadeepListRouting::NotifyRemoveAddress (uint32_t interface, RadeepInterfaceAddress address)
{
  NS_LOG_FUNCTION (this << interface << address);
  for (RadeepRoutingProtocolList::const_iterator rprotoIter =
         m_routingProtocols.begin ();
       rprotoIter != m_routingProtocols.end ();
       rprotoIter++)
    {
      (*rprotoIter).second->NotifyRemoveAddress (interface, address);
    }
}
void 
RadeepListRouting::SetRadeep (Ptr<Radeep> radeep)
{
  NS_LOG_FUNCTION (this << radeep);
  NS_ASSERT (m_radeep == 0);
  for (RadeepRoutingProtocolList::const_iterator rprotoIter =
         m_routingProtocols.begin ();
       rprotoIter != m_routingProtocols.end ();
       rprotoIter++)
    {
      (*rprotoIter).second->SetRadeep (radeep);
    }
  m_radeep = radeep;
}

void
RadeepListRouting::AddRoutingProtocol (Ptr<RadeepRoutingProtocol> routingProtocol, int16_t priority)
{
  NS_LOG_FUNCTION (this << routingProtocol->GetInstanceTypeId () << priority);
  m_routingProtocols.push_back (std::make_pair (priority, routingProtocol));
  m_routingProtocols.sort ( Compare );
  if (m_radeep != 0)
    {
      routingProtocol->SetRadeep (m_radeep);
    }
}

uint32_t 
RadeepListRouting::GetNRoutingProtocols (void) const
{
  NS_LOG_FUNCTION (this);
  return m_routingProtocols.size (); 
}

Ptr<RadeepRoutingProtocol> 
RadeepListRouting::GetRoutingProtocol (uint32_t index, int16_t& priority) const
{
  NS_LOG_FUNCTION (this << index << priority);
  if (index > m_routingProtocols.size ())
    {
      NS_FATAL_ERROR ("RadeepListRouting::GetRoutingProtocol():  index " << index << " out of range");
    }
  uint32_t i = 0;
  for (RadeepRoutingProtocolList::const_iterator rprotoIter = m_routingProtocols.begin ();
       rprotoIter != m_routingProtocols.end (); rprotoIter++, i++)
    {
      if (i == index)
        {
          priority = (*rprotoIter).first;
          return (*rprotoIter).second;
        }
    }
  return 0;
}

bool 
RadeepListRouting::Compare (const RadeepRoutingProtocolEntry& a, const RadeepRoutingProtocolEntry& b)
{
  NS_LOG_FUNCTION_NOARGS ();
  return a.first > b.first;
}


}