#ifndef RADEEP_ROUTING_HELPER_H
#define RADEEP_ROUTING_HELPER_H

#include "ns3/ptr.h"
#include "ns3/nstime.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/Radeep-list-routing.h"

namespace ns3 {

class RadeepRoutingProtocol;
class Node;

/**
 * \ingroup RadeepHelpers
 *
 * \brief a factory to create ns3::RadeepRoutingProtocol objects
 *
 * For each new routing protocol created as a subclass of 
 * ns3::RadeepRoutingProtocol, you need to create a subclass of 
 * ns3::RadeepRoutingHelper which can be used by 
 * ns3::InternetStackHelper::SetRoutingHelper and 
 * ns3::InternetStackHelper::Install.
 */
class RadeepRoutingHelper
{
public:
  /*
   * Destroy an instance of an RadeepRoutingHelper
   */
  virtual ~RadeepRoutingHelper ();

  /**
   * \brief virtual constructor
   * \returns pointer to clone of this RadeepRoutingHelper 
   * 
   * This method is mainly for internal use by the other helpers;
   * clients are expected to free the dynamic memory allocated by this method
   */
  virtual RadeepRoutingHelper* Copy (void) const = 0;

  /**
   * \param node the node within which the new routing protocol will run
   * \returns a newly-created routing protocol
   */
  virtual Ptr<RadeepRoutingProtocol> Create (Ptr<Node> node) const = 0;

  /**
   * \brief prints the routing tables of all nodes at a particular time.
   * \param printTime the time at which the routing table is supposed to be printed.
   * \param stream The output stream object to use 
   * \param unit The time unit to be used in the report
   *
   * This method calls the PrintRoutingTable() method of the 
   * RadeepRoutingProtocol stored in the Radeep object, for all nodes at the
   * specified time; the output format is routing protocol-specific.
   */
  static void PrintRoutingTableAllAt (Time printTime, Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S);

  /**
   * \brief prints the routing tables of all nodes at regular intervals specified by user.
   * \param printInterval the time interval for which the routing table is supposed to be printed.
   * \param stream The output stream object to use
   * \param unit The time unit to be used in the report
   *
   * This method calls the PrintRoutingTable() method of the 
   * RadeepRoutingProtocol stored in the Radeep object, for all nodes at the
   * specified time interval; the output format is routing protocol-specific.
   */
  static void PrintRoutingTableAllEvery (Time printInterval, Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S);

  /**
   * \brief prints the routing tables of a node at a particular time.
   * \param printTime the time at which the routing table is supposed to be printed.
   * \param node The node ptr for which we need the routing table to be printed
   * \param stream The output stream object to use
   * \param unit The time unit to be used in the report
   *
   * This method calls the PrintRoutingTable() method of the 
   * RadeepRoutingProtocol stored in the Radeep object, for the selected node 
   * at the specified time; the output format is routing protocol-specific.
   */
  static void PrintRoutingTableAt (Time printTime, Ptr<Node> node, Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S);

  /**
   * \brief prints the routing tables of a node at regular intervals specified by user.
   * \param printInterval the time interval for which the routing table is supposed to be printed.
   * \param node The node ptr for which we need the routing table to be printed
   * \param stream The output stream object to use
   * \param unit The time unit to be used in the report
   *
   * This method calls the PrintRoutingTable() method of the 
   * RadeepRoutingProtocol stored in the Radeep object, for the selected node 
   * at the specified interval; the output format is routing protocol-specific.
   */
  static void PrintRoutingTableEvery (Time printInterval, Ptr<Node> node, Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S);

  /**
   * \brief prints the neighbor cache of all nodes at a particular time.
   * \param printTime the time at which the neighbor cache is supposed to be printed.
   * \param stream The output stream object to use
   *
   * This method calls the PrintArpCache() method of the
   * ArpCache associated with each RadeepInterface stored in the Radeep object, for all nodes at the
   * specified time. The output format is similar to:
   * \verbatim
     10.1.1.2 dev 1 lladdr 00-06-00:00:00:00:00:02 REACHABLE
     \endverbatim
   * Note that the MAC address is printed as "type"-"size"-"actual address"
   */
  static void PrintNeighborCacheAllAt (Time printTime, Ptr<OutputStreamWrapper> stream);

  /**
   * \brief prints the neighbor cache of all nodes at regular intervals specified by user.
   * \param printInterval the time interval for which the neighbor cache is supposed to be printed.
   * \param stream The output stream object to use
   *
   * This method calls the PrintArpCache() method of the
   * ArpCache associated with each RadeepInterface stored in the Radeep object, for all nodes at the
   * specified time. The output format is similar to:
   * \verbatim
     10.1.1.2 dev 1 lladdr 00-06-00:00:00:00:00:02 REACHABLE
     \endverbatim
   * Note that the MAC address is printed as "type"-"size"-"actual address"
   */
  static void PrintNeighborCacheAllEvery (Time printInterval, Ptr<OutputStreamWrapper> stream);

  /**
   * \brief prints the neighbor cache of a node at a particular time.
   * \param printTime the time at which the neighbor cache is supposed to be printed.
   * \param node The node ptr for which we need the neighbor cache to be printed
   * \param stream The output stream object to use
   *
   * This method calls the PrintArpCache() method of the
   * ArpCache associated with each RadeepInterface stored in the Radeep object, for all nodes at the
   * specified time. The output format is similar to:
   * \verbatim
     10.1.1.2 dev 1 lladdr 00-06-00:00:00:00:00:02 REACHABLE
     \endverbatim
   * Note that the MAC address is printed as "type"-"size"-"actual address"
   */
  static void PrintNeighborCacheAt (Time printTime, Ptr<Node> node, Ptr<OutputStreamWrapper> stream);

  /**
   * \brief prints the neighbor cache of a node at regular intervals specified by user.
   * \param printInterval the time interval for which the neighbor cache is supposed to be printed.
   * \param node The node ptr for which we need the neighbor cache to be printed
   * \param stream The output stream object to use
   *
   * This method calls the PrintArpCache() method of the
   * ArpCache associated with each RadeepInterface stored in the Radeep object, for all nodes at the
   * specified time. The output format is similar to:
   * \verbatim
     10.1.1.2 dev 1 lladdr 00-06-00:00:00:00:00:02 REACHABLE
     \endverbatim
   * Note that the MAC address is printed as "type"-"size"-"actual address"
   */
  static void PrintNeighborCacheEvery (Time printInterval, Ptr<Node> node, Ptr<OutputStreamWrapper> stream);

  /**
   * \brief Request a specified routing protocol &lt;T&gt; from RadeepRoutingProtocol protocol
   *
   * If protocol is RadeepListRouting, then protocol will be searched in the list,
   * otherwise a simple DynamicCast will be performed
   *
   * \param protocol Smart pointer to RadeepRoutingProtocol object
   * \return a Smart Pointer to the requested protocol (zero if the protocol can't be found)
   */
  template<class T>
  static Ptr<T> GetRouting (Ptr<RadeepRoutingProtocol> protocol);
  
private:
  /**
   * \brief prints the routing tables of a node.
   * \param node The node ptr for which we need the routing table to be printed
   * \param stream The output stream object to use
   * \param unit The time unit to be used in the report
   *
   * This method calls the PrintRoutingTable() method of the
   * RadeepRoutingProtocol stored in the Radeep object;
   * the output format is routing protocol-specific.
   */
  static void Print (Ptr<Node> node, Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S);

  /**
   * \brief prints the routing tables of a node at regular intervals specified by user.
   * \param printInterval the time interval for which the routing table is supposed to be printed.
   * \param node The node ptr for which we need the routing table to be printed
   * \param stream The output stream object to use
   * \param unit The time unit to be used in the report
   *
   * This method calls the PrintRoutingTable() method of the
   * RadeepRoutingProtocol stored in the Radeep object, for the selected node
   * at the specified interval; the output format is routing protocol-specific.
   */
  static void PrintEvery (Time printInterval, Ptr<Node> node, Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S);

  /**
   * \brief prints the neighbor cache of a node.
   * \param node The node ptr for which we need the neighbor cache to be printed
   * \param stream The output stream object to use
   *
   * This method calls the PrintArpCache() method of the
   * ArpCache associated with each RadeepInterface stored in the Radeep object, for all nodes at the
   * specified time. The output format is similar to:
   * \verbatim
     10.1.1.2 dev 1 lladdr 00-06-00:00:00:00:00:02 REACHABLE
     \endverbatim
   * Note that the MAC address is printed as "type"-"size"-"actual address"
   */
  static void PrintArpCache (Ptr<Node> node, Ptr<OutputStreamWrapper> stream);

  /**
   * \brief prints the neighbor cache of a node at regular intervals specified by user.
   * \param printInterval the time interval for which the neighbor cache is supposed to be printed.
   * \param node The node ptr for which we need the neighbor cache to be printed
   * \param stream The output stream object to use
   *
   * This method calls the PrintArpCache() method of the
   * ArpCache associated with each RadeepInterface stored in the Radeep object, for all nodes at the
   * specified time. The output format is similar to:
   * \verbatim
     10.1.1.2 dev 1 lladdr 00-06-00:00:00:00:00:02 REACHABLE
     \endverbatim
   * Note that the MAC address is printed as "type"-"size"-"actual address"
   */
  static void PrintArpCacheEvery (Time printInterval, Ptr<Node> node, Ptr<OutputStreamWrapper> stream);
};


/**
 * \brief Request a specified routing protocol &lt;T&gt; from RadeepRoutingProtocol protocol
 *
 * If protocol is RadeepListRouting, then protocol will be searched in the list,
 * otherwise a simple DynamicCast will be performed
 *
 * \param protocol Smart pointer to RadeepRoutingProtocol object
 * \return a Smart Pointer to the requested protocol (zero if the protocol can't be found)
 */
template<class T>
Ptr<T> RadeepRoutingHelper::GetRouting (Ptr<RadeepRoutingProtocol> protocol)
{
  Ptr<T> ret = DynamicCast<T> (protocol);
  if (ret == 0)
    {
      // trying to check if protocol is a list routing
      Ptr<RadeepListRouting> lrp = DynamicCast<RadeepListRouting> (protocol);
      if (lrp != 0)
        {
          for (uint32_t i = 0; i < lrp->GetNRoutingProtocols ();  i++)
            {
              int16_t priority;
              ret = GetRouting<T> (lrp->GetRoutingProtocol (i, priority)); // potential recursion, if inside ListRouting is ListRouting
              if (ret != 0)
                break;
            }
        }
    }

  return ret;
}

} // namespace ns3


#endif /* RADEEP_ROUTING_HELPER_H */
