/* Copyright 2013-present Barefoot Networks, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Gordon Bailey (gb@gordonbailey.net)
 *
 */

#include <bf_lpm_trie/bf_lpm_trie.h>

#include <algorithm>  // for std::swap
#include <unordered_map>
#include <vector>
#include <tuple>
#include <limits>

#include "bm_sim/lookup_structures.h"
#include "bm_sim/match_unit_types.h"


namespace bm {
namespace {  // anonymous

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

  bool lookup(const ByteContainer & key_data,
              internal_handle_t * handle) override {
    return bf_lpm_trie_lookup(trie, key_data.data(),
                              reinterpret_cast<value_t *>(handle));
  }

  bool entry_exists(const LPMMatchKey & key) override {
    return bf_lpm_trie_has_prefix(trie, key.data.data(), key.prefix_length);
  }

  void store_entry(const LPMMatchKey & key,
                   internal_handle_t handle) override {
    bf_lpm_trie_insert(trie, key.data.data(), key.prefix_length,
        (value_t) handle);
  }

  void delete_entry(const LPMMatchKey & key) override {
    bf_lpm_trie_delete(trie, key.data.data(), key.prefix_length);
  }

  void clear() override {
    bf_lpm_trie_destroy(trie);
    trie = bf_lpm_trie_create(key_width_bytes, true);
  }

 private:
  size_t key_width_bytes{0};
  bf_lpm_trie_t *trie{nullptr};
};

class ExactMap : public LookupStructure<ExactMatchKey> {
 public:
  explicit ExactMap(size_t size) {
    entries_map.reserve(size);
  }

  bool lookup(const ByteContainer & key,
              internal_handle_t * handle) override {
    const auto entry_it = entries_map.find(key);
    if (entry_it == entries_map.end()) {
      return false;  // Nothing found
    } else {
      *handle = entry_it->second;
      return true;
    }
  }

  bool entry_exists(const ExactMatchKey & key) override {
    (void) key;
    return entries_map.find(key.data) != entries_map.end();
  }

  void store_entry(const ExactMatchKey & key,
      internal_handle_t handle) override {
    entries_map[key.data] = handle;  // key is copied, which is not great
  }

  void delete_entry(const ExactMatchKey & key) override {
    entries_map.erase(key.data);
  }

  void clear() override {
    entries_map.clear();
  }

 private:
  // TODO(gordon) Should this be internal_handle_t ?
  std::unordered_map<ByteContainer, entry_handle_t, ByteContainerKeyHash>
    entries_map{};
};

class TernaryMap : public LookupStructure<TernaryMatchKey> {
 public:
  explicit TernaryMap(size_t nbytes_key)
    : nbytes_key(nbytes_key) {}

  bool lookup(const ByteContainer & key_data,
      internal_handle_t * handle) override {
    int min_priority = std::numeric_limits<int>::max();;
    bool match;

    const TernaryMatchKey *entry;
    const TernaryMatchKey *min_entry = nullptr;
    entry_handle_t min_handle = 0;

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

  bool entry_exists(const TernaryMatchKey & key) override {
    auto it = find_handle(key);

    return it != this->handles.end();
  }

  void store_entry(const TernaryMatchKey & key,
      internal_handle_t handle) override {
    // TODO(gordon) avoid copying key ?
    handles.emplace_back(std::make_tuple(key, handle));
  }

  void delete_entry(const TernaryMatchKey & key) override {
    auto it = find_handle(key);
    this->handles.erase(it);
  }

  void clear() override {
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

}  // anonymous namespace

void LookupStructureFactory::create(
       std::unique_ptr<LookupStructure<ExactMatchKey>> * ls,
       size_t size, size_t nbytes_key) {
  (void) nbytes_key;
  ls->reset(new ExactMap(size));
}

void LookupStructureFactory::create(
       std::unique_ptr<LookupStructure<LPMMatchKey>> * ls,
       size_t size, size_t nbytes_key) {
  (void)size;
  ls->reset(new LPMTrie(nbytes_key));
}

void LookupStructureFactory::create(
       std::unique_ptr<LookupStructure<TernaryMatchKey>> * ls,
       size_t size, size_t nbytes_key) {
  (void) size;
  ls->reset(new TernaryMap(nbytes_key));
}

}  // namespace bm
