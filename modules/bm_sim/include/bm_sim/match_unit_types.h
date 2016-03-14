#ifndef BM_SIM_INCLUDE_BM_SIM_MATCH_UNIT_TYPES_H_
#define BM_SIM_INCLUDE_BM_SIM_MATCH_UNIT_TYPES_H_

#include "bytecontainer.h"

namespace bm {

typedef uintptr_t internal_handle_t;
typedef uint64_t entry_handle_t;

enum class MatchUnitType {
  EXACT, LPM, TERNARY
};

// Entry types.
struct MatchKey {
  inline MatchKey() {}
  inline MatchKey(ByteContainer data, uint32_t version)
    : data(std::move(data)), version(version) {}

  ByteContainer data{};
  uint32_t version{0};
};

struct ExactMatchKey : public MatchKey {
  using MatchKey::MatchKey;

  static constexpr MatchUnitType mut = MatchUnitType::EXACT;
};

struct LPMMatchKey : public MatchKey {
  inline LPMMatchKey() {}
  inline LPMMatchKey(ByteContainer data, int prefix_length, uint32_t version)
    : MatchKey(data, version), prefix_length(prefix_length) {}

  int prefix_length{0};

  static constexpr MatchUnitType mut = MatchUnitType::LPM;
};

struct TernaryMatchKey : public MatchKey {
  inline TernaryMatchKey() {}
  inline TernaryMatchKey(ByteContainer data, ByteContainer mask, int priority,
      uint32_t version)
    : MatchKey(data, version), mask(std::move(mask)),
    priority(priority) {}

  ByteContainer mask{};
  int priority{0};

  static constexpr MatchUnitType mut = MatchUnitType::TERNARY;
};

} // namespace bm

#endif
