#ifndef RADEEP_LIST_ROUTING_HELPER_H
#define RADEEP_LIST_ROUTING_HELPER_H

#include "ns3/Radeep-routing-helper.h"
#include <stdint.h>
#include <list>

namespace ns3 {

/**
 * \ingroup RadeepHelpers
 *
 * \brief Helper class that adds ns3::RadeepListRouting objects
 *
 * This class is expected to be used in conjunction with 
 * ns3::InternetStackHelper::SetRoutingHelper
 */
class RadeepListRoutingHelper : public RadeepRoutingHelper
{
public:
  /*
   * Construct an RadeepListRoutingHelper used to make installing routing
   * protocols easier.
   */
  RadeepListRoutingHelper ();

  /*
   * Destroy an RadeepListRoutingHelper.
   */
  virtual ~RadeepListRoutingHelper ();

  /**
   * \brief Construct an RadeepListRoutingHelper from another previously 
   * initialized instance (Copy Constructor).
   */
  RadeepListRoutingHelper (const RadeepListRoutingHelper &);

  /**
   * \returns pointer to clone of this RadeepListRoutingHelper 
   * 
   * This method is mainly for internal use by the other helpers;
   * clients are expected to free the dynamic memory allocated by this method
   */
  RadeepListRoutingHelper* Copy (void) const;

  /**
   * \param routing a routing helper
   * \param priority the priority of the associated helper
   *
   * Store in the internal list a reference to the input routing helper
   * and associated priority. These helpers will be used later by
   * the ns3::RadeepListRoutingHelper::Create method to create
   * an ns3::RadeepListRouting object and add in it routing protocols
   * created with the helpers.
   */
  void Add (const RadeepRoutingHelper &routing, int16_t priority);
  /**
   * \param node the node on which the routing protocol will run
   * \returns a newly-created routing protocol
   *
   * This method will be called by ns3::InternetStackHelper::Install
   */
  virtual Ptr<RadeepRoutingProtocol> Create (Ptr<Node> node) const;
private:
  /**
   * \brief Assignment operator declared private and not implemented to disallow
   * assignment and prevent the compiler from happily inserting its own.
   * \return
   */
  RadeepListRoutingHelper &operator = (const RadeepListRoutingHelper &);

  /**
   * \brief Container for pairs of RadeepRoutingHelper pointer / priority.
   */
  std::list<std::pair<const RadeepRoutingHelper *,int16_t> > m_list;
};

} // namespace ns3

#endif /* RADEEP_LIST_ROUTING_HELPER_H */
