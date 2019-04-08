#ifndef RADEEP_ROUTE_H
#define RADEEP_ROUTE_H

#include <list>
#include <map>
#include <ostream>

#include "ns3/simple-ref-count.h"
#include "ns3/Radeep-address.h"

namespace ns3 {

class NetDevice;

 
class RadeepRoute : public SimpleRefCount<RadeepRoute> 
{
public:
  RadeepRoute ();
 
  void SetDestination (RadeepAddress dest);
   
  RadeepAddress GetDestination (void) const;

 
  void SetSource (RadeepAddress src);
 
  RadeepAddress GetSource (void) const;

 
  void SetGateway (RadeepAddress gw);
 
  RadeepAddress GetGateway (void) const;

 
  void SetOutputDevice (Ptr<NetDevice> outputDevice);
 
  Ptr<NetDevice> GetOutputDevice (void) const;

#ifdef NOTYET
  // rtable.idev
  void SetInputIfIndex (uint32_t iif);
  uint32_t GetInputIfIndex (void) const;
#endif

private:
  RadeepAddress m_dest;             
  RadeepAddress m_source;           
  RadeepAddress m_gateway;          
  Ptr<NetDevice> m_outputDevice;  
#ifdef NOTYET
  uint32_t m_inputIfIndex;
#endif
};

 
std::ostream& operator<< (std::ostream& os, RadeepRoute const& route);

 
class RadeepMulticastRoute : public SimpleRefCount<RadeepMulticastRoute> 
{
public:
  RadeepMulticastRoute ();

    
  void SetGroup (const RadeepAddress group);
 
  RadeepAddress GetGroup (void) const; 

   
  void SetOrigin (const RadeepAddress origin);
 
  RadeepAddress GetOrigin (void) const; 

  
  void SetParent (uint32_t iif);
     uint32_t GetParent (void) const;

   
  void SetOutputTtl (uint32_t oif, uint32_t ttl);
 
  std::map<uint32_t, uint32_t> GetOutputTtlMap () const;

  static const uint32_t MAX_INTERFACES = 16;  //!< Maximum number of multicast interfaces on a router
  static const uint32_t MAX_TTL = 255;  //!< Maximum time-to-live (TTL)

private:
  RadeepAddress m_group;      //!< Group
  RadeepAddress m_origin;     //!< Source of packet
  uint32_t m_parent;        //!< Source interface
  std::map<uint32_t, uint32_t> m_ttls; //!< Time to Live container
};

}  

#endif /* RADEEP_ROUTE_H */