#ifndef RADEEP_GLOBAL_ROUTING_HELPER_H
#define RADEEP_GLOBAL_ROUTING_HELPER_H

#include "ns3/node-container.h"
#include "ns3/Radeep-routing-helper.h"

namespace ns3 {

/**
 * \ingroup RadeepHelpers
 *
 * \brief Helper class that adds ns3::RadeepGlobalRouting objects
 */
class RadeepGlobalRoutingHelper  : public RadeepRoutingHelper
{
public:
  /**
   * \brief Construct a GlobalRoutingHelper to make life easier for managing
   * global routing tasks.
   */
  RadeepGlobalRoutingHelper ();

  /**
   * \brief Construct a GlobalRoutingHelper from another previously initialized
   * instance (Copy Constructor).
   */
  RadeepGlobalRoutingHelper (const RadeepGlobalRoutingHelper &);

  /**
   * \returns pointer to clone of this RadeepGlobalRoutingHelper
   *
   * This method is mainly for internal use by the other helpers;
   * clients are expected to free the dynamic memory allocated by this method
   */
  RadeepGlobalRoutingHelper* Copy (void) const;

  /**
   * \param node the node on which the routing protocol will run
   * \returns a newly-created routing protocol
   *
   * This method will be called by ns3::InternetStackHelper::Install
   */
  virtual Ptr<RadeepRoutingProtocol> Create (Ptr<Node> node) const;

  /**
   * \brief Build a routing database and initialize the routing tables of
   * the nodes in the simulation.  Makes all nodes in the simulation into
   * routers.
   *
   * All this function does is call the functions
   * BuildGlobalRoutingDatabase () and  InitializeRoutes ().
   *
   */
  static void PopulateRoutingTables (void);
  /**
   * \brief Remove all routes that were previously installed in a prior call
   * to either PopulateRoutingTables() or RecomputeRoutingTables(), and 
   * add a new set of routes.
   * 
   * This method does not change the set of nodes
   * over which GlobalRouting is being used, but it will dynamically update
   * its representation of the global topology before recomputing routes.
   * Users must first call PopulateRoutingTables() and then may subsequently
   * call RecomputeRoutingTables() at any later time in the simulation.
   *
   */
  static void RecomputeRoutingTables (void);
private:
  /**
   * \brief Assignment operator declared private and not implemented to disallow
   * assignment and prevent the compiler from happily inserting its own.
   * \return
   */
  RadeepGlobalRoutingHelper &operator = (const RadeepGlobalRoutingHelper &);
};

} // namespace ns3

#endif /* Radeep_GLOBAL_ROUTING_HELPER_H */
