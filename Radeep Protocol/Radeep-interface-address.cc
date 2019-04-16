
#include "ns3/log.h"
#include "ns3/assert.h"
#include "Radeep-interface-address.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RadeepInterfaceAddress");

RadeepInterfaceAddress::RadeepInterfaceAddress ()
  : m_scope (GLOBAL), 
    m_secondary (false)
{
  NS_LOG_FUNCTION (this);
}

RadeepInterfaceAddress::RadeepInterfaceAddress (RadeepAddress local, RadeepMask mask)
  : m_scope (GLOBAL), 
    m_secondary (false)
{
  NS_LOG_FUNCTION (this << local << mask);
  m_local = local;
  m_mask = mask;
  m_broadcast = RadeepAddress (local.Get () | (~mask.Get ()));
}

RadeepInterfaceAddress::RadeepInterfaceAddress (const RadeepInterfaceAddress &o)
  : m_local (o.m_local),
    m_mask (o.m_mask),
    m_broadcast (o.m_broadcast),
    m_scope (o.m_scope),
    m_secondary (o.m_secondary)
{
  NS_LOG_FUNCTION (this << &o);
}

void 
RadeepInterfaceAddress::SetLocal (RadeepAddress local)
{
  NS_LOG_FUNCTION (this << local);
  m_local = local;
}

RadeepAddress 
RadeepInterfaceAddress::GetLocal (void) const
{
  NS_LOG_FUNCTION (this);
  return m_local; 
}

void 
RadeepInterfaceAddress::SetMask (RadeepMask mask) 
{
  NS_LOG_FUNCTION (this << mask);
  m_mask = mask;
}

RadeepMask 
RadeepInterfaceAddress::GetMask (void) const
{
  NS_LOG_FUNCTION (this);
  return m_mask;
}

void 
RadeepInterfaceAddress::SetBroadcast (RadeepAddress broadcast)
{
  NS_LOG_FUNCTION (this << broadcast);
  m_broadcast = broadcast;
}

RadeepAddress 
RadeepInterfaceAddress::GetBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return m_broadcast;
}

void 
RadeepInterfaceAddress::SetScope (RadeepInterfaceAddress::InterfaceAddressScope_e scope)
{
  NS_LOG_FUNCTION (this << scope);
  m_scope = scope;
}

RadeepInterfaceAddress::InterfaceAddressScope_e 
RadeepInterfaceAddress::GetScope (void) const
{
  NS_LOG_FUNCTION (this);
  return m_scope;
}

bool 
RadeepInterfaceAddress::IsSecondary (void) const
{
  NS_LOG_FUNCTION (this);
  return m_secondary;
}

void 
RadeepInterfaceAddress::SetSecondary (void)
{
  NS_LOG_FUNCTION (this);
  m_secondary = true;
}

void 
RadeepInterfaceAddress::SetPrimary (void)
{
  NS_LOG_FUNCTION (this);
  m_secondary = false;
}

std::ostream& operator<< (std::ostream& os, const RadeepInterfaceAddress &addr)
{ 
  os << "m_local=" << addr.GetLocal () << "; m_mask=" <<
  addr.GetMask () << "; m_broadcast=" << addr.GetBroadcast () << "; m_scope=" << addr.GetScope () <<
  "; m_secondary=" << addr.IsSecondary ();
  return os;
} 

} // namespace ns3
