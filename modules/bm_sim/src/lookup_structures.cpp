#include <bf_lpm_trie/bf_lpm_trie.h>

#include <algorithm> // for std::swap
#include <unordered_map>
#include <vector>
#include <tuple>

#include "bm_sim/lookup_structures.h"
#include "bm_sim/match_unit_types.h"


namespace bm {
namespace { // anonymous

static_assert(sizeof(value_t) == sizeof(internal_handle_t),
              "Invalid type sizes");

// We don't need or want to export these classes outside of this
// compilation unit.

class LPMTrie : public LookupStructure<LPMMatchKey> {
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

    virtual bool lookup(const ByteContainer & key_data,
        internal_handle_t * handle) override{
      return bf_lpm_trie_lookup(trie, key_data.data(),
                                reinterpret_cast<value_t *>(handle));
    }

    virtual bool entry_exists(const LPMMatchKey & key) override{
      return bf_lpm_trie_has_prefix(trie, key.data.data(), key.prefix_length);
    }

    virtual void store_entry(const LPMMatchKey & key,
        internal_handle_t handle) override{
      bf_lpm_trie_insert(trie, key.data.data(), key.prefix_length,
          (value_t) handle);
    }

    virtual void delete_entry(const LPMMatchKey & key) override{
      bf_lpm_trie_delete(trie, key.data.data(), key.prefix_length);
    }

    virtual void clear() override{
      bf_lpm_trie_destroy(trie);
      trie = bf_lpm_trie_create(key_width_bytes, true);
    }

  private:
    size_t key_width_bytes{0};
    bf_lpm_trie_t *trie{nullptr};

};

class ExactMap : public LookupStructure<ExactMatchKey> {
  public:
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

    virtual bool entry_exists(const ExactMatchKey & key) override{
      (void) key;
      return entries_map.find(key.data) != entries_map.end();
    }

    virtual void store_entry(const ExactMatchKey & key,
        internal_handle_t handle) override{
      // TODO same thing, I think the type of entries_map is wrong ...
      entries_map[key.data] = handle;  // key is copied, which is not great
    }

    virtual void delete_entry(const ExactMatchKey & key) override{
      entries_map.erase(key.data);
    }

    virtual void clear() override{
      entries_map.clear();
    }
  private:
    // TODO should be internal_handle_t I think .. ?
    std::unordered_map<ByteContainer, entry_handle_t, ByteContainerKeyHash>
      entries_map{};
};

class TernaryMap : public LookupStructure<TernaryMatchKey> {
  public:
    TernaryMap(size_t nbytes_key)
      : nbytes_key(nbytes_key) {}

    virtual bool lookup(const ByteContainer & key_data,
        internal_handle_t * handle) override{
      int min_priority = std::numeric_limits<int>::max();;
      bool match;

      const TernaryMatchKey *entry;
      const TernaryMatchKey *min_entry = nullptr;
      entry_handle_t min_handle = 0;

      // Loop through every handle that we have. Return the entry with matching mask|key combo
      // and highest (numerically lowest) priority

      for (auto it = this->handles.begin(); it != this->handles.end(); ++it) {
        entry = &std::get<0>(*it);

        if (entry->priority >= min_priority) continue;

        match = true;
        for (size_t byte_index = 0; byte_index < this->nbytes_key; byte_index++) {
          if (entry->data[byte_index] !=
              (key_data[byte_index] & entry->mask[byte_index])) {
            match = false;
            break;
          }
        }

        if (match) {
          min_priority = entry->priority;
          min_entry = entry;
          min_handle = std::get<1>(*it);
        }
      }

      if (min_entry) {
        *handle = min_handle;
        return true;
      }

      return false;
    }

    virtual bool entry_exists(const TernaryMatchKey & key) override{
      auto it = find_handle(key);

      return it != this->handles.end();
    }

    virtual void store_entry(const TernaryMatchKey & key,
        internal_handle_t handle) override{
      // TODO avoid copying key ?
      handles.emplace_back(std::make_tuple(key, handle));
    }

    virtual void delete_entry(const TernaryMatchKey & key) override{
      auto it = find_handle(key);
      this->handles.erase(it);
    }

    virtual void clear() override{
      this->handles.clear();
    }
  private:
    std::vector<std::tuple<TernaryMatchKey, internal_handle_t>> handles;
    size_t nbytes_key;

    decltype(handles)::iterator find_handle(const TernaryMatchKey & key) {
      auto it = this->handles.begin();
      for (; it != this->handles.end(); ++it) {
        const TernaryMatchKey &entry = std::get<0>(*it);

        if (entry.priority == key.priority &&
            entry.data == key.data &&
            entry.mask == key.mask) {
          break;
        }
      }
      return it;
    }
};

} // anonymous namespace

void LookupStructureFactory::create(std::unique_ptr<LookupStructure<ExactMatchKey>> & ls, size_t size, size_t nbytes_key){
  (void) nbytes_key;
  ls.reset(new ExactMap(size));
}

void LookupStructureFactory::create(std::unique_ptr<LookupStructure<LPMMatchKey>> & ls, size_t size, size_t nbytes_key){
  (void)size;
  ls.reset(new LPMTrie(nbytes_key));
}

void LookupStructureFactory::create(std::unique_ptr<LookupStructure<TernaryMatchKey>> & ls, size_t size, size_t nbytes_key){
  (void) size;
  ls.reset(new TernaryMap(nbytes_key));
}

} // namespace bm
