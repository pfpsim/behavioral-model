#ifndef BM_SIM_INCLUDE_BM_SIM_MATCH_UNIT_TYPES_H_
#define BM_SIM_INCLUDE_BM_SIM_MATCH_UNIT_TYPES_H_

namespace bm {

typedef uintptr_t internal_handle_t;
typedef uint64_t entry_handle_t;

enum class MatchUnitType {
  EXACT, LPM, TERNARY
};

} // namespace bm

#endif
