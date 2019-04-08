#ifndef RADEEP_QUEUE_DISC_ITEM_H
#define RADEEP_QUEUE_DISC_ITEM_H

#include "ns3/packet.h"
#include "ns3/queue-item.h"
#include "Radeep-header.h"

namespace ns3 {

/**
 * \ingroup Radeepv4
 * \ingroup traffic-control
 *
 * Radeepv4QueueDiscItem is a subclass of QueueDiscItem which stores Radeep packets.
 * Header and payload are kept separate to allow the queue disc to manipulate
 * the header, which is added to the packet when the packet is dequeued.
 */
class RadeepQueueDiscItem : public QueueDiscItem {
public:
  /**
   * \brief Create an Radeep queue disc item containing an Radeep packet.
   * \param p the packet included in the created item.
   * \param addr the destination MAC address
   * \param protocol the protocol number
   * \param header the Radeep header
   */
  RadeepQueueDiscItem (Ptr<Packet> p, const Address & addr, uint16_t protocol, const RadeepHeader & header);

  virtual ~RadeepQueueDiscItem ();

  /**
   * \return the correct packet size (header plus payload).
   */
  virtual uint32_t GetSize (void) const;

  /**
   * \return the header stored in this item..
   */
  const RadeepHeader & GetHeader (void) const;

  /**
   * \brief Add the header to the packet
   */
  virtual void AddHeader (void);

  /**
   * \brief Print the item contents.
   * \param os output stream in which the data should be printed.
   */
  virtual void Print (std::ostream &os) const;

  /*
   * The values for the fields of the Radeep header are taken from m_header and
   * thus might differ from those present in the packet in case the header is
   * modified after being added to the packet. However, this function is likely
   * to be called before the header is added to the packet (i.e., before the
   * packet is dequeued from the queue disc)
   */
  virtual bool GetUint8Value (Uint8Values field, uint8_t &value) const;

  /**
   * \brief Marks the packet by setting ECN_CE bits if the packet has ECN_ECT0 or ECN_ECT1 bits set
   * \return true if the packet gets marked, false otherwise
   */
  virtual bool Mark (void);

  /**
   * \brief Computes the hash of the packet's 5-tuple
   *
   * Computes the hash of the source and destination IP addresses, protocol
   * number and, if the transport protocol is either UDP or TCP, the source
   * and destination port
   *
   * \param perturbation hash perturbation value
   * \return the hash of the packet's 5-tuple
   */
  virtual uint32_t Hash (uint32_t perturbation) const;

private:
  /**
   * \brief Default constructor
   *
   * Defined and unimplemented to avoid misuse
   */
  RadeepQueueDiscItem ();
  /**
   * \brief Copy constructor
   *
   * Defined and unimplemented to avoid misuse
   */
  RadeepQueueDiscItem (const RadeepQueueDiscItem &);
  /**
   * \brief Assignment operator
   *
   * Defined and unimplemented to avoid misuse
   * \returns
   */
  RadeepQueueDiscItem &operator = (const RadeepQueueDiscItem &);

  RadeepHeader m_header;  //!< The Radeep header.
  bool m_headerAdded;   //!< True if the header has already been added to the packet.
};

} // namespace ns3

#endif /* RADEEP_QUEUE_DISC_ITEM_H */
