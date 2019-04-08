#ifndef RADEEP_INTERFACE_CONTAINER_H
#define RADEEP_INTERFACE_CONTAINER_H

#include <stdint.h>
#include <vector>
#include "ns3/Radeep.h"
#include "ns3/Radeep-address.h"

namespace ns3 {

/**
 * \ingroup Radeep
 *
 * \brief holds a vector of std::pair of Ptr<Radeep> and interface index.
 *
 * Typically ns-3 RadeepInterfaces are installed on devices using an Radeep address
 * helper.  The helper's Assign() method takes a NetDeviceContainer which holds 
 * some number of Ptr<NetDevice>.  For each of the NetDevices in the 
 * NetDeviceContainer the helper will find the associated Ptr<Node> and
 * Ptr<Radeep>.  It makes sure that an interface exists on the node for the 
 * device and then adds an RadeepAddress according to the address helper settings
 * (incrementing the RadeepAddress somehow as it goes).  The helper then converts
 * the Ptr<Radeep> and the interface index to a std::pair and adds them to a 
 * container -- a container of this type.
 *
 * The point is then to be able to implicitly associate an index into the 
 * original NetDeviceContainer (that identifies a particular net device) with
 * an identical index into the RadeepInterfaceContainer that has a std::pair with
 * the Ptr<Radeep> and interface index you need to play with the interface.
 *
 * @see RadeepAddressHelper
 * @see Radeep
 */
class RadeepInterfaceContainer
{
public:
  /**
   * \brief Container Const Iterator for pairs of Radeep smart pointer / Interface Index.
   */
  typedef std::vector<std::pair<Ptr<Radeep>, uint32_t> >::const_iterator Iterator;

  /**
   * Create an empty RadeepInterfaceContainer.
   */
  RadeepInterfaceContainer ();

  /**
   * Concatenate the entries in the other container with ours.
   * \param other container
   */
  void Add (const RadeepInterfaceContainer& other);

  /**
   * \brief Get an iterator which refers to the first pair in the 
   * container.
   *
   * Pairs can be retrieved from the container in two ways.  First,
   * directly by an index into the container, and second, using an iterator.
   * This method is used in the iterator method and is typically used in a 
   * for-loop to run through the pairs
   *
   * \code
   *   RadeepInterfaceContainer::Iterator i;
   *   for (i = container.Begin (); i != container.End (); ++i)
   *     {
   *       std::pair<Ptr<Radeep>, uint32_t> pair = *i;
   *       method (pair.first, pair.second);  // use the pair
   *     }
   * \endcode
   *
   * \returns an iterator which refers to the first pair in the container.
   */
  Iterator Begin (void) const;

  /**
   * \brief Get an iterator which indicates past-the-last Node in the 
   * container.
   *
   * Nodes can be retrieved from the container in two ways.  First,
   * directly by an index into the container, and second, using an iterator.
   * This method is used in the iterator method and is typically used in a 
   * for-loop to run through the Nodes
   *
   * \code
   *   NodeContainer::Iterator i;
   *   for (i = container.Begin (); i != container.End (); ++i)
   *     {
   *       std::pair<Ptr<Radeep>, uint32_t> pair = *i;
   *       method (pair.first, pair.second);  // use the pair
   *     }
   * \endcode
   *
   * \returns an iterator which indicates an ending condition for a loop.
   */
  Iterator End (void) const;

  /**
   * \returns the number of Ptr<Radeep> and interface pairs stored in this 
   * RadeepInterfaceContainer.
   *
   * Pairs can be retrieved from the container in two ways.  First,
   * directly by an index into the container, and second, using an iterator.
   * This method is used in the direct method and is typically used to
   * define an ending condition in a for-loop that runs through the stored
   * Nodes
   *
   * \code
   *   uint32_t nNodes = container.GetN ();
   *   for (uint32_t i = 0 i < nNodes; ++i)
   *     {
   *       std::pair<Ptr<Radeep>, uint32_t> pair = container.Get (i);
   *       method (pair.first, pair.second);  // use the pair
   *     }
   * \endcode
   *
   * \returns the number of Ptr<Node> stored in this container.
   */
  uint32_t GetN (void) const;

  /**
   * \param i index of ipInterfacePair in container
   * \param j interface address index (if interface has multiple addresses)
   * \returns the Radeep address of the j'th address of the interface
   *  corresponding to index i.
   * 
   * If the second parameter is omitted, the zeroth indexed address of 
   * the interface is returned.  Unless IP aliasing is being used on
   * the interface, the second parameter may typically be omitted.
   */
  RadeepAddress GetAddress (uint32_t i, uint32_t j = 0) const;

  /**
   * \brief Set a metric for the given interface
   * \param i Interface index
   * \param metric the interface metric
   */
  void SetMetric (uint32_t i, uint16_t metric);

  /**
   * Manually add an entry to the container consisting of the individual parts
   * of an entry std::pair.
   *
   * \param Radeep pointer to Radeep object
   * \param interface interface index of the RadeepInterface to add to the container
   */
  void Add (Ptr<Radeep> radeep, uint32_t interface);

  /**
   * Manually add an entry to the container consisting of a previously composed 
   * entry std::pair.
   *
   * \param ipInterfacePair the pair of a pointer to Radeep object and interface index of the RadeepInterface to add to the container
   */
  void Add (std::pair<Ptr<Radeep>, uint32_t> radeepInterfacePair);

  /**
   * Manually add an entry to the container consisting of the individual parts
   * of an entry std::pair.
   *
   * \param RadeepName std:string referring to the saved name of an Radeep Object that
   *        has been previously named using the Object Name Service.
   * \param interface interface index of the RadeepInterface to add to the container
   */
  void Add (std::string radeepName, uint32_t interface);

  /**
   * Get the std::pair of an Ptr<Radeep> and interface stored at the location 
   * specified by the index.
   *
   * \param i the index of the container entry to retrieve.
   * \return the std::pair of a Ptr<Radeep> and an interface index
   *
   * \note The returned Ptr<Radeep> cannot be used directly to fetch the
   *       RadeepInterface using the returned index (the GetInterface () method
   *       is provided in class RadeepL3Protocol, and not class Radeep). An
   *       example usage is provided below.
   *
   * \code
   *   RadeepInterfaceContainer c;
   *   ...
   *   std::pair<Ptr<Radeep>, uint32_t> returnValue = c.Get (0);
   *   Ptr<Radeep> Radeep = returnValue.first;
   *   uint32_t index = returnValue.second;
   *   Ptr<RadeepInterface> iface =  Radeep->GetObject<RadeepL3Protocol> ()->GetInterface (index);
   * \endcode
   */
  std::pair<Ptr<Radeep>, uint32_t> Get (uint32_t i) const;

private:
  /**
   * \brief Container for pairs of Radeep smart pointer / Interface Index.
   */
  typedef std::vector<std::pair<Ptr<Radeep>,uint32_t> > InterfaceVector;

  /**
   * \brief List of Radeep stack and interfaces index.
   */
  InterfaceVector m_interfaces;
};

} // namespace ns3

#endif /* RADEEP_INTERFACE_CONTAINER_H */
