#include "ns3/assert.h" 
#include "ns3/node.h" 
#include "ns3/boolean.h"
#include "Radeep.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Radeep");

NS_OBJECT_ENSURE_REGISTERED (Radeep);

TypeId 
Radeep::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Radeep")
    .SetParent<Object> ()
    .SetGroupName ("Internet")
    .AddAttribute ("IpForward", "Globally enable or disable Radeep forwarding for all current and future Ipv4 devices.",
                   BooleanValue (true),
                   MakeBooleanAccessor (&Radeep::SetRadeepForward,
                                        &Radeep::GetRadeepForward),
                   MakeBooleanChecker ())
    .AddAttribute ("WeakEsModel", 
                   "RFC1122 term for whether host accepts datagram with a dest. address on another interface",
                   BooleanValue (true),
                   MakeBooleanAccessor (&Radeep::SetWeakEsModel,
                                        &Radeep::GetWeakEsModel),
                   MakeBooleanChecker ())
#if 0
    .AddAttribute ("MtuDiscover", "If enabled, every outgoing radeep packet will have the DF flag set.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&UdpSocket::SetMtuDiscover,
                                        &UdpSocket::GetMtuDiscover),
                   MakeBooleanChecker ())
#endif
  ;
  return tid;
}

Radeep::Radeep ()
{
  NS_LOG_FUNCTION (this);
}

Radeep::~Radeep ()
{
  NS_LOG_FUNCTION (this);
}

}
