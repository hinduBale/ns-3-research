#include "Radeep-interface-container.h"
#include "ns3/node-list.h"
#include "ns3/names.h"

namespace ns3 {

RadeepInterfaceContainer::RadeepInterfaceContainer ()
{
}

void
RadeepInterfaceContainer::Add (const RadeepInterfaceContainer& other)
{
  for (InterfaceVector::const_iterator i = other.m_interfaces.begin (); i != other.m_interfaces.end (); i++)
    {
      m_interfaces.push_back (*i);
    }
}

RadeepInterfaceContainer::Iterator
RadeepInterfaceContainer::Begin (void) const
{
  return m_interfaces.begin ();
}

RadeepInterfaceContainer::Iterator
RadeepInterfaceContainer::End (void) const
{
  return m_interfaces.end ();
}

uint32_t
RadeepInterfaceContainer::GetN (void) const
{
  return m_interfaces.size ();
}

RadeepAddress
RadeepInterfaceContainer::GetAddress (uint32_t i, uint32_t j) const
{
  Ptr<Radeep> radeep = m_interfaces[i].first;
  uint32_t interface = m_interfaces[i].second;
  return #include "Radeep-interface-container.h"
#include "ns3/node-list.h"
#include "ns3/names.h"

namespace ns3 {

RadeepInterfaceContainer::RadeepInterfaceContainer ()
{
}

void
RadeepInterfaceContainer::Add (const RadeepInterfaceContainer& other)
{
  for (InterfaceVector::const_iterator i = other.m_interfaces.begin (); i != other.m_interfaces.end (); i++)
    {
      m_interfaces.push_back (*i);
    }
}

RadeepInterfaceContainer::Iterator
RadeepInterfaceContainer::Begin (void) const
{
  return m_interfaces.begin ();
}

RadeepInterfaceContainer::Iterator
RadeepInterfaceContainer::End (void) const
{
  return m_interfaces.end ();
}

uint32_t
RadeepInterfaceContainer::GetN (void) const
{
  return m_interfaces.size ();
}

RadeepAddress
RadeepInterfaceContainer::GetAddress (uint32_t i, uint32_t j) const
{
  Ptr<Radeep> radeep = m_interfaces[i].first;
  uint32_t interface = m_interfaces[i].second;
  return radeep->GetAddress (interface, j).GetLocal ();
}

void 
RadeepInterfaceContainer::SetMetric (uint32_t i, uint16_t metric)
{
  Ptr<Radeep> radeep = m_interfaces[i].first;
  uint32_t interface = m_interfaces[i].second;
  radeep->SetMetric (interface, metric);
}
void 
RadeepInterfaceContainer::Add (Ptr<Radeep> radeep, uint32_t interface)
{
  m_interfaces.push_back (std::make_pair (radeep, interface));
}
void RadeepInterfaceContainer::Add (std::pair<Ptr<Radeep>, uint32_t> a)
{
  Add (a.first, a.second);
}
void 
RadeepInterfaceContainer::Add (std::string radeepName, uint32_t interface)
{
  Ptr<Radeep> radeep = Names::Find<Radeep> (radeepName);
  m_interfaces.push_back (std::make_pair (radeep, interface));
}

std::pair<Ptr<Radeep>, uint32_t>
RadeepInterfaceContainer::Get (uint32_t i) const
{
  return m_interfaces[i];
}


} // namespace ns3
->GetAddress (interface, j).GetLocal ();
}

void 
RadeepInterfaceContainer::SetMetric (uint32_t i, uint16_t metric)
{
  Ptr<Radeep> radeep = m_interfaces[i].first;
  uint32_t interface = m_interfaces[i].second;
  radeep->SetMetric (interface, metric);
}
void 
RadeepInterfaceContainer::Add (Ptr<Radeep> radeep, uint32_t interface)
{
  m_interfaces.push_back (std::make_pair (radeep, interface));
}
void RadeepInterfaceContainer::Add (std::pair<Ptr<Radeep>, uint32_t> a)
{
  Add (a.first, a.second);
}
void 
RadeepInterfaceContainer::Add (std::string radeepName, uint32_t interface)
{
  Ptr<Radeep> radeep = Names::Find<Radeep> (radeepName);
  m_interfaces.push_back (std::make_pair (radeep, interface));
}

std::pair<Ptr<Radeep>, uint32_t>
RadeepInterfaceContainer::Get (uint32_t i) const
{
  return m_interfaces[i];
}


} // namespace ns3
