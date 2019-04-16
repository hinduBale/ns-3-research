#include "ns3/node.h"
#include "ns3/node-list.h"
#include "ns3/simulator.h"
#include "ns3/Radeep-routing-protocol.h"
#include "ns3/Radeep-list-routing.h"
#include "ns3/radeep-l3-protocol.h"
#include "ns3/Radeep-interface.h"
#include "ns3/arp-cache.h"
#include "ns3/names.h"
#include "Radeep-routing-helper.h"

namespace ns3 {

RadeepRoutingHelper::~RadeepRoutingHelper ()
{
}

void
RadeepRoutingHelper::PrintRoutingTableAllAt (Time printTime, Ptr<OutputStreamWrapper> stream, Time::Unit unit)
{
  for (uint32_t i = 0; i < NodeList::GetNNodes (); i++)
    {
      Ptr<Node> node = NodeList::GetNode (i);
      Simulator::Schedule (printTime, &RadeepRoutingHelper::Print, node, stream, unit);
    }
}

void
RadeepRoutingHelper::PrintRoutingTableAllEvery (Time printInterval, Ptr<OutputStreamWrapper> stream, Time::Unit unit)
{
  for (uint32_t i = 0; i < NodeList::GetNNodes (); i++)
    {
      Ptr<Node> node = NodeList::GetNode (i);
      Simulator::Schedule (printInterval, &RadeepRoutingHelper::PrintEvery, printInterval, node, stream, unit);
    }
}

void
RadeepRoutingHelper::PrintRoutingTableAt (Time printTime, Ptr<Node> node, Ptr<OutputStreamWrapper> stream, Time::Unit unit)
{
  Simulator::Schedule (printTime, &RadeepRoutingHelper::Print, node, stream, unit);
}

void
RadeepRoutingHelper::PrintRoutingTableEvery (Time printInterval, Ptr<Node> node, Ptr<OutputStreamWrapper> stream, Time::Unit unit)
{
  Simulator::Schedule (printInterval, &RadeepRoutingHelper::PrintEvery, printInterval, node, stream, unit);
}

void
RadeepRoutingHelper::Print (Ptr<Node> node, Ptr<OutputStreamWrapper> stream, Time::Unit unit)
{
  Ptr<Radeep> radeep = node->GetObject<Radeep> ();
  if (radeep)
    {
      Ptr<RadeepRoutingProtocol> rp = radeep->GetRoutingProtocol ();
      NS_ASSERT (rp);
      rp->PrintRoutingTable (stream, unit);
    }
}

void
RadeepRoutingHelper::PrintEvery (Time printInterval, Ptr<Node> node, Ptr<OutputStreamWrapper> stream, Time::Unit unit)
{
  Ptr<Radeep> radeep = node->GetObject<Radeep> ();
  if (radeep)
    {
      Ptr<RadeepRoutingProtocol> rp = radeep->GetRoutingProtocol ();
      NS_ASSERT (rp);
      rp->PrintRoutingTable (stream, unit);
      Simulator::Schedule (printInterval, &RadeepRoutingHelper::PrintEvery, printInterval, node, stream, unit);
    }
}

void
RadeepRoutingHelper::PrintNeighborCacheAllAt (Time printTime, Ptr<OutputStreamWrapper> stream)
{
  for (uint32_t i = 0; i < NodeList::GetNNodes (); i++)
    {
      Ptr<Node> node = NodeList::GetNode (i);
      Simulator::Schedule (printTime, &RadeepRoutingHelper::PrintArpCache, node, stream);
    }
}

void
RadeepRoutingHelper::PrintNeighborCacheAllEvery (Time printInterval, Ptr<OutputStreamWrapper> stream)
{
  for (uint32_t i = 0; i < NodeList::GetNNodes (); i++)
    {
      Ptr<Node> node = NodeList::GetNode (i);
      Simulator::Schedule (printInterval, &RadeepRoutingHelper::PrintArpCacheEvery, printInterval, node, stream);
    }
}

void
RadeepRoutingHelper::PrintNeighborCacheAt (Time printTime, Ptr<Node> node, Ptr<OutputStreamWrapper> stream)
{
  Simulator::Schedule (printTime, &RadeepRoutingHelper::PrintArpCache, node, stream);
}

void
RadeepRoutingHelper::PrintNeighborCacheEvery (Time printInterval,Ptr<Node> node, Ptr<OutputStreamWrapper> stream)
{
  Simulator::Schedule (printInterval, &RadeepRoutingHelper::PrintArpCacheEvery, printInterval, node, stream);
}

void
RadeepRoutingHelper::PrintArpCache (Ptr<Node> node, Ptr<OutputStreamWrapper> stream)
{
  Ptr<RadeepL3Protocol> radeep = node->GetObject<RadeepL3Protocol> ();
  if (radeep)
    {
      std::ostream* os = stream->GetStream ();

      *os << "ARP Cache of node ";
      std::string found = Names::FindName (node);
      if (Names::FindName (node) != "")
        {
          *os << found;
        }
      else
        {
          *os << static_cast<int> (node->GetId ());
        }
      *os << " at time " << Simulator::Now ().GetSeconds () << "\n";

      for (uint32_t i=0; i<radeep->GetNInterfaces(); i++)
        {
          Ptr<ArpCache> arpCache = radeep->GetInterface (i)->GetArpCache ();
          if (arpCache)
            {
              arpCache->PrintArpCache (stream);
            }
        }
    }
}

void
RadeepRoutingHelper::PrintArpCacheEvery (Time printInterval, Ptr<Node> node, Ptr<OutputStreamWrapper> stream)
{
  Ptr<RadeepL3Protocol> radeep = node->GetObject<RadeepL3Protocol> ();
  if (radeep)
    {
      std::ostream* os = stream->GetStream ();

      *os << "ARP Cache of node ";
      std::string found = Names::FindName (node);
      if (Names::FindName (node) != "")
        {
          *os << found;
        }
      else
        {
          *os << static_cast<int> (node->GetId ());
        }
      *os << " at time " << Simulator::Now ().GetSeconds () << "\n";

      for (uint32_t i=0; i<radeep->GetNInterfaces(); i++)
        {
          Ptr<ArpCache> arpCache = radeep->GetInterface (i)->GetArpCache ();
          if (arpCache)
            {
              arpCache->PrintArpCache (stream);
            }
        }
      Simulator::Schedule (printInterval, &RadeepRoutingHelper::PrintArpCacheEvery, printInterval, node, stream);
    }
}

} // namespace ns3
