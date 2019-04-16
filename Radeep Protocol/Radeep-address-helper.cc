/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 University of Washington
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
 */

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/ptr.h"
#include "ns3/node.h"
#include "ns3/net-device.h"
#include "ns3/loopback-net-device.h"
#include "ns3/Radeep.h"
#include "ns3/Radeep-address-generator.h"
#include "ns3/simulator.h"
#include "ns3/traffic-control-helper.h"
#include "ns3/traffic-control-layer.h"
#include "Radeep-address-helper.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RadeepAddressHelper");

RadeepAddressHelper::RadeepAddressHelper () 
{
  NS_LOG_FUNCTION_NOARGS ();

//
// Set the default values to an illegal state.  Do this so the client is 
// forced to think at least briefly about what addresses get used and what
// is going on here.
//
  m_network = 0xffffffff;
  m_mask = 0;
  m_address = 0xffffffff;
  m_base = 0xffffffff;
  m_shift = 0xffffffff;
  m_max = 0xffffffff;
}

RadeepAddressHelper::RadeepAddressHelper (
  const RadeepAddress network, 
  const RadeepMask    mask,
  const RadeepAddress address)
{
  NS_LOG_FUNCTION_NOARGS ();
  SetBase (network, mask, address);
}

void
RadeepAddressHelper::SetBase (
  const RadeepAddress network, 
  const RadeepMask mask,
  const RadeepAddress address)
{
  NS_LOG_FUNCTION_NOARGS ();

  m_network = network.Get ();
  m_mask = mask.Get ();
  m_base = m_address = address.Get ();

//
// Some quick reasonableness testing.
//
  NS_ASSERT_MSG ((m_network & ~m_mask) == 0,
                 "RadeepAddressHelper::SetBase(): Inconsistent network and mask");

//
// Figure out how much to shift network numbers to get them aligned, and what
// the maximum allowed address is with respect to the current mask.
//
  m_shift = NumAddressBits (m_mask);
  m_max = (1 << m_shift) - 2;

  NS_ASSERT_MSG (m_shift <= 32,
                 "RadeepAddressHelper::SetBase(): Unreasonable address length");

//
// Shift the network down into the normalized position.
//
  m_network >>= m_shift;

  NS_LOG_LOGIC ("m_network == " << m_network);
  NS_LOG_LOGIC ("m_mask == " << m_mask);
  NS_LOG_LOGIC ("m_address == " << m_address);
}

RadeepAddress
RadeepAddressHelper::NewAddress (void)
{
//
// The way this is expected to be used is that an address and network number
// are initialized, and then NewAddress() is called repeatedly to allocate and
// get new addresses on a given subnet.  The client will expect that the first
// address she gets back is the one she used to initialize the generator with.
// This implies that this operation is a post-increment.
//
  NS_ASSERT_MSG (m_address <= m_max,
                 "RadeepAddressHelper::NewAddress(): Address overflow");

  RadeepAddress addr ((m_network << m_shift) | m_address);
  ++m_address;
//
// The RadeepAddressGenerator allows us to keep track of the addresses we have
// allocated and will assert if we accidentally generate a duplicate.  This
// avoids some really hard to debug problems.
//
  RadeepAddressGenerator::AddAllocated (addr);
  return addr;
}

RadeepAddress
RadeepAddressHelper::NewNetwork (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  ++m_network;
  m_address = m_base;
  return RadeepAddress (m_network << m_shift);
}

RadeepInterfaceContainer
RadeepAddressHelper::Assign (const NetDeviceContainer &c)
{
  NS_LOG_FUNCTION_NOARGS ();
  RadeepInterfaceContainer retval;
  for (uint32_t i = 0; i < c.GetN (); ++i) {
      Ptr<NetDevice> device = c.Get (i);

      Ptr<Node> node = device->GetNode ();
      NS_ASSERT_MSG (node, "RadeepAddressHelper::Assign(): NetDevice is not not associated "
                     "with any node -> fail");

      Ptr<Radeep> radeep = node->GetObject<Radeep> ();
      NS_ASSERT_MSG (radeep, "RadeepAddressHelper::Assign(): NetDevice is associated"
                     " with a node without Radeep stack installed -> fail "
                     "(maybe need to use InternetStackHelper?)");

      int32_t interface = radeep->GetInterfaceForDevice (device);
      if (interface == -1)
        {
          interface = radeep->AddInterface (device);
        }
      NS_ASSERT_MSG (interface >= 0, "RadeepAddressHelper::Assign(): "
                     "Interface index not found");

      RadeepInterfaceAddress radeepAddr = RadeepInterfaceAddress (NewAddress (), m_mask);
      radeep->AddAddress (interface, radeepAddr);
      radeep->SetMetric (interface, 1);
      radeep->SetUp (interface);
      retval.Add (radeep, interface);

      // Install the default traffic control configuration if the traffic
      // control layer has been aggregated, if this is not 
      // a loopback interface, and there is no queue disc installed already
      Ptr<TrafficControlLayer> tc = node->GetObject<TrafficControlLayer> ();
      if (tc && DynamicCast<LoopbackNetDevice> (device) == 0 && tc->GetRootQueueDiscOnDevice (device) == 0)
        {
          NS_LOG_LOGIC ("Installing default traffic control configuration");
          TrafficControlHelper tcHelper = TrafficControlHelper::Default ();
          tcHelper.Install (device);
        }
    }
  return retval;
}

const uint32_t N_BITS = 32; //!< number of bits in a Radeep address

uint32_t
RadeepAddressHelper::NumAddressBits (uint32_t maskbits) const
{
  NS_LOG_FUNCTION_NOARGS ();
  for (uint32_t i = 0; i < N_BITS; ++i)
    {
      if (maskbits & 1)
        {
          NS_LOG_LOGIC ("NumAddressBits -> " << i);
          return i;
        }
      maskbits >>= 1;
    }

  NS_ASSERT_MSG (false, "RadeepAddressHelper::NumAddressBits(): Bad Mask");
  return 0;
}

} // namespace ns3

