#ifndef RADEEP_LIST_ROUTING_H
#define RADEEP_LIST_ROUTING_H

#include <list>
#include "ns3/Radeep-routing-protocol.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"

namespace ns3 {

/**
 * \ingroup RadeepRouting
 *
 * \brief Radeep list routing.
 *
 * This class is a specialization of RadeepRoutingProtocol that allows 
 * other instances of RadeepRoutingProtocol to be inserted in a 
 * prioritized list.  Routing protocols in the list are consulted one
 * by one, from highest to lowest priority, until a routing protocol
 * is found that will take the packet (this corresponds to a non-zero
 * return value to RouteOutput, or a return value of true to RouteInput).
 * The order by which routing protocols with the same priority value 
 * are consulted is undefined.
 * 
 */
class RadeepListRouting : public RadeepRoutingProtocol
{
public:
  /**
   * \brief Get the type ID of this class.
   * \return type ID
   */
  static TypeId GetTypeId (void);

  RadeepListRouting ();
  virtual ~RadeepListRouting ();

  /**
   * \brief Register a new routing protocol to be used in this Radeep stack
   *
   * \param routingProtocol new routing protocol implementation object
   * \param priority priority to give to this routing protocol.
   * Values may range between -32768 and +32767.
   */
  virtual void AddRoutingProtocol (Ptr<RadeepRoutingProtocol> routingProtocol, int16_t priority);
  /**
   * \return number of routing protocols in the list
   */
  virtual uint32_t GetNRoutingProtocols (void) const;
  /**
   * Return pointer to routing protocol stored at index, with the
   * first protocol (index 0) the highest priority, the next one (index 1)
   * the second highest priority, and so on.  The priority parameter is an
   * output parameter and it returns the integer priority of the protocol.
   * 
   * \return pointer to routing protocol indexed by 
   * \param index index of protocol to return
   * \param priority output parameter, set to the priority of the protocol
            being returned
   */
  virtual Ptr<RadeepRoutingProtocol> GetRoutingProtocol (uint32_t index, int16_t& priority) const;

  // Below are from RadeepRoutingProtocol
  virtual Ptr<RadeepRoute> RouteOutput (Ptr<Packet> p, const RadeepHeader &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr);

  virtual bool RouteInput (Ptr<const Packet> p, const RadeepHeader &header, Ptr<const NetDevice> idev,
                           UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                           LocalDeliverCallback lcb, ErrorCallback ecb);
  virtual void NotifyInterfaceUp (uint32_t interface);
  virtual void NotifyInterfaceDown (uint32_t interface);
  virtual void NotifyAddAddress (uint32_t interface, RadeepInterfaceAddress address);
  virtual void NotifyRemoveAddress (uint32_t interface, RadeepInterfaceAddress address);
  virtual void SetRadeep (Ptr<Radeep> radeep);
  virtual void PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S) const;

protected:
  virtual void DoDispose (void);
  virtual void DoInitialize (void);
private:
  /**
   * \brief Container identifying an Radeep Routing Protocol entry in the list.
   */
  typedef std::pair<int16_t, Ptr<RadeepRoutingProtocol> > RadeepRoutingProtocolEntry;
  /**
   * \brief Container of the Radeep Routing Protocols.
   */
  typedef std::list<RadeepRoutingProtocolEntry> RadeepRoutingProtocolList;
  RadeepRoutingProtocolList m_routingProtocols; //!<  List of routing protocols.

  /**
   * \brief Compare two routing protocols.
   * \param a first object to compare
   * \param b second object to compare
   * \return true if they are the same, false otherwise
   */
  static bool Compare (const RadeepRoutingProtocolEntry& a, const RadeepRoutingProtocolEntry& b);
  Ptr<Radeep> m_radeep; //!< Radeep this protocol is associated with.


};

} // namespace ns3

#endif /* RADEEP_LIST_ROUTING_H */
