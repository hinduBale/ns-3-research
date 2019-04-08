
#ifndef RADEEP_STATIC_ROUTING_HELPER_H
#define RADEEP_STATIC_ROUTING_HELPER_H

#include "ns3/Radeep.h"
#include "ns3/Radeep-static-routing.h"
#include "ns3/ptr.h"
#include "ns3/Radeep-address.h"
#include "ns3/node.h"
#include "ns3/net-device.h"
#include "ns3/Radeep-routing-helper.h"
#include "ns3/node-container.h"
#include "ns3/net-device-container.h"

namespace ns3 {

/**
 * \ingroup RadeepHelpers
 *
 * \brief Helper class that adds ns3::RadeepStaticRouting objects
 *
 * This class is expected to be used in conjunction with 
 * ns3::InternetStackHelper::SetRoutingHelper
 */
class RadeepStaticRoutingHelper : public RadeepRoutingHelper
{
public:
  /*
   * Construct an RadeepStaticRoutingHelper object, used to make configuration
   * of static routing easier.
   */
  RadeepStaticRoutingHelper ();

  /**
   * \brief Construct an RadeepStaticRoutingHelper from another previously 
   * initialized instance (Copy Constructor).
   */
  RadeepStaticRoutingHelper (const RadeepStaticRoutingHelper &);

  /**
   * \returns pointer to clone of this RadeepStaticRoutingHelper
   *
   * This method is mainly for internal use by the other helpers;
   * clients are expected to free the dynamic memory allocated by this method
   */
  RadeepStaticRoutingHelper* Copy (void) const;

  /**
   * \param node the node on which the routing protocol will run
   * \returns a newly-created routing protocol
   *
   * This method will be called by ns3::InternetStackHelper::Install
   */
  virtual Ptr<RadeepRoutingProtocol> Create (Ptr<Node> node) const;

  /**
   * Try and find the static routing protocol as either the main routing
   * protocol or in the list of routing protocols associated with the 
   * Radeep provided.
   *
   * \param Radeep the Ptr<Radeep> to search for the static routing protocol
   * \returns RadeepStaticRouting pointer or 0 if not found
   */
  Ptr<RadeepStaticRouting> GetStaticRouting (Ptr<Radeep> radeep) const;

  /**
   * \brief Add a multicast route to a node and net device using explicit 
   * Ptr<Node> and Ptr<NetDevice>
   *
   * \param n The node.
   * \param source Source address.
   * \param group Multicast group.
   * \param input Input NetDevice.
   * \param output Output NetDevices.
   */
  void AddMulticastRoute (Ptr<Node> n, RadeepAddress source, RadeepAddress group,
                          Ptr<NetDevice> input, NetDeviceContainer output);

  /**
   * \brief Add a multicast route to a node and device using a name string 
   * previously associated to the node using the Object Name Service and a
   * Ptr<NetDevice>
   *
   * \param n The node.
   * \param source Source address.
   * \param group Multicast group.
   * \param input Input NetDevice.
   * \param output Output NetDevices.
   */
  void AddMulticastRoute (std::string n, RadeepAddress source, RadeepAddress group,
                          Ptr<NetDevice> input, NetDeviceContainer output);

  /**
   * \brief Add a multicast route to a node and device using a Ptr<Node> and a 
   * name string previously associated to the device using the Object Name Service.
   *
   * \param n The node.
   * \param source Source address.
   * \param group Multicast group.
   * \param inputName Input NetDevice.
   * \param output Output NetDevices.
   */
  void AddMulticastRoute (Ptr<Node> n, RadeepAddress source, RadeepAddress group,
                          std::string inputName, NetDeviceContainer output);

  /**
   * \brief Add a multicast route to a node and device using name strings
   * previously associated to both the node and device using the Object Name 
   * Service.
   *
   * \param nName The node.
   * \param source Source address.
   * \param group Multicast group.
   * \param inputName Input NetDevice.
   * \param output Output NetDevices.
   */
  void AddMulticastRoute (std::string nName, RadeepAddress source, RadeepAddress group,
                          std::string inputName, NetDeviceContainer output);

  /**
   * \brief Add a default route to the static routing protocol to forward
   *        packets out a particular interface
   *
   * Functionally equivalent to:
   * route add 224.0.0.0 netmask 240.0.0.0 dev nd
   * \param n node
   * \param nd device of the node to add default route
   */
  void SetDefaultMulticastRoute (Ptr<Node> n, Ptr<NetDevice> nd);

  /**
   * \brief Add a default route to the static routing protocol to forward
   *        packets out a particular interface
   *
   * Functionally equivalent to:
   * route add 224.0.0.0 netmask 240.0.0.0 dev nd
   * \param n node
   * \param ndName string with name previously associated to device using the 
   *        Object Name Service
   */
  void SetDefaultMulticastRoute (Ptr<Node> n, std::string ndName);

  /**
   * \brief Add a default route to the static routing protocol to forward
   *        packets out a particular interface
   *
   * Functionally equivalent to:
   * route add 224.0.0.0 netmask 240.0.0.0 dev nd
   * \param nName string with name previously associated to node using the 
   *        Object Name Service
   * \param nd device of the node to add default route
   */
  void SetDefaultMulticastRoute (std::string nName, Ptr<NetDevice> nd);

  /**
   * \brief Add a default route to the static routing protocol to forward
   *        packets out a particular interface
   *
   * Functionally equivalent to:
   * route add 224.0.0.0 netmask 240.0.0.0 dev nd
   * \param nName string with name previously associated to node using the 
   *        Object Name Service
   * \param ndName string with name previously associated to device using the 
   *        Object Name Service
   */
  void SetDefaultMulticastRoute (std::string nName, std::string ndName);
private:
  /**
   * \brief Assignment operator declared private and not implemented to disallow
   * assignment and prevent the compiler from happily inserting its own.
   * \returns
   */
  RadeepStaticRoutingHelper &operator = (const RadeepStaticRoutingHelper &);
};

} // namespace ns3

#endif /* RADEEP_STATIC_ROUTING_HELPER_H */
