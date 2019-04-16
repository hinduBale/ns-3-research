
#include <vector>
#include "ns3/log.h"
#include "ns3/ptr.h"
#include "ns3/names.h"
#include "ns3/node.h"
#include "ns3/Radeep.h"
#include "ns3/Radeep-route.h"
#include "ns3/Radeep-list-routing.h"
#include "ns3/assert.h"
#include "ns3/Radeep-address.h"
#include "ns3/Radeep-routing-protocol.h"
#include "Radeep-static-routing-helper.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RadeepStaticRoutingHelper");

RadeepStaticRoutingHelper::RadeepStaticRoutingHelper()
{
}

RadeepStaticRoutingHelper::RadeepStaticRoutingHelper (const RadeepStaticRoutingHelper &o)
{
}

RadeepStaticRoutingHelper*
RadeepStaticRoutingHelper::Copy (void) const
{
  return new RadeepStaticRoutingHelper (*this);
}

Ptr<RadeepRoutingProtocol>
RadeepStaticRoutingHelper::Create (Ptr<Node> node) const
{
  return CreateObject<RadeepStaticRouting> ();
}


Ptr<RadeepStaticRouting>
RadeepStaticRoutingHelper::GetStaticRouting (Ptr<Radeep> radeep) const
{
  NS_LOG_FUNCTION (this);
  Ptr<RadeepRoutingProtocol> radeeprp = radeep->GetRoutingProtocol ();
  NS_ASSERT_MSG (radeeprp, "No routing protocol associated with Radeep");
  if (DynamicCast<RadeepStaticRouting> (radeeprp))
    {
      NS_LOG_LOGIC ("Static routing found as the main Radeep routing protocol.");
      return DynamicCast<RadeepStaticRouting> (radeeprp); 
    } 
  if (DynamicCast<RadeepListRouting> (radeeprp))
    {
      Ptr<RadeepListRouting> lrp = DynamicCast<RadeepListRouting> (radeeprp);
      int16_t priority;
      for (uint32_t i = 0; i < lrp->GetNRoutingProtocols ();  i++)
        {
          NS_LOG_LOGIC ("Searching for static routing in list");
          Ptr<RadeepRoutingProtocol> temp = lrp->GetRoutingProtocol (i, priority);
          if (DynamicCast<RadeepStaticRouting> (temp))
            {
              NS_LOG_LOGIC ("Found static routing in list");
              return DynamicCast<RadeepStaticRouting> (temp);
            }
        }
    }
  NS_LOG_LOGIC ("Static routing not found");
  return 0;
}

void
RadeepStaticRoutingHelper::AddMulticastRoute (
  Ptr<Node> n,
  RadeepAddress source, 
  RadeepAddress group,
  Ptr<NetDevice> input, 
  NetDeviceContainer output)
{
  Ptr<Radeep> radeep = n->GetObject<Radeep> ();

  // We need to convert the NetDeviceContainer to an array of interface 
  // numbers
  std::vector<uint32_t> outputInterfaces;
  for (NetDeviceContainer::Iterator i = output.Begin (); i != output.End (); ++i)
    {
      Ptr<NetDevice> nd = *i;
      int32_t interface = radeep->GetInterfaceForDevice (nd);
      NS_ASSERT_MSG (interface >= 0,
                     "RadeepStaticRoutingHelper::AddMulticastRoute(): "
                     "Expected an interface associated with the device nd");
      outputInterfaces.push_back (interface);
    }

  int32_t inputInterface = radeep->GetInterfaceForDevice (input);
  NS_ASSERT_MSG (inputInterface >= 0,
                 "RadeepStaticRoutingHelper::AddMulticastRoute(): "
                 "Expected an interface associated with the device input");
  RadeepStaticRoutingHelper helper;
  Ptr<RadeepStaticRouting> radeepStaticRouting = helper.GetStaticRouting (radeep);
  if (!radeepStaticRouting)
    {
      NS_ASSERT_MSG (radeepStaticRouting,
                     "RadeepStaticRoutingHelper::SetDefaultMulticastRoute(): "
                     "Expected an RadeepStaticRouting associated with this node");
    }
  radeepStaticRouting->AddMulticastRoute (source, group, inputInterface, outputInterfaces);
}

void
RadeepStaticRoutingHelper::AddMulticastRoute (
  Ptr<Node> n,
  RadeepAddress source, 
  RadeepAddress group,
  std::string inputName, 
  NetDeviceContainer output)
{
  Ptr<NetDevice> input = Names::Find<NetDevice> (inputName);
  AddMulticastRoute (n, source, group, input, output);
}

void
RadeepStaticRoutingHelper::AddMulticastRoute (
  std::string nName,
  RadeepAddress source, 
  RadeepAddress group,
  Ptr<NetDevice> input, 
  NetDeviceContainer output)
{
  Ptr<Node> n = Names::Find<Node> (nName);
  AddMulticastRoute (n, source, group, input, output);
}

void
RadeepStaticRoutingHelper::AddMulticastRoute (
  std::string nName,
  RadeepAddress source, 
  RadeepAddress group,
  std::string inputName, 
  NetDeviceContainer output)
{
  Ptr<NetDevice> input = Names::Find<NetDevice> (inputName);
  Ptr<Node> n = Names::Find<Node> (nName);
  AddMulticastRoute (n, source, group, input, output);
}

void
RadeepStaticRoutingHelper::SetDefaultMulticastRoute (
  Ptr<Node> n, 
  Ptr<NetDevice> nd)
{
  Ptr<Radeep> radeep = n->GetObject<Radeep> ();
  int32_t interfaceSrc = radeep->GetInterfaceForDevice (nd);
  NS_ASSERT_MSG (interfaceSrc >= 0,
                 "RadeepStaticRoutingHelper::SetDefaultMulticastRoute(): "
                 "Expected an interface associated with the device");
  RadeepStaticRoutingHelper helper;
  Ptr<RadeepStaticRouting> radeepStaticRouting = helper.GetStaticRouting (radeep);
  if (!radeepStaticRouting)
    {
      NS_ASSERT_MSG (radeepStaticRouting, 
                     "RadeepStaticRoutingHelper::SetDefaultMulticastRoute(): "
                     "Expected an RadeepStaticRouting associated with this node");
    }
  radeepStaticRouting->SetDefaultMulticastRoute (interfaceSrc);
}

void
RadeepStaticRoutingHelper::SetDefaultMulticastRoute (
  Ptr<Node> n, 
  std::string ndName)
{
  Ptr<NetDevice> nd = Names::Find<NetDevice> (ndName);
  SetDefaultMulticastRoute (n, nd);
}

void
RadeepStaticRoutingHelper::SetDefaultMulticastRoute (
  std::string nName, 
  Ptr<NetDevice> nd)
{
  Ptr<Node> n = Names::Find<Node> (nName);
  SetDefaultMulticastRoute (n, nd);
}

void
RadeepStaticRoutingHelper::SetDefaultMulticastRoute (
  std::string nName, 
  std::string ndName)
{
  Ptr<Node> n = Names::Find<Node> (nName);
  Ptr<NetDevice> nd = Names::Find<NetDevice> (ndName);
  SetDefaultMulticastRoute (n, nd);
}

} // namespace ns3
