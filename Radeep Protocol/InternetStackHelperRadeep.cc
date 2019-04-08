/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 * Author: Faker Moatamri <faker.moatamri@sophia.inria.fr>
 */

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/object.h"
#include "ns3/names.h"
#include "ns3/Radeep.h"
#include "ns3/Radeep.h"
#include "ns3/ipv6.h"
#include "ns3/packet-socket-factory.h"
#include "ns3/config.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/net-device.h"
#include "ns3/callback.h"
#include "ns3/node.h"
#include "ns3/node-list.h"
#include "ns3/core-config.h"
#include "ns3/arp-l3-protocol.h"
#include "InternetStackHelperRadeep.h"
#include "ns3/Radeep-global-routing.h"
#include "ns3/Radeep-global-routing.h"
#include "ns3/Radeep-list-routing-helper.h"
#include "ns3/Radeep-static-routing-helper.h"
#include "ns3/Radeep-global-routing-helper.h"
#include "ns3/ipv6-static-routing-helper.h"
#include "ns3/ipv6-extension.h"
#include "ns3/ipv6-extension-demux.h"
#include "ns3/ipv6-extension-header.h"
#include "ns3/icmpv6-l4-protocol.h"
#include "ns3/global-router-interface.h"
#include "ns3/traffic-control-layer.h"
#include <limits>
#include <map>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("InternetStackHelper");

//
// Historically, the only context written to ascii traces was the protocol.
// Traces from the protocols include the interface, though.  It is not 
// possible to really determine where an event originated without including
// this.  If you want the additional context information, define 
// INTERFACE_CONTEXT.  If you want compatibility with the old-style traces
// comment it out.
//
#define INTERFACE_CONTEXT

//
// Things are going to work differently here with respect to trace file handling
// than in most places because the Tx and Rx trace sources we are interested in
// are going to multiplex receive and transmit callbacks for all Radeep and 
// interface pairs through one callback.  We want packets to or from each 
// distinct pair to go to an individual file, so we have got to demultiplex the
// Radeep and interface pair into a corresponding Ptr<PcapFileWrapper> at the 
// callback.
//
// A complication in this situation is that the trace sources are hooked on 
// a protocol basis.  There is no trace source hooked by an Radeep and interface
// pair.  This means that if we naively proceed to hook, say, a drop trace
// for a given Radeep with interface 0, and then hook for Radeep with interface 1
// we will hook the drop trace twice and get two callbacks per event.  What
// we need to do is to hook the event once, and that will result in a single
// callback per drop event, and the trace source will provide the interface
// which we filter on in the trace sink.
// 
// This has got to continue to work properly after the helper has been 
// destroyed; but must be cleaned up at the end of time to avoid leaks. 
// Global maps of protocol/interface pairs to file objects seems to fit the 
// bill.
//
typedef std::pair<Ptr<Radeep>, uint32_t> InterfacePairRadeep;  /**< Radeep/interface pair */
typedef std::map<InterfacePairRadeep, Ptr<PcapFileWrapper> > InterfaceFileMapRadeep;  /**< Radeep/interface and Pcap file wrapper container */
typedef std::map<InterfacePairRadeep, Ptr<OutputStreamWrapper> > InterfaceStreamMapRadeep;  /**< Radeep/interface and output stream container */

static InterfaceFileMapRadeep g_interfaceFileMapRadeep; /**< A mapping of Radeep/interface pairs to pcap files */
static InterfaceStreamMapRadeep g_interfaceStreamMapRadeep; /**< A mapping of Radeep/interface pairs to ascii streams */

typedef std::pair<Ptr<Ipv6>, uint32_t> InterfacePairIpv6;  /**< Ipv6/interface pair */
typedef std::map<InterfacePairIpv6, Ptr<PcapFileWrapper> > InterfaceFileMapIpv6;  /**< Ipv6/interface and Pcap file wrapper container */
typedef std::map<InterfacePairIpv6, Ptr<OutputStreamWrapper> > InterfaceStreamMapIpv6;  /**< Ipv6/interface and output stream container */

static InterfaceFileMapIpv6 g_interfaceFileMapIpv6; /**< A mapping of Ipv6/interface pairs to pcap files */
static InterfaceStreamMapIpv6 g_interfaceStreamMapIpv6; /**< A mapping of Ipv6/interface pairs to pcap files */

InternetStackHelper::InternetStackHelper ()
  : m_routing (0),
    m_routingv6 (0),
    m_radeepEnabled (true),
    m_ipv6Enabled (true),
    m_radeepArpJitterEnabled (true),
    m_ipv6NsRsJitterEnabled (true)

{
  Initialize ();
}

// private method called by both constructor and Reset ()
void
InternetStackHelper::Initialize ()
{
  SetTcp ("ns3::TcpL4Protocol");
  RadeepStaticRoutingHelper staticRouting;
  RadeepGlobalRoutingHelper globalRouting;
  RadeepListRoutingHelper listRouting;
  Ipv6StaticRoutingHelper staticRoutingv6;
  listRouting.Add (staticRouting, 0);
  listRouting.Add (globalRouting, -10);
  SetRoutingHelper (listRouting);
  SetRoutingHelper (staticRoutingv6);
}

InternetStackHelper::~InternetStackHelper ()
{
  delete m_routing;
  delete m_routingv6;
}

InternetStackHelper::InternetStackHelper (const InternetStackHelper &o)
{
  m_routing = o.m_routing->Copy ();
  m_routingv6 = o.m_routingv6->Copy ();
  m_radeepEnabled = o.m_radeepEnabled;
  m_ipv6Enabled = o.m_ipv6Enabled;
  m_tcpFactory = o.m_tcpFactory;
  m_radeepArpJitterEnabled = o.m_radeepArpJitterEnabled;
  m_ipv6NsRsJitterEnabled = o.m_ipv6NsRsJitterEnabled;
}

InternetStackHelper &
InternetStackHelper::operator = (const InternetStackHelper &o)
{
  if (this == &o)
    {
      return *this;
    }
  m_routing = o.m_routing->Copy ();
  m_routingv6 = o.m_routingv6->Copy ();
  return *this;
}

void
InternetStackHelper::Reset (void)
{
  delete m_routing;
  m_routing = 0;
  delete m_routingv6;
  m_routingv6 = 0;
  m_radeepEnabled = true;
  m_ipv6Enabled = true;
  m_radeepArpJitterEnabled = true;
  m_ipv6NsRsJitterEnabled = true;
  Initialize ();
}

void 
InternetStackHelper::SetRoutingHelper (const RadeepRoutingHelper &routing)
{
  delete m_routing;
  m_routing = routing.Copy ();
}

void
InternetStackHelper::SetRoutingHelper (const Ipv6RoutingHelper &routing)
{
  delete m_routingv6;
  m_routingv6 = routing.Copy ();
}

void
InternetStackHelper::SetradeepStackInstall (bool enable)
{
  m_radeepEnabled = enable;
}

void InternetStackHelper::SetIpv6StackInstall (bool enable)
{
  m_ipv6Enabled = enable;
}

void InternetStackHelper::SetRadeepArpJitter (bool enable)
{
  m_radeepArpJitterEnabled = enable;
}

void InternetStackHelper::SetIpv6NsRsJitter (bool enable)
{
  m_ipv6NsRsJitterEnabled = enable;
}

int64_t
InternetStackHelper::AssignStreams (NodeContainer c, int64_t stream)
{
  int64_t currentStream = stream;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<GlobalRouter> router = node->GetObject<GlobalRouter> ();
      if (router != 0)
        {
          Ptr<RadeepGlobalRouting> gr = router->GetRoutingProtocol ();
          if (gr != 0)
            {
              currentStream += gr->AssignStreams (currentStream);
            }
        }
      Ptr<Ipv6ExtensionDemux> demux = node->GetObject<Ipv6ExtensionDemux> ();
      if (demux != 0)
        {
          Ptr<Ipv6Extension> fe = demux->GetExtension (Ipv6ExtensionFragment::EXT_NUMBER);
          NS_ASSERT (fe);  // should always exist in the demux
          currentStream += fe->AssignStreams (currentStream);
        }
      Ptr<Radeep> radeep = node->GetObject<Radeep> ();
      if (radeep != 0)
        {
          Ptr<ArpL3Protocol> arpL3Protocol = radeep->GetObject<ArpL3Protocol> ();
          if (arpL3Protocol != 0)
            {
              currentStream += arpL3Protocol->AssignStreams (currentStream);
            }
        }
      Ptr<Ipv6> ipv6 = node->GetObject<Ipv6> ();
      if (ipv6 != 0)
        {
          Ptr<Icmpv6L4Protocol> icmpv6L4Protocol = ipv6->GetObject<Icmpv6L4Protocol> ();
          if (icmpv6L4Protocol != 0)
            {
              currentStream += icmpv6L4Protocol->AssignStreams (currentStream);
            }
        }
    }
  return (currentStream - stream);
}

void
InternetStackHelper::SetTcp (const std::string tid)
{
  m_tcpFactory.SetTypeId (tid);
}

void 
InternetStackHelper::SetTcp (std::string tid, std::string n0, const AttributeValue &v0)
{
  m_tcpFactory.SetTypeId (tid);
  m_tcpFactory.Set (n0,v0);
}

void 
InternetStackHelper::Install (NodeContainer c) const
{
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Install (*i);
    }
}

void 
InternetStackHelper::InstallAll (void) const
{
  Install (NodeContainer::GetGlobal ());
}

void
InternetStackHelper::CreateAndAggregateObjectFromTypeId (Ptr<Node> node, const std::string typeId)
{
  ObjectFactory factory;
  factory.SetTypeId (typeId);
  Ptr<Object> protocol = factory.Create <Object> ();
  node->AggregateObject (protocol);
}

void
InternetStackHelper::Install (Ptr<Node> node) const
{
  if (m_radeepEnabled)
    {
      if (node->GetObject<Radeep> () != 0)
        {
          NS_FATAL_ERROR ("InternetStackHelper::Install (): Aggregating " 
                          "an InternetStack to a node with an existing Radeep object");
          return;
        }

      CreateAndAggregateObjectFromTypeId (node, "ns3::ArpL3Protocol");
      CreateAndAggregateObjectFromTypeId (node, "ns3::RadeepL3Protocol");
      CreateAndAggregateObjectFromTypeId (node, "ns3::Icmpv4L4Protocol");
      if (m_radeepArpJitterEnabled == false)
        {
          Ptr<ArpL3Protocol> arp = node->GetObject<ArpL3Protocol> ();
          NS_ASSERT (arp);
          arp->SetAttribute ("RequestJitter", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));
        }
      // Set routing
      Ptr<Radeep> radeep = node->GetObject<Radeep> ();
      Ptr<RadeepRoutingProtocol> radeepRouting = m_routing->Create (node);
      radeep->SetRoutingProtocol (radeepRouting);
    }

  if (m_ipv6Enabled)
    {
      /* IPv6 stack */
      if (node->GetObject<Ipv6> () != 0)
        {
          NS_FATAL_ERROR ("InternetStackHelper::Install (): Aggregating " 
                          "an InternetStack to a node with an existing Ipv6 object");
          return;
        }

      CreateAndAggregateObjectFromTypeId (node, "ns3::Ipv6L3Protocol");
      CreateAndAggregateObjectFromTypeId (node, "ns3::Icmpv6L4Protocol");
      if (m_ipv6NsRsJitterEnabled == false)
        {
          Ptr<Icmpv6L4Protocol> icmpv6l4 = node->GetObject<Icmpv6L4Protocol> ();
          NS_ASSERT (icmpv6l4);
          icmpv6l4->SetAttribute ("SolicitationJitter", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));
        }
      // Set routing
      Ptr<Ipv6> ipv6 = node->GetObject<Ipv6> ();
      Ptr<Ipv6RoutingProtocol> ipv6Routing = m_routingv6->Create (node);
      ipv6->SetRoutingProtocol (ipv6Routing);

      /* register IPv6 extensions and options */
      ipv6->RegisterExtensions ();
      ipv6->RegisterOptions ();
    }

  if (m_radeepEnabled || m_ipv6Enabled)
    {
      CreateAndAggregateObjectFromTypeId (node, "ns3::TrafficControlLayer");
      CreateAndAggregateObjectFromTypeId (node, "ns3::UdpL4Protocol");
      node->AggregateObject (m_tcpFactory.Create<Object> ());
      Ptr<PacketSocketFactory> factory = CreateObject<PacketSocketFactory> ();
      node->AggregateObject (factory);
    }

  if (m_radeepEnabled)
    {
      Ptr<ArpL3Protocol> arp = node->GetObject<ArpL3Protocol> ();
      Ptr<TrafficControlLayer> tc = node->GetObject<TrafficControlLayer> ();
      NS_ASSERT (arp);
      NS_ASSERT (tc);
      arp->SetTrafficControl (tc);
    }
}

void
InternetStackHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  Install (node);
}

/**
 * \brief Sync function for Radeep packet - Pcap output
 * \param p smart pointer to the packet
 * \param Radeep smart pointer to the node's Radeep stack
 * \param interface incoming interface
 */
static void
RadeepL3ProtocolRxTxSink (Ptr<const Packet> p, Ptr<Radeep> radeep, uint32_t interface)
{
  NS_LOG_FUNCTION (p << radeep << interface);

  //
  // Since trace sources are independent of interface, if we hook a source
  // on a particular protocol we will get traces for all of its interfaces.
  // We need to filter this to only report interfaces for which the user 
  // has expressed interest.
  //
  InterfacePairRadeep pair = std::make_pair (radeep, interface);
  if (g_interfaceFileMapRadeep.find (pair) == g_interfaceFileMapRadeep.end ())
    {
      NS_LOG_INFO ("Ignoring packet to/from interface " << interface);
      return;
    }

  Ptr<PcapFileWrapper> file = g_interfaceFileMapRadeep[pair];
  file->Write (Simulator::Now (), p);
}

bool
InternetStackHelper::PcapHooked (Ptr<Radeep> radeep)
{
  for (  InterfaceFileMapRadeep::const_iterator i = g_interfaceFileMapRadeep.begin (); 
         i != g_interfaceFileMapRadeep.end (); 
         ++i)
    {
      if ((*i).first.first == radeep)
        {
          return true;
        }
    }
  return false;
}

void 
InternetStackHelper::EnablePcapRadeepInternal (std::string prefix, Ptr<Radeep> radeep, uint32_t interface, bool explicitFilename)
{
  NS_LOG_FUNCTION (prefix << radeep << interface);

  if (!m_radeepEnabled)
    {
      NS_LOG_INFO ("Call to enable Radeep pcap tracing but Radeep not enabled");
      return;
    }

  //
  // We have to create a file and a mapping from protocol/interface to file 
  // irrespective of how many times we want to trace a particular protocol.
  //
  PcapHelper pcapHelper;

  std::string filename;
  if (explicitFilename)
    {
      filename = prefix;
    }
  else
    {
      filename = pcapHelper.GetFilenameFromInterfacePair (prefix, radeep, interface);
    }

  Ptr<PcapFileWrapper> file = pcapHelper.CreateFile (filename, std::ios::out, PcapHelper::DLT_RAW);

  //
  // However, we only hook the trace source once to avoid multiple trace sink
  // calls per event (connect is independent of interface).
  //
  if (!PcapHooked (radeep))
    {
      //
      // Ptr<Radeep> is aggregated to node and RadeepL3Protocol is aggregated to 
      // node so we can get to RadeepL3Protocol through Radeep.
      //
      Ptr<RadeepL3Protocol> radeepL3Protocol = radeeo->GetObject<RadeepL3Protocol> ();
      NS_ASSERT_MSG (radeepL3Protocol, "InternetStackHelper::EnablePcapRadeepInternal(): "
                     "m_radeepEnabled and radeepL3Protocol inconsistent");

      bool result = radeepL3Protocol->TraceConnectWithoutContext ("Tx", MakeCallback (&RadeepL3ProtocolRxTxSink));
      NS_ASSERT_MSG (result == true, "InternetStackHelper::EnablePcapRadeepInternal():  "
                     "Unable to connect radeepL3Protocol \"Tx\"");

      result = radeepL3Protocol->TraceConnectWithoutContext ("Rx", MakeCallback (&RadeepL3ProtocolRxTxSink));
      NS_ASSERT_MSG (result == true, "InternetStackHelper::EnablePcapRadeepInternal():  "
                     "Unable to connect radeepL3Protocol \"Rx\"");
    }

  g_interfaceFileMapRadeep[std::make_pair (radeep, interface)] = file;
}

/**
 * \brief Sync function for IPv6 packet - Pcap output
 * \param p smart pointer to the packet
 * \param ipv6 smart pointer to the node's IPv6 stack
 * \param interface incoming interface
 */
static void
Ipv6L3ProtocolRxTxSink (Ptr<const Packet> p, Ptr<Ipv6> ipv6, uint32_t interface)
{
  NS_LOG_FUNCTION (p << ipv6 << interface);

  //
  // Since trace sources are independent of interface, if we hook a source
  // on a particular protocol we will get traces for all of its interfaces.
  // We need to filter this to only report interfaces for which the user 
  // has expressed interest.
  //
  InterfacePairIpv6 pair = std::make_pair (ipv6, interface);
  if (g_interfaceFileMapIpv6.find (pair) == g_interfaceFileMapIpv6.end ())
    {
      NS_LOG_INFO ("Ignoring packet to/from interface " << interface);
      return;
    }

  Ptr<PcapFileWrapper> file = g_interfaceFileMapIpv6[pair];
  file->Write (Simulator::Now (), p);
}

bool
InternetStackHelper::PcapHooked (Ptr<Ipv6> ipv6)
{
  for (  InterfaceFileMapIpv6::const_iterator i = g_interfaceFileMapIpv6.begin (); 
         i != g_interfaceFileMapIpv6.end (); 
         ++i)
    {
      if ((*i).first.first == ipv6)
        {
          return true;
        }
    }
  return false;
}

void 
InternetStackHelper::EnablePcapIpv6Internal (std::string prefix, Ptr<Ipv6> ipv6, uint32_t interface, bool explicitFilename)
{
  NS_LOG_FUNCTION (prefix << ipv6 << interface);

  if (!m_ipv6Enabled)
    {
      NS_LOG_INFO ("Call to enable Ipv6 pcap tracing but Ipv6 not enabled");
      return;
    }

  //
  // We have to create a file and a mapping from protocol/interface to file 
  // irrespective of how many times we want to trace a particular protocol.
  //
  PcapHelper pcapHelper;

  std::string filename;
  if (explicitFilename)
    {
      filename = prefix;
    }
  else
    {
      filename = pcapHelper.GetFilenameFromInterfacePair (prefix, ipv6, interface);
    }

  Ptr<PcapFileWrapper> file = pcapHelper.CreateFile (filename, std::ios::out, PcapHelper::DLT_RAW);

  //
  // However, we only hook the trace source once to avoid multiple trace sink
  // calls per event (connect is independent of interface).
  //
  if (!PcapHooked (ipv6))
    {
      //
      // Ptr<Ipv6> is aggregated to node and Ipv6L3Protocol is aggregated to 
      // node so we can get to Ipv6L3Protocol through Ipv6.
      //
      Ptr<Ipv6L3Protocol> ipv6L3Protocol = ipv6->GetObject<Ipv6L3Protocol> ();
      NS_ASSERT_MSG (ipv6L3Protocol, "InternetStackHelper::EnablePcapIpv6Internal(): "
                     "m_ipv6Enabled and ipv6L3Protocol inconsistent");

      bool result = ipv6L3Protocol->TraceConnectWithoutContext ("Tx", MakeCallback (&Ipv6L3ProtocolRxTxSink));
      NS_ASSERT_MSG (result == true, "InternetStackHelper::EnablePcapIpv6Internal():  "
                     "Unable to connect ipv6L3Protocol \"Tx\"");

      result = ipv6L3Protocol->TraceConnectWithoutContext ("Rx", MakeCallback (&Ipv6L3ProtocolRxTxSink));
      NS_ASSERT_MSG (result == true, "InternetStackHelper::EnablePcapIpv6Internal():  "
                     "Unable to connect ipv6L3Protocol \"Rx\"");
    }

  g_interfaceFileMapIpv6[std::make_pair (ipv6, interface)] = file;
}

/**
 * \brief Sync function for Radeep dropped packet - Ascii output
 * \param stream the output stream
 * \param header Radeep header
 * \param packet smart pointer to the packet
 * \param reason the reason for the dropping
 * \param Radeep smart pointer to the node's Radeep stack
 * \param interface incoming interface
 */
static void
RadeepL3ProtocolDropSinkWithoutContext (
  Ptr<OutputStreamWrapper> stream,
  RadeepHeader const &header, 
  Ptr<const Packet> packet,
  RadeepL3Protocol::DropReason reason, 
  Ptr<Radeep> radeep, 
  uint32_t interface)
{
  //
  // Since trace sources are independent of interface, if we hook a source
  // on a particular protocol we will get traces for all of its interfaces.
  // We need to filter this to only report interfaces for which the user 
  // has expressed interest.
  //
  InterfacePairRadeep pair = std::make_pair (radeep, interface);
  if (g_interfaceStreamMapRadeep.find (pair) == g_interfaceStreamMapRadeep.end ())
    {
      NS_LOG_INFO ("Ignoring packet to/from interface " << interface);
      return;
    }

  Ptr<Packet> p = packet->Copy ();
  p->AddHeader (header);
  *stream->GetStream () << "d " << Simulator::Now ().GetSeconds () << " " << *p << std::endl;
}

/**
 * \brief Sync function for Radeep transmitted packet - Ascii output
 * \param stream the output stream
 * \param packet smart pointer to the packet
 * \param Radeep smart pointer to the node's Radeep stack
 * \param interface incoming interface
 */
static void
RadeepL3ProtocolTxSinkWithoutContext (
  Ptr<OutputStreamWrapper> stream,
  Ptr<const Packet> packet,
  Ptr<Radeep> radeep, 
  uint32_t interface)
{
  InterfacePairRadeep pair = std::make_pair (radeep, interface);
  if (g_interfaceStreamMapRadeep.find (pair) == g_interfaceStreamMapRadeep.end ())
    {
      NS_LOG_INFO ("Ignoring packet to/from interface " << interface);
      return;
    }

  *stream->GetStream () << "t " << Simulator::Now ().GetSeconds () << " " << *packet << std::endl;
}

/**
 * \brief Sync function for Radeep received packet - Ascii output
 * \param stream the output stream
 * \param packet smart pointer to the packet
 * \param Radeep smart pointer to the node's Radeep stack
 * \param interface incoming interface
 */
static void
RadeepL3ProtocolRxSinkWithoutContext (
  Ptr<OutputStreamWrapper> stream,
  Ptr<const Packet> packet,
  Ptr<Radeep> radeep, 
  uint32_t interface)
{
  InterfacePairRadeep pair = std::make_pair (radeep, interface);
  if (g_interfaceStreamMapRadeep.find (pair) == g_interfaceStreamMapRadeep.end ())
    {
      NS_LOG_INFO ("Ignoring packet to/from interface " << interface);
      return;
    }

  *stream->GetStream () << "r " << Simulator::Now ().GetSeconds () << " " << *packet << std::endl;
}

/**
 * \brief Sync function for Radeep dropped packet - Ascii output
 * \param stream the output stream
 * \param context the context
 * \param header Radeep header
 * \param packet smart pointer to the packet
 * \param reason the reason for the dropping
 * \param Radeep smart pointer to the node's Radeep stack
 * \param interface incoming interface
 */
static void
RadeepL3ProtocolDropSinkWithContext (
  Ptr<OutputStreamWrapper> stream,
  std::string context,
  RadeepHeader const &header, 
  Ptr<const Packet> packet,
  RadeepL3Protocol::DropReason reason, 
  Ptr<Radeep> radeep, 
  uint32_t interface)
{
  //
  // Since trace sources are independent of interface, if we hook a source
  // on a particular protocol we will get traces for all of its interfaces.
  // We need to filter this to only report interfaces for which the user 
  // has expressed interest.
  //
  InterfacePairRadeep pair = std::make_pair (radeeo, interface);
  if (g_interfaceStreamMapRadeep.find (pair) == g_interfaceStreamMapRadeep.end ())
    {
      NS_LOG_INFO ("Ignoring packet to/from interface " << interface);
      return;
    }

  Ptr<Packet> p = packet->Copy ();
  p->AddHeader (header);
#ifdef INTERFACE_CONTEXT
  *stream->GetStream () << "d " << Simulator::Now ().GetSeconds () << " " << context << "(" << interface << ") " 
                        << *p << std::endl;
#else
  *stream->GetStream () << "d " << Simulator::Now ().GetSeconds () << " " << context << " "  << *p << std::endl;
#endif
}

/**
 * \brief Sync function for Radeep transmitted packet - Ascii output
 * \param stream the output stream
 * \param context the context
 * \param packet smart pointer to the packet
 * \param Radeep smart pointer to the node's Radeep stack
 * \param interface incoming interface
 */
static void
RadeepL3ProtocolTxSinkWithContext (
  Ptr<OutputStreamWrapper> stream,
  std::string context,
  Ptr<const Packet> packet,
  Ptr<Radeep> radeep, 
  uint32_t interface)
{
  InterfacePairRadeep pair = std::make_pair (radeep, interface);
  if (g_interfaceStreamMapRadeep.find (pair) == g_interfaceStreamMapRadeep.end ())
    {
      NS_LOG_INFO ("Ignoring packet to/from interface " << interface);
      return;
    }

#ifdef INTERFACE_CONTEXT
  *stream->GetStream () << "t " << Simulator::Now ().GetSeconds () << " " << context << "(" << interface << ") " 
                        << *packet << std::endl;
#else
  *stream->GetStream () << "t " << Simulator::Now ().GetSeconds () << " " << context << " "  << *packet << std::endl;
#endif
}

/**
 * \brief Sync function for Radeep received packet - Ascii output
 * \param stream the output stream
 * \param context the context
 * \param packet smart pointer to the packet
 * \param Radeep smart pointer to the node's Radeep stack
 * \param interface incoming interface
 */
static void
RadeepL3ProtocolRxSinkWithContext (
  Ptr<OutputStreamWrapper> stream,
  std::string context,
  Ptr<const Packet> packet,
  Ptr<Radeep> radeep, 
  uint32_t interface)
{
  InterfacePairRadeep pair = std::make_pair (radeep, interface);
  if (g_interfaceStreamMapRadeep.find (pair) == g_interfaceStreamMapRadeep.end ())
    {
      NS_LOG_INFO ("Ignoring packet to/from interface " << interface);
      return;
    }

#ifdef INTERFACE_CONTEXT
  *stream->GetStream () << "r " << Simulator::Now ().GetSeconds () << " " << context << "(" << interface << ") " 
                        << *packet << std::endl;
#else
  *stream->GetStream () << "r " << Simulator::Now ().GetSeconds () << " " << context << " "  << *packet << std::endl;
#endif
}

bool
InternetStackHelper::AsciiHooked (Ptr<Radeep> radeep)
{
  for (  InterfaceStreamMapRadeep::const_iterator i = g_interfaceStreamMapRadeep.begin (); 
         i != g_interfaceStreamMapRadeep.end (); 
         ++i)
    {
      if ((*i).first.first == radeep)
        {
          return true;
        }
    }
  return false;
}

void 
InternetStackHelper::EnableAsciiRadeepInternal (
  Ptr<OutputStreamWrapper> stream, 
  std::string prefix, 
  Ptr<Radeep> radeep, 
  uint32_t interface,
  bool explicitFilename)
{
  if (!m_radeepEnabled)
    {
      NS_LOG_INFO ("Call to enable Radeep ascii tracing but Radeep not enabled");
      return;
    }

  //
  // Our trace sinks are going to use packet printing, so we have to 
  // make sure that is turned on.
  //
  Packet::EnablePrinting ();

  //
  // If we are not provided an OutputStreamWrapper, we are expected to create 
  // one using the usual trace filename conventions and hook WithoutContext
  // since there will be one file per context and therefore the context would
  // be redundant.
  //
  if (stream == 0)
    {
      //
      // Set up an output stream object to deal with private ofstream copy 
      // constructor and lifetime issues.  Let the helper decide the actual
      // name of the file given the prefix.
      //
      // We have to create a stream and a mapping from protocol/interface to 
      // stream irrespective of how many times we want to trace a particular 
      // protocol.
      //
      AsciiTraceHelper asciiTraceHelper;

      std::string filename;
      if (explicitFilename)
        {
          filename = prefix;
        }
      else
        {
          filename = asciiTraceHelper.GetFilenameFromInterfacePair (prefix, radeep, interface);
        }

      Ptr<OutputStreamWrapper> theStream = asciiTraceHelper.CreateFileStream (filename);

      //
      // However, we only hook the trace sources once to avoid multiple trace sink
      // calls per event (connect is independent of interface).
      //
      if (!AsciiHooked (radeep))
        {
          //
          // We can use the default drop sink for the ArpL3Protocol since it has
          // the usual signature.  We can get to the Ptr<ArpL3Protocol> through
          // our Ptr<Radeep> since they must both be aggregated to the same node.
          //
          Ptr<ArpL3Protocol> arpL3Protocol = radeep->GetObject<ArpL3Protocol> ();
          asciiTraceHelper.HookDefaultDropSinkWithoutContext<ArpL3Protocol> (arpL3Protocol, "Drop", theStream);

          //
          // The drop sink for the RadeepL3Protocol uses a different signature than
          // the default sink, so we have to cook one up for ourselves.  We can get
          // to the Ptr<RadeepL3Protocol> through our Ptr<Radeep> since they must both 
          // be aggregated to the same node.
          //
          Ptr<RadeepL3Protocol> radeepL3Protocol = radeep->GetObject<RadeepL3Protocol> ();
          bool result = radeepL3Protocol->TraceConnectWithoutContext ("Drop",
                                                                    MakeBoundCallback (&RadeepL3ProtocolDropSinkWithoutContext, theStream));
          NS_ASSERT_MSG (result == true, "InternetStackHelper::EnableAsciiRadeepInternal():  "
                         "Unable to connect radeepL3Protocol \"Drop\"");
          result = radeepL3Protocol->TraceConnectWithoutContext ("Tx", 
                                                               MakeBoundCallback (&RadeepL3ProtocolTxSinkWithoutContext, theStream));
          NS_ASSERT_MSG (result == true, "InternetStackHelper::EnableAsciiRadeepInternal():  "
                         "Unable to connect radeepL3Protocol \"Tx\"");
          result = radeepL3Protocol->TraceConnectWithoutContext ("Rx", 
                                                               MakeBoundCallback (&RadeepL3ProtocolRxSinkWithoutContext, theStream));
          NS_ASSERT_MSG (result == true, "InternetStackHelper::EnableAsciiRadeepInternal():  "
                         "Unable to connect RadeepL3Protocol \"Rx\"");
        }

      g_interfaceStreamMapRadeep[std::make_pair (Radeep, interface)] = theStream;
      return;
    }

  //
  // If we are provided an OutputStreamWrapper, we are expected to use it, and
  // to provide a context.  We are free to come up with our own context if we
  // want, and use the AsciiTraceHelper Hook*WithContext functions, but for 
  // compatibility and simplicity, we just use Config::Connect and let it deal
  // with the context.
  //
  // We need to associate the Radeep/interface with a stream to express interest
  // in tracing events on that pair, however, we only hook the trace sources 
  // once to avoid multiple trace sink calls per event (connect is independent
  // of interface).
  //
  if (!AsciiHooked (Radeep))
    {
      Ptr<Node> node = Radeep->GetObject<Node> ();
      std::ostringstream oss;

      //
      // For the ARP Drop, we are going to use the default trace sink provided by 
      // the ascii trace helper.  There is actually no AsciiTraceHelper in sight 
      // here, but the default trace sinks are actually publicly available static 
      // functions that are always there waiting for just such a case.
      //
      oss << "/NodeList/" << node->GetId () << "/$ns3::ArpL3Protocol/Drop";
      Config::Connect (oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultDropSinkWithContext, stream));

      //
      // This has all kinds of parameters coming with, so we have to cook up our
      // own sink.
      //
      oss.str ("");
      oss << "/NodeList/" << node->GetId () << "/$ns3::RadeepL3Protocol/Drop";
      Config::Connect (oss.str (), MakeBoundCallback (&RadeepL3ProtocolDropSinkWithContext, stream));
      oss.str ("");
      oss << "/NodeList/" << node->GetId () << "/$ns3::RadeepL3Protocol/Tx";
      Config::Connect (oss.str (), MakeBoundCallback (&RadeepL3ProtocolTxSinkWithContext, stream));
      oss.str ("");
      oss << "/NodeList/" << node->GetId () << "/$ns3::RadeepL3Protocol/Rx";
      Config::Connect (oss.str (), MakeBoundCallback (&RadeepL3ProtocolRxSinkWithContext, stream));
    }

  g_interfaceStreamMapRadeep[std::make_pair (Radeep, interface)] = stream;
}

/**
 * \brief Sync function for IPv6 dropped packet - Ascii output
 * \param stream the output stream
 * \param header IPv6 header
 * \param packet smart pointer to the packet
 * \param reason the reason for the dropping
 * \param ipv6 smart pointer to the node's IPv6 stack
 * \param interface incoming interface
 */
static void
Ipv6L3ProtocolDropSinkWithoutContext (
  Ptr<OutputStreamWrapper> stream,
  Ipv6Header const &header, 
  Ptr<const Packet> packet,
  Ipv6L3Protocol::DropReason reason, 
  Ptr<Ipv6> ipv6, 
  uint32_t interface)
{
  //
  // Since trace sources are independent of interface, if we hook a source
  // on a particular protocol we will get traces for all of its interfaces.
  // We need to filter this to only report interfaces for which the user 
  // has expressed interest.
  //
  InterfacePairIpv6 pair = std::make_pair (ipv6, interface);
  if (g_interfaceStreamMapIpv6.find (pair) == g_interfaceStreamMapIpv6.end ())
    {
      NS_LOG_INFO ("Ignoring packet to/from interface " << interface);
      return;
    }

  Ptr<Packet> p = packet->Copy ();
  p->AddHeader (header);
  *stream->GetStream () << "d " << Simulator::Now ().GetSeconds () << " " << *p << std::endl;
}

/**
 * \brief Sync function for IPv6 transmitted packet - Ascii output
 * \param stream the output stream
 * \param packet smart pointer to the packet
 * \param ipv6 smart pointer to the node's IPv6 stack
 * \param interface incoming interface
 */
static void
Ipv6L3ProtocolTxSinkWithoutContext (
  Ptr<OutputStreamWrapper> stream,
  Ptr<const Packet> packet,
  Ptr<Ipv6> ipv6, 
  uint32_t interface)
{
  InterfacePairIpv6 pair = std::make_pair (ipv6, interface);
  if (g_interfaceStreamMapIpv6.find (pair) == g_interfaceStreamMapIpv6.end ())
    {
      NS_LOG_INFO ("Ignoring packet to/from interface " << interface);
      return;
    }

  *stream->GetStream () << "t " << Simulator::Now ().GetSeconds () << " " << *packet << std::endl;
}

/**
 * \brief Sync function for IPv6 received packet - Ascii output
 * \param stream the output stream
 * \param packet smart pointer to the packet
 * \param ipv6 smart pointer to the node's IPv6 stack
 * \param interface incoming interface
 */
static void
Ipv6L3ProtocolRxSinkWithoutContext (
  Ptr<OutputStreamWrapper> stream,
  Ptr<const Packet> packet,
  Ptr<Ipv6> ipv6, 
  uint32_t interface)
{
  InterfacePairIpv6 pair = std::make_pair (ipv6, interface);
  if (g_interfaceStreamMapIpv6.find (pair) == g_interfaceStreamMapIpv6.end ())
    {
      NS_LOG_INFO ("Ignoring packet to/from interface " << interface);
      return;
    }

  *stream->GetStream () << "r " << Simulator::Now ().GetSeconds () << " " << *packet << std::endl;
}

/**
 * \brief Sync function for IPv6 dropped packet - Ascii output
 * \param stream the output stream
 * \param context the context
 * \param header IPv6 header
 * \param packet smart pointer to the packet
 * \param reason the reason for the dropping
 * \param ipv6 smart pointer to the node's IPv6 stack
 * \param interface incoming interface
 */
static void
Ipv6L3ProtocolDropSinkWithContext (
  Ptr<OutputStreamWrapper> stream,
  std::string context,
  Ipv6Header const &header, 
  Ptr<const Packet> packet,
  Ipv6L3Protocol::DropReason reason, 
  Ptr<Ipv6> ipv6, 
  uint32_t interface)
{
  //
  // Since trace sources are independent of interface, if we hook a source
  // on a particular protocol we will get traces for all of its interfaces.
  // We need to filter this to only report interfaces for which the user 
  // has expressed interest.
  //
  InterfacePairIpv6 pair = std::make_pair (ipv6, interface);
  if (g_interfaceStreamMapIpv6.find (pair) == g_interfaceStreamMapIpv6.end ())
    {
      NS_LOG_INFO ("Ignoring packet to/from interface " << interface);
      return;
    }

  Ptr<Packet> p = packet->Copy ();
  p->AddHeader (header);
#ifdef INTERFACE_CONTEXT
  *stream->GetStream () << "d " << Simulator::Now ().GetSeconds () << " " << context << "(" << interface << ") " 
                        << *p << std::endl;
#else
  *stream->GetStream () << "d " << Simulator::Now ().GetSeconds () << " " << context << " " << *p << std::endl;
#endif
}

/**
 * \brief Sync function for IPv6 transmitted packet - Ascii output
 * \param stream the output stream
 * \param context the context
 * \param packet smart pointer to the packet
 * \param ipv6 smart pointer to the node's IPv6 stack
 * \param interface incoming interface
 */
static void
Ipv6L3ProtocolTxSinkWithContext (
  Ptr<OutputStreamWrapper> stream,
  std::string context,
  Ptr<const Packet> packet,
  Ptr<Ipv6> ipv6, 
  uint32_t interface)
{
  InterfacePairIpv6 pair = std::make_pair (ipv6, interface);
  if (g_interfaceStreamMapIpv6.find (pair) == g_interfaceStreamMapIpv6.end ())
    {
      NS_LOG_INFO ("Ignoring packet to/from interface " << interface);
      return;
    }

#ifdef INTERFACE_CONTEXT
  *stream->GetStream () << "t " << Simulator::Now ().GetSeconds () << " " << context << "(" << interface << ") " 
                        << *packet << std::endl;
#else
  *stream->GetStream () << "t " << Simulator::Now ().GetSeconds () << " " << context << " " << *packet << std::endl;
#endif
}

/**
 * \brief Sync function for IPv6 received packet - Ascii output
 * \param stream the output stream
 * \param context the context
 * \param packet smart pointer to the packet
 * \param ipv6 smart pointer to the node's IPv6 stack
 * \param interface incoming interface
 */
static void
Ipv6L3ProtocolRxSinkWithContext (
  Ptr<OutputStreamWrapper> stream,
  std::string context,
  Ptr<const Packet> packet,
  Ptr<Ipv6> ipv6, 
  uint32_t interface)
{
  InterfacePairIpv6 pair = std::make_pair (ipv6, interface);
  if (g_interfaceStreamMapIpv6.find (pair) == g_interfaceStreamMapIpv6.end ())
    {
      NS_LOG_INFO ("Ignoring packet to/from interface " << interface);
      return;
    }

#ifdef INTERFACE_CONTEXT
  *stream->GetStream () << "r " << Simulator::Now ().GetSeconds () << " " << context << "(" << interface << ") " 
                        << *packet << std::endl;
#else
  *stream->GetStream () << "r " << Simulator::Now ().GetSeconds () << " " << context << " " << *packet << std::endl;
#endif
}

bool
InternetStackHelper::AsciiHooked (Ptr<Ipv6> ipv6)
{
  for (  InterfaceStreamMapIpv6::const_iterator i = g_interfaceStreamMapIpv6.begin (); 
         i != g_interfaceStreamMapIpv6.end (); 
         ++i)
    {
      if ((*i).first.first == ipv6)
        {
          return true;
        }
    }
  return false;
}

void 
InternetStackHelper::EnableAsciiIpv6Internal (
  Ptr<OutputStreamWrapper> stream, 
  std::string prefix, 
  Ptr<Ipv6> ipv6, 
  uint32_t interface,
  bool explicitFilename)
{
  if (!m_ipv6Enabled)
    {
      NS_LOG_INFO ("Call to enable Ipv6 ascii tracing but Ipv6 not enabled");
      return;
    }

  //
  // Our trace sinks are going to use packet printing, so we have to 
  // make sure that is turned on.
  //
  Packet::EnablePrinting ();

  //
  // If we are not provided an OutputStreamWrapper, we are expected to create 
  // one using the usual trace filename conventions and do a hook WithoutContext
  // since there will be one file per context and therefore the context would
  // be redundant.
  //
  if (stream == 0)
    {
      //
      // Set up an output stream object to deal with private ofstream copy 
      // constructor and lifetime issues.  Let the helper decide the actual
      // name of the file given the prefix.
      //
      // We have to create a stream and a mapping from protocol/interface to 
      // stream irrespective of how many times we want to trace a particular 
      // protocol.
      //
      AsciiTraceHelper asciiTraceHelper;

      std::string filename;
      if (explicitFilename)
        {
          filename = prefix;
        }
      else
        {
          filename = asciiTraceHelper.GetFilenameFromInterfacePair (prefix, ipv6, interface);
        }

      Ptr<OutputStreamWrapper> theStream = asciiTraceHelper.CreateFileStream (filename);

      //
      // However, we only hook the trace sources once to avoid multiple trace sink
      // calls per event (connect is independent of interface).
      //
      if (!AsciiHooked (ipv6))
        {
          //
          // The drop sink for the Ipv6L3Protocol uses a different signature than
          // the default sink, so we have to cook one up for ourselves.  We can get
          // to the Ptr<Ipv6L3Protocol> through our Ptr<Ipv6> since they must both 
          // be aggregated to the same node.
          //
          Ptr<Ipv6L3Protocol> ipv6L3Protocol = ipv6->GetObject<Ipv6L3Protocol> ();
          bool result = ipv6L3Protocol->TraceConnectWithoutContext ("Drop",
                                                                    MakeBoundCallback (&Ipv6L3ProtocolDropSinkWithoutContext, theStream));
          NS_ASSERT_MSG (result == true, "InternetStackHelper::EnableAsciiIpv6Internal():  "
                         "Unable to connect ipv6L3Protocol \"Drop\"");
          result = ipv6L3Protocol->TraceConnectWithoutContext ("Tx", 
                                                               MakeBoundCallback (&Ipv6L3ProtocolTxSinkWithoutContext, theStream));
          NS_ASSERT_MSG (result == true, "InternetStackHelper::EnableAsciiIpv6Internal():  "
                         "Unable to connect ipv6L3Protocol \"Tx\"");
          result = ipv6L3Protocol->TraceConnectWithoutContext ("Rx", 
                                                               MakeBoundCallback (&Ipv6L3ProtocolRxSinkWithoutContext, theStream));
          NS_ASSERT_MSG (result == true, "InternetStackHelper::EnableAsciiIpv6Internal():  "
                         "Unable to connect ipv6L3Protocol \"Rx\"");
        }

      g_interfaceStreamMapIpv6[std::make_pair (ipv6, interface)] = theStream;
      return;
    }

  //
  // If we are provided an OutputStreamWrapper, we are expected to use it, and
  // to provide a context.  We are free to come up with our own context if we
  // want, and use the AsciiTraceHelper Hook*WithContext functions, but for 
  // compatibility and simplicity, we just use Config::Connect and let it deal
  // with the context.
  //
  // We need to associate the Radeep/interface with a stream to express interest
  // in tracing events on that pair, however, we only hook the trace sources 
  // once to avoid multiple trace sink calls per event (connect is independent
  // of interface).
  //
  if (!AsciiHooked (ipv6))
    {
      Ptr<Node> node = ipv6->GetObject<Node> ();
      std::ostringstream oss;

      oss.str ("");
      oss << "/NodeList/" << node->GetId () << "/$ns3::Ipv6L3Protocol/Drop";
      Config::Connect (oss.str (), MakeBoundCallback (&Ipv6L3ProtocolDropSinkWithContext, stream));
      oss.str ("");
      oss << "/NodeList/" << node->GetId () << "/$ns3::Ipv6L3Protocol/Tx";
      Config::Connect (oss.str (), MakeBoundCallback (&Ipv6L3ProtocolTxSinkWithContext, stream));
      oss.str ("");
      oss << "/NodeList/" << node->GetId () << "/$ns3::Ipv6L3Protocol/Rx";
      Config::Connect (oss.str (), MakeBoundCallback (&Ipv6L3ProtocolRxSinkWithContext, stream));
    }

  g_interfaceStreamMapIpv6[std::make_pair (ipv6, interface)] = stream;
}

} // namespace ns3
