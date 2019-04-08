#ifndef RADEEP_STATIC_ROUTING_H
#define RADEEP_STATIC_ROUTING_H

#include <list>
#include <utility>
#include <stdint.h>
#include "ns3/Radeep-address.h"
#include "ns3/Radeep-header.h"
#include "ns3/socket.h"
#include "ns3/ptr.h"
#include "ns3/Radeep.h"
#include "ns3/Radeep-routing-protocol.h"

namespace ns3 {

class Packet;
class NetDevice;
class RadeepInterface;
class RadeepAddress;
class RadeepHeader;
class RadeepRoutingTableEntry;
class RadeepMulticastRoutingTableEntry;
class Node;

/**
 * \ingroup RadeepRouting
 * 
 * \brief Static routing protocol for IP version 4 stacks.
 *
 * This class provides a basic set of methods for inserting static
 * unicast and multicast routes into the Radeep routing system.
 * This particular protocol is designed to be inserted into an 
 * RadeepListRouting protocol but can be used also as a standalone
 * protocol.
 * 
 * The RadeepStaticRouting class inherits from the abstract base class 
 * RadeepRoutingProtocol that defines the interface methods that a routing 
 * protocol must support.
 *
 * \see RadeepRoutingProtocol
 * \see RadeepListRouting
 * \see RadeepListRouting::AddRoutingProtocol
 */
class RadeepStaticRouting : public RadeepRoutingProtocol
{
public:
  /**
   * \brief The interface Id associated with this class.
   * \return type identifier
   */
  static TypeId GetTypeId (void);

  RadeepStaticRouting ();
  virtual ~RadeepStaticRouting ();

  virtual Ptr<RadeepRoute> RouteOutput (Ptr<Packet> p, const RadeepHeader &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr);

  virtual bool RouteInput  (Ptr<const Packet> p, const RadeepHeader &header, Ptr<const NetDevice> idev,
                            UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                            LocalDeliverCallback lcb, ErrorCallback ecb);

  virtual void NotifyInterfaceUp (uint32_t interface);
  virtual void NotifyInterfaceDown (uint32_t interface);
  virtual void NotifyAddAddress (uint32_t interface, RadeepInterfaceAddress address);
  virtual void NotifyRemoveAddress (uint32_t interface, RadeepInterfaceAddress address);
  virtual void SetRadeep (Ptr<Radeep> radeep);
  virtual void PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S) const;

/**
 * \brief Add a network route to the static routing table.
 *
 * \param network The RadeepAddress network for this route.
 * \param networkMask The RadeepMask to extract the network.
 * \param nextHop The next hop in the route to the destination network.
 * \param interface The network interface index used to send packets to the
 * destination.
 * \param metric Metric of route in case of multiple routes to same destination
 *
 * \see RadeepAddress
 */
  void AddNetworkRouteTo (RadeepAddress network, 
                          RadeepMask networkMask, 
                          RadeepAddress nextHop, 
                          uint32_t interface,
                          uint32_t metric = 0);

/**
 * \brief Add a network route to the static routing table.
 *
 * \param network The RadeepAddress network for this route.
 * \param networkMask The RadeepMask to extract the network.
 * \param interface The network interface index used to send packets to the
 * destination.
 * \param metric Metric of route in case of multiple routes to same destination
 *
 * \see RadeepAddress
 */
  void AddNetworkRouteTo (RadeepAddress network, 
                          RadeepMask networkMask, 
                          uint32_t interface,
                          uint32_t metric = 0);

/**
 * \brief Add a host route to the static routing table.
 *
 * \param dest The RadeepAddress destination for this route.
 * \param nextHop The RadeepAddress of the next hop in the route.
 * \param interface The network interface index used to send packets to the
 * destination.
 * \param metric Metric of route in case of multiple routes to same destination
 *
 * \see RadeepAddress
 */
  void AddHostRouteTo (RadeepAddress dest, 
                       RadeepAddress nextHop, 
                       uint32_t interface,
                       uint32_t metric = 0);
/**
 * \brief Add a host route to the static routing table.
 *
 * \param dest The RadeepAddress destination for this route.
 * \param interface The network interface index used to send packets to the
 * destination.
 * \param metric Metric of route in case of multiple routes to same destination
 *
 * \see RadeepAddress
 */
  void AddHostRouteTo (RadeepAddress dest, 
                       uint32_t interface,
                       uint32_t metric = 0);
/**
 * \brief Add a default route to the static routing table.
 *
 * This method tells the routing system what to do in the case where a specific
 * route to a destination is not found.  The system forwards packets to the
 * specified node in the hope that it knows better how to route the packet.
 * 
 * If the default route is set, it is returned as the selected route from 
 * LookupStatic irrespective of destination address if no specific route is
 * found.
 *
 * \param nextHop The RadeepAddress to send packets to in the hope that they
 * will be forwarded correctly.
 * \param interface The network interface index used to send packets.
 * \param metric Metric of route in case of multiple routes to same destination
 *
 * \see RadeepAddress
 * \see RadeepStaticRouting::Lookup
 */
  void SetDefaultRoute (RadeepAddress nextHop, 
                        uint32_t interface,
                        uint32_t metric = 0);

/**
 * \brief Get the number of individual unicast routes that have been added
 * to the routing table.
 *
 * \warning The default route counts as one of the routes.
 * \return number of entries
 */
  uint32_t GetNRoutes (void) const;

/**
 * \brief Get the default route with lowest metric from the static routing table.
 *
 * \return If the default route is set, a pointer to that RadeepRoutingTableEntry is
 * returned, otherwise an empty routing table entry is returned. 
*  If multiple default routes exist, the one with lowest metric is returned.
 *
 * \see RadeepRoutingTableEntry
 */
  RadeepRoutingTableEntry GetDefaultRoute (void);

/**
 * \brief Get a route from the static unicast routing table.
 *
 * Externally, the unicast static routing table appears simply as a table with
 * n entries.
 *
 * \param i The index (into the routing table) of the route to retrieve.
 * \return If route is set, a pointer to that RadeepRoutingTableEntry is returned, otherwise
 * a zero pointer is returned.
 *
 * \see RadeepRoutingTableEntry
 * \see RadeepStaticRouting::RemoveRoute
 */
  RadeepRoutingTableEntry GetRoute (uint32_t i) const;

/**
 * \brief Get a metric for route from the static unicast routing table.
 *
 * \param index The index (into the routing table) of the route to retrieve.
 * \return If route is set, the metric is returned. If not, an infinity metric (0xffffffff) is returned
 *
 */
  uint32_t GetMetric (uint32_t index) const;

/**
 * \brief Remove a route from the static unicast routing table.
 *
 * Externally, the unicast static routing table appears simply as a table with
 * n entries.
 *
 * \param i The index (into the routing table) of the route to remove.
 *
 * \see RadeepRoutingTableEntry
 * \see RadeepStaticRouting::GetRoute
 * \see RadeepStaticRouting::AddRoute
 */
  void RemoveRoute (uint32_t i);

/**
 * \brief Add a multicast route to the static routing table.
 *
 * A multicast route must specify an origin IP address, a multicast group and
 * an input network interface index as conditions and provide a vector of
 * output network interface indices over which packets matching the conditions
 * are sent.
 *
 * Typically there are two main types of multicast routes:  routes of the 
 * first kind are used during forwarding.  All of the conditions must be
 * explicitly provided.  The second kind of routes are used to get packets off
 * of a local node.  The difference is in the input interface.  Routes for
 * forwarding will always have an explicit input interface specified.  Routes
 * off of a node will always set the input interface to a wildcard specified
 * by the index RadeepRoutingProtocol::INTERFACE_ANY.
 *
 * For routes off of a local node wildcards may be used in the origin and
 * multicast group addresses.  The wildcard used for RadeepAdresses is that 
 * address returned by RadeepAddress::GetAny () -- typically "0.0.0.0".  Usage
 * of a wildcard allows one to specify default behavior to varying degrees.
 *
 * For example, making the origin address a wildcard, but leaving the 
 * multicast group specific allows one (in the case of a node with multiple
 * interfaces) to create different routes using different output interfaces
 * for each multicast group.
 *
 * If the origin and multicast addresses are made wildcards, you have created
 * essentially a default multicast address that can forward to multiple 
 * interfaces.  Compare this to the actual default multicast address that is
 * limited to specifying a single output interface for compatibility with
 * existing functionality in other systems.
 * 
 * \param origin The RadeepAddress of the origin of packets for this route.  May
 * be RadeepAddress:GetAny for open groups.
 * \param group The RadeepAddress of the multicast group or this route.
 * \param inputInterface The input network interface index over which to 
 * expect packets destined for this route.  May be
 * RadeepRoutingProtocol::INTERFACE_ANY for packets of local origin.
 * \param outputInterfaces A vector of network interface indices used to specify
 * how to send packets to the destination(s).
 *
 * \see RadeepAddress
 */
  void AddMulticastRoute (RadeepAddress origin,
                          RadeepAddress group,
                          uint32_t inputInterface,
                          std::vector<uint32_t> outputInterfaces);

/**
 * \brief Add a default multicast route to the static routing table.
 *
 * This is the multicast equivalent of the unicast version SetDefaultRoute.
 * We tell the routing system what to do in the case where a specific route
 * to a destination multicast group is not found.  The system forwards 
 * packets out the specified interface in the hope that "something out there"
 * knows better how to route the packet.  This method is only used in 
 * initially sending packets off of a host.  The default multicast route is
 * not consulted during forwarding -- exact routes must be specified using
 * AddMulticastRoute for that case.
 *
 * Since we're basically sending packets to some entity we think may know
 * better what to do, we don't pay attention to "subtleties" like origin
 * address, nor do we worry about forwarding out multiple  interfaces.  If the
 * default multicast route is set, it is returned as the selected route from 
 * LookupStatic irrespective of origin or multicast group if another specific
 * route is not found.
 *
 * \param outputInterface The network interface index used to specify where
 * to send packets in the case of unknown routes.
 *
 * \see RadeepAddress
 */
  void SetDefaultMulticastRoute (uint32_t outputInterface);

/**
 * \brief Get the number of individual multicast routes that have been added
 * to the routing table.
 *
 * \warning The default multicast route counts as one of the routes.
 * \return number of entries
 */
  uint32_t GetNMulticastRoutes (void) const;

/**
 * \brief Get a route from the static multicast routing table.
 *
 * Externally, the multicast static routing table appears simply as a table 
 * with n entries.
 * 
 * \param i The index (into the routing table) of the multicast route to
 * retrieve.
 * \return If route \e i is set, a pointer to that RadeepMulticastRoutingTableEntry is
 * returned, otherwise a zero pointer is returned.
 *
 * \see RadeepMulticastRoutingTableEntry
 * \see RadeepStaticRouting::RemoveRoute
 */
  RadeepMulticastRoutingTableEntry GetMulticastRoute (uint32_t i) const;

/**
 * \brief Remove a route from the static multicast routing table.
 *
 * Externally, the multicast static routing table appears simply as a table 
 * with n entries.
 * This method causes the multicast routing table to be searched for the first
 * route that matches the parameters and removes it.
 *
 * Wildcards may be provided to this function, but the wildcards are used to
 * exactly match wildcards in the routes (see AddMulticastRoute).  That is,
 * calling RemoveMulticastRoute with the origin set to "0.0.0.0" will not
 * remove routes with any address in the origin, but will only remove routes
 * with "0.0.0.0" set as the the origin.
 *
 * \param origin The IP address specified as the origin of packets for the
 * route.
 * \param group The IP address specified as the multicast group address of
 * the route.
 * \param inputInterface The network interface index specified as the expected
 * input interface for the route.
 * \returns true if a route was found and removed, false otherwise.
 *
 * \see RadeepMulticastRoutingTableEntry
 * \see RadeepStaticRouting::AddMulticastRoute
 */
  bool RemoveMulticastRoute (RadeepAddress origin,
                             RadeepAddress group,
                             uint32_t inputInterface);

/**
 * \brief Remove a route from the static multicast routing table.
 *
 * Externally, the multicast static routing table appears simply as a table 
 * with n entries.
 *
 * \param index The index (into the multicast routing table) of the route to
 * remove.
 *
 * \see RadeepRoutingTableEntry
 * \see RadeepStaticRouting::GetRoute
 * \see RadeepStaticRouting::AddRoute
 */
  void RemoveMulticastRoute (uint32_t index);

protected:
  virtual void DoDispose (void);

private:
  /// Container for the network routes
  typedef std::list<std::pair <RadeepRoutingTableEntry *, uint32_t> > NetworkRoutes;

  /// Const Iterator for container for the network routes
  typedef std::list<std::pair <RadeepRoutingTableEntry *, uint32_t> >::const_iterator NetworkRoutesCI;

  /// Iterator for container for the network routes
  typedef std::list<std::pair <RadeepRoutingTableEntry *, uint32_t> >::iterator NetworkRoutesI;

  /// Container for the multicast routes
  typedef std::list<RadeepMulticastRoutingTableEntry *> MulticastRoutes;

  /// Const Iterator for container for the multicast routes
  typedef std::list<RadeepMulticastRoutingTableEntry *>::const_iterator MulticastRoutesCI;

  /// Iterator for container for the multicast routes
  typedef std::list<RadeepMulticastRoutingTableEntry *>::iterator MulticastRoutesI;

  /**
   * \brief Lookup in the forwarding table for destination.
   * \param dest destination address
   * \param oif output interface if any (put 0 otherwise)
   * \return RadeepRoute to route the packet to reach dest address
   */
  Ptr<RadeepRoute> LookupStatic (RadeepAddress dest, Ptr<NetDevice> oif = 0);

  /**
   * \brief Lookup in the multicast forwarding table for destination.
   * \param origin source address
   * \param group group multicast address
   * \param interface interface index
   * \return RadeepMulticastRoute to route the packet to reach dest address
   */
  Ptr<RadeepMulticastRoute> LookupStatic (RadeepAddress origin, RadeepAddress group,
                                        uint32_t interface);

  /**
   * \brief the forwarding table for network.
   */
  NetworkRoutes m_networkRoutes;

  /**
   * \brief the forwarding table for multicast.
   */
  MulticastRoutes m_multicastRoutes;

  /**
   * \brief Radeep reference.
   */
  Ptr<Radeep> m_radeep;
};

} // Namespace ns3

#endif /* RADEEP_STATIC_ROUTING_H */
