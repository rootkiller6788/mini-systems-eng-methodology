# API Reference - Soft Systems Methodology

## Architecture
```
include/  ssm_types.h       Core types, 8 enums, 12 structs
          ssm_core.h         Convenience header + 30+ constants
          ssm_catwoe.h       CATWOE + Weltanschauung operations
          ssm_rootdef.h      Root Definition + PQR/5E + multi-RD
          ssm_conceptual.h   Conceptual Model + activity graph
          ssm_comparison.h   Comparison + SSM Cycle + accommodation

src/      ssm_core.c         CATWOE lifecycle + Weltanschauung (30+ functions)
          ssm_rootdef.c      Root definitions, PQR, 5E analysis
          ssm_conceptual.c   Conceptual model, connectivity, topological sort
          ssm_comparison.c   Problem situations, gap analysis, SSM cycle
          soft_systems.lean  Formal theorems (worldview, CATWOE, 3E)
```

## Export Functions (50+)
- CATWOE: create/free/clone, 8 setters, 4 adders, 3 analysis
- Weltanschauung: create/free, add assumption/implication, conflict/potential
- Root Definition: create/free, PQR set/synthesize/parse, 5E, viability, ranking
- Conceptual Model: create/free, add activity/dependency, connectivity, sort
- Situation: create/free, add stakeholder/issue/worldview, complexity/conflict
- Comparison: create/free, add gap/recommendation, alignment, agenda
- SSM Cycle: create/free, advance, add learning/RD/model/comparison

## Key Constants (30+)
```c
SSM_ACCOMMODATION_THRESHOLD 0.70
SSM_EFFICACY_MIN 0.50, SSM_EFFICIENCY_MIN 0.40
SSM_EFFECTIVENESS_MIN 0.60, SSM_ETHICALITY_MIN 0.70
SSM_GAP_SIGNIFICANT 0.40, SSM_GAP_CRITICAL 0.70
SSM_MAX_ITERATIONS 10, SSM_MAX_CYCLE_DEPTH 5
```

## Lean 4 Formalization
`src/soft_systems.lean` provides:
- `Weltanschauung` structure with conflict/accommodation
- `accommodation_symmetric` theorem
- `RootDefinition` with well-formulated condition
- `ThreeE` viability theorem
- `ConceptualModel` with acyclicity property
- `SSMStage` inductive type (7 stages)
- `InquiringSystem` (Churchman's classification)
