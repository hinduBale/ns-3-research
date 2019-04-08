#ifndef RADEEP_ROUTING_TABLE_ENTRY_H
#define RADEEP_ROUTING_TABLE_ENTRY_H

#include <list>
#include <vector>
#include <ostream>

#include "ns3/Radeep-address.h"

namespace ns3 {

/**
 * \ingroup RadeepRouting
 *
 * A record of an Radeep routing table entry for RadeepGlobalRouting and 
 * RadeepStaticRouting.  This is not a reference counted object.
 */
class RadeepRoutingTableEntry {
public:
  /**
   * \brief This constructor does nothing
   */
  RadeepRoutingTableEntry ();
  /**
   * \brief Copy Constructor
   * \param route The route to copy
   */
  RadeepRoutingTableEntry (RadeepRoutingTableEntry const &route);
  /**
   * \brief Copy Constructor
   * \param route The route to copy
   */
  RadeepRoutingTableEntry (RadeepRoutingTableEntry const *route);
  /**
   * \return True if this route is a host route (mask of all ones); false otherwise
   */
  bool IsHost (void) const;
  /**
   * \return True if this route is not a host route (mask is not all ones); false otherwise
   *
   * This method is implemented as !IsHost ().
   */
  bool IsNetwork (void) const;
  /**
   * \return True if this route is a default route; false otherwise
   */
  bool IsDefault (void) const;
  /**
   * \return True if this route is a gateway route; false otherwise
   */
  bool IsGateway (void) const;
  /**
   * \return address of the gateway stored in this entry
   */
  RadeepAddress GetGateway (void) const;
  /**
   * \return The Radeep address of the destination of this route
   */
  RadeepAddress GetDest (void) const;
  /**
   * \return The Radeep network number of the destination of this route
   */
  RadeepAddress GetDestNetwork (void) const;
  /**
   * \return The Radeep network mask of the destination of this route
   */
  RadeepMask GetDestNetworkMask (void) const;
  /**
   * \return The Radeep interface number used for sending outgoing packets
   */
  uint32_t GetInterface (void) const;
  /**
   * \return An RadeepRoutingTableEntry object corresponding to the input parameters.
   * \param dest RadeepAddress of the destination
   * \param nextHop RadeepAddress of the next hop
   * \param interface Outgoing interface 
   */
  static RadeepRoutingTableEntry CreateHostRouteTo (RadeepAddress dest, 
                                                  RadeepAddress nextHop,
                                                  uint32_t interface);
  /**
   * \return An RadeepRoutingTableEntry object corresponding to the input parameters.
   * \param dest RadeepAddress of the destination
   * \param interface Outgoing interface 
   */
  static RadeepRoutingTableEntry CreateHostRouteTo (RadeepAddress dest, 
                                                  uint32_t interface);
  /**
   * \return An RadeepRoutingTableEntry object corresponding to the input parameters.
   * \param network RadeepAddress of the destination network
   * \param networkMask RadeepMask of the destination network mask
   * \param nextHop RadeepAddress of the next hop
   * \param interface Outgoing interface 
   */
  static RadeepRoutingTableEntry CreateNetworkRouteTo (RadeepAddress network, 
                                                     RadeepMask networkMask,
                                                     RadeepAddress nextHop,
                                                     uint32_t interface);
  /**
   * \return An RadeepRoutingTableEntry object corresponding to the input parameters.
   * \param network RadeepAddress of the destination network
   * \param networkMask RadeepMask of the destination network mask
   * \param interface Outgoing interface 
   */
  static RadeepRoutingTableEntry CreateNetworkRouteTo (RadeepAddress network, 
                                                     RadeepMask networkMask,
                                                     uint32_t interface);
  /**
   * \return An RadeepRoutingTableEntry object corresponding to the input 
   * parameters.  This route is distinguished; it will match any 
   * destination for which a more specific route does not exist.
   * \param nextHop RadeepAddress of the next hop
   * \param interface Outgoing interface 
   */
  static RadeepRoutingTableEntry CreateDefaultRoute (RadeepAddress nextHop, 
                                                   uint32_t interface);

private:
  /**
   * \brief Constructor.
   * \param network network address
   * \param mask network mask
   * \param gateway the gateway
   * \param interface the interface index
   */
  RadeepRoutingTableEntry (RadeepAddress network,
                         RadeepMask mask,
                         RadeepAddress gateway,
                         uint32_t interface);
  /**
   * \brief Constructor.
   * \param dest destination address
   * \param mask network mask
   * \param interface the interface index
   */
  RadeepRoutingTableEntry (RadeepAddress dest,
                         RadeepMask mask,
                         uint32_t interface);
  /**
   * \brief Constructor.
   * \param dest destination address
   * \param gateway the gateway
   * \param interface the interface index
   */
  RadeepRoutingTableEntry (RadeepAddress dest,
                         RadeepAddress gateway,
                         uint32_t interface);
  /**
   * \brief Constructor.
   * \param dest destination address
   * \param interface the interface index
   */
  RadeepRoutingTableEntry (RadeepAddress dest,
                         uint32_t interface);

  RadeepAddress m_dest;         //!< destination address
  RadeepMask m_destNetworkMask; //!< destination network mask
  RadeepAddress m_gateway;      //!< gateway
  uint32_t m_interface;       //!< output interface
};

/**
 * \brief Stream insertion operator.
 *
 * \param os the reference to the output stream
 * \param route the Radeep routing table entry
 * \returns the reference to the output stream
 */
std::ostream& operator<< (std::ostream& os, RadeepRoutingTableEntry const& route);

/**
 * \brief Equality operator.
 *
 * \param a lhs
 * \param b rhs
 * \returns true if operands are equal, false otherwise
 */
bool operator== (const RadeepRoutingTableEntry a, const RadeepRoutingTableEntry b);

/**
 * \ingroup RadeepRouting
 *
 * \brief A record of an Radeep multicast route for RadeepGlobalRouting and RadeepStaticRouting
 */
class RadeepMulticastRoutingTableEntry {
public:
  /**
   * \brief This constructor does nothing
   */
  RadeepMulticastRoutingTableEntry ();

  /**
   * \brief Copy Constructor
   * \param route The route to copy
   */
  RadeepMulticastRoutingTableEntry (RadeepMulticastRoutingTableEntry const &route);
  /**
   * \brief Copy Constructor
   * \param route The route to copy
   */
  RadeepMulticastRoutingTableEntry (RadeepMulticastRoutingTableEntry const *route);
  /**
   * \return The Radeep address of the source of this route
   */
  RadeepAddress GetOrigin (void) const;
  /**
   * \return The Radeep address of the multicast group of this route
   */
  RadeepAddress GetGroup (void) const;
  /**
   * \return The Radeep address of the input interface of this route
   */
  uint32_t GetInputInterface (void) const;
  /**
   * \return The number of output interfaces of this route
   */
  uint32_t GetNOutputInterfaces (void) const;
  /**
   * \param n interface index
   * \return A specified output interface.
   */
  uint32_t GetOutputInterface (uint32_t n) const;
  /**
   * \return A vector of all of the output interfaces of this route.
   */
  std::vector<uint32_t> GetOutputInterfaces (void) const;
  /**
   * \return RadeepMulticastRoutingTableEntry corresponding to the input parameters.
   * \param origin Source address for the multicast route 
   * \param group Group destination address for the multicast route
   * \param inputInterface Input interface that multicast datagram must be received on
   * \param outputInterfaces vector of output interfaces to copy and forward the datagram to
   */
  static RadeepMulticastRoutingTableEntry CreateMulticastRoute (RadeepAddress origin, 
                                                              RadeepAddress group, uint32_t inputInterface,
                                                              std::vector<uint32_t> outputInterfaces);

private:
  /**
   * \brief Constructor.
   * \param origin source address
   * \param group destination address
   * \param inputInterface input interface
   * \param outputInterfaces output interfaces
   */
  RadeepMulticastRoutingTableEntry (RadeepAddress origin, RadeepAddress group, 
                                  uint32_t inputInterface, std::vector<uint32_t> outputInterfaces);

  RadeepAddress m_origin;   //!< source address
  RadeepAddress m_group;    //!< destination address
  uint32_t m_inputInterface;    //!< input interface
  std::vector<uint32_t> m_outputInterfaces;   //!< output interfaces
};

/**
 * \brief Stream insertion operator.
 *
 * \param os the reference to the output stream
 * \param route the Radeep multicast routing table entry
 * \returns the reference to the output stream
 */
std::ostream& operator<< (std::ostream& os, RadeepMulticastRoutingTableEntry const& route);

/**
 * \brief Equality operator.
 *
 * \param a lhs
 * \param b rhs
 * \returns true if operands are equal, false otherwise
 */
bool operator== (const RadeepMulticastRoutingTableEntry a, const RadeepMulticastRoutingTableEntry b);

} // namespace ns3

#endif /* RADEEP_ROUTING_TABLE_ENTRY_H */
