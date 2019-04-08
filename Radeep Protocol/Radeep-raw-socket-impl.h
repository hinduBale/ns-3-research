#ifndef RADEEP_RAW_SOCKET_IMPL_H
#define RADEEP_RAW_SOCKET_IMPL_H

#include "ns3/socket.h"
#include "ns3/Radeep-header.h"
#include "ns3/Radeep-route.h"
#include "ns3/Radeep-interface.h"
#include <list>

namespace ns3 {

class NetDevice;
class Node;
 
class RadeepRawSocketImpl : public Socket
{
public:
 
  static TypeId GetTypeId (void);

  RadeepRawSocketImpl ();

  /**
   * \brief Set the node associated with this socket.
   * \param node node to set
   */
  void SetNode (Ptr<Node> node);

  virtual enum Socket::SocketErrno GetErrno () const;

  /**
   * \brief Get socket type (NS3_SOCK_RAW)
   * \return socket type
   */
  virtual enum Socket::SocketType GetSocketType (void) const;

  virtual Ptr<Node> GetNode (void) const;
  virtual int Bind (const Address &address);
  virtual int Bind ();
  virtual int Bind6 ();
  virtual int GetSockName (Address &address) const; 
  virtual int GetPeerName (Address &address) const;
  virtual int Close (void);
  virtual int ShutdownSend (void);
  virtual int ShutdownRecv (void);
  virtual int Connect (const Address &address);
  virtual int Listen (void);
  virtual uint32_t GetTxAvailable (void) const;
  virtual int Send (Ptr<Packet> p, uint32_t flags);
  virtual int SendTo (Ptr<Packet> p, uint32_t flags, 
                      const Address &toAddress);
  virtual uint32_t GetRxAvailable (void) const;
  virtual Ptr<Packet> Recv (uint32_t maxSize, uint32_t flags);
  virtual Ptr<Packet> RecvFrom (uint32_t maxSize, uint32_t flags,
                                Address &fromAddress);


  /**
   * \brief Set protocol field.
   * \param protocol protocol to set
   */
  void SetProtocol (uint16_t protocol);

  /**
   * \brief Forward up to receive method.
   * \param p packet
   * \param radeepHeader Radeep header
   * \param incomingInterface incoming interface
   * \return true if forwarded, false otherwise
   */
  bool ForwardUp (Ptr<const Packet> p, RadeepHeader radeepHeader, Ptr<RadeepInterface> incomingInterface);
  virtual bool SetAllowBroadcast (bool allowBroadcast);
  virtual bool GetAllowBroadcast () const;

private:
  virtual void DoDispose (void);

  /**
   * \struct Data
   * \brief Radeep raw data and additional information.
   */
  struct Data {
    Ptr<Packet> packet;  /**< Packet data */
    RadeepAddress fromRadeep;  /**< Source address */
    uint16_t fromProtocol;   /**< Protocol used */
  };

  mutable enum Socket::SocketErrno m_err; //!< Last error number.
  Ptr<Node> m_node;                 //!< Node
  RadeepAddress m_src;                //!< Source address.
  RadeepAddress m_dst;                //!< Destination address.
  uint16_t m_protocol;              //!< Protocol.
  std::list<struct Data> m_recv;    //!< Packet waiting to be processed.
  bool m_shutdownSend;              //!< Flag to shutdown send capability.
  bool m_shutdownRecv;              //!< Flag to shutdown receive capability.
  uint32_t m_icmpFilter;            //!< ICMPv4 filter specification
  bool m_radeephdrincl;                 //!< Include Radeep Header information (a.k.a setsockopt (RADEEP_HDRINCL))
};

} // namespace ns3

#endif /* RADEEP_RAW_SOCKET_IMPL_H */
