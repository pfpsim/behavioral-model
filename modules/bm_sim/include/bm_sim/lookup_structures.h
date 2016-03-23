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

#ifndef BM_SIM_INCLUDE_BM_SIM_LOOKUP_STRUCTURES_H_
#define BM_SIM_INCLUDE_BM_SIM_LOOKUP_STRUCTURES_H_

#include "match_unit_types.h"
#include "bytecontainer.h"

namespace bm {

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

template <class K>
class LookupStructureFactoryPart {
 public:
  virtual void create(std::unique_ptr<LookupStructure<K>> * ptr,
                      size_t size, size_t nbytes_key) = 0;
};

class LookupStructureFactory
  // Inherits the virtual method to create each type of lookup structure
  // from each of these base classes
  : public LookupStructureFactoryPart<ExactMatchKey>
  , public LookupStructureFactoryPart<LPMMatchKey>
  , public LookupStructureFactoryPart<TernaryMatchKey> {
 public:
  void create(std::unique_ptr< LookupStructure<ExactMatchKey>> *,
     size_t size, size_t nbytes_key) override;
  void create(std::unique_ptr< LookupStructure<LPMMatchKey>> *,
     size_t size, size_t nbytes_key) override;
  void create(std::unique_ptr< LookupStructure<TernaryMatchKey>> *,
     size_t size, size_t nbytes_key) override;
};


}  // namespace bm


#endif  // BM_SIM_INCLUDE_BM_SIM_LOOKUP_STRUCTURES_H_
