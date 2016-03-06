#ifndef BM_SIM_INCLUDE_BM_SIM_LOOKUP_STRUCTURES_H_
#define BM_SIM_INCLUDE_BM_SIM_LOOKUP_STRUCTURES_H_

#include "match_unit_types.h"
#include "bytecontainer.h"
#include "lookup_structures_interfaces.h"
#include "match_tables.h"

namespace bm {

class LookupStructureFactory
  // Inherits the virtual method to create each type of lookup structure
  // from each of these base classes
  : public LookupStructureFactoryPart
              <MatchTableAbstract::ActionEntry,   ExactEntry>
  , public LookupStructureFactoryPart
              <MatchTableIndirect::IndirectIndex, ExactEntry>
  , public LookupStructureFactoryPart
              <MatchTableAbstract::ActionEntry,   LPMEntry>
  , public LookupStructureFactoryPart
              <MatchTableIndirect::IndirectIndex, LPMEntry>
  , public LookupStructureFactoryPart
              <MatchTableAbstract::ActionEntry,   TernaryEntry>
  , public LookupStructureFactoryPart
              <MatchTableIndirect::IndirectIndex, TernaryEntry>
{
  public:
    virtual void create ( std::unique_ptr< LookupStructure
            < MatchTableAbstract::ActionEntry   , ExactEntry   >> & ) override = 0;
    virtual void create ( std::unique_ptr< LookupStructure
            < MatchTableIndirect::IndirectIndex , ExactEntry   >> & ) override = 0;
    virtual void create ( std::unique_ptr< LookupStructure
            < MatchTableAbstract::ActionEntry   , LPMEntry     >> & ) override = 0;
    virtual void create ( std::unique_ptr< LookupStructure
            < MatchTableIndirect::IndirectIndex , LPMEntry     >> & ) override = 0;
    virtual void create ( std::unique_ptr< LookupStructure
            < MatchTableAbstract::ActionEntry   , TernaryEntry >> & ) override = 0;
    virtual void create ( std::unique_ptr< LookupStructure
            < MatchTableIndirect::IndirectIndex , TernaryEntry >> & ) override = 0;
    // TODO :S
    //virtual void create ( std::unique_ptr<ExactAction>     )  override  = 0;
    //virtual void create ( std::unique_ptr<ExactIndirect>   )  override  = 0;
    //virtual void create ( std::unique_ptr<LPMAction>       )  override  = 0;
    //virtual void create ( std::unique_ptr<LPMIndirect>     )  override  = 0;
    //virtual void create ( std::unique_ptr<TernaryAction>   )  override  = 0;
    //virtual void create ( std::unique_ptr<TernaryIndirect> )  override  = 0;

};

// end clearly horrible
//! This class defines an interface for all data structures used
//! in Match Units to perform lookups. Custom data strucures can
//! be created by implementing this interface, and creating a
//! factory class which will create them.
template <typename K>
class LookupStructure {
  public:
    virtual ~LookupStructure() = default;

    //! Look up a given key in the data structure.
    //! Return true and set handle to the found value if there is a match
    //! Return false if there is not match (value in handle is undefined)
    virtual bool lookup(const ByteContainer & key_data,
        internal_handle_t * handle) = 0;

    //! Check whether an entry exists. This is distinct from a lookup operation
    //! in that this will also match against the prefix length in the case of
    //! an LPM structure, and against the mask and priority in the case of a
    //! Ternary structure.
    virtual bool entry_exists(const K & key) = 0;

    //! Store an entry in the lookup structure. Associates the given handle
    //! with the given entry.
    virtual void store_entry(const K & key, internal_handle_t handle) = 0;

    //! Remove a given entry from the structure. Has no effect if the entry
    //! does not exist.
    virtual void delete_entry(const K & key) = 0;

    //! Completely remove all entries from the data structure.
    virtual void clear() = 0;

};


} // namespace bm


#endif
