#define NS_LOG_APPEND_CONTEXT                                   \
  if (m_radeep && m_radeep->GetObject<Node> ()) { \
      std::clog << Simulator::Now ().GetSeconds () \
                << " [node " << m_radeep->GetObject<Node> ()->GetId () << "] "; }

#include <iomanip>
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/simulator.h"
#include "ns3/Radeep-route.h"
#include "ns3/output-stream-wrapper.h"
#include "Radeep-static-routing.h"
#include "Radeep-routing-table-entry.h"

using std::make_pair;

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RadeepStaticRouting");

NS_OBJECT_ENSURE_REGISTERED (RadeepStaticRouting);

TypeId
RadeepStaticRouting::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RadeepStaticRouting")
    .SetParent<RadeepRoutingProtocol> ()
    .SetGroupName ("Internet")
    .AddConstructor<RadeepStaticRouting> ()
  ;
  return tid;
}

RadeepStaticRouting::RadeepStaticRouting () 
  : m_radeep (0)
{
  NS_LOG_FUNCTION (this);
}

void 
RadeepStaticRouting::AddNetworkRouteTo (RadeepAddress network, 
                                      RadeepMask networkMask, 
                                      RadeepAddress nextHop, 
                                      uint32_t interface,
                                      uint32_t metric)
{
  NS_LOG_FUNCTION (this << network << " " << networkMask << " " << nextHop << " " << interface << " " << metric);
  RadeepRoutingTableEntry *route = new RadeepRoutingTableEntry ();
  *route = RadeepRoutingTableEntry::CreateNetworkRouteTo (network,
                                                        networkMask,
                                                        nextHop,
                                                        interface);
  m_networkRoutes.push_back (make_pair (route,metric));
}

void 
RadeepStaticRouting::AddNetworkRouteTo (RadeepAddress network, 
                                      RadeepMask networkMask, 
                                      uint32_t interface,
                                      uint32_t metric)
{
  NS_LOG_FUNCTION (this << network << " " << networkMask << " " << interface << " " << metric);
  RadeepRoutingTableEntry *route = new RadeepRoutingTableEntry ();
  *route = RadeepRoutingTableEntry::CreateNetworkRouteTo (network,
                                                        networkMask,
                                                        interface);
  m_networkRoutes.push_back (make_pair (route,metric));
}

void 
RadeepStaticRouting::AddHostRouteTo (RadeepAddress dest, 
                                   RadeepAddress nextHop,
                                   uint32_t interface,
                                   uint32_t metric)
{
  NS_LOG_FUNCTION (this << dest << " " << nextHop << " " << interface << " " << metric);
  AddNetworkRouteTo (dest, RadeepMask::GetOnes (), nextHop, interface, metric);
}

void 
RadeepStaticRouting::AddHostRouteTo (RadeepAddress dest, 
                                   uint32_t interface,
                                   uint32_t metric)
{
  NS_LOG_FUNCTION (this << dest << " " << interface << " " << metric);
  AddNetworkRouteTo (dest, RadeepMask::GetOnes (), interface, metric);
}

void 
RadeepStaticRouting::SetDefaultRoute (RadeepAddress nextHop, 
                                    uint32_t interface,
                                    uint32_t metric)
{
  NS_LOG_FUNCTION (this << nextHop << " " << interface << " " << metric);
  AddNetworkRouteTo (RadeepAddress ("0.0.0.0"), RadeepMask::GetZero (), nextHop, interface, metric);
}

void 
RadeepStaticRouting::AddMulticastRoute (RadeepAddress origin,
                                      RadeepAddress group,
                                      uint32_t inputInterface,
                                      std::vector<uint32_t> outputInterfaces)
{
  NS_LOG_FUNCTION (this << origin << " " << group << " " << inputInterface << " " << &outputInterfaces);
  RadeepMulticastRoutingTableEntry *route = new RadeepMulticastRoutingTableEntry ();
  *route = RadeepMulticastRoutingTableEntry::CreateMulticastRoute (origin, group, 
                                                                 inputInterface, outputInterfaces);
  m_multicastRoutes.push_back (route);
}

// default multicast routes are stored as a network route
// these routes are _not_ consulted in the forwarding process-- only
// for originating packets
void 
RadeepStaticRouting::SetDefaultMulticastRoute (uint32_t outputInterface)
{
  NS_LOG_FUNCTION (this << outputInterface);
  RadeepRoutingTableEntry *route = new RadeepRoutingTableEntry ();
  RadeepAddress network = RadeepAddress ("224.0.0.0");
  RadeepMask networkMask = RadeepMask ("240.0.0.0");
  *route = RadeepRoutingTableEntry::CreateNetworkRouteTo (network,
                                                        networkMask,
                                                        outputInterface);
  m_networkRoutes.push_back (make_pair (route,0));
}

uint32_t 
RadeepStaticRouting::GetNMulticastRoutes (void) const
{
  NS_LOG_FUNCTION (this);
  return m_multicastRoutes.size ();
}

RadeepMulticastRoutingTableEntry
RadeepStaticRouting::GetMulticastRoute (uint32_t index) const
{
  NS_LOG_FUNCTION (this << index);
  NS_ASSERT_MSG (index < m_multicastRoutes.size (),
                 "RadeepStaticRouting::GetMulticastRoute ():  Index out of range");

  if (index < m_multicastRoutes.size ())
    {
      uint32_t tmp = 0;
      for (MulticastRoutesCI i = m_multicastRoutes.begin (); 
           i != m_multicastRoutes.end (); 
           i++) 
        {
          if (tmp  == index)
            {
              return *i;
            }
          tmp++;
        }
    }
  return 0;
}

bool
RadeepStaticRouting::RemoveMulticastRoute (RadeepAddress origin,
                                         RadeepAddress group,
                                         uint32_t inputInterface)
{
  NS_LOG_FUNCTION (this << origin << " " << group << " " << inputInterface);
  for (MulticastRoutesI i = m_multicastRoutes.begin (); 
       i != m_multicastRoutes.end (); 
       i++) 
    {
      RadeepMulticastRoutingTableEntry *route = *i;
      if (origin == route->GetOrigin () &&
          group == route->GetGroup () &&
          inputInterface == route->GetInputInterface ())
        {
          delete *i;
          m_multicastRoutes.erase (i);
          return true;
        }
    }
  return false;
}

void 
RadeepStaticRouting::RemoveMulticastRoute (uint32_t index)
{
  NS_LOG_FUNCTION (this << index);
  uint32_t tmp = 0;
  for (MulticastRoutesI i = m_multicastRoutes.begin (); 
       i != m_multicastRoutes.end (); 
       i++) 
    {
      if (tmp  == index)
        {
          delete *i;
          m_multicastRoutes.erase (i);
          return;
        }
      tmp++;
    }
}

Ptr<RadeepRoute>
RadeepStaticRouting::LookupStatic (RadeepAddress dest, Ptr<NetDevice> oif)
{
  NS_LOG_FUNCTION (this << dest << " " << oif);
  Ptr<RadeepRoute> rtentry = 0;
  uint16_t longest_mask = 0;
  uint32_t shortest_metric = 0xffffffff;
  /* when sending on local multicast, there have to be interface specified */
  if (dest.IsLocalMulticast ())
    {
      NS_ASSERT_MSG (oif, "Try to send on link-local multicast address, and no interface index is given!");

      rtentry = Create<RadeepRoute> ();
      rtentry->SetDestination (dest);
      rtentry->SetGateway (RadeepAddress::GetZero ());
      rtentry->SetOutputDevice (oif);
      rtentry->SetSource (m_radeep->GetAddress (m_radeep->GetInterfaceForDevice (oif), 0).GetLocal ());
      return rtentry;
    }


  for (NetworkRoutesI i = m_networkRoutes.begin (); 
       i != m_networkRoutes.end (); 
       i++) 
    {
      RadeepRoutingTableEntry *j=i->first;
      uint32_t metric =i->second;
      RadeepMask mask = (j)->GetDestNetworkMask ();
      uint16_t masklen = mask.GetPrefixLength ();
      RadeepAddress entry = (j)->GetDestNetwork ();
      NS_LOG_LOGIC ("Searching for route to " << dest << ", checking against route to " << entry << "/" << masklen);
      if (mask.IsMatch (dest, entry)) 
        {
          NS_LOG_LOGIC ("Found global network route " << j << ", mask length " << masklen << ", metric " << metric);
          if (oif != 0)
            {
              if (oif != m_radeep->GetNetDevice (j->GetInterface ()))
                {
                  NS_LOG_LOGIC ("Not on requested interface, skipping");
                  continue;
                }
            }
          if (masklen < longest_mask) // Not interested if got shorter mask
            {
              NS_LOG_LOGIC ("Previous match longer, skipping");
              continue;
            }
          if (masklen > longest_mask) // Reset metric if longer masklen
            {
              shortest_metric = 0xffffffff;
            }
          longest_mask = masklen;
          if (metric > shortest_metric)
            {
              NS_LOG_LOGIC ("Equal mask length, but previous metric shorter, skipping");
              continue;
            }
          shortest_metric = metric;
          RadeepRoutingTableEntry* route = (j);
          uint32_t interfaceIdx = route->GetInterface ();
          rtentry = Create<RadeepRoute> ();
          rtentry->SetDestination (route->GetDest ());
          rtentry->SetSource (m_radeep->SourceAddressSelection (interfaceIdx, route->GetDest ()));
          rtentry->SetGateway (route->GetGateway ());
          rtentry->SetOutputDevice (m_radeep->GetNetDevice (interfaceIdx));
          if (masklen == 32)
            {
              break;
            }
        }
    }
  if (rtentry != 0)
    {
      NS_LOG_LOGIC ("Matching route via " << rtentry->GetGateway () << " at the end");
    }
  else
    {
      NS_LOG_LOGIC ("No matching route to " << dest << " found");
    }
  return rtentry;
}

Ptr<RadeepMulticastRoute>
RadeepStaticRouting::LookupStatic (
  RadeepAddress origin, 
  RadeepAddress group,
  uint32_t    interface)
{
  NS_LOG_FUNCTION (this << origin << " " << group << " " << interface);
  Ptr<RadeepMulticastRoute> mrtentry = 0;

  for (MulticastRoutesI i = m_multicastRoutes.begin (); 
       i != m_multicastRoutes.end (); 
       i++) 
    {
      RadeepMulticastRoutingTableEntry *route = *i;
//
// We've been passed an origin address, a multicast group address and an 
// interface index.  We have to decide if the current route in the list is
// a match.
//
// The first case is the restrictive case where the origin, group and index
// matches.
//
      if (origin == route->GetOrigin () && group == route->GetGroup ())
        {
          // Skipping this case (SSM) for now
          NS_LOG_LOGIC ("Found multicast source specific route" << *i);
        }
      if (group == route->GetGroup ())
        {
          if (interface == Radeep::IF_ANY || 
              interface == route->GetInputInterface ())
            {
              NS_LOG_LOGIC ("Found multicast route" << *i);
              mrtentry = Create<RadeepMulticastRoute> ();
              mrtentry->SetGroup (route->GetGroup ());
              mrtentry->SetOrigin (route->GetOrigin ());
              mrtentry->SetParent (route->GetInputInterface ());
              for (uint32_t j = 0; j < route->GetNOutputInterfaces (); j++)
                {
                  if (route->GetOutputInterface (j))
                    {
                      NS_LOG_LOGIC ("Setting output interface index " << route->GetOutputInterface (j));
                      mrtentry->SetOutputTtl (route->GetOutputInterface (j), RadeepMulticastRoute::MAX_TTL - 1);
                    }
                }
              return mrtentry;
            }
        }
    }
  return mrtentry;
}

uint32_t 
RadeepStaticRouting::GetNRoutes (void) const
{
  NS_LOG_FUNCTION (this);
  return m_networkRoutes.size ();;
}

RadeepRoutingTableEntry
RadeepStaticRouting::GetDefaultRoute ()
{
  NS_LOG_FUNCTION (this);
  // Basically a repeat of LookupStatic, retained for backward compatibility
  RadeepAddress dest ("0.0.0.0");
  uint32_t shortest_metric = 0xffffffff;
  RadeepRoutingTableEntry *result = 0;
  for (NetworkRoutesI i = m_networkRoutes.begin (); 
       i != m_networkRoutes.end (); 
       i++) 
    {
      RadeepRoutingTableEntry *j = i->first;
      uint32_t metric = i->second;
      RadeepMask mask = (j)->GetDestNetworkMask ();
      uint16_t masklen = mask.GetPrefixLength ();
      if (masklen != 0)
        {
          continue;
        }
      if (metric > shortest_metric)
        {
          continue;
        }
      shortest_metric = metric;
      result = j;
    }
  if (result)
    {
      return result;
    }
  else
    {
      return RadeepRoutingTableEntry ();
    }
}

RadeepRoutingTableEntry 
RadeepStaticRouting::GetRoute (uint32_t index) const
{
  NS_LOG_FUNCTION (this << index);
  uint32_t tmp = 0;
  for (NetworkRoutesCI j = m_networkRoutes.begin (); 
       j != m_networkRoutes.end (); 
       j++) 
    {
      if (tmp  == index)
        {
          return j->first;
        }
      tmp++;
    }
  NS_ASSERT (false);
  // quiet compiler.
  return 0;
}

uint32_t
RadeepStaticRouting::GetMetric (uint32_t index) const
{
  NS_LOG_FUNCTION (this << index);
  uint32_t tmp = 0;
  for (NetworkRoutesCI j = m_networkRoutes.begin ();
       j != m_networkRoutes.end (); 
       j++) 
    {
      if (tmp == index)
        {
          return j->second;
        }
      tmp++;
    }
  NS_ASSERT (false);
  // quiet compiler.
  return 0;
}
void 
RadeepStaticRouting::RemoveRoute (uint32_t index)
{
  NS_LOG_FUNCTION (this << index);
  uint32_t tmp = 0;
  for (NetworkRoutesI j = m_networkRoutes.begin (); 
       j != m_networkRoutes.end (); 
       j++) 
    {
      if (tmp == index)
        {
          delete j->first;
          m_networkRoutes.erase (j);
          return;
        }
      tmp++;
    }
  NS_ASSERT (false);
}

Ptr<RadeepRoute> 
RadeepStaticRouting::RouteOutput (Ptr<Packet> p, const RadeepHeader &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
{
  NS_LOG_FUNCTION (this << p<< header << oif << sockerr);
  RadeepAddress destination = header.GetDestination ();
  Ptr<RadeepRoute> rtentry = 0;

  // Multicast goes here
  if (destination.IsMulticast ())
    {
      // Note:  Multicast routes for outbound packets are stored in the
      // normal unicast table.  An implication of this is that it is not
      // possible to source multicast datagrams on multiple interfaces.
      // This is a well-known property of sockets implementation on 
      // many Unix variants.
      // So, we just log it and fall through to LookupStatic ()
      NS_LOG_LOGIC ("RouteOutput()::Multicast destination");
    }
  rtentry = LookupStatic (destination, oif);
  if (rtentry)
    { 
      sockerr = Socket::ERROR_NOTERROR;
    }
  else
    { 
      sockerr = Socket::ERROR_NOROUTETOHOST;
    }
  return rtentry;
}

bool 
RadeepStaticRouting::RouteInput  (Ptr<const Packet> p, const RadeepHeader &ipHeader, Ptr<const NetDevice> idev,
                                UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                                LocalDeliverCallback lcb, ErrorCallback ecb)
{
  NS_LOG_FUNCTION (this << p << ipHeader << ipHeader.GetSource () << ipHeader.GetDestination () << idev << &ucb << &mcb << &lcb << &ecb);

  NS_ASSERT (m_radeep != 0);
  // Check if input device supports IP 
  NS_ASSERT (m_radeep->GetInterfaceForDevice (idev) >= 0);
  uint32_t iif = m_radeep->GetInterfaceForDevice (idev); 

  // Multicast recognition; handle local delivery here

  if (ipHeader.GetDestination ().IsMulticast ())
    {
      NS_LOG_LOGIC ("Multicast destination");
      Ptr<RadeepMulticastRoute> mrtentry =  LookupStatic (ipHeader.GetSource (),
                                                        ipHeader.GetDestination (), m_radeep->GetInterfaceForDevice (idev));

      if (mrtentry)
        {
          NS_LOG_LOGIC ("Multicast route found");
          mcb (mrtentry, p, ipHeader); // multicast forwarding callback
          return true;
        }
      else
        {
          NS_LOG_LOGIC ("Multicast route not found");
          return false; // Let other routing protocols try to handle this
        }
    }

  if (m_radeep->IsDestinationAddress (ipHeader.GetDestination (), iif))
    {
      if (!lcb.IsNull ())
        {
          NS_LOG_LOGIC ("Local delivery to " << ipHeader.GetDestination ());
          lcb (p, ipHeader, iif);
          return true;
        }
      else
        {
          // The local delivery callback is null.  This may be a multicast
          // or broadcast packet, so return false so that another
          // multicast routing protocol can handle it.  It should be possible
          // to extend this to explicitly check whether it is a unicast
          // packet, and invoke the error callback if so
          return false;
        }
    }

  // Check if input device supports IP forwarding
  if (m_radeep->IsForwarding (iif) == false)
    {
      NS_LOG_LOGIC ("Forwarding disabled for this interface");
      ecb (p, ipHeader, Socket::ERROR_NOROUTETOHOST);
      return true;
    }
  // Next, try to find a route
  Ptr<RadeepRoute> rtentry = LookupStatic (ipHeader.GetDestination ());
  if (rtentry != 0)
    {
      NS_LOG_LOGIC ("Found unicast destination- calling unicast callback");
      ucb (rtentry, p, ipHeader);  // unicast forwarding callback
      return true;
    }
  else
    {
      NS_LOG_LOGIC ("Did not find unicast destination- returning false");
      return false; // Let other routing protocols try to handle this
    }
}

RadeepStaticRouting::~RadeepStaticRouting ()
{
  NS_LOG_FUNCTION (this);
}

void
RadeepStaticRouting::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  for (NetworkRoutesI j = m_networkRoutes.begin (); 
       j != m_networkRoutes.end (); 
       j = m_networkRoutes.erase (j)) 
    {
      delete (j->first);
    }
  for (MulticastRoutesI i = m_multicastRoutes.begin (); 
       i != m_multicastRoutes.end (); 
       i = m_multicastRoutes.erase (i)) 
    {
      delete (*i);
    }
  m_radeep = 0;
  RadeepRoutingProtocol::DoDispose ();
}

void 
RadeepStaticRouting::NotifyInterfaceUp (uint32_t i)
{
  NS_LOG_FUNCTION (this << i);
  // If interface address and network mask have been set, add a route
  // to the network of the interface (like e.g. ifconfig does on a
  // Linux box)
  for (uint32_t j = 0; j < m_radeep->GetNAddresses (i); j++)
    {
      if (m_radeep->GetAddress (i,j).GetLocal () != RadeepAddress () &&
          m_radeep->GetAddress (i,j).GetMask () != RadeepMask () &&
          m_radeep->GetAddress (i,j).GetMask () != RadeepMask::GetOnes ())
        {
          AddNetworkRouteTo (m_radeep->GetAddress (i,j).GetLocal ().CombineMask (m_radeep->GetAddress (i,j).GetMask ()),
                             m_radeep->GetAddress (i,j).GetMask (), i);
        }
    }
}

void 
RadeepStaticRouting::NotifyInterfaceDown (uint32_t i)
{
  NS_LOG_FUNCTION (this << i);
  // Remove all static routes that are going through this interface
  for (NetworkRoutesI it = m_networkRoutes.begin (); it != m_networkRoutes.end (); )
    {
      if (it->first->GetInterface () == i)
        {
          delete it->first;
          it = m_networkRoutes.erase (it);
        }
      else
        {
          it++;
        }
    }
}

void 
RadeepStaticRouting::NotifyAddAddress (uint32_t interface, RadeepInterfaceAddress address)
{
  NS_LOG_FUNCTION (this << interface << " " << address.GetLocal ());
  if (!m_radeep->IsUp (interface))
    {
      return;
    }

  RadeepAddress networkAddress = address.GetLocal ().CombineMask (address.GetMask ());
  RadeepMask networkMask = address.GetMask ();
  if (address.GetLocal () != RadeepAddress () &&
      address.GetMask () != RadeepMask ())
    {
      AddNetworkRouteTo (networkAddress,
                         networkMask, interface);
    }
}
void 
RadeepStaticRouting::NotifyRemoveAddress (uint32_t interface, RadeepInterfaceAddress address)
{
  NS_LOG_FUNCTION (this << interface << " " << address.GetLocal ());
  if (!m_radeep->IsUp (interface))
    {
      return;
    }
  RadeepAddress networkAddress = address.GetLocal ().CombineMask (address.GetMask ());
  RadeepMask networkMask = address.GetMask ();
  // Remove all static routes that are going through this interface
  // which reference this network
  for (NetworkRoutesI it = m_networkRoutes.begin (); it != m_networkRoutes.end (); )
    {
      if (it->first->GetInterface () == interface
          && it->first->IsNetwork ()
          && it->first->GetDestNetwork () == networkAddress
          && it->first->GetDestNetworkMask () == networkMask)
        {
          delete it->first;
          it = m_networkRoutes.erase (it);
        }
      else
        {
          it++;
        }
    }
}

void 
RadeepStaticRouting::SetRadeep (Ptr<Radeep> radeep)
{
  NS_LOG_FUNCTION (this << radeep);
  NS_ASSERT (m_radeep == 0 && Radeep != 0);
  m_radeep = radeep;
  for (uint32_t i = 0; i < m_radeep->GetNInterfaces (); i++)
    {
      if (m_radeep->IsUp (i))
        {
          NotifyInterfaceUp (i);
        }
      else
        {
          NotifyInterfaceDown (i);
        }
    }
}
// Formatted like output of "route -n" command
void
RadeepStaticRouting::PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit) const
{
  NS_LOG_FUNCTION (this << stream);
  std::ostream* os = stream->GetStream ();

  *os << "Node: " << m_radeep->GetObject<Node> ()->GetId ()
      << ", Time: " << Now().As (unit)
      << ", Local time: " << GetObject<Node> ()->GetLocalTime ().As (unit)
      << ", RadeepStaticRouting table" << std::endl;

  if (GetNRoutes () > 0)
    {
      *os << "Destination     Gateway         Genmask         Flags Metric Ref    Use Iface" << std::endl;
      for (uint32_t j = 0; j < GetNRoutes (); j++)
        {
          std::ostringstream dest, gw, mask, flags;
          RadeepRoutingTableEntry route = GetRoute (j);
          dest << route.GetDest ();
          *os << std::setiosflags (std::ios::left) << std::setw (16) << dest.str ();
          gw << route.GetGateway ();
          *os << std::setiosflags (std::ios::left) << std::setw (16) << gw.str ();
          mask << route.GetDestNetworkMask ();
          *os << std::setiosflags (std::ios::left) << std::setw (16) << mask.str ();
          flags << "U";
          if (route.IsHost ())
            {
              flags << "HS";
            }
          else if (route.IsGateway ())
            {
              flags << "GS";
            }
          *os << std::setiosflags (std::ios::left) << std::setw (6) << flags.str ();
          *os << std::setiosflags (std::ios::left) << std::setw (7) << GetMetric (j);
          // Ref ct not implemented
          *os << "-" << "      ";
          // Use not implemented
          *os << "-" << "   ";
          if (Names::FindName (m_radeep->GetNetDevice (route.GetInterface ())) != "")
            {
              *os << Names::FindName (m_radeep->GetNetDevice (route.GetInterface ()));
            }
          else
            {
              *os << route.GetInterface ();
            }
          *os << std::endl;
        }
    }
  *os << std::endl;
}

} 
