#include "Radeep-list-routing-helper.h"
#include "ns3/Radeep-list-routing.h"
#include "ns3/node.h"

namespace ns3 {

RadeepListRoutingHelper::RadeepListRoutingHelper()
{
}

RadeepListRoutingHelper::~RadeepListRoutingHelper()
{
  for (std::list<std::pair<const RadeepRoutingHelper *, int16_t> >::iterator i = m_list.begin ();
       i != m_list.end (); ++i)
    {
      delete i->first;
    }
}

RadeepListRoutingHelper::RadeepListRoutingHelper (const RadeepListRoutingHelper &o)
{
  std::list<std::pair<const RadeepRoutingHelper *, int16_t> >::const_iterator i;
  for (i = o.m_list.begin (); i != o.m_list.end (); ++i)
    {
      m_list.push_back (std::make_pair (const_cast<const RadeepRoutingHelper *> (i->first->Copy ()), i->second));
    }
}

RadeepListRoutingHelper* 
RadeepListRoutingHelper::Copy (void) const 
{
  return new RadeepListRoutingHelper (*this); 
}

void 
RadeepListRoutingHelper::Add (const RadeepRoutingHelper &routing, int16_t priority)
{
  m_list.push_back (std::make_pair (const_cast<const RadeepRoutingHelper *> (routing.Copy ()), priority));
}

Ptr<RadeepRoutingProtocol> 
RadeepListRoutingHelper::Create (Ptr<Node> node) const
{
  Ptr<RadeepListRouting> list = CreateObject<RadeepListRouting> ();
  for (std::list<std::pair<const RadeepRoutingHelper *, int16_t> >::const_iterator i = m_list.begin ();
       i != m_list.end (); ++i)
    {
      Ptr<RadeepRoutingProtocol> prot = i->first->Create (node);
      list->AddRoutingProtocol (prot,i->second);
    }
  return list;
}

} // namespace ns3
