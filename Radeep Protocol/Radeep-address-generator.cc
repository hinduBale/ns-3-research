#include <list>
#include "ns3/abort.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/simulation-singleton.h"
#include "Radeep-address-generator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RadeepAddressGenerator");

/**
 * \ingroup address
 *
 * \brief Implementation class of RadeepAddressGenerator
 * This generator assigns addresses sequentially from a provided
 * network address; used in topology code. It also keeps track of all
 * addresses assigned to perform duplicate detection.
 */
class RadeepAddressGeneratorImpl
{
public:
  RadeepAddressGeneratorImpl ();
  virtual ~RadeepAddressGeneratorImpl ();

  /**
   * \brief Initialise the base network, mask and address for the generator
   *
   * The first call to NextAddress() or GetAddress() will return the
   * value passed in.
   *
   * \param net The network for the base RadeepAddress
   * \param mask The network mask of the base RadeepAddress
   * \param addr The base address used for initialization
   */
  void Init (const RadeepAddress net, const RadeepMask mask, 
             const RadeepAddress addr);

  /**
   * \brief Get the current network of the given RadeepMask
   *
   * Does not change the internal state; this just peeks at the current
   * network
   *
   * \param mask The RadeepMask for the current network
   * \returns the Radeep address of the current network
   */
  RadeepAddress GetNetwork (const RadeepMask mask) const;

  /**
   * \brief Get the next network according to the given RadeepMask
   *
   * This operation is a pre-increment, meaning that the internal state
   * is changed before returning the new network address.
   *
   * This also resets the address to the base address that was
   * used for initialization.
   *
   * \param mask The RadeepMask used to set the next network
   * \returns the Radeep address of the next network
   */
  RadeepAddress NextNetwork (const RadeepMask mask);

  /**
   * \brief Set the address for the given mask
   *
   * \param addr The address to set for the current mask
   * \param mask The RadeepMask whose address is to be set
   */
  void InitAddress (const RadeepAddress addr, const RadeepMask mask);

  /**
   * \brief Allocate the next RadeepAddress for the configured network and mask
   *
   * This operation is a post-increment, meaning that the first address
   * allocated will be the one that was initially configured.
   *
   * \param mask The RadeepMask for the current network
   * \returns the Radeep address
   */
  RadeepAddress NextAddress (const RadeepMask mask);

  /**
   * \brief Get the RadeepAddress that will be allocated upon NextAddress ()
   *
   * Does not change the internal state; just is used to peek the next
   * address that will be allocated upon NextAddress ()
   *
   * \param mask The RadeepMask for the current network
   * \returns the Radeep address
   */
  RadeepAddress GetAddress (const RadeepMask mask) const;

  /**
   * \brief Reset the networks and RadeepAddress to zero
   */
  void Reset (void);

  /**
   * \brief Add the RadeepAddress to the list of Radeep entries
   *
   * Typically, this is used by external address allocators that want
   * to make use of this class's ability to track duplicates.  AddAllocated
   * is always called internally for any address generated by NextAddress ()
   *
   * \param addr The RadeepAddress to be added to the list of Radeep entries
   * \returns true on success
   */
  bool AddAllocated (const RadeepAddress addr);

  /**
   * \brief Check the RadeepAddress allocation in the list of Radeep entries
   *
   * \param addr The RadeepAddress to be checked in the list of Radeep entries
   * \returns true if the address is already allocated
   */
  bool IsAddressAllocated (const RadeepAddress addr);

  /**
   * \brief Check if a network has already allocated addresses
   *
   * \param addr The Radeep network to be checked
   * \param mask The Radeep network mask
   * \returns true if the network is already allocated
   */
  bool IsNetworkAllocated (const RadeepAddress addr, const RadeepMask mask);

  /**
   * \brief Used to turn off fatal errors and assertions, for testing
   */
  void TestMode (void);
private:
  static const uint32_t N_BITS = 32;  //!< the number of bits in the address
  static const uint32_t MOST_SIGNIFICANT_BIT = 0x80000000; //!< MSB set to 1

  /**
   * \brief Create an index number for the network mask
   * \param mask the mask to index
   * \returns an index
   */
  uint32_t MaskToIndex (RadeepMask mask) const;

  /**
   * \brief This class holds the state for a given network
   */
  class NetworkState
  {
public:
    uint32_t mask;      //!< the network mask
    uint32_t shift;     //!< a shift
    uint32_t network;   //!< the network
    uint32_t addr;      //!< the address
    uint32_t addrMax;   //!< the maximum address
  };

  NetworkState m_netTable[N_BITS]; //!< the available networks

  /**
   * \brief This class holds the allocated addresses
   */
  class Entry
  {
public:
    uint32_t addrLow;  //!< the lowest allocated address
    uint32_t addrHigh; //!< the highest allocated address
  };

  std::list<Entry> m_entries; //!< contained of allocated addresses
  bool m_test; //!< test mode (if true)
};

RadeepAddressGeneratorImpl::RadeepAddressGeneratorImpl () 
  : m_entries (), m_test (false)
{
  NS_LOG_FUNCTION (this);
  Reset ();
}

void
RadeepAddressGeneratorImpl::Reset (void)
{
  NS_LOG_FUNCTION (this);

  uint32_t mask = 0;
//
// There are 32 possible masks in a 32-bit integer.  Two of these are illegal
// for a network mask (0x00000000 and 0xffffffff).  Valid network masks
// correspond to some nonzero number of high order bits set to one followed by 
// some nonzero number of lower order bits set to zero.
//
// We look at a network number as an n-bit number where n is defined as the 
// number of bits in each mask.  Allocating a new network number is simply 
// incrementing this number.
//
// In order to combine an allocated network number with an Radeep address, we have
// to shift the network into the correct alignment with respect to its mask.
// For example, a network mask of 0xff000000 admits the possibility of 256
// different network numbers since there are eight bits available.  To create
// Radeep addresses, we need to shift the network number counter left by 24 bits
// to put it in correct alignment.  This leaves 24 bits left for addresses.
// We make sure we don't overflow by saving a maximum address number which is
// just the inverse of the mask (~mask).
//
  for (uint32_t i = 0; i < N_BITS; ++i)
    {
      m_netTable[i].mask = mask;
      mask >>= 1;
      mask |= MOST_SIGNIFICANT_BIT;
      m_netTable[i].network = 1;
      m_netTable[i].addr = 1;
      m_netTable[i].addrMax = ~m_netTable[i].mask;
      m_netTable[i].shift = N_BITS - i;
    }
  m_entries.clear ();
  m_test = false;
}

RadeepAddressGeneratorImpl::~RadeepAddressGeneratorImpl ()
{
  NS_LOG_FUNCTION (this);
}

void
RadeepAddressGeneratorImpl::Init (
  const RadeepAddress net,
  const RadeepMask mask,
  const RadeepAddress addr)
{
  NS_LOG_FUNCTION (this << net << mask << addr);
//
// We're going to be playing with the actual bits in the network and mask so
// pull them out into ints.
//
  uint32_t maskBits = mask.Get ();
  uint32_t netBits = net.Get ();
  uint32_t addrBits = addr.Get ();
//
// Some quick reasonableness testing.
//
  NS_ABORT_MSG_UNLESS ((netBits & ~maskBits) == 0, "RadeepAddressGeneratorImpl::Init (): Inconsistent network and mask");
  NS_ABORT_MSG_UNLESS ((addrBits & maskBits) == 0, "RadeepAddressGeneratorImpl::Init (): Inconsistent address and mask");

//
// Convert the network mask into an index into the network number table.
// The network number comes in to us properly aligned for the mask and so
// needs to be shifted right into the normalized position (lowest bit of the
// network number at bit zero of the int that holds it).
//
  uint32_t index = MaskToIndex (mask);

  m_netTable[index].network = netBits >> m_netTable[index].shift;

  NS_ABORT_MSG_UNLESS (addrBits <= m_netTable[index].addrMax, "RadeepAddressGeneratorImpl::Init(): Address overflow");
  m_netTable[index].addr = addrBits;
  return;
}

RadeepAddress
RadeepAddressGeneratorImpl::GetNetwork (
  const RadeepMask mask) const
{
  NS_LOG_FUNCTION (this << mask);

  uint32_t index = MaskToIndex (mask);
  return RadeepAddress (m_netTable[index].network << m_netTable[index].shift);
}

RadeepAddress
RadeepAddressGeneratorImpl::NextNetwork (
  const RadeepMask mask)
{
  NS_LOG_FUNCTION (this << mask);
//
// The way this is expected to be used is that an address and network prefix
// are initialized, and then NextAddress() is called repeatedly to set the
// addresses on a given subnet.  The client will expect that the first 
// addresses will use the network prefix she used to initialize the generator
// with.  After a subnet is assigned, the client will call NextNetwork to 
// get the network number of the next subnet.  This implies that that this
// operation is a pre-increment.
//
  uint32_t index = MaskToIndex (mask);
  ++m_netTable[index].network;
  return RadeepAddress (m_netTable[index].network << m_netTable[index].shift);
}

void
RadeepAddressGeneratorImpl::InitAddress (
  const RadeepAddress addr,
  const RadeepMask mask)
{
  NS_LOG_FUNCTION (this << addr << mask);

  uint32_t index = MaskToIndex (mask);
  uint32_t addrBits = addr.Get ();

  NS_ABORT_MSG_UNLESS (addrBits <= m_netTable[index].addrMax, "RadeepAddressGeneratorImpl::InitAddress(): Address overflow");
  m_netTable[index].addr = addrBits;
}

RadeepAddress
RadeepAddressGeneratorImpl::GetAddress (
  const RadeepMask mask) const
{
  NS_LOG_FUNCTION (this << mask);

  uint32_t index = MaskToIndex (mask);

  return RadeepAddress (
           (m_netTable[index].network << m_netTable[index].shift) |
           m_netTable[index].addr);
}

RadeepAddress
RadeepAddressGeneratorImpl::NextAddress (const RadeepMask mask)
{
  NS_LOG_FUNCTION (this << mask);
//
// The way this is expected to be used is that an address and network prefix
// are initialized, and then NextAddress() is called repeatedly to set the
// addresses on a given subnet.  The client will expect that the first address
// she gets back is the one she used to initialize the generator with.  This
// implies that this operation is a post-increment.
//
  uint32_t index = MaskToIndex (mask);

  NS_ABORT_MSG_UNLESS (m_netTable[index].addr <= m_netTable[index].addrMax,
                       "RadeepAddressGeneratorImpl::NextAddress(): Address overflow");

  RadeepAddress addr = RadeepAddress (
      (m_netTable[index].network << m_netTable[index].shift) |
      m_netTable[index].addr);

  ++m_netTable[index].addr;
//
// Make a note that we've allocated this address -- used for address collision
// detection.
//
  AddAllocated (addr);
  return addr;
}

bool
RadeepAddressGeneratorImpl::AddAllocated (const RadeepAddress address)
{
  NS_LOG_FUNCTION (this << address);

  uint32_t addr = address.Get ();

  NS_ABORT_MSG_UNLESS (addr, "RadeepAddressGeneratorImpl::Add(): Allocating the broadcast address is not a good idea"); 
 
  std::list<Entry>::iterator i;

  for (i = m_entries.begin (); i != m_entries.end (); ++i)
    {
      NS_LOG_LOGIC ("examine entry: " << RadeepAddress ((*i).addrLow) << 
                    " to " << RadeepAddress ((*i).addrHigh));
//
// First things first.  Is there an address collision -- that is, does the
// new address fall in a previously allocated block of addresses.
//
      if (addr >= (*i).addrLow && addr <= (*i).addrHigh)
        {
          NS_LOG_LOGIC ("RadeepAddressGeneratorImpl::Add(): Address Collision: " << RadeepAddress (addr)); 
          if (!m_test) 
            {
              NS_FATAL_ERROR ("RadeepAddressGeneratorImpl::Add(): Address Collision: " << RadeepAddress (addr));
            }
          return false;
        }
//
// If the new address is less than the lowest address in the current block,
// and can't be merged into to the current block, then insert it as a new
// block before the current block.
//
      if (addr < (*i).addrLow - 1)
        {
          break;
        }
//
// If the new address fits at the end of the block, look ahead to the next 
// block and make sure it's not a collision there.  If we won't overlap, then
// just extend the current block by one address.  We expect that completely
// filled network ranges will be a fairly rare occurrence, so we don't worry
// about collapsing address range blocks.
// 
      if (addr == (*i).addrHigh + 1)
        {
          std::list<Entry>::iterator j = i;
          ++j;

          if (j != m_entries.end ())
            {
              if (addr == (*j).addrLow)
                {
                  NS_LOG_LOGIC ("RadeepAddressGeneratorImpl::Add(): "
                                "Address Collision: " << RadeepAddress (addr));
                  if (!m_test)
                    {
                      NS_FATAL_ERROR ("RadeepAddressGeneratorImpl::Add(): Address Collision: " << RadeepAddress (addr));
                    }
                  return false;
                }
            }

          NS_LOG_LOGIC ("New addrHigh = " << RadeepAddress (addr));
          (*i).addrHigh = addr;
          return true;
        }
//
// If we get here, we know that the next lower block of addresses couldn't 
// have been extended to include this new address since the code immediately 
// above would have been executed and that next lower block extended upward.
// So we know it's safe to extend the current block down to include the new
// address.
//
      if (addr == (*i).addrLow - 1)
        {
          NS_LOG_LOGIC ("New addrLow = " << RadeepAddress (addr));
          (*i).addrLow = addr;
          return true;
        }
    }

  Entry entry;
  entry.addrLow = entry.addrHigh = addr;
  m_entries.insert (i, entry);
  return true;
}

bool
RadeepAddressGeneratorImpl::IsAddressAllocated (const RadeepAddress address)
{
  NS_LOG_FUNCTION (this << address);

  uint32_t addr = address.Get ();

  NS_ABORT_MSG_UNLESS (addr, "RadeepAddressGeneratorImpl::IsAddressAllocated(): Don't check for the broadcast address...");

  std::list<Entry>::iterator i;

  for (i = m_entries.begin (); i != m_entries.end (); ++i)
    {
      NS_LOG_LOGIC ("examine entry: " << RadeepAddress ((*i).addrLow) <<
                    " to " << RadeepAddress ((*i).addrHigh));
      if (addr >= (*i).addrLow && addr <= (*i).addrHigh)
        {
          NS_LOG_LOGIC ("RadeepAddressGeneratorImpl::IsAddressAllocated(): Address Collision: " << RadeepAddress (addr));
          return false;
        }
    }
  return true;
}

bool
RadeepAddressGeneratorImpl::IsNetworkAllocated (const RadeepAddress address, const RadeepMask mask)
{
  NS_LOG_FUNCTION (this << address << mask);

  NS_ABORT_MSG_UNLESS (address == address.CombineMask (mask),
                       "RadeepAddressGeneratorImpl::IsNetworkAllocated(): network address and mask don't match " << address << " " << mask);

  std::list<Entry>::iterator i;

  for (i = m_entries.begin (); i != m_entries.end (); ++i)
    {
      NS_LOG_LOGIC ("examine entry: " << RadeepAddress ((*i).addrLow) << " to " << RadeepAddress ((*i).addrHigh));
      RadeepAddress low = RadeepAddress ((*i).addrLow);
      RadeepAddress high = RadeepAddress ((*i).addrHigh);

      if (address == low.CombineMask (mask) || address == high.CombineMask (mask))
        {
          NS_LOG_LOGIC ("RadeepAddressGeneratorImpl::IsNetworkAllocated(): Network already allocated: " <<
                        address << " " << low << "-" << high);
          return false;
        }

    }
  return true;
}


void
RadeepAddressGeneratorImpl::TestMode (void)
{
  NS_LOG_FUNCTION (this);
  m_test = true;
}

uint32_t
RadeepAddressGeneratorImpl::MaskToIndex (RadeepMask mask) const
{
  
  NS_LOG_FUNCTION (this << mask);
  
//
// We've been given a mask that has a higher order bit set for each bit of the
// network number.  In order to translate this mask into an index, we just need
// to count the number of zero bits in the mask.  We do this in a loop in which
// we shift the mask right until we find the first nonzero bit.  This tells us
// the number of zero bits, and from this we infer the number of nonzero bits
// which is the number of bits in the mask.
//
// We use the number of bits in the mask as the number of bits in the network
// number and as the index into the network number state table.
//
  uint32_t maskBits = mask.Get ();

  for (uint32_t i = 0; i < N_BITS; ++i)
    {
      if (maskBits & 1)
        {
          uint32_t index = N_BITS - i;
          NS_ABORT_MSG_UNLESS (index > 0 && index < N_BITS, "RadeepAddressGenerator::MaskToIndex(): Illegal Mask");
          return index;
        }
      maskBits >>= 1;
    }
  NS_ASSERT_MSG (false, "RadeepAddressGenerator::MaskToIndex(): Impossible");
  return 0;
}

void
RadeepAddressGenerator::Init (
  const RadeepAddress net,
  const RadeepMask mask,
  const RadeepAddress addr)
{
  NS_LOG_FUNCTION_NOARGS ();

  SimulationSingleton<RadeepAddressGeneratorImpl>::Get ()
  ->Init (net, mask, addr);
}

RadeepAddress
RadeepAddressGenerator::NextNetwork (const RadeepMask mask)
{
  NS_LOG_FUNCTION_NOARGS ();

  return SimulationSingleton<RadeepAddressGeneratorImpl>::Get ()
         ->NextNetwork (mask);
}

RadeepAddress
RadeepAddressGenerator::GetNetwork (const RadeepMask mask)
{
  NS_LOG_FUNCTION_NOARGS ();

  return SimulationSingleton<RadeepAddressGeneratorImpl>::Get ()
         ->GetNetwork (mask);
}

void
RadeepAddressGenerator::InitAddress (
  const RadeepAddress addr,
  const RadeepMask mask)
{
  NS_LOG_FUNCTION_NOARGS ();

  SimulationSingleton<RadeepAddressGeneratorImpl>::Get ()
  ->InitAddress (addr, mask);
}

RadeepAddress
RadeepAddressGenerator::GetAddress (const RadeepMask mask)
{
  NS_LOG_FUNCTION_NOARGS ();

  return SimulationSingleton<RadeepAddressGeneratorImpl>::Get ()
         ->GetAddress (mask);
}

RadeepAddress
RadeepAddressGenerator::NextAddress (const RadeepMask mask)
{
  NS_LOG_FUNCTION_NOARGS ();

  return SimulationSingleton<RadeepAddressGeneratorImpl>::Get ()
         ->NextAddress (mask);
}

void
RadeepAddressGenerator::Reset (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  return SimulationSingleton<RadeepAddressGeneratorImpl>::Get ()
         ->Reset ();
}

bool
RadeepAddressGenerator::AddAllocated (const RadeepAddress addr)
{
  NS_LOG_FUNCTION_NOARGS ();

  return SimulationSingleton<RadeepAddressGeneratorImpl>::Get ()
         ->AddAllocated (addr);
}

bool
RadeepAddressGenerator::IsAddressAllocated (const RadeepAddress addr)
{
  NS_LOG_FUNCTION_NOARGS ();

  return SimulationSingleton<RadeepAddressGeneratorImpl>::Get ()
         ->IsAddressAllocated (addr);
}

bool
RadeepAddressGenerator::IsNetworkAllocated (const RadeepAddress addr, const RadeepMask mask)
{
  NS_LOG_FUNCTION_NOARGS ();

  return SimulationSingleton<RadeepAddressGeneratorImpl>::Get ()
         ->IsNetworkAllocated (addr, mask);
}

void
RadeepAddressGenerator::TestMode (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  SimulationSingleton<RadeepAddressGeneratorImpl>::Get ()
  ->TestMode ();
}

} // namespace ns3

