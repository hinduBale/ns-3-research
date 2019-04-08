
#ifndef RADEEP_INTERFACE_H
#define RADEEP_INTERFACE_H

#include <list>
#include "ns3/ptr.h"
#include "ns3/object.h"

namespace ns3 {

class NetDevice;
class Packet;
class Node;
class ArpCache;
class RadeepInterfaceAddress;
class RadeepAddress;
class RadeepHeader;
class TrafficControlLayer;

/**
 * \ingroup Radeep
 *
 * \brief The Radeep representation of a network interface
 *
 * This class roughly corresponds to the struct in_device
 * of Linux; the main purpose is to provide address-family
 * specific information (addresses) about an interface.
 *
 * By default, Radeep interface are created in the "down" state
 * no Radeep addresses.  Before becoming usable, the user must
 * add an address of some type and invoke Setup on them.
 */
class RadeepInterface  : public Object
{
public:
  /**
   * \brief Get the type ID
   * \return type ID
   */
  static TypeId GetTypeId (void);

  RadeepInterface ();
  virtual ~RadeepInterface();

  /**
   * \brief Set node associated with interface.
   * \param node node
   */
  void SetNode (Ptr<Node> node); 
  /**
   * \brief Set the NetDevice.
   * \param device NetDevice
   */
  void SetDevice (Ptr<NetDevice> device);
  /**
   * \brief Set the TrafficControlLayer.
   * \param tc TrafficControlLayer object
   */
  void SetTrafficControl (Ptr<TrafficControlLayer> tc);
  /**
   * \brief Set ARP cache used by this interface
   * \param arpCache the ARP cache
   */
  void SetArpCache (Ptr<ArpCache> arpCache);

  /**
   * \returns the underlying NetDevice. This method cannot return zero.
   */
  Ptr<NetDevice> GetDevice (void) const;

  /**
   * \return ARP cache used by this interface
   */
  Ptr<ArpCache> GetArpCache () const;

  /**
   * \param metric configured routing metric (cost) of this interface
   *
   * Note:  This is synonymous to the Metric value that ifconfig prints
   * out.  It is used by ns-3 global routing, but other routing daemons
   * choose to ignore it. 
   */
  void SetMetric (uint16_t metric);

  /**
   * \returns configured routing metric (cost) of this interface
   *
   * Note:  This is synonymous to the Metric value that ifconfig prints
   * out.  It is used by ns-3 global routing, but other routing daemons 
   * may choose to ignore it. 
   */
  uint16_t GetMetric (void) const;

  /**
   * These are Radeep interface states and may be distinct from 
   * NetDevice states, such as found in real implementations
   * (where the device may be down but Radeep interface state is still up).
   */
  /**
   * \returns true if this interface is enabled, false otherwise.
   */
  bool IsUp (void) const;

  /**
   * \returns true if this interface is disabled, false otherwise.
   */
  bool IsDown (void) const;

  /**
   * Enable this interface
   */
  void SetUp (void);

  /**
   * Disable this interface
   */
  void SetDown (void);

  /**
   * \returns true if this interface is enabled for Radeep forwarding of input datagrams
   */
  bool IsForwarding (void) const;

  /**
   * \param val Whether to enable or disable Radeep forwarding for input datagrams
   */
  void SetForwarding (bool val);

  /**
   * \param p packet to send
   * \param hdr Radeep header
   * \param dest next hop address of packet.
   *
   * This method will eventually call the private
   * SendTo method which must be implemented by subclasses.
   */ 
  void Send (Ptr<Packet> p, const RadeepHeader & hdr, RadeepAddress dest);

  /**
   * \param address The RadeepInterfaceAddress to add to the interface
   * \returns true if succeeded
   */
  bool AddAddress (RadeepInterfaceAddress address);

  /**
   * \param index Index of RadeepInterfaceAddress to return
   * \returns The RadeepInterfaceAddress address whose index is i
   */
  RadeepInterfaceAddress GetAddress (uint32_t index) const;

  /**
   * \returns the number of RadeepInterfaceAddresss stored on this interface
   */
  uint32_t GetNAddresses (void) const;

  /**
   * \param index Index of RadeepInterfaceAddress to remove
   * \returns The RadeepInterfaceAddress address whose index is index 
   */
  RadeepInterfaceAddress RemoveAddress (uint32_t index);

  /**
   * \brief Remove the given Radeep address from the interface.
   * \param address The Radeep address to remove
   * \returns The removed Radeep interface address 
   * \returns The null interface address if the interface did not contain the 
   * address or if loopback address was passed as argument
   */
  RadeepInterfaceAddress RemoveAddress (RadeepAddress address);

protected:
  virtual void DoDispose (void);
private:
  /**
   * \brief Copy constructor
   * \param o object to copy
   *
   * Defined and unimplemented to avoid misuse
   */
  RadeepInterface (const RadeepInterface &o);

  /**
   * \brief Assignment operator
   * \param o object to copy
   * \returns the copied object
   *
   * Defined and unimplemented to avoid misuse
   */
  RadeepInterface &operator = (const RadeepInterface &o);

  /**
   * \brief Initialize interface.
   */
  void DoSetup (void);


  /**
   * \brief Container for the RadeepInterfaceAddresses.
   */
  typedef std::list<RadeepInterfaceAddress> RadeepInterfaceAddressList;

  /**
   * \brief Container Iterator for the RadeepInterfaceAddresses.
   */
  typedef std::list<RadeepInterfaceAddress>::const_iterator RadeepInterfaceAddressListCI;

  /**
   * \brief Const Container Iterator for the RadeepInterfaceAddresses.
   */
  typedef std::list<RadeepInterfaceAddress>::iterator RadeepInterfaceAddressListI;



  bool m_ifup; //!< The state of this interface
  bool m_forwarding;  //!< Forwarding state.
  uint16_t m_metric;  //!< Interface metric
  RadeepInterfaceAddressList m_ifaddrs; //!< Address list
  Ptr<Node> m_node; //!< The associated node
  Ptr<NetDevice> m_device; //!< The associated NetDevice
  Ptr<TrafficControlLayer> m_tc; //!< The associated TrafficControlLayer
  Ptr<ArpCache> m_cache; //!< ARP cache
};

} // namespace ns3

#endif