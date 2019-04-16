#include "ns3/assert.h"
#include "ns3/abort.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "Radeep-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RadeepHeader");

NS_OBJECT_ENSURE_REGISTERED (RadeepHeader);

RadeepHeader::RadeepHeader ()
  : m_calcChecksum (false),
    m_payloadSize (0),
    m_identification (0),
    m_tos (0),
    m_ttl (0),
    m_protocol (0),
    m_flags (0),
    m_fragmentOffset (0),
    m_checksum (0),
    m_goodChecksum (true),
    m_headerSize(5*4)
{
}

void
RadeepHeader::EnableChecksum (void)
{
  NS_LOG_FUNCTION (this);
  m_calcChecksum = true;
}

void
RadeepHeader::SetPayloadSize (uint16_t size)
{
  NS_LOG_FUNCTION (this << size);
  m_payloadSize = size;
}
uint16_t
RadeepHeader::GetPayloadSize (void) const
{
  NS_LOG_FUNCTION (this);
  return m_payloadSize;
}

uint16_t
RadeepHeader::GetIdentification (void) const
{
  NS_LOG_FUNCTION (this);
  return m_identification;
}
void
RadeepHeader::SetIdentification (uint16_t identification)
{
  NS_LOG_FUNCTION (this << identification);
  m_identification = identification;
}

void 
RadeepHeader::SetTos (uint8_t tos)
{
  NS_LOG_FUNCTION (this << static_cast<uint32_t> (tos));
  m_tos = tos;
}

void
RadeepHeader::SetDscp (DscpType dscp)
{
  NS_LOG_FUNCTION (this << dscp);
  m_tos &= 0x3; // Clear out the DSCP part, retain 2 bits of ECN
  m_tos |= (dscp << 2);
}

void
RadeepHeader::SetEcn (EcnType ecn)
{
  NS_LOG_FUNCTION (this << ecn);
  m_tos &= 0xFC; // Clear out the ECN part, retain 6 bits of DSCP
  m_tos |= ecn;
}

RadeepHeader::DscpType 
RadeepHeader::GetDscp (void) const
{
  NS_LOG_FUNCTION (this);
  // Extract only first 6 bits of TOS byte, i.e 0xFC
  return DscpType ((m_tos & 0xFC) >> 2);
}

std::string 
RadeepHeader::DscpTypeToString (DscpType dscp) const
{
  NS_LOG_FUNCTION (this << dscp);
  switch (dscp)
    {
      case DscpDefault:
        return "Default";
      case DSCP_CS1:
        return "CS1";
      case DSCP_AF11:
        return "AF11";
      case DSCP_AF12:
        return "AF12";
      case DSCP_AF13:
        return "AF13";
      case DSCP_CS2:
        return "CS2";
      case DSCP_AF21:
        return "AF21";
      case DSCP_AF22:
        return "AF22";
      case DSCP_AF23:
        return "AF23";
      case DSCP_CS3:
        return "CS3";
      case DSCP_AF31:
        return "AF31";
      case DSCP_AF32:
        return "AF32";
      case DSCP_AF33:
        return "AF33";
      case DSCP_CS4:
        return "CS4";
      case DSCP_AF41:
        return "AF41";
      case DSCP_AF42:
        return "AF42";
      case DSCP_AF43:
        return "AF43";
      case DSCP_CS5:
        return "CS5";
      case DSCP_EF:
        return "EF";
      case DSCP_CS6:
        return "CS6";
      case DSCP_CS7:
        return "CS7";
      default:
        return "Unrecognized DSCP";
    };
}


RadeepHeader::EcnType 
RadeepHeader::GetEcn (void) const
{
  NS_LOG_FUNCTION (this);
  // Extract only last 2 bits of TOS byte, i.e 0x3
  return EcnType (m_tos & 0x3);
}

std::string 
RadeepHeader::EcnTypeToString (EcnType ecn) const
{
  NS_LOG_FUNCTION (this << ecn);
  switch (ecn)
    {
      case ECN_NotECT:
        return "Not-ECT";
      case ECN_ECT1:
        return "ECT (1)";
      case ECN_ECT0:
        return "ECT (0)";
      case ECN_CE:
        return "CE";      
      default:
        return "Unknown ECN";
    };
}

uint8_t 
RadeepHeader::GetTos (void) const
{
  NS_LOG_FUNCTION (this);
  return m_tos;
}
void 
RadeepHeader::SetMoreFragments (void)
{
  NS_LOG_FUNCTION (this);
  m_flags |= MORE_FRAGMENTS;
}
void
RadeepHeader::SetLastFragment (void)
{
  NS_LOG_FUNCTION (this);
  m_flags &= ~MORE_FRAGMENTS;
}
bool 
RadeepHeader::IsLastFragment (void) const
{
  NS_LOG_FUNCTION (this);
  return !(m_flags & MORE_FRAGMENTS);
}

void 
RadeepHeader::SetDontFragment (void)
{
  NS_LOG_FUNCTION (this);
  m_flags |= DONT_FRAGMENT;
}
void 
RadeepHeader::SetMayFragment (void)
{
  NS_LOG_FUNCTION (this);
  m_flags &= ~DONT_FRAGMENT;
}
bool 
RadeepHeader::IsDontFragment (void) const
{
  NS_LOG_FUNCTION (this);
  return (m_flags & DONT_FRAGMENT);
}

void 
RadeepHeader::SetFragmentOffset (uint16_t offsetBytes)
{
  NS_LOG_FUNCTION (this << offsetBytes);
  // check if the user is trying to set an invalid offset
  NS_ABORT_MSG_IF ((offsetBytes & 0x7), "offsetBytes must be multiple of 8 bytes");
  m_fragmentOffset = offsetBytes;
}
uint16_t 
RadeepHeader::GetFragmentOffset (void) const
{
  NS_LOG_FUNCTION (this);
  // -fstrict-overflow sensitive, see bug 1868
  if ( m_fragmentOffset + m_payloadSize > 65535 - 5*4 )
    {
      NS_LOG_WARN("Fragment will exceed the maximum packet size once reassembled");
    }

  return m_fragmentOffset;
}

void 
RadeepHeader::SetTtl (uint8_t ttl)
{
  NS_LOG_FUNCTION (this << static_cast<uint32_t> (ttl));
  m_ttl = ttl;
}
uint8_t 
RadeepHeader::GetTtl (void) const
{
  NS_LOG_FUNCTION (this);
  return m_ttl;
}

uint8_t 
RadeepHeader::GetProtocol (void) const
{
  NS_LOG_FUNCTION (this);
  return m_protocol;
}
void 
RadeepHeader::SetProtocol (uint8_t protocol)
{
  NS_LOG_FUNCTION (this << static_cast<uint32_t> (protocol));
  m_protocol = protocol;
}

void 
RadeepHeader::SetSource (RadeepAddress source)
{
  NS_LOG_FUNCTION (this << source);
  m_source = source;
}
RadeepAddress
RadeepHeader::GetSource (void) const
{
  NS_LOG_FUNCTION (this);
  return m_source;
}

void 
RadeepHeader::SetDestination (RadeepAddress dst)
{
  NS_LOG_FUNCTION (this << dst);
  m_destination = dst;
}
RadeepAddress
RadeepHeader::GetDestination (void) const
{
  NS_LOG_FUNCTION (this);
  return m_destination;
}


bool
RadeepHeader::IsChecksumOk (void) const
{
  NS_LOG_FUNCTION (this);
  return m_goodChecksum;
}

TypeId 
RadeepHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RadeepHeader")
    .SetParent<Header> ()
    .SetGroupName ("Internet")
    .AddConstructor<RadeepHeader> ()
  ;
  return tid;
}
TypeId 
RadeepHeader::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);
  return GetTypeId ();
}
void 
RadeepHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);
  // Radeep, right ?
  std::string flags;
  if (m_flags == 0)
    {
      flags = "none";
    }
  else if ((m_flags & MORE_FRAGMENTS) &&
           (m_flags & DONT_FRAGMENT))
    {
      flags = "MF|DF";
    }
  else if (m_flags & DONT_FRAGMENT)
    {
      flags = "DF";
    }
  else if (m_flags & MORE_FRAGMENTS)
    {
      flags = "MF";
    }
  else
    {
      flags = "XX";
    }
  os << "tos 0x" << std::hex << m_tos << std::dec << " "
     << "DSCP " << DscpTypeToString (GetDscp ()) << " "
     << "ECN " << EcnTypeToString (GetEcn ()) << " "
     << "ttl " << m_ttl << " "
     << "id " << m_identification << " "
     << "protocol " << m_protocol << " "
     << "offset (bytes) " << m_fragmentOffset << " "
     << "flags [" << flags << "] "
     << "length: " << (m_payloadSize + 5 * 4)
     << " " 
     << m_source << " > " << m_destination
  ;
}
uint32_t 
RadeepHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  //return 5 * 4;
  return m_headerSize;
}

void
RadeepHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;

  uint8_t verIhl = (4 << 4) | (5);
  i.WriteU8 (verIhl);
  i.WriteU8 (m_tos);
  i.WriteHtonU16 (m_payloadSize + 5*4);
  i.WriteHtonU16 (m_identification);
  uint32_t fragmentOffset = m_fragmentOffset / 8;
  uint8_t flagsFrag = (fragmentOffset >> 8) & 0x1f;
  if (m_flags & DONT_FRAGMENT) 
    {
      flagsFrag |= (1<<6);
    }
  if (m_flags & MORE_FRAGMENTS) 
    {
      flagsFrag |= (1<<5);
    }
  i.WriteU8 (flagsFrag);
  uint8_t frag = fragmentOffset & 0xff;
  i.WriteU8 (frag);
  i.WriteU8 (m_ttl);
  i.WriteU8 (m_protocol);
  i.WriteHtonU16 (0);
  i.WriteHtonU32 (m_source.Get ());
  i.WriteHtonU32 (m_destination.Get ());

  if (m_calcChecksum) 
    {
      i = start;
      uint16_t checksum = i.CalculateRadeepChecksum (20);
      NS_LOG_LOGIC ("checksum=" <<checksum);
      i = start;
      i.Next (10);
      i.WriteU16 (checksum);
    }
}
uint32_t
RadeepHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;

  uint8_t verIhl = i.ReadU8 ();
  uint8_t ihl = verIhl & 0x0f; 
  uint16_t headerSize = ihl * 4;

  if ((verIhl >> 4) != 4)
    {
      NS_LOG_WARN ("Trying to decode a non-Radeep header, refusing to do it.");
      return 0;
    }

  m_tos = i.ReadU8 ();
  uint16_t size = i.ReadNtohU16 ();
  m_payloadSize = size - headerSize;
  m_identification = i.ReadNtohU16 ();
  uint8_t flags = i.ReadU8 ();
  m_flags = 0;
  if (flags & (1<<6)) 
    {
      m_flags |= DONT_FRAGMENT;
    }
  if (flags & (1<<5)) 
    {
      m_flags |= MORE_FRAGMENTS;
    }
  i.Prev ();
  m_fragmentOffset = i.ReadU8 () & 0x1f;
  m_fragmentOffset <<= 8;
  m_fragmentOffset |= i.ReadU8 ();
  m_fragmentOffset <<= 3;
  m_ttl = i.ReadU8 ();
  m_protocol = i.ReadU8 ();
  m_checksum = i.ReadU16 ();
  /* i.Next (2); // checksum */
  m_source.Set (i.ReadNtohU32 ());
  m_destination.Set (i.ReadNtohU32 ());
  m_headerSize = headerSize;

  if (m_calcChecksum) 
    {
      i = start;
      uint16_t checksum = i.CalculateRadeepChecksum (headerSize);
      NS_LOG_LOGIC ("checksum=" <<checksum);

      m_goodChecksum = (checksum == 0);
    }
  return GetSerializedSize ();
}

} // namespace ns3
