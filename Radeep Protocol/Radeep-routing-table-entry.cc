#include "Radeep-routing-table-entry.h"
#include "ns3/assert.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RadeepRoutingTableEntry");

/*****************************************************
 *     Network RadeepRoutingTableEntry
 *****************************************************/

RadeepRoutingTableEntry::RadeepRoutingTableEntry ()
{
  NS_LOG_FUNCTION (this);
}

RadeepRoutingTableEntry::RadeepRoutingTableEntry (RadeepRoutingTableEntry const &route)
  : m_dest (route.m_dest),
    m_destNetworkMask (route.m_destNetworkMask),
    m_gateway (route.m_gateway),
    m_interface (route.m_interface)
{
  NS_LOG_FUNCTION (this << route);
}

RadeepRoutingTableEntry::RadeepRoutingTableEntry (RadeepRoutingTableEntry const *route)
  : m_dest (route->m_dest),
    m_destNetworkMask (route->m_destNetworkMask),
    m_gateway (route->m_gateway),
    m_interface (route->m_interface)
{
  NS_LOG_FUNCTION (this << route);
}

RadeepRoutingTableEntry::RadeepRoutingTableEntry (RadeepAddress dest,
                                              RadeepAddress gateway,
                                              uint32_t interface)
  : m_dest (dest),
    m_destNetworkMask (RadeepMask::GetOnes ()),
    m_gateway (gateway),
    m_interface (interface)
{
}
RadeepRoutingTableEntry::RadeepRoutingTableEntry (RadeepAddress dest,
                                              uint32_t interface)
  : m_dest (dest),
    m_destNetworkMask (RadeepMask::GetOnes ()),
    m_gateway (RadeepAddress::GetZero ()),
    m_interface (interface)
{
}
RadeepRoutingTableEntry::RadeepRoutingTableEntry (RadeepAddress network,
                                              RadeepMask networkMask,
                                              RadeepAddress gateway,
                                              uint32_t interface)
  : m_dest (network),
    m_destNetworkMask (networkMask),
    m_gateway (gateway),
    m_interface (interface)
{
  NS_LOG_FUNCTION (this << network << networkMask << gateway << interface);
}
RadeepRoutingTableEntry::RadeepRoutingTableEntry (RadeepAddress network,
                                              RadeepMask networkMask,
                                              uint32_t interface)
  : m_dest (network),
    m_destNetworkMask (networkMask),
    m_gateway (RadeepAddress::GetZero ()),
    m_interface (interface)
{
  NS_LOG_FUNCTION (this << network << networkMask << interface);
}

bool
RadeepRoutingTableEntry::IsHost (void) const
{
  NS_LOG_FUNCTION (this);
  if (m_destNetworkMask.IsEqual (RadeepMask::GetOnes ()))
    {
      return true;
    }
  else
    {
      return false;
    }
}
RadeepAddress
RadeepRoutingTableEntry::GetDest (void) const
{
  NS_LOG_FUNCTION (this);
  return m_dest;
}
bool
RadeepRoutingTableEntry::IsNetwork (void) const
{
  NS_LOG_FUNCTION (this);
  return !IsHost ();
}
bool
RadeepRoutingTableEntry::IsDefault (void) const
{
  NS_LOG_FUNCTION (this);
  if (m_dest.IsEqual (RadeepAddress::GetZero ()))
    {
      return true;
    }
  else
    {
      return false;
    }
}
RadeepAddress
RadeepRoutingTableEntry::GetDestNetwork (void) const
{
  NS_LOG_FUNCTION (this);
  return m_dest;
}
RadeepMask
RadeepRoutingTableEntry::GetDestNetworkMask (void) const
{
  NS_LOG_FUNCTION (this);
  return m_destNetworkMask;
}
bool
RadeepRoutingTableEntry::IsGateway (void) const
{
  NS_LOG_FUNCTION (this);
  if (m_gateway.IsEqual (RadeepAddress::GetZero ()))
    {
      return false;
    }
  else
    {
      return true;
    }
}
RadeepAddress
RadeepRoutingTableEntry::GetGateway (void) const
{
  NS_LOG_FUNCTION (this);
  return m_gateway;
}
uint32_t
RadeepRoutingTableEntry::GetInterface (void) const
{
  NS_LOG_FUNCTION (this);
  return m_interface;
}

RadeepRoutingTableEntry 
RadeepRoutingTableEntry::CreateHostRouteTo (RadeepAddress dest, 
                                          RadeepAddress nextHop,
                                          uint32_t interface)
{
  NS_LOG_FUNCTION_NOARGS ();
  return RadeepRoutingTableEntry (dest, nextHop, interface);
}
RadeepRoutingTableEntry 
RadeepRoutingTableEntry::CreateHostRouteTo (RadeepAddress dest,
                                          uint32_t interface)
{
  NS_LOG_FUNCTION_NOARGS ();
  return RadeepRoutingTableEntry (dest, interface);
}
RadeepRoutingTableEntry 
RadeepRoutingTableEntry::CreateNetworkRouteTo (RadeepAddress network, 
                                             RadeepMask networkMask,
                                             RadeepAddress nextHop,
                                             uint32_t interface)
{
  NS_LOG_FUNCTION_NOARGS ();
  return RadeepRoutingTableEntry (network, networkMask, 
                                nextHop, interface);
}
RadeepRoutingTableEntry 
RadeepRoutingTableEntry::CreateNetworkRouteTo (RadeepAddress network, 
                                             RadeepMask networkMask,
                                             uint32_t interface)
{
  NS_LOG_FUNCTION_NOARGS ();
  return RadeepRoutingTableEntry (network, networkMask, 
                                interface);
}
RadeepRoutingTableEntry 
RadeepRoutingTableEntry::CreateDefaultRoute (RadeepAddress nextHop, 
                                           uint32_t interface)
{
  NS_LOG_FUNCTION_NOARGS ();
  return RadeepRoutingTableEntry (RadeepAddress::GetZero (), RadeepMask::GetZero (), nextHop, interface);
}


std::ostream& operator<< (std::ostream& os, RadeepRoutingTableEntry const& route)
{
  if (route.IsDefault ())
    {
      NS_ASSERT (route.IsGateway ());
      os << "default out=" << route.GetInterface () << ", next hop=" << route.GetGateway ();
    }
  else if (route.IsHost ())
    {
      if (route.IsGateway ())
        {
          os << "host="<< route.GetDest () << 
          ", out=" << route.GetInterface () <<
          ", next hop=" << route.GetGateway ();
        }
      else
        {
          os << "host="<< route.GetDest () << 
          ", out=" << route.GetInterface ();
        }
    }
  else if (route.IsNetwork ()) 
    {
      if (route.IsGateway ())
        {
          os << "network=" << route.GetDestNetwork () <<
          ", mask=" << route.GetDestNetworkMask () <<
          ",out=" << route.GetInterface () <<
          ", next hop=" << route.GetGateway ();
        }
      else
        {
          os << "network=" << route.GetDestNetwork () <<
          ", mask=" << route.GetDestNetworkMask () <<
          ",out=" << route.GetInterface ();
        }
    }
  else
    {
      NS_ASSERT (false);
    }
  return os;
}

bool operator== (const RadeepRoutingTableEntry a, const RadeepRoutingTableEntry b)
{
  return (a.GetDest () == b.GetDest () && 
          a.GetDestNetworkMask () == b.GetDestNetworkMask () &&
          a.GetGateway () == b.GetGateway () &&
          a.GetInterface () == b.GetInterface ());
}

/*****************************************************
 *     RadeepMulticastRoutingTableEntry
 *****************************************************/

RadeepMulticastRoutingTableEntry::RadeepMulticastRoutingTableEntry ()
{
  NS_LOG_FUNCTION (this);
}

RadeepMulticastRoutingTableEntry::RadeepMulticastRoutingTableEntry (RadeepMulticastRoutingTableEntry const &route)
  :
    m_origin (route.m_origin),
    m_group (route.m_group),
    m_inputInterface (route.m_inputInterface),
    m_outputInterfaces (route.m_outputInterfaces)
{
  NS_LOG_FUNCTION (this << route);
}

RadeepMulticastRoutingTableEntry::RadeepMulticastRoutingTableEntry (RadeepMulticastRoutingTableEntry const *route)
  :
    m_origin (route->m_origin),
    m_group (route->m_group),
    m_inputInterface (route->m_inputInterface),
    m_outputInterfaces (route->m_outputInterfaces)
{
  NS_LOG_FUNCTION (this << route);
}

RadeepMulticastRoutingTableEntry::RadeepMulticastRoutingTableEntry (
  RadeepAddress origin, 
  RadeepAddress group, 
  uint32_t inputInterface, 
  std::vector<uint32_t> outputInterfaces)
{
  NS_LOG_FUNCTION (this << origin << group << inputInterface << &outputInterfaces);
  m_origin = origin;
  m_group = group;
  m_inputInterface = inputInterface;
  m_outputInterfaces = outputInterfaces;
}

RadeepAddress 
RadeepMulticastRoutingTableEntry::GetOrigin (void) const
{
  NS_LOG_FUNCTION (this);
  return m_origin;
}

RadeepAddress 
RadeepMulticastRoutingTableEntry::GetGroup (void) const
{
  NS_LOG_FUNCTION (this);
  return m_group;
}

uint32_t 
RadeepMulticastRoutingTableEntry::GetInputInterface (void) const
{
  NS_LOG_FUNCTION (this);
  return m_inputInterface;
}

uint32_t
RadeepMulticastRoutingTableEntry::GetNOutputInterfaces (void) const
{
  NS_LOG_FUNCTION (this);
  return m_outputInterfaces.size ();
}

uint32_t
RadeepMulticastRoutingTableEntry::GetOutputInterface (uint32_t n) const
{
  NS_LOG_FUNCTION (this << n);
  NS_ASSERT_MSG (n < m_outputInterfaces.size (),
                 "RadeepMulticastRoutingTableEntry::GetOutputInterface (): index out of bounds");

  return m_outputInterfaces[n];
}

std::vector<uint32_t>
RadeepMulticastRoutingTableEntry::GetOutputInterfaces (void) const
{
  NS_LOG_FUNCTION (this);
  return m_outputInterfaces;
}

RadeepMulticastRoutingTableEntry 
RadeepMulticastRoutingTableEntry::CreateMulticastRoute (
  RadeepAddress origin, 
  RadeepAddress group, 
  uint32_t inputInterface,
  std::vector<uint32_t> outputInterfaces)
{
  NS_LOG_FUNCTION_NOARGS ();
  return RadeepMulticastRoutingTableEntry (origin, group, inputInterface, outputInterfaces);
}

std::ostream& 
operator<< (std::ostream& os, RadeepMulticastRoutingTableEntry const& route)
{
  os << "origin=" << route.GetOrigin () << 
  ", group=" << route.GetGroup () <<
  ", input interface=" << route.GetInputInterface () <<
  ", output interfaces=";

  for (uint32_t i = 0; i < route.GetNOutputInterfaces (); ++i)
    {
      os << route.GetOutputInterface (i) << " ";

    }

  return os;
}

bool operator== (const RadeepMulticastRoutingTableEntry a, const RadeepMulticastRoutingTableEntry b)
{
  return (a.GetOrigin () == b.GetOrigin () && 
          a.GetGroup () == b.GetGroup () &&
          a.GetInputInterface () == b.GetInputInterface () &&
          a.GetOutputInterfaces () == b.GetOutputInterfaces ());
}

} // namespace ns3
