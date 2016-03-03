#ifndef BM_SIM_INCLUDE_BM_SIM_LOOKUP_STRUCTURES_INTERFACES_H_
#define BM_SIM_INCLUDE_BM_SIM_LOOKUP_STRUCTURES_INTERFACES_H_

#include "match_unit_types.h"
#include "bytecontainer.h"

namespace bm {

//! This class defines an interface for all data structures used
//! in Match Units to perform lookups. Custom data strucures can
//! be created by implementing this interface, and creating a
//! factory class which will create them.
template <class V, template <typename EV=V> class Entry>
class LookupStructure {
  public:
    virtual ~LookupStructure() = default;

    //! Look up a given key in the data structure.
    //! Return true and set handle to the found value if there is a match
    //! Return false if there is not match (value in handle is undefined)
    virtual bool lookup(const ByteContainer & key,
        internal_handle_t * handle) = 0;

    //! Check whether an entry exists. This is distinct from a lookup operation
    //! in that this will also match against the prefix length in the case of
    //! an LPM structure, and against the mask and priority in the case of a
    //! Ternary structure.
    virtual bool entry_exists(const Entry<> & entry) = 0;

    //! Store an entry in the lookup structure. Associates the given handle
    //! with the given entry.
    virtual void store_entry(const Entry<> & entry, internal_handle_t handle) = 0;

    //! Remove a given entry from the structure. Has no effect if the entry
    //! does not exist.
    virtual void delete_entry(const Entry<> & entry) = 0;

    //! Completely remove all entries from the data structure.
    virtual void clear() = 0;

};

template <class V, template <typename EV> class E>
class LookupStructureFactoryPart {
  public:
    virtual void create( std::unique_ptr<LookupStructure<V,E>> & ptr ) = 0;
};

} // namespace bm


#endif
