//Author: Rahul Saxena
//Derived heavily from Radeep-routing-protocol.h

#ifndef RADEEP_ROUTING_PROTOCOL_H
#define RADEEP_ROUTING_PROTOCOL_H

#include "ns3/packet.h"
#include "ns3/callback.h"
#include "ns3/object.h"
#include "ns3/socket.h"
#include "Radeep-header.h"
#include "ns3/Radeep-interface-address.h"
#include "Radeep.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/nstime.h"

namespace ns3 {

class RadeepMulticastRoute;
class RadeepRoute;
class NetDevice;

class RadeepRoutingProtocol : public Object
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /// Callback for unicast packets to be forwarded
  typedef Callback<void, Ptr<RadeepRoute>, Ptr<const Packet>, const RadeepHeader &> UnicastForwardCallback;

  /// Callback for multicast packets to be forwarded
  typedef Callback<void, Ptr<RadeepMulticastRoute>, Ptr<const Packet>, const RadeepHeader &> MulticastForwardCallback;

 
  typedef Callback<void, Ptr<const Packet>, const RadeepHeader &, uint32_t > LocalDeliverCallback;

  typedef Callback<void, Ptr<const Packet>, const RadeepHeader &, Socket::SocketErrno > ErrorCallback;

  virtual Ptr<RadeepRoute> RouteOutput (Ptr<Packet> p, const RadeepHeader &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr) = 0;

  
  virtual bool RouteInput  (Ptr<const Packet> p, const RadeepHeader &header, Ptr<const NetDevice> idev, 
                            UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                            LocalDeliverCallback lcb, ErrorCallback ecb) = 0;


  virtual void NotifyInterfaceUp (uint32_t interface) = 0;

  virtual void NotifyInterfaceDown (uint32_t interface) = 0;


   
  virtual void NotifyAddAddress (uint32_t interface, RadeepInterfaceAddress address) = 0;


 
  virtual void NotifyRemoveAddress (uint32_t interface, RadeepInterfaceAddress address) = 0;

  virtual void SetRadeep (Ptr<Radeep> radeep) = 0;

  virtual void PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S) const = 0;

};

} // namespace ns3

#endif /* Radeep_ROUTING_PROTOCOL_H */
