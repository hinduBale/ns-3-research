#ifndef RADEEP_GLOBAL_ROUTING_H
#define RADEEP_GLOBAL_ROUTING_H

#include <list>
#include <stdint.h>
#include "ns3/Radeep-address.h"
#include "ns3/Radeep-header.h"
#include "ns3/ptr.h"
#include "ns3/Radeep.h"
#include "ns3/Radeep-routing-protocol.h"
#include "ns3/random-variable-stream.h"

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
 * \ingroup Radeep
 *
 * \brief Global routing protocol for Radeep stacks.
 *
 * In ns-3 we have the concept of a pluggable routing protocol.  Routing
 * protocols are added to a list maintained by the RadeepL3Protocol.  Every 
 * stack gets one routing protocol for free -- the RadeepStaticRouting routing
 * protocol is added in the constructor of the RadeepL3Protocol (this is the 
 * piece of code that implements the functionality of the Radeep layer).
 *
 * As an option to running a dynamic routing protocol, a GlobalRouteManager
 * object has been created to allow users to build routes for all participating
 * nodes.  One can think of this object as a "routing oracle"; it has
 * an omniscient view of the topology, and can construct shortest path
 * routes between all pairs of nodes.  These routes must be stored 
 * somewhere in the node, so therefore this class RadeepGlobalRouting
 * is used as one of the pluggable routing protocols.  It is kept distinct
 * from RadeepStaticRouting because these routes may be dynamically cleared
 * and rebuilt in the middle of the simulation, while manually entered
 * routes into the RadeepStaticRouting may need to be kept distinct.
 *
 * This class deals with Radeep unicast routes only.
 *
 * \see RadeepRoutingProtocol
 * \see GlobalRouteManager
 */
class RadeepGlobalRouting : public RadeepRoutingProtocol
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  /**
   * \brief Construct an empty RadeepGlobalRouting routing protocol,
   *
   * The RadeepGlobalRouting class supports host and network unicast routes.
   * This method initializes the lists containing these routes to empty.
   *
   * \see RadeepGlobalRouting
   */
  RadeepGlobalRouting ();
  virtual ~RadeepGlobalRouting ();

  // These methods inherited from base class
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
   * \brief Add a host route to the global routing table.
   *
   * \param dest The Radeepv4Address destination for this route.
   * \param nextHop The RadeepAddress of the next hop in the route.
   * \param interface The network interface index used to send packets to the
   * destination.
   *
   * \see RadeepAddress
   */
  void AddHostRouteTo (RadeepAddress dest, 
                       RadeepAddress nextHop, 
                       uint32_t interface);
  /**
   * \brief Add a host route to the global routing table.
   *
   * \param dest The RadeepAddress destination for this route.
   * \param interface The network interface index used to send packets to the
   * destination.
   *
   * \see RadeepAddress
   */
  void AddHostRouteTo (RadeepAddress dest, 
                       uint32_t interface);

  /**
   * \brief Add a network route to the global routing table.
   *
   * \param network The RadeepAddress network for this route.
   * \param networkMask The RadeepMask to extract the network.
   * \param nextHop The next hop in the route to the destination network.
   * \param interface The network interface index used to send packets to the
   * destination.
   *
   * \see RadeepAddress
   */
  void AddNetworkRouteTo (RadeepAddress network, 
                          RadeepMask networkMask, 
                          RadeepAddress nextHop, 
                          uint32_t interface);

  /**
   * \brief Add a network route to the global routing table.
   *
   * \param network The RadeepAddress network for this route.
   * \param networkMask The RadeepMask to extract the network.
   * \param interface The network interface index used to send packets to the
   * destination.
   *
   * \see RadeepAddress
   */
  void AddNetworkRouteTo (RadeepAddress network, 
                          RadeepMask networkMask, 
                          uint32_t interface);

  /**
   * \brief Add an external route to the global routing table.
   *
   * \param network The RadeepAddress network for this route.
   * \param networkMask The RadeepMask to extract the network.
   * \param nextHop The next hop RadeepAddress
   * \param interface The network interface index used to send packets to the
   * destination.
   */
  void AddASExternalRouteTo (RadeepAddress network,
                             RadeepMask networkMask,
                             RadeepAddress nextHop,
                             uint32_t interface);

  /**
   * \brief Get the number of individual unicast routes that have been added
   * to the routing table.
   *
   * \warning The default route counts as one of the routes.
   * \returns the number of routes
   */
  uint32_t GetNRoutes (void) const;

  /**
   * \brief Get a route from the global unicast routing table.
   *
   * Externally, the unicast global routing table appears simply as a table with
   * n entries.  The one subtlety of note is that if a default route has been set
   * it will appear as the zeroth entry in the table.  This means that if you
   * add only a default route, the table will have one entry that can be accessed
   * either by explicitly calling GetDefaultRoute () or by calling GetRoute (0).
   *
   * Similarly, if the default route has been set, calling RemoveRoute (0) will
   * remove the default route.
   *
   * \param i The index (into the routing table) of the route to retrieve.  If
   * the default route has been set, it will occupy index zero.
   * \return If route is set, a pointer to that RadeepRoutingTableEntry is returned, otherwise
   * a zero pointer is returned.
   *
   * \see RadeepRoutingTableEntry
   * \see RadeepGlobalRouting::RemoveRoute
   */
  RadeepRoutingTableEntry *GetRoute (uint32_t i) const;

  /**
   * \brief Remove a route from the global unicast routing table.
   *
   * Externally, the unicast global routing table appears simply as a table with
   * n entries.  The one subtlety of note is that if a default route has been set
   * it will appear as the zeroth entry in the table.  This means that if the
   * default route has been set, calling RemoveRoute (0) will remove the
   * default route.
   *
   * \param i The index (into the routing table) of the route to remove.  If
   * the default route has been set, it will occupy index zero.
   *
   * \see RadeepRoutingTableEntry
   * \see RadeepGlobalRouting::GetRoute
   * \see RadeepGlobalRouting::AddRoute
   */
  void RemoveRoute (uint32_t i);

  /**
   * Assign a fixed random variable stream number to the random variables
   * used by this model.  Return the number of streams (possibly zero) that
   * have been assigned.
   *
   * \param stream first stream index to use
   * \return the number of stream indices assigned by this model
   */
  int64_t AssignStreams (int64_t stream);

protected:
  void DoDispose (void);

private:
  /// Set to true if packets are randomly routed among ECMP; set to false for using only one route consistently
  bool m_randomEcmpRouting;
  /// Set to true if this interface should respond to interface events by globallly recomputing routes 
  bool m_respondToInterfaceEvents;
  /// A uniform random number generator for randomly routing packets among ECMP 
  Ptr<UniformRandomVariable> m_rand;

  /// container of RadeepRoutingTableEntry (routes to hosts)
  typedef std::list<RadeepRoutingTableEntry *> HostRoutes;
  /// const iterator of container of RadeepRoutingTableEntry (routes to hosts)
  typedef std::list<RadeepRoutingTableEntry *>::const_iterator HostRoutesCI;
  /// iterator of container of RadeepRoutingTableEntry (routes to hosts)
  typedef std::list<RadeepRoutingTableEntry *>::iterator HostRoutesI;

  /// container of RadeepRoutingTableEntry (routes to networks)
  typedef std::list<RadeepRoutingTableEntry *> NetworkRoutes;
  /// const iterator of container of RadeepRoutingTableEntry (routes to networks)
  typedef std::list<RadeepRoutingTableEntry *>::const_iterator NetworkRoutesCI;
  /// iterator of container of RadeepRoutingTableEntry (routes to networks)
  typedef std::list<RadeepRoutingTableEntry *>::iterator NetworkRoutesI;

  /// container of RadeepRoutingTableEntry (routes to external AS)
  typedef std::list<RadeepRoutingTableEntry *> ASExternalRoutes;
  /// const iterator of container of RadeepRoutingTableEntry (routes to external AS)
  typedef std::list<RadeepRoutingTableEntry *>::const_iterator ASExternalRoutesCI;
  /// iterator of container of RadeepRoutingTableEntry (routes to external AS)
  typedef std::list<RadeepRoutingTableEntry *>::iterator ASExternalRoutesI;

  /**
   * \brief Lookup in the forwarding table for destination.
   * \param dest destination address
   * \param oif output interface if any (put 0 otherwise)
   * \return RadeepRoute to route the packet to reach dest address
   */
  Ptr<RadeepRoute> LookupGlobal (RadeepAddress dest, Ptr<NetDevice> oif = 0);

  HostRoutes m_hostRoutes;             //!< Routes to hosts
  NetworkRoutes m_networkRoutes;       //!< Routes to networks
  ASExternalRoutes m_ASexternalRoutes; //!< External routes imported

  Ptr<Radeep> m_radeep; //!< associated Radeep instance
};

} // Namespace ns3

#endif /* RADEEP_GLOBAL_ROUTING_H */
