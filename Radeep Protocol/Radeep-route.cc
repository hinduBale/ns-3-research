#include "Radeep-route.h"
#include "ns3/net-device.h"
#include "ns3/assert.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RadeepRoute");

RadeepRoute::RadeepRoute ()
{
  NS_LOG_FUNCTION (this);
}

void
RadeepRoute::SetDestination (RadeepAddress dest)
{
  NS_LOG_FUNCTION (this << dest);
  m_dest = dest;
}

RadeepAddress
RadeepRoute::GetDestination (void) const
{
  NS_LOG_FUNCTION (this);
  return m_dest;
}

void
RadeepRoute::SetSource (RadeepAddress src)
{
  NS_LOG_FUNCTION (this << src);
  m_source = src;
}

RadeepAddress
RadeepRoute::GetSource (void) const
{
  NS_LOG_FUNCTION (this);
  return m_source;
}

void
RadeepRoute::SetGateway (RadeepAddress gw)
{
  NS_LOG_FUNCTION (this << gw);
  m_gateway = gw;
}

RadeepAddress
RadeepRoute::GetGateway (void) const
{
  NS_LOG_FUNCTION (this);
  return m_gateway;
}

void
RadeepRoute::SetOutputDevice (Ptr<NetDevice> outputDevice)
{
  NS_LOG_FUNCTION (this << outputDevice);
  m_outputDevice = outputDevice;
}

Ptr<NetDevice>
RadeepRoute::GetOutputDevice (void) const
{
  NS_LOG_FUNCTION (this);
  return m_outputDevice;
}

std::ostream& operator<< (std::ostream& os, RadeepRoute const& route)
{
  os << "source=" << route.GetSource () << " dest="<< route.GetDestination () <<" gw=" << route.GetGateway ();
  return os;
}

RadeepMulticastRoute::RadeepMulticastRoute ()
{
  NS_LOG_FUNCTION (this);
  m_ttls.clear ();
}

void 
RadeepMulticastRoute::SetGroup (const RadeepAddress group)
{
  NS_LOG_FUNCTION (this << group);
  m_group = group;
}

RadeepAddress 
RadeepMulticastRoute::GetGroup (void) const
{
  NS_LOG_FUNCTION (this);
  return m_group;
}

void 
RadeepMulticastRoute::SetOrigin (const RadeepAddress origin)
{
  NS_LOG_FUNCTION (this << origin);
  m_origin = origin;
}

RadeepAddress 
RadeepMulticastRoute::GetOrigin (void) const
{
  NS_LOG_FUNCTION (this);
  return m_origin;
}

void 
RadeepMulticastRoute::SetParent (uint32_t parent)
{
  NS_LOG_FUNCTION (this << parent);
  m_parent = parent;
}

uint32_t 
RadeepMulticastRoute::GetParent (void) const
{
  NS_LOG_FUNCTION (this);
  return m_parent;
}

void 
RadeepMulticastRoute::SetOutputTtl (uint32_t oif, uint32_t ttl)
{
  NS_LOG_FUNCTION (this << oif << ttl);
  if (ttl >= MAX_TTL)
    {
      // This TTL value effectively disables the interface
      std::map<uint32_t, uint32_t>::iterator iter;
      iter = m_ttls.find (oif);
      if (iter != m_ttls.end ())
        {
          m_ttls.erase (iter);
        }
    }
  else
    {
      m_ttls[oif] = ttl;
    }
}

std::map<uint32_t, uint32_t>
RadeepMulticastRoute::GetOutputTtlMap () const
{
  NS_LOG_FUNCTION (this);
  return(m_ttls);
}

}