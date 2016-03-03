#ifndef BM_SIM_INCLUDE_BM_SIM_LOOKUP_STRUCTURES_H_
#define BM_SIM_INCLUDE_BM_SIM_LOOKUP_STRUCTURES_H_

#include <bf_lpm_trie/bf_lpm_trie.h>

#include <type_traits>
#include <algorithm> // for std::swap

#include "match_unit_types.h"
#include "bytecontainer.h"


namespace bm {

//! This class defines an interface for all data structures used
//! in Match Units to perform lookups. Custom data strucures can
//! be created by implementing this interface, and creating a
//! factory class which will create them.
template <typename Entry>
class LookupStructureInterface {
  public:
    virtual ~LookupStructureInterface() = default;

    //! Look up a given key in the data structure.
    //! Return true and set handle to the found value if there is a match
    //! Return false if there is not match (value in handle is undefined)
    virtual bool lookup(const ByteContainer & key,
        internal_handle_t * handle) = 0;

    //! Check whether an entry exists. This is distinct from a lookup operation
    //! in that this will also match against the prefix length in the case of
    //! an LPM structure, and against the mask and priority in the case of a
    //! Ternary structure.
    virtual bool entry_exists(const Entry & entry) = 0;

    //! Store an entry in the lookup structure. Associates the given handle
    //! with the given entry.
    virtual void store_entry(const Entry & entry, internal_handle_t handle) = 0;

    //! Remove a given entry from the structure. Has no effect if the entry
    //! does not exist.
    virtual void delete_entry(const Entry & entry) = 0;

    //! Completely remove all entries from the data structure.
    virtual void clear() = 0;
};

static_assert(sizeof(value_t) == sizeof(internal_handle_t),
              "Invalid type sizes");

template <typename Entry,
          typename std::enable_if<Entry::mut == MatchUnitType::LPM, int>::type = 0>
class LPMTrie : public LookupStructureInterface<Entry> {
  public:
    explicit LPMTrie(size_t key_width_bytes)
      : key_width_bytes(key_width_bytes) {
        trie = bf_lpm_trie_create(key_width_bytes, true);
      }

    /* Copy constructor */
    LPMTrie(const LPMTrie& other) = delete;

    /* Move constructor */
    LPMTrie(LPMTrie&& other) noexcept
      : key_width_bytes(other.key_width_bytes), trie(other.trie) {}

    ~LPMTrie() {
      bf_lpm_trie_destroy(trie);
    }

    /* Copy assignment operator */
    LPMTrie &operator=(const LPMTrie &other) = delete;

    /* Move assignment operator */
    LPMTrie &operator=(LPMTrie &&other) noexcept {
      key_width_bytes = other.key_width_bytes;
      std::swap(trie, other.trie);
      return *this;
    }

    virtual bool lookup(const ByteContainer & key,
        internal_handle_t * handle) override{
      return bf_lpm_trie_lookup(trie, key.data(),
                                reinterpret_cast<value_t *>(handle));
    }

    virtual bool entry_exists(const Entry & entry) override{
      return bf_lpm_trie_has_prefix(trie, entry.key.data(), entry.prefix_length);
    }

    virtual void store_entry(const Entry & entry,
        internal_handle_t handle) override{
      bf_lpm_trie_insert(trie, entry.key.data(), entry.prefix_length,
          (value_t) handle);
    }

    virtual void delete_entry(const Entry & entry) override{
      bf_lpm_trie_delete(trie, entry.key.data(), entry.prefix_length);
    }

    virtual void clear() override{
      bf_lpm_trie_destroy(trie);
      trie = bf_lpm_trie_create(key_width_bytes, true);
    }

  private:
    size_t key_width_bytes{0};
    bf_lpm_trie_t *trie{nullptr};

};

template <typename Entry,
          typename std::enable_if<Entry::mut == MatchUnitType::EXACT, int>::type = 0>
class ExactMap : public LookupStructureInterface<Entry> {
  public:
    virtual bool lookup(const ByteContainer & key,
        internal_handle_t * handle) override{
      (void) key;
      (void) handle;
      return true;
    }

    virtual bool entry_exists(const Entry & entry) override{
      (void) entry;
      return false;
    }

    virtual void store_entry(const Entry & entry,
        internal_handle_t handle) override{
      (void) entry;
      (void) handle;
    }

    virtual void delete_entry(const Entry & entry) override{
      (void) entry;
    }

    virtual void clear() override{
    }
};

template <typename Entry,
          typename std::enable_if<Entry::mut == MatchUnitType::TERNARY, int>::type = 0>
class TernaryMap : public LookupStructureInterface<Entry> {
  public:
    virtual bool lookup(const ByteContainer & key,
        internal_handle_t * handle) override{
      (void) key;
      (void) handle;
      return true;
    }

    virtual bool entry_exists(const Entry & entry) override{
      (void) entry;
      return false;
    }

    virtual void store_entry(const Entry & entry,
        internal_handle_t handle) override{
      (void) entry;
      (void) handle;
    }

    virtual void delete_entry(const Entry & entry) override{
      (void) entry;
    }

    virtual void clear() override{
    }
};


} // namespace bm


#endif
