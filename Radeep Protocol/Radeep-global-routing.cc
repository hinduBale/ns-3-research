

#include <vector>
#include <iomanip>
#include "ns3/names.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/net-device.h"
#include "ns3/Radeep-route.h"
#include "ns3/Radeep-routing-table-entry.h"
#include "ns3/boolean.h"
#include "ns3/node.h"
#include "Radeep-global-routing.h"
#include "global-route-manager.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RadeepGlobalRouting");

NS_OBJECT_ENSURE_REGISTERED (RadeepGlobalRouting);

TypeId 
RadeepGlobalRouting::GetTypeId (void)
{ 
  static TypeId tid = TypeId ("ns3::RadeepGlobalRouting")
    .SetParent<Object> ()
    .SetGroupName ("Internet")
    .AddAttribute ("RandomEcmpRouting",
                   "Set to true if packets are randomly routed among ECMP; set to false for using only one route consistently",
                   BooleanValue (false),
                   MakeBooleanAccessor (&RadeepGlobalRouting::m_randomEcmpRouting),
                   MakeBooleanChecker ())
    .AddAttribute ("RespondToInterfaceEvents",
                   "Set to true if you want to dynamically recompute the global routes upon Interface notification events (up/down, or add/remove address)",
                   BooleanValue (false),
                   MakeBooleanAccessor (&RadeepGlobalRouting::m_respondToInterfaceEvents),
                   MakeBooleanChecker ())
  ;
  return tid;
}

RadeepGlobalRouting::RadeepGlobalRouting () 
  : m_randomEcmpRouting (false),
    m_respondToInterfaceEvents (false)
{
  NS_LOG_FUNCTION (this);

  m_rand = CreateObject<UniformRandomVariable> ();
}

RadeepGlobalRouting::~RadeepGlobalRouting ()
{
  NS_LOG_FUNCTION (this);
}

void 
RadeepGlobalRouting::AddHostRouteTo (RadeepAddress dest, 
                                   RadeepAddress nextHop, 
                                   uint32_t interface)
{
  NS_LOG_FUNCTION (this << dest << nextHop << interface);
  RadeepRoutingTableEntry *route = new RadeepRoutingTableEntry ();
  *route = RadeepRoutingTableEntry::CreateHostRouteTo (dest, nextHop, interface);
  m_hostRoutes.push_back (route);
}

void 
RadeepGlobalRouting::AddHostRouteTo (RadeepAddress dest, 
                                   uint32_t interface)
{
  NS_LOG_FUNCTION (this << dest << interface);
  RadeepRoutingTableEntry *route = new RadeepRoutingTableEntry ();
  *route = RadeepRoutingTableEntry::CreateHostRouteTo (dest, interface);
  m_hostRoutes.push_back (route);
}

void 
RadeepGlobalRouting::AddNetworkRouteTo (RadeepAddress network, 
                                      RadeepMask networkMask, 
                                      RadeepAddress nextHop, 
                                      uint32_t interface)
{
  NS_LOG_FUNCTION (this << network << networkMask << nextHop << interface);
  RadeepRoutingTableEntry *route = new RadeepRoutingTableEntry ();
  *route = RadeepRoutingTableEntry::CreateNetworkRouteTo (network,
                                                        networkMask,
                                                        nextHop,
                                                        interface);
  m_networkRoutes.push_back (route);
}

void 
RadeepGlobalRouting::AddNetworkRouteTo (RadeepAddress network, 
                                      RadeepMask networkMask, 
                                      uint32_t interface)
{
  NS_LOG_FUNCTION (this << network << networkMask << interface);
  RadeepRoutingTableEntry *route = new RadeepRoutingTableEntry ();
  *route = RadeepRoutingTableEntry::CreateNetworkRouteTo (network,
                                                        networkMask,
                                                        interface);
  m_networkRoutes.push_back (route);
}

void 
RadeepGlobalRouting::AddASExternalRouteTo (RadeepAddress network, 
                                         RadeepMask networkMask,
                                         RadeepAddress nextHop,
                                         uint32_t interface)
{
  NS_LOG_FUNCTION (this << network << networkMask << nextHop << interface);
  RadeepRoutingTableEntry *route = new RadeepRoutingTableEntry ();
  *route = RadeepRoutingTableEntry::CreateNetworkRouteTo (network,
                                                        networkMask,
                                                        nextHop,
                                                        interface);
  m_ASexternalRoutes.push_back (route);
}


Ptr<RadeepRoute>
RadeepGlobalRouting::LookupGlobal (RadeepAddress dest, Ptr<NetDevice> oif)
{
  NS_LOG_FUNCTION (this << dest << oif);
  NS_LOG_LOGIC ("Looking for route for destination " << dest);
  Ptr<RadeepRoute> rtentry = 0;
  // store all available routes that bring packets to their destination
  typedef std::vector<RadeepRoutingTableEntry*> RouteVec_t;
  RouteVec_t allRoutes;

  NS_LOG_LOGIC ("Number of m_hostRoutes = " << m_hostRoutes.size ());
  for (HostRoutesCI i = m_hostRoutes.begin (); 
       i != m_hostRoutes.end (); 
       i++) 
    {
      NS_ASSERT ((*i)->IsHost ());
      if ((*i)->GetDest ().IsEqual (dest)) 
        {
          if (oif != 0)
            {
              if (oif != m_radeep->GetNetDevice ((*i)->GetInterface ()))
                {
                  NS_LOG_LOGIC ("Not on requested interface, skipping");
                  continue;
                }
            }
          allRoutes.push_back (*i);
          NS_LOG_LOGIC (allRoutes.size () << "Found global host route" << *i); 
        }
    }
  if (allRoutes.size () == 0) // if no host route is found
    {
      NS_LOG_LOGIC ("Number of m_networkRoutes" << m_networkRoutes.size ());
      for (NetworkRoutesI j = m_networkRoutes.begin (); 
           j != m_networkRoutes.end (); 
           j++) 
        {
          RadeepMask mask = (*j)->GetDestNetworkMask ();
          RadeepAddress entry = (*j)->GetDestNetwork ();
          if (mask.IsMatch (dest, entry)) 
            {
              if (oif != 0)
                {
                  if (oif != m_radeep->GetNetDevice ((*j)->GetInterface ()))
                    {
                      NS_LOG_LOGIC ("Not on requested interface, skipping");
                      continue;
                    }
                }
              allRoutes.push_back (*j);
              NS_LOG_LOGIC (allRoutes.size () << "Found global network route" << *j);
            }
        }
    }
  if (allRoutes.size () == 0)  // consider external if no host/network found
    {
      for (ASExternalRoutesI k = m_ASexternalRoutes.begin ();
           k != m_ASexternalRoutes.end ();
           k++)
        {
          RadeepMask mask = (*k)->GetDestNetworkMask ();
          RadeepAddress entry = (*k)->GetDestNetwork ();
          if (mask.IsMatch (dest, entry))
            {
              NS_LOG_LOGIC ("Found external route" << *k);
              if (oif != 0)
                {
                  if (oif != m_radeep->GetNetDevice ((*k)->GetInterface ()))
                    {
                      NS_LOG_LOGIC ("Not on requested interface, skipping");
                      continue;
                    }
                }
              allRoutes.push_back (*k);
              break;
            }
        }
    }
  if (allRoutes.size () > 0 ) // if route(s) is found
    {
      // pick up one of the routes uniformly at random if random
      // ECMP routing is enabled, or always select the first route
      // consistently if random ECMP routing is disabled
      uint32_t selectIndex;
      if (m_randomEcmpRouting)
        {
          selectIndex = m_rand->GetInteger (0, allRoutes.size ()-1);
        }
      else 
        {
          selectIndex = 0;
        }
      RadeepRoutingTableEntry* route = allRoutes.at (selectIndex); 
      // create a RadeepRoute object from the selected routing table entry
      rtentry = Create<RadeepRoute> ();
      rtentry->SetDestination (route->GetDest ());
      /// \todo handle multi-address case
      rtentry->SetSource (m_radeep->GetAddress (route->GetInterface (), 0).GetLocal ());
      rtentry->SetGateway (route->GetGateway ());
      uint32_t interfaceIdx = route->GetInterface ();
      rtentry->SetOutputDevice (m_radeep->GetNetDevice (interfaceIdx));
      return rtentry;
    }
  else 
    {
      return 0;
    }
}

uint32_t 
RadeepGlobalRouting::GetNRoutes (void) const
{
  NS_LOG_FUNCTION (this);
  uint32_t n = 0;
  n += m_hostRoutes.size ();
  n += m_networkRoutes.size ();
  n += m_ASexternalRoutes.size ();
  return n;
}

RadeepRoutingTableEntry *
RadeepGlobalRouting::GetRoute (uint32_t index) const
{
  NS_LOG_FUNCTION (this << index);
  if (index < m_hostRoutes.size ())
    {
      uint32_t tmp = 0;
      for (HostRoutesCI i = m_hostRoutes.begin (); 
           i != m_hostRoutes.end (); 
           i++) 
        {
          if (tmp  == index)
            {
              return *i;
            }
          tmp++;
        }
    }
  index -= m_hostRoutes.size ();
  uint32_t tmp = 0;
  if (index < m_networkRoutes.size ())
    {
      for (NetworkRoutesCI j = m_networkRoutes.begin (); 
           j != m_networkRoutes.end ();
           j++)
        {
          if (tmp == index)
            {
              return *j;
            }
          tmp++;
        }
    }
  index -= m_networkRoutes.size ();
  tmp = 0;
  for (ASExternalRoutesCI k = m_ASexternalRoutes.begin (); 
       k != m_ASexternalRoutes.end (); 
       k++) 
    {
      if (tmp == index)
        {
          return *k;
        }
      tmp++;
    }
  NS_ASSERT (false);
  // quiet compiler.
  return 0;
}
void 
RadeepGlobalRouting::RemoveRoute (uint32_t index)
{
  NS_LOG_FUNCTION (this << index);
  if (index < m_hostRoutes.size ())
    {
      uint32_t tmp = 0;
      for (HostRoutesI i = m_hostRoutes.begin (); 
           i != m_hostRoutes.end (); 
           i++) 
        {
          if (tmp  == index)
            {
              NS_LOG_LOGIC ("Removing route " << index << "; size = " << m_hostRoutes.size ());
              delete *i;
              m_hostRoutes.erase (i);
              NS_LOG_LOGIC ("Done removing host route " << index << "; host route remaining size = " << m_hostRoutes.size ());
              return;
            }
          tmp++;
        }
    }
  index -= m_hostRoutes.size ();
  uint32_t tmp = 0;
  for (NetworkRoutesI j = m_networkRoutes.begin (); 
       j != m_networkRoutes.end (); 
       j++) 
    {
      if (tmp == index)
        {
          NS_LOG_LOGIC ("Removing route " << index << "; size = " << m_networkRoutes.size ());
          delete *j;
          m_networkRoutes.erase (j);
          NS_LOG_LOGIC ("Done removing network route " << index << "; network route remaining size = " << m_networkRoutes.size ());
          return;
        }
      tmp++;
    }
  index -= m_networkRoutes.size ();
  tmp = 0;
  for (ASExternalRoutesI k = m_ASexternalRoutes.begin (); 
       k != m_ASexternalRoutes.end ();
       k++)
    {
      if (tmp == index)
        {
          NS_LOG_LOGIC ("Removing route " << index << "; size = " << m_ASexternalRoutes.size ());
          delete *k;
          m_ASexternalRoutes.erase (k);
          NS_LOG_LOGIC ("Done removing network route " << index << "; network route remaining size = " << m_networkRoutes.size ());
          return;
        }
      tmp++;
    }
  NS_ASSERT (false);
}

int64_t
RadeepGlobalRouting::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_rand->SetStream (stream);
  return 1;
}

void
RadeepGlobalRouting::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  for (HostRoutesI i = m_hostRoutes.begin (); 
       i != m_hostRoutes.end (); 
       i = m_hostRoutes.erase (i)) 
    {
      delete (*i);
    }
  for (NetworkRoutesI j = m_networkRoutes.begin (); 
       j != m_networkRoutes.end (); 
       j = m_networkRoutes.erase (j)) 
    {
      delete (*j);
    }
  for (ASExternalRoutesI l = m_ASexternalRoutes.begin (); 
       l != m_ASexternalRoutes.end ();
       l = m_ASexternalRoutes.erase (l))
    {
      delete (*l);
    }

  RadeepRoutingProtocol::DoDispose ();
}

// Formatted like output of "route -n" command
void
RadeepGlobalRouting::PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit) const
{
  NS_LOG_FUNCTION (this << stream);
  std::ostream* os = stream->GetStream ();

  *os << "Node: " << m_radeep->GetObject<Node> ()->GetId ()
      << ", Time: " << Now().As (unit)
      << ", Local time: " << GetObject<Node> ()->GetLocalTime ().As (unit)
      << ", RadeepGlobalRouting table" << std::endl;

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
              flags << "H";
            }
          else if (route.IsGateway ())
            {
              flags << "G";
            }
          *os << std::setiosflags (std::ios::left) << std::setw (6) << flags.str ();
          // Metric not implemented
          *os << "-" << "      ";
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

Ptr<RadeepRoute>
RadeepGlobalRouting::RouteOutput (Ptr<Packet> p, const RadeepHeader &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
{
  NS_LOG_FUNCTION (this << p << &header << oif << &sockerr);
//
// First, see if this is a multicast packet we have a route for.  If we
// have a route, then send the packet down each of the specified interfaces.
//
  if (header.GetDestination ().IsMulticast ())
    {
      NS_LOG_LOGIC ("Multicast destination-- returning false");
      return 0; // Let other routing protocols try to handle this
    }
//
// See if this is a unicast packet we have a route for.
//
  NS_LOG_LOGIC ("Unicast destination- looking up");
  Ptr<RadeepRoute> rtentry = LookupGlobal (header.GetDestination (), oif);
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
RadeepGlobalRouting::RouteInput  (Ptr<const Packet> p, const RadeepHeader &header, Ptr<const NetDevice> idev,                             UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                                LocalDeliverCallback lcb, ErrorCallback ecb)
{ 
  NS_LOG_FUNCTION (this << p << header << header.GetSource () << header.GetDestination () << idev << &lcb << &ecb);
  // Check if input device supports IP
  NS_ASSERT (m_radeep->GetInterfaceForDevice (idev) >= 0);
  uint32_t iif = m_radeep->GetInterfaceForDevice (idev);

  if (m_radeep->IsDestinationAddress (header.GetDestination (), iif))
    {
      if (!lcb.IsNull ())
        {
          NS_LOG_LOGIC ("Local delivery to " << header.GetDestination ());
          lcb (p, header, iif);
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
      ecb (p, header, Socket::ERROR_NOROUTETOHOST);
      return true;
    }
  // Next, try to find a route
  NS_LOG_LOGIC ("Unicast destination- looking up global route");
  Ptr<RadeepRoute> rtentry = LookupGlobal (header.GetDestination ());
  if (rtentry != 0)
    {
      NS_LOG_LOGIC ("Found unicast destination- calling unicast callback");
      ucb (rtentry, p, header);
      return true;
    }
  else
    {
      NS_LOG_LOGIC ("Did not find unicast destination- returning false");
      return false; // Let other routing protocols try to handle this
                    // route request.
    }
}
void 
RadeepGlobalRouting::NotifyInterfaceUp (uint32_t i)
{
  NS_LOG_FUNCTION (this << i);
  if (m_respondToInterfaceEvents && Simulator::Now ().GetSeconds () > 0)  // avoid startup events
    {
      GlobalRouteManager::DeleteGlobalRoutes ();
      GlobalRouteManager::BuildGlobalRoutingDatabase ();
      GlobalRouteManager::InitializeRoutes ();
    }
}

void 
RadeepGlobalRouting::NotifyInterfaceDown (uint32_t i)
{
  NS_LOG_FUNCTION (this << i);
  if (m_respondToInterfaceEvents && Simulator::Now ().GetSeconds () > 0)  // avoid startup events
    {
      GlobalRouteManager::DeleteGlobalRoutes ();
      GlobalRouteManager::BuildGlobalRoutingDatabase ();
      GlobalRouteManager::InitializeRoutes ();
    }
}

void 
RadeepGlobalRouting::NotifyAddAddress (uint32_t interface, RadeepInterfaceAddress address)
{
  NS_LOG_FUNCTION (this << interface << address);
  if (m_respondToInterfaceEvents && Simulator::Now ().GetSeconds () > 0)  // avoid startup events
    {
      GlobalRouteManager::DeleteGlobalRoutes ();
      GlobalRouteManager::BuildGlobalRoutingDatabase ();
      GlobalRouteManager::InitializeRoutes ();
    }
}

void 
RadeepGlobalRouting::NotifyRemoveAddress (uint32_t interface, RadeepInterfaceAddress address)
{
  NS_LOG_FUNCTION (this << interface << address);
  if (m_respondToInterfaceEvents && Simulator::Now ().GetSeconds () > 0)  // avoid startup events
    {
      GlobalRouteManager::DeleteGlobalRoutes ();
      GlobalRouteManager::BuildGlobalRoutingDatabase ();
      GlobalRouteManager::InitializeRoutes ();
    }
}

void 
RadeepGlobalRouting::SetRadeep (Ptr<Radeep> radeep)
{
  NS_LOG_FUNCTION (this << radeep);
  NS_ASSERT (m_radeep == 0 && radeep != 0);
  m_radeep = radeep;
}


} // namespace ns3
