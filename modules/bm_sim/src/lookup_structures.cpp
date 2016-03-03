#include <bf_lpm_trie/bf_lpm_trie.h>

#include <type_traits>
#include <algorithm> // for std::swap

#include "bm_sim/lookup_structures.h"
#include "bm_sim/match_unit_types.h"
#include "bm_sim/match_units.h"
#include "bm_sim/match_tables.h"

namespace bm {

static_assert(sizeof(value_t) == sizeof(internal_handle_t),
              "Invalid type sizes");

// We don't need or want to export these classes outside of this
// compilation unit.
namespace /*anonymous*/ {
template <typename V>
class LPMTrie : public LookupStructure<V, LPMEntry> {
  public:
    typedef LPMEntry<V> Entry;
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

// TODO
template <typename V>
class ExactMap : public LookupStructure<V, ExactEntry> {
  public:
    typedef ExactEntry<V> Entry;

    ExactMap(size_t size){
      entries_map.reserve(size);
    }

    virtual bool lookup(const ByteContainer & key,
        internal_handle_t * handle) override{
      const auto entry_it = entries_map.find(key);
      if (entry_it == entries_map.end()) {
        return false; // Nothing found
      }else{
        *handle = entry_it->second; // TODO handle is internal_handle_t,
                                    // entry_it->second is entry_handle_t .....
        return true;
      }
    }

    virtual bool entry_exists(const Entry & entry) override{
      (void) entry;
      return entries_map.find(entry.key) != entries_map.end();
    }

    virtual void store_entry(const Entry & entry,
        internal_handle_t handle) override{
      // TODO same thing, I think the type of entries_map is wrong ...
      entries_map[entry.key] = handle;  // key is copied, which is not great
    }

    virtual void delete_entry(const Entry & entry) override{
      entries_map.erase(entry.key);
    }

    virtual void clear() override{
      entries_map.clear();
    }
  private:
    // TODO should be internal_handle_t I think .. ?
    std::unordered_map<ByteContainer, entry_handle_t, ByteContainerKeyHash>
      entries_map{};
};

// TODO
// uuuuuuuuuuuuugh ok so we will have to do a little bit of redundant
// storage of some stuff.
//
// ok but already LPM and Exact are storing handles two places...
// but they're not duplicating the keys and stuff...
//
// but we need this to be completely seperated to be able to model it....
//
// blargh
//
template <typename V>
class TernaryMap : public LookupStructure<V, TernaryEntry> {
  public:
    typedef TernaryEntry<V> Entry;

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

} // anonymous namespace

// We'll use a macro to avoid lengthy and repetitive specialization of
// all of these templates, since we can't partially specialize
#define LOOKUP_STRUCTURE_FACTORY_CREATE_1V(v, e, lookup_structure, args) \
  template <> \
  LookupStructure<v, e> * \
  LookupStructureFactory::create<v, e>() { \
    return new lookup_structure<v>args; \
  }

#define LOOKUP_STRUCTURE_FACTORY_CREATE(e, lookup_structure, args) \
  LOOKUP_STRUCTURE_FACTORY_CREATE_1V(MatchTableAbstract::ActionEntry,   e, lookup_structure, args) \
  LOOKUP_STRUCTURE_FACTORY_CREATE_1V(MatchTableIndirect::IndirectIndex, e, lookup_structure, args)

LOOKUP_STRUCTURE_FACTORY_CREATE(ExactEntry, ExactMap, (500))
LOOKUP_STRUCTURE_FACTORY_CREATE(LPMEntry, LPMTrie, (32))
LOOKUP_STRUCTURE_FACTORY_CREATE(TernaryEntry, TernaryMap, ())

#undef LOOKUP_STRUCTURE_FACTORY_CREATE
#undef LOOKUP_STRUCTURE_FACTORY_CREATE_1V


} // namespace bm
