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

} // namespace bm


#endif
