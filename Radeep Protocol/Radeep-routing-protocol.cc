#include "ns3/assert.h"
#include "Radeep-route.h"
#include "Radeep-routing-protocol.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RadeepRoutingProtocol");

NS_OBJECT_ENSURE_REGISTERED (RadeepRoutingProtocol);

TypeId RadeepRoutingProtocol::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RadeepRoutingProtocol")
    .SetParent<Object> ()
    .SetGroupName ("Internet")
  ;
  return tid;
}

} // namespace ns3
