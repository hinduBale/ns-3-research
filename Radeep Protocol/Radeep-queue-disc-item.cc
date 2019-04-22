#include "ns3/log.h"
#include "Radeep-queue-disc-item.h"
#include "ns3/tcp-header.h"
#include "ns3/udp-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RadeepQueueDiscItem");

RadeepQueueDiscItem::RadeepQueueDiscItem (Ptr<Packet> p, const Address& addr,
                                      uint16_t protocol, const RadeepHeader & header)
  : QueueDiscItem (p, addr, protocol),
    m_header (header),
    m_headerAdded (false)
{
}

RadeepQueueDiscItem::~RadeepQueueDiscItem ()
{
  NS_LOG_FUNCTION (this);
}

uint32_t RadeepQueueDiscItem::GetSize (void) const
{
  NS_LOG_FUNCTION (this);
  Ptr<Packet> p = GetPacket ();
  NS_ASSERT (p != 0);
  uint32_t ret = p->GetSize ();
  if (!m_headerAdded)
    {
      ret += m_header.GetSerializedSize ();
    }
  return ret;
}

const RadeepHeader&
RadeepQueueDiscItem::GetHeader (void) const
{
  return m_header;
}

void RadeepQueueDiscItem::AddHeader (void)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT_MSG (!m_headerAdded, "The header has been already added to the packet");
  Ptr<Packet> p = GetPacket ();
  NS_ASSERT (p != 0);
  p->AddHeader (m_header);
  m_headerAdded = true;
}

void
RadeepQueueDiscItem::Print (std::ostream& os) const
{
  if (!m_headerAdded)
    {
      os << m_header << " ";
    }
  os << GetPacket () << " "
     << "Dst addr " << GetAddress () << " "
     << "proto " << (uint16_t) GetProtocol () << " "
     << "txq " << (uint8_t) GetTxQueueIndex ()
  ;
}

bool
RadeepQueueDiscItem::Mark (void)
{
  NS_LOG_FUNCTION (this);
  if (!m_headerAdded && m_header.GetEcn () != RadeepHeader::ECN_NotECT)
    {
      m_header.SetEcn (RadeepHeader::ECN_CE);
      return true;
    }
  return false;
}


bool
RadeepQueueDiscItem::GetUint8Value (QueueItem::Uint8Values field, uint8_t& value) const
{
  bool ret = false;

  switch (field)
    {
    case Radeep_DSFIELD:
      value = m_header.GetTos ();
      ret = true;
      break;
    }

  return ret;
}

uint32_t
RadeepQueueDiscItem::Hash (uint32_t perturbation) const
{
  NS_LOG_FUNCTION (this << perturbation);

  RadeepAddress src = m_header.GetSource ();
  RadeepAddress dest = m_header.GetDestination ();
  uint8_t prot = m_header.GetProtocol ();
  uint16_t fragOffset = m_header.GetFragmentOffset ();

  TcpHeader tcpHdr;
  UdpHeader udpHdr;
  uint16_t srcPort = 0;
  uint16_t destPort = 0;

  if (prot == 6 && fragOffset == 0) // TCP
    {
      GetPacket ()->PeekHeader (tcpHdr);
      srcPort = tcpHdr.GetSourcePort ();
      destPort = tcpHdr.GetDestinationPort ();
    }
  else if (prot == 17 && fragOffset == 0) // UDP
    {
      GetPacket ()->PeekHeader (udpHdr);
      srcPort = udpHdr.GetSourcePort ();
      destPort = udpHdr.GetDestinationPort ();
    }
  if (prot != 6 && prot != 17)
    {
      NS_LOG_WARN ("Unknown transport protocol, no port number included in hash computation");
    }

  /* serialize the 5-tuple and the perturbation in buf */
  uint8_t buf[17];
  src.Serialize (buf);
  dest.Serialize (buf + 4);
  buf[8] = prot;
  buf[9] = (srcPort >> 8) & 0xff;
  buf[10] = srcPort & 0xff;
  buf[11] = (destPort >> 8) & 0xff;
  buf[12] = destPort & 0xff;
  buf[13] = (perturbation >> 24) & 0xff;
  buf[14] = (perturbation >> 16) & 0xff;
  buf[15] = (perturbation >> 8) & 0xff;
  buf[16] = perturbation & 0xff;

  // Linux calculates jhash2 (jenkins hash), we calculate murmur3 because it is
  // already available in ns-3
  uint32_t hash = Hash32 ((char*) buf, 17);

  NS_LOG_DEBUG ("Hash value " << hash);

  return hash;
}

} // namespace ns3
