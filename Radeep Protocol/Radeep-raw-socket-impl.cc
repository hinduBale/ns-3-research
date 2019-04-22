
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "Radeep-raw-socket-impl.h"
#include "radeep-l3-protocol.h"
#include "icmpv4.h"
#include "ns3/Radeep-packet-info-tag.h"
#include "ns3/inet-socket-address.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RadeepRawSocketImpl");

NS_OBJECT_ENSURE_REGISTERED (RadeepRawSocketImpl);

TypeId 
RadeepRawSocketImpl::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RadeepRawSocketImpl")
    .SetParent<Socket> ()
    .SetGroupName ("Internet")
    .AddAttribute ("Protocol", "Protocol number to match.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&RadeepRawSocketImpl::m_protocol),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("IcmpFilter", 
                   "Any icmp header whose type field matches a bit in this filter is dropped. Type must be less than 32.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&RadeepRawSocketImpl::m_icmpFilter),
                   MakeUintegerChecker<uint32_t> ())
    // 
    //  from raw (7), linux, returned length of Send/Recv should be
    // 
    //            | Radeep_HDRINC on  |      off    |
    //  ----------+---------------+-------------+-
    //  Send(Radeep)| hdr + payload | payload     |
    //  Recv(Radeep)| hdr + payload | hdr+payload |
    //  ----------+---------------+-------------+-
    .AddAttribute ("RadeepHeaderInclude", 
                   "Include Radeep Header information (a.k.a setsockopt (Radeep_HDRINCL)).",
                   BooleanValue (false),
                   MakeBooleanAccessor (&RadeepRawSocketImpl::m_Radeephdrincl),
                   MakeBooleanChecker ())
  ;
  return tid;
}

RadeepRawSocketImpl::RadeepRawSocketImpl ()
{
  NS_LOG_FUNCTION (this);
  m_err = Socket::ERROR_NOTERROR;
  m_node = 0;
  m_src = RadeepAddress::GetAny ();
  m_dst = RadeepAddress::GetAny ();
  m_protocol = 0;
  m_shutdownSend = false;
  m_shutdownRecv = false;
}

void 
RadeepRawSocketImpl::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  m_node = node;
}

void
RadeepRawSocketImpl::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_node = 0;
  Socket::DoDispose ();
}

enum Socket::SocketErrno 
RadeepRawSocketImpl::GetErrno (void) const
{
  NS_LOG_FUNCTION (this);
  return m_err;
}

enum Socket::SocketType
RadeepRawSocketImpl::GetSocketType (void) const
{
  NS_LOG_FUNCTION (this);
  return NS3_SOCK_RAW;
}

Ptr<Node> 
RadeepRawSocketImpl::GetNode (void) const
{
  NS_LOG_FUNCTION (this);
  return m_node;
}
int 
RadeepRawSocketImpl::Bind (const Address &address)
{
  NS_LOG_FUNCTION (this << address);
  if (!InetSocketAddress::IsMatchingType (address))
    {
      m_err = Socket::ERROR_INVAL;
      return -1;
    }
  InetSocketAddress ad = InetSocketAddress::ConvertFrom (address);
  m_src = ad.GetRadeep ();
  return 0;
}
int 
RadeepRawSocketImpl::Bind (void)
{
  NS_LOG_FUNCTION (this);
  m_src = RadeepAddress::GetAny ();
  return 0;
}
int 
RadeepRawSocketImpl::Bind6 (void)
{
  NS_LOG_FUNCTION (this);
  return (-1);
}
int 
RadeepRawSocketImpl::GetSockName (Address &address) const
{
  NS_LOG_FUNCTION (this << address);
  address = InetSocketAddress (m_src, 0);
  return 0;
}
int
RadeepRawSocketImpl::GetPeerName (Address &address) const
{
  NS_LOG_FUNCTION (this << address);

  if (m_dst == RadeepAddress::GetAny ())
    {
      m_err = ERROR_NOTCONN;
      return -1;
    }

  address = InetSocketAddress (m_dst, 0);

  return 0;
}
int 
RadeepRawSocketImpl::Close (void)
{
  NS_LOG_FUNCTION (this);
  Ptr<Radeep> radeep = m_node->GetObject<Radeep> ();
  if (radeep != 0)
    {
      radeep->DeleteRawSocket (this);
    }
  return 0;
}
int 
RadeepRawSocketImpl::ShutdownSend (void)
{
  NS_LOG_FUNCTION (this);
  m_shutdownSend = true;
  return 0;
}
int 
RadeepRawSocketImpl::ShutdownRecv (void)
{
  NS_LOG_FUNCTION (this);
  m_shutdownRecv = true;
  return 0;
}
int 
RadeepRawSocketImpl::Connect (const Address &address)
{
  NS_LOG_FUNCTION (this << address);
  if (!InetSocketAddress::IsMatchingType (address))
    {
      m_err = Socket::ERROR_INVAL;
      return -1;
    }
  InetSocketAddress ad = InetSocketAddress::ConvertFrom (address);
  m_dst = ad.GetRadeep ();
  SetRadeepTos (ad.GetTos ());

  return 0;
}
int 
RadeepRawSocketImpl::Listen (void)
{
  NS_LOG_FUNCTION (this);
  m_err = Socket::ERROR_OPNOTSUPP;
  return -1;
}
uint32_t 
RadeepRawSocketImpl::GetTxAvailable (void) const
{
  NS_LOG_FUNCTION (this);
  return 0xffffffff;
}
int 
RadeepRawSocketImpl::Send (Ptr<Packet> p, uint32_t flags)
{
  NS_LOG_FUNCTION (this << p << flags);
  InetSocketAddress to = InetSocketAddress (m_dst, m_protocol);
  to.SetTos (GetRadeepTos ());
  return SendTo (p, flags, to);
}
int 
RadeepRawSocketImpl::SendTo (Ptr<Packet> p, uint32_t flags, 
                           const Address &toAddress)
{
  NS_LOG_FUNCTION (this << p << flags << toAddress);
  if (!InetSocketAddress::IsMatchingType (toAddress))
    {
      m_err = Socket::ERROR_INVAL;
      return -1;
    }
  if (m_shutdownSend)
    {
      return 0;
    }

  InetSocketAddress ad = InetSocketAddress::ConvertFrom (toAddress);
  Ptr<Radeep> radeep = m_node->GetObject<Radeep> ();
  RadeepAddress dst = ad.GetRadeep ();
  RadeepAddress src = m_src;
  uint8_t tos = ad.GetTos ();

  uint8_t priority = GetPriority ();
  if (tos)
    {
      SocketRadeepTosTag RadeepTosTag;
      RadeepTosTag.SetTos (tos);
      // This packet may already have a SocketRadeepTosTag (see BUG 2440)
      p->ReplacePacketTag (RadeepTosTag);
      priority = RadeepTos2Priority (tos);
    }
  if (priority)
    {
      SocketPriorityTag priorityTag;
      priorityTag.SetPriority (priority);
      p->ReplacePacketTag (priorityTag);
    }

  if (IsManualRadeepTtl () && GetRadeepTtl () != 0 && !dst.IsMulticast () && !dst.IsBroadcast ())
    {
      SocketRadeepTtlTag tag;
      tag.SetTtl (GetRadeepTtl ());
      p->AddPacketTag (tag);
    }

  bool subnetDirectedBroadcast = false;
  if (m_boundnetdevice)
    {
      uint32_t iif = radeep->GetInterfaceForDevice (m_boundnetdevice);
      for (uint32_t j = 0; j < radeep->GetNAddresses (iif); j++)
        {
          RadeepInterfaceAddress ifAddr = radeep->GetAddress (iif, j);
          if (dst.IsSubnetDirectedBroadcast (ifAddr.GetMask ()))
            {
              subnetDirectedBroadcast = true;
            }
        }
    }

  if (dst.IsBroadcast () || subnetDirectedBroadcast)
    {
      Ptr <NetDevice> boundNetDevice = m_boundnetdevice;
      if (radeep->GetNInterfaces () == 1)
        {
          boundNetDevice = radeep->GetNetDevice (0);
        }
      if (boundNetDevice == 0)
        {
          NS_LOG_DEBUG ("dropped because no outgoing route.");
          return -1;
        }

      RadeepHeader header;
      uint32_t pktSize = p->GetSize ();
      if (!m_Radeephdrincl)
        {
          header.SetDestination (dst);
          header.SetProtocol (m_protocol);
          Ptr<RadeepRoute> route = Create <RadeepRoute> ();
          route->SetSource (src);
          route->SetDestination (dst);
          route->SetOutputDevice (boundNetDevice);
          radeep->Send (p, route->GetSource (), dst, m_protocol, route);
        }
      else
        {
          p->RemoveHeader (header);
          dst = header.GetDestination ();
          src = header.GetSource ();
          pktSize += header.GetSerializedSize ();
          Ptr<RadeepRoute> route = Create <RadeepRoute> ();
          route->SetSource (src);
          route->SetDestination (dst);
          route->SetOutputDevice (boundNetDevice);
          radeep->SendWithHeader (p, header, route);
        }
      NotifyDataSent (pktSize);
      NotifySend (GetTxAvailable ());
      return pktSize;
    }


  if (radeep->GetRoutingProtocol ())
    {
      RadeepHeader header;
      if (!m_Radeephdrincl)
        {
          header.SetDestination (dst);
          header.SetProtocol (m_protocol);
        }
      else
        {
          p->RemoveHeader (header);
          dst = header.GetDestination ();
          src = header.GetSource ();
        }
      SocketErrno errno_ = ERROR_NOTERROR; //do not use errno as it is the standard C last error number
      Ptr<RadeepRoute> route;
      Ptr<NetDevice> oif = m_boundnetdevice; //specify non-zero if bound to a source address
      if (!oif && src != RadeepAddress::GetAny ())
        {
          int32_t index = radeep->GetInterfaceForAddress (src);
          NS_ASSERT (index >= 0);
          oif = radeep->GetNetDevice (index);
          NS_LOG_LOGIC ("Set index " << oif << "from source " << src);
        }

      // TBD-- we could cache the route and just check its validity
      route = radeep->GetRoutingProtocol ()->RouteOutput (p, header, oif, errno_);
      if (route != 0)
        {
          NS_LOG_LOGIC ("Route exists");
          uint32_t pktSize = p->GetSize ();
          if (!m_Radeephdrincl)
            {
              radeep->Send (p, route->GetSource (), dst, m_protocol, route);
            }
          else
            {
              pktSize += header.GetSerializedSize ();
              radeep->SendWithHeader (p, header, route);
            }
          NotifyDataSent (pktSize);
          NotifySend (GetTxAvailable ());
          return pktSize;
        }
      else
        {
          NS_LOG_DEBUG ("dropped because no outgoing route.");
          return -1;
        }
    }
  return 0;
}
uint32_t 
RadeepRawSocketImpl::GetRxAvailable (void) const
{
  NS_LOG_FUNCTION (this);
  uint32_t rx = 0;
  for (std::list<Data>::const_iterator i = m_recv.begin (); i != m_recv.end (); ++i)
    {
      rx += (i->packet)->GetSize ();
    }
  return rx;
}
Ptr<Packet> 
RadeepRawSocketImpl::Recv (uint32_t maxSize, uint32_t flags)
{
  NS_LOG_FUNCTION (this << maxSize << flags);
  Address tmp;
  return RecvFrom (maxSize, flags, tmp);
}
Ptr<Packet> 
RadeepRawSocketImpl::RecvFrom (uint32_t maxSize, uint32_t flags,
                             Address &fromAddress)
{
  NS_LOG_FUNCTION (this << maxSize << flags << fromAddress);
  if (m_recv.empty ())
    {
      return 0;
    }
  struct Data data = m_recv.front ();
  m_recv.pop_front ();
  InetSocketAddress inet = InetSocketAddress (data.fromRadeep, data.fromProtocol);
  fromAddress = inet;
  if (data.packet->GetSize () > maxSize)
    {
      Ptr<Packet> first = data.packet->CreateFragment (0, maxSize);
      if (!(flags & MSG_PEEK))
        {
          data.packet->RemoveAtStart (maxSize);
        }
      m_recv.push_front (data);
      return first;
    }
  return data.packet;
}

void 
RadeepRawSocketImpl::SetProtocol (uint16_t protocol)
{
  NS_LOG_FUNCTION (this << protocol);
  m_protocol = protocol;
}

bool 
RadeepRawSocketImpl::ForwardUp (Ptr<const Packet> p, RadeepHeader RadeepHeader, Ptr<RadeepInterface> incomingInterface)
{
  NS_LOG_FUNCTION (this << *p << RadeepHeader << incomingInterface);
  if (m_shutdownRecv)
    {
      return false;
    }

  Ptr<NetDevice> boundNetDevice = Socket::GetBoundNetDevice();
  if (boundNetDevice)
    {
      if (boundNetDevice != incomingInterface->GetDevice())
        {
          return false;
        }
    }

  NS_LOG_LOGIC ("src = " << m_src << " dst = " << m_dst);
  if ((m_src == RadeepAddress::GetAny () || RadeepHeader.GetDestination () == m_src) &&
      (m_dst == RadeepAddress::GetAny () || RadeepHeader.GetSource () == m_dst) &&
      RadeepHeader.GetProtocol () == m_protocol)
    {
      Ptr<Packet> copy = p->Copy ();
      // Should check via getsockopt ()..
      if (IsRecvPktInfo ())
        {
          RadeepPacketInfoTag tag;
          copy->RemovePacketTag (tag);
          tag.SetRecvIf (incomingInterface->GetDevice ()->GetIfIndex ());
          copy->AddPacketTag (tag);
        }

      //Check only version 4 options
      if (IsRadeepRecvTos ())
        {
          SocketRadeepTosTag RadeepTosTag;
          RadeepTosTag.SetTos (RadeepHeader.GetTos ());
          copy->AddPacketTag (RadeepTosTag);
        }

      if (IsRadeepRecvTtl ())
        {
          SocketRadeepTtlTag RadeepTtlTag;
          RadeepTtlTag.SetTtl (RadeepHeader.GetTtl ());
          copy->AddPacketTag (RadeepTtlTag);
        }

     if (m_protocol == 1)
        {
          Icmpv4Header icmpHeader;
          copy->PeekHeader (icmpHeader);
          uint8_t type = icmpHeader.GetType ();
          if (type < 32 &&
              ((uint32_t(1) << type) & m_icmpFilter))
            {
              // filter out icmp packet.
              return false;
            }
        }
      copy->AddHeader (RadeepHeader);
      struct Data data;
      data.packet = copy;
      data.fromRadeep = RadeepHeader.GetSource ();
      data.fromProtocol = RadeepHeader.GetProtocol ();
      m_recv.push_back (data);
      NotifyDataRecv ();
      return true;
    }
  return false;
}

bool
RadeepRawSocketImpl::SetAllowBroadcast (bool allowBroadcast)
{
  NS_LOG_FUNCTION (this << allowBroadcast);
  if (!allowBroadcast)
    {
      return false;
    }
  return true;
}

bool
RadeepRawSocketImpl::GetAllowBroadcast () const
{
  NS_LOG_FUNCTION (this);
  return true;
}

} 
