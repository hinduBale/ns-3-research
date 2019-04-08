//Author: Rahul Saxena
//Derived heavily from ipv4-l3-protocol.h

#ifndef Radeep_L3_PROTOCOL_H
#define Radeep_L3_PROTOCOL_H

#include <list>
#include <map>
#include <vector>
#include <stdint.h>
#include "ns3/Radeep-address.h"
#include "ns3/ptr.h"
#include "ns3/net-device.h"
#include "ns3/Radeep.h"
#include "ns3/traced-callback.h"
#include "ns3/Radeep-header.h"
#include "ns3/Radeep-routing-protocol.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"

class RadeepL3ProtocolTestCase;

namespace ns3 {

class Packet;
class NetDevice;
class RadeepInterface;
class RadeepAddress;
class RadeepHeader;
class RadeepRoutingTableEntry;
class RadeepRoute;
class Node;
class Socket;
class RadeepRawSocketImpl;
class RadeepL4Protocol;
class Icmpv4L4Protocol;

/**
 * \ingroup Radeep
 *
 * \brief Implement the Radeep layer.
 * 
 * This is the actual implementation of Radeep.  It contains APIs to send and
 * receive packets at the Radeep layer, as well as APIs for Radeep routing.
 *
 * This class contains two distinct groups of trace sources.  The
 * trace sources 'Rx' and 'Tx' are called, respectively, immediately
 * after receiving from the NetDevice and immediately before sending
 * to a NetDevice for transmitting a packet.  These are low level
 * trace sources that include the RadeepHeader already serialized into
 * the packet.  In contrast, the Drop, SendOutgoing, UnicastForward,
 * and LocalDeliver trace sources are slightly higher-level and pass
 * around the RadeepHeader as an explicit parameter and not as part of
 * the packet.
 *
 * Radeep fragmentation and reassembly is handled at this level.
 * At the moment the fragmentation does not handle Radeep option headers,
 * and in particular the ones that shall not be fragmented.
 * Moreover, the actual implementation does not mimic exactly the Linux
 * kernel. Hence it is not possible, for instance, to test a fragmentation
 * attack.
 */
class RadeepL3Protocol : public Radeep
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  static const uint16_t PROT_NUMBER; //!< Protocol number (0x0800)

  RadeepL3Protocol();
  virtual ~RadeepL3Protocol ();

  /**
   * \enum DropReason
   * \brief Reason why a packet has been dropped.
   */
  enum DropReason 
  {
    DROP_TTL_EXPIRED = 1,   /**< Packet TTL has expired */
    DROP_NO_ROUTE,   /**< No route to host */
    DROP_BAD_CHECKSUM,   /**< Bad checksum */
    DROP_INTERFACE_DOWN,   /**< Interface is down so can not send packet */
    DROP_ROUTE_ERROR,   /**< Route error */
    DROP_FRAGMENT_TIMEOUT /**< Fragment timeout exceeded */
  };

  /**
   * \brief Set node associated with this stack.
   * \param node node to set
   */
  void SetNode (Ptr<Node> node);

  // functions defined in base class Radeep

  void SetRoutingProtocol (Ptr<RadeepRoutingProtocol> routingProtocol);
  Ptr<RadeepRoutingProtocol> GetRoutingProtocol (void) const;

  Ptr<Socket> CreateRawSocket (void);
  void DeleteRawSocket (Ptr<Socket> socket);

  virtual void Insert (Ptr<RadeepL4Protocol> protocol);
  virtual void Insert (Ptr<RadeepL4Protocol> protocol, uint32_t interfaceIndex);

  virtual void Remove (Ptr<RadeepL4Protocol> protocol);
  virtual void Remove (Ptr<RadeepL4Protocol> protocol, uint32_t interfaceIndex);

  virtual Ptr<RadeepL4Protocol> GetProtocol (int protocolNumber) const;
  virtual Ptr<RadeepL4Protocol> GetProtocol (int protocolNumber, int32_t interfaceIndex) const;

  virtual RadeepAddress SourceAddressSelection (uint32_t interface, RadeepAddress dest);

  /**
   * \param ttl default ttl to use
   *
   * When we need to send an Radeep packet, we use this default
   * ttl value.
   */
  void SetDefaultTtl (uint8_t ttl);

  /**
   * Lower layer calls this method after calling L3Demux::Lookup
   * The ARP subclass needs to know from which NetDevice this
   * packet is coming to:
   *    - implement a per-NetDevice ARP cache
   *    - send back arp replies on the right device
   * \param device network device
   * \param p the packet
   * \param protocol protocol value
   * \param from address of the correspondent
   * \param to address of the destination
   * \param packetType type of the packet
   */
  void Receive ( Ptr<NetDevice> device, Ptr<const Packet> p, uint16_t protocol, const Address &from,
                 const Address &to, NetDevice::PacketType packetType);

  /**
   * \param packet packet to send
   * \param source source address of packet
   * \param destination address of packet
   * \param protocol number of packet
   * \param route route entry
   *
   * Higher-level layers call this method to send a packet
   * down the stack to the MAC and PHY layers.
   */
  void Send (Ptr<Packet> packet, RadeepAddress source, 
             RadeepAddress destination, uint8_t protocol, Ptr<RadeepRoute> route);
  /**
   * \param packet packet to send
   * \param RadeepHeader Radeep Header
   * \param route route entry
   *
   * Higher-level layers call this method to send a packet with Radeep Header
   * (Intend to be used with RadeepHeaderInclude attribute.)
   */
  void SendWithHeader (Ptr<Packet> packet, RadeepHeader radeepHeader, Ptr<RadeepRoute> route);

  uint32_t AddInterface (Ptr<NetDevice> device);
  /**
   * \brief Get an interface.
   * \param i interface index
   * \return Radeep interface pointer
   */
  Ptr<RadeepInterface> GetInterface (uint32_t i) const;
  uint32_t GetNInterfaces (void) const;

  int32_t GetInterfaceForAddress (RadeepAddress addr) const;
  int32_t GetInterfaceForPrefix (RadeepAddress addr, RadeepMask mask) const;
  int32_t GetInterfaceForDevice (Ptr<const NetDevice> device) const;
  bool IsDestinationAddress (RadeepAddress address, uint32_t iif) const;

  bool AddAddress (uint32_t i, RadeepInterfaceAddress address);
  RadeepInterfaceAddress GetAddress (uint32_t interfaceIndex, uint32_t addressIndex) const;
  uint32_t GetNAddresses (uint32_t interface) const;
  bool RemoveAddress (uint32_t interfaceIndex, uint32_t addressIndex);
  bool RemoveAddress (uint32_t interface, RadeepAddress address);
  RadeepAddress SelectSourceAddress (Ptr<const NetDevice> device,
                                   RadeepAddress dst, RadeepInterfaceAddress::InterfaceAddressScope_e scope);


  void SetMetric (uint32_t i, uint16_t metric);
  uint16_t GetMetric (uint32_t i) const;
  uint16_t GetMtu (uint32_t i) const;
  bool IsUp (uint32_t i) const;
  void SetUp (uint32_t i);
  void SetDown (uint32_t i);
  bool IsForwarding (uint32_t i) const;
  void SetForwarding (uint32_t i, bool val);

  Ptr<NetDevice> GetNetDevice (uint32_t i);

  /**
   * \brief Check if an Radeep address is unicast according to the node.
   *
   * This function checks all the node's interfaces and the respective subnet masks.
   * An address is considered unicast if it's not broadcast, subnet-broadcast or multicast.
   *
   * \param ad address
   *
   * \return true if the address is unicast
   */
  bool IsUnicast (RadeepAddress ad) const;

  /**
   * TracedCallback signature for packet send, forward, or local deliver events.
   *
   * \param [in] header The Radeepv6Header.
   * \param [in] packet The packet.
   * \param [in] interface
   */
  typedef void (* SentTracedCallback)
    (const RadeepHeader & header, Ptr<const Packet> packet, uint32_t interface);
   
  /**
   * TracedCallback signature for packet transmission or reception events.
   *
   * \param [in] header The RadeepHeader.
   * \param [in] packet The packet.
   * \param [in] Radeep
   * \param [in] interface
   * \deprecated The non-const \c Ptr<Radeep> argument is deprecated
   * and will be changed to \c Ptr<const Radeep> in a future release.
   */
  typedef void (* TxRxTracedCallback)
    (Ptr<const Packet> packet, Ptr<Radeep> Radeep, uint32_t interface);

  /**
   * TracedCallback signature for packet drop events.
   *
   * \param [in] header The RadeepHeader.
   * \param [in] packet The packet.
   * \param [in] reason The reason the packet was dropped.
   * \param [in] Radeep
   * \param [in] interface
   * \deprecated The non-const \c Ptr<Radeep> argument is deprecated
   * and will be changed to \c Ptr<const Radeep> in a future release.
   */
  typedef void (* DropTracedCallback)
    (const RadeepHeader & header, Ptr<const Packet> packet,
     DropReason reason, Ptr<Radeep> Radeep,
     uint32_t interface);
   
protected:

  virtual void DoDispose (void);
  /**
   * This function will notify other components connected to the node that a new stack member is now connected
   * This will be used to notify Layer 3 protocol of layer 4 protocol stack to connect them together.
   */
  virtual void NotifyNewAggregate ();
private:
  /**
   * \brief RadeepL3ProtocolTestCase test case.
   * \relates RadeepL3ProtocolTestCase
   */
  friend class ::RadeepL3ProtocolTestCase;

  /**
   * \brief Copy constructor.
   *
   * Defined but not implemented to avoid misuse
   */
  RadeepL3Protocol(const RadeepL3Protocol &);

  /**
   * \brief Copy constructor.
   *
   * Defined but not implemented to avoid misuse
   * \returns the copied object
   */
  RadeepL3Protocol &operator = (const RadeepL3Protocol &);

  // class Radeep attributes
  virtual void SetRadeepForward (bool forward);
  virtual bool GetRadeepForward (void) const;
  virtual void SetWeakEsModel (bool model);
  virtual bool GetWeakEsModel (void) const;

  /**
   * \brief Construct an Radeep header.
   * \param source source Radeep address
   * \param destination destination Radeep address
   * \param protocol L4 protocol
   * \param payloadSize payload size
   * \param ttl Time to Live
   * \param tos Type of Service
   * \param mayFragment true if the packet can be fragmented
   * \return newly created Radeep header
   */
  RadeepHeader BuildHeader (
    RadeepAddress source,
    RadeepAddress destination,
    uint8_t protocol,
    uint16_t payloadSize,
    uint8_t ttl,
    uint8_t tos,
    bool mayFragment);

  /**
   * \brief Send packet with route.
   * \param route route
   * \param packet packet to send
   * \param RadeepHeader Radeep header to add to the packet
   */
  void
  SendRealOut (Ptr<RadeepRoute> route,
               Ptr<Packet> packet,
               RadeepHeader const &radeepHeader);

  /**
   * \brief Forward a packet.
   * \param rtentry route
   * \param p packet to forward
   * \param header Radeep header to add to the packet
   */
  void 
  RadeepForward (Ptr<RadeepRoute> rtentry, 
             Ptr<const Packet> p, 
             const RadeepHeader &header);

  /**
   * \brief Forward a multicast packet.
   * \param mrtentry route
   * \param p packet to forward
   * \param header Radeep header to add to the packet
   */
  void
  RadeepMulticastForward (Ptr<RadeepMulticastRoute> mrtentry, 
                      Ptr<const Packet> p, 
                      const RadeepHeader &header);

  /**
   * \brief Deliver a packet.
   * \param p packet delivered
   * \param Radeep Radeep header
   * \param iif input interface packet was received
   */
  void LocalDeliver (Ptr<const Packet> p, RadeepHeader const&radeep, uint32_t iif);

  /**
   * \brief Fallback when no route is found.
   * \param p packet
   * \param RadeepHeader Radeep header
   * \param sockErrno error number
   */
  void RouteInputError (Ptr<const Packet> p, const RadeepHeader & radeepHeader, Socket::SocketErrno sockErrno);

  /**
   * \brief Add an Radeep interface to the stack.
   * \param interface interface to add
   * \return index of newly added interface
   */
  uint32_t AddRadeepInterface (Ptr<RadeepInterface> interface);

  /**
   * \brief Setup loopback interface.
   */
  void SetupLoopback (void);

  /**
   * \brief Get ICMPv4 protocol.
   * \return Icmpv4L4Protocol pointer
   */
  Ptr<Icmpv4L4Protocol> GetIcmp (void) const;

  /**
   * \brief Check if an Radeep address is unicast.
   * \param ad address
   * \param interfaceMask the network mask
   * \return true if the address is unicast
   */
  bool IsUnicast (RadeepAddress ad, RadeepMask interfaceMask) const;

  /**
   * \brief Pair of a packet and an Radeep header.
   */
  typedef std::pair<Ptr<Packet>, RadeepHeader> RadeepPayloadHeaderPair;

  /**
   * \brief Fragment a packet
   * \param packet the packet
   * \param RadeepHeader the Radeep header
   * \param outIfaceMtu the MTU of the interface
   * \param listFragments the list of fragments
   */
  void DoFragmentation (Ptr<Packet> packet, const RadeepHeader& RadeepHeader, uint32_t outIfaceMtu, std::list<RadeepPayloadHeaderPair>& listFragments);

  /**
   * \brief Process a packet fragment
   * \param packet the packet
   * \param RadeepHeader the Radeep header
   * \param iif Input Interface
   * \return true is the fragment completed the packet
   */
  bool ProcessFragment (Ptr<Packet>& packet, RadeepHeader & radeepHeader, uint32_t iif);

  /**
   * \brief Process the timeout for packet fragments
   * \param key representing the packet fragments
   * \param RadeepHeader the Radeep header of the original packet
   * \param iif Input Interface
   */
  void HandleFragmentsTimeout ( std::pair<uint64_t, uint32_t> key, RadeepHeader & radeepHeader, uint32_t iif);

  /**
   * \brief Make a copy of the packet, add the header and invoke the TX trace callback
   * \param RadeepHeader the Radeep header that will be added to the packet
   * \param packet the packet
   * \param Radeep the Radeep protocol
   * \param interface the interface index
   *
   * Note: If the TracedCallback API ever is extended, we could consider
   * to check for connected functions before adding the header
   */
  void CallTxTrace (const RadeepHeader & radeepHeader, Ptr<Packet> packet, Ptr<Radeep> Radeep, uint32_t interface);

  /**
   * \brief Container of the Radeep Interfaces.
   */
  typedef std::vector<Ptr<RadeepInterface> > RadeepInterfaceList;
  /**
   * \brief Container of NetDevices registered to Radeep and their interface indexes.
   */
  typedef std::map<Ptr<const NetDevice>, uint32_t > RadeepInterfaceReverseContainer;
  /**
   * \brief Container of the Radeep Raw Sockets.
   */
  typedef std::list<Ptr<RadeepRawSocketImpl> > SocketList;

  /**
   * \brief Container of the Radeep L4 keys: protocol number, interface index
   */
  typedef std::pair<int, int32_t> L4ListKey_t;

  /**
   * \brief Container of the Radeep L4 instances.
   */
  typedef std::map<L4ListKey_t, Ptr<RadeepL4Protocol> > L4List_t;

  bool m_radeepForward;      //!< Forwarding packets (i.e. router mode) state.
  bool m_weakEsModel;    //!< Weak ES model state
  L4List_t m_protocols;  //!< List of transport protocol.
  RadeepInterfaceList m_interfaces; //!< List of Radeep interfaces.
  RadeepInterfaceReverseContainer m_reverseInterfacesContainer; //!< Container of NetDevice / Interface index associations.
  uint8_t m_defaultTtl;  //!< Default TTL
  std::map<std::pair<uint64_t, uint8_t>, uint16_t> m_identification; //!< Identification (for each {src, dst, proto} tuple)
  Ptr<Node> m_node; //!< Node attached to stack.

  /// Trace of sent packets
  TracedCallback<const RadeepHeader &, Ptr<const Packet>, uint32_t> m_sendOutgoingTrace;
  /// Trace of unicast forwarded packets
  TracedCallback<const RadeepHeader &, Ptr<const Packet>, uint32_t> m_unicastForwardTrace;
  /// Trace of locally delivered packets
  TracedCallback<const RadeepHeader &, Ptr<const Packet>, uint32_t> m_localDeliverTrace;

  // The following two traces pass a packet with an Radeep header
  /// Trace of transmitted packets
  /// \deprecated The non-const \c Ptr<Radeep> argument is deprecated
  /// and will be changed to \c Ptr<const Radeep> in a future release.
  TracedCallback<Ptr<const Packet>, Ptr<Radeep>,  uint32_t> m_txTrace;
  /// Trace of received packets
  /// \deprecated The non-const \c Ptr<Radeep> argument is deprecated
  /// and will be changed to \c Ptr<const Radeep> in a future release.
  TracedCallback<Ptr<const Packet>, Ptr<Radeep>, uint32_t> m_rxTrace;
  // <Radeep-header, payload, reason, ifindex> (ifindex not valid if reason is DROP_NO_ROUTE)
  /// Trace of dropped packets
  /// \deprecated The non-const \c Ptr<Radeep> argument is deprecated
  /// and will be changed to \c Ptr<const Radeep> in a future release.
  TracedCallback<const RadeepHeader &, Ptr<const Packet>, DropReason, Ptr<Radeep>, uint32_t> m_dropTrace;

  Ptr<RadeepRoutingProtocol> m_routingProtocol; //!< Routing protocol associated with the stack

  SocketList m_sockets; //!< List of Radeep raw sockets.

  /**
   * \brief A Set of Fragment belonging to the same packet (src, dst, identification and proto)
   */
  class Fragments : public SimpleRefCount<Fragments>
  {
public:
    /**
     * \brief Constructor.
     */
    Fragments ();

    /**
     * \brief Destructor.
     */
    ~Fragments ();

    /**
     * \brief Add a fragment.
     * \param fragment the fragment
     * \param fragmentOffset the offset of the fragment
     * \param moreFragment the bit "More Fragment"
     */
    void AddFragment (Ptr<Packet> fragment, uint16_t fragmentOffset, bool moreFragment);

    /**
     * \brief If all fragments have been added.
     * \returns true if the packet is entire
     */
    bool IsEntire () const;

    /**
     * \brief Get the entire packet.
     * \return the entire packet
     */
    Ptr<Packet> GetPacket () const;

    /**
     * \brief Get the complete part of the packet.
     * \return the part we have comeplete
     */
    Ptr<Packet> GetPartialPacket () const;

private:
    /**
     * \brief True if other fragments will be sent.
     */
    bool m_moreFragment;

    /**
     * \brief The current fragments.
     */
    std::list<std::pair<Ptr<Packet>, uint16_t> > m_fragments;

  };

  /// Container of fragments, stored as pairs(src+dst addr, src+dst port) / fragment
  typedef std::map< std::pair<uint64_t, uint32_t>, Ptr<Fragments> > MapFragments_t;
  /// Container of fragment timeout event, stored as pairs(src+dst addr, src+dst port) / EventId
  typedef std::map< std::pair<uint64_t, uint32_t>, EventId > MapFragmentsTimers_t;

  MapFragments_t       m_fragments; //!< Fragmented packets.
  Time                 m_fragmentExpirationTimeout; //!< Expiration timeout
  MapFragmentsTimers_t m_fragmentsTimers; //!< Expiration events.

};

} // Namespace ns3

#endif /* Radeep_L3_PROTOCOL_H */
