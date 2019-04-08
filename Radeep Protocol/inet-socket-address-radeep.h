#ifndef INET_SOCKET_ADDRESS_H
#define INET_SOCKET_ADDRESS_H

#include "ns3/address.h"
#include "Radeep-address.h"
#include <stdint.h>

namespace ns3 {


/**
 * \ingroup address
 *
 * \brief an Inet address class
 *
 * This class is similar to inet_sockaddr in the BSD socket
 * API. i.e., this class holds an Ipv4Address and a port number
 * to form an ipv4 transport endpoint.
 */
class InetSocketAddress
{
public:
  /**
   * \param ipv4 the ipv4 address
   * \param port the port number
   */
  InetSocketAddress (RadeepAddress radeep, uint16_t port);
  /**
   * \param ipv4 the ipv4 address
   *
   * The port number is set to zero by default.
   */
  InetSocketAddress (RadeepAddress radeep);
  /**
   * \param port the port number
   *
   * The ipv4 address is set to the "Any" address by default.
   */
  InetSocketAddress (uint16_t port);
  /**
   * \param ipv4 string which represents an ipv4 address
   * \param port the port number
   */
  InetSocketAddress (const char *radeep, uint16_t port);
  /**
   * \param ipv4 string which represents an ipv4 address
   *
   * The port number is set to zero.
   */
  InetSocketAddress (const char *radeep);
  /**
   * \returns the port number
   */
  uint16_t GetPort (void) const;
  /**
   * \returns the ipv4 address
   */
  RadeepAddress GetRadeep (void) const;
  /**
   * \returns the ToS
   */
  uint8_t GetTos (void) const;

  /**
   * \param port the new port number.
   */
  void SetPort (uint16_t port);
  /**
   * \param address the new ipv4 address
   */
  void SetRadeep (RadeepAddress address);
  /**
   * \param tos the new ToS.
   */
  void SetTos (uint8_t tos);

  /**
   * \param address address to test
   * \returns true if the address matches, false otherwise.
   */
  static bool IsMatchingType (const Address &address);

  /**
   * \returns an Address instance which represents this
   * InetSocketAddress instance.
   */
  operator Address () const;

  /**
   * \brief Returns an InetSocketAddress which corresponds to the input
   * Address.
   *
   * \param address the Address instance to convert from.
   * \returns an InetSocketAddress
   */
  static InetSocketAddress ConvertFrom (const Address &address);
private:
  /**
   * \brief Convert to an Address type
   * \return the Address corresponding to this object.
   */
  Address ConvertTo (void) const;

  /**
   * \brief Get the underlying address type (automatically assigned).
   *
   * \returns the address type
   */
  static uint8_t GetType (void);
  RadeepAddress m_radeep; //!< the IPv4 address
  uint16_t m_port;    //!< the port
  uint8_t m_tos;      //!< the ToS
};

} // namespace ns3


#endif /* INET_SOCKET_ADDRESS_H */
