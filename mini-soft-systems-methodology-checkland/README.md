# Mini Soft Systems Methodology (Checkland)

> "SSM is a learning system, not an optimizing system."
> -- Peter Checkland, 1981

A C library implementing Checkland's Soft Systems Methodology with Lean 4 verification.

## Module Structure
```
include/  6 headers (ssm_types, catwoe, rootdef, conceptual, comparison, core)
src/      5 C files + 1 Lean file (full SSM lifecycle)
tests/    test_ssm.c (30+ asserts)
examples/ 4 demos (CATWOE, PQR/5E, conceptual model, comparison)
docs/     2 docs (SSM theory + API reference)
```

## Quick Start
```bash
make          # Build static library
make test     # Run test suite
make examples # Build all demos
```

## Core Concepts
- **Weltanschauung**: Different stakeholders, different worldviews
- **CATWOE**: Customers, Actors, Transformation, Worldview, Owners, Environment
- **Root Definition**: "A system to do P by Q in order to achieve R"
- **5E Analysis**: Efficacy, Efficiency, Effectiveness, Ethicality, Elegance
- **Conceptual Model**: NOT reality -- what MUST be done to BE the system
- **7-Stage Cycle**: Situation -> Expression -> RDs -> Models -> Compare -> Changes -> Action
- **Accommodation**: Coexistence of conflicting worldviews (not consensus)

## Key API (50+ functions)
```c
CATWOE* c = ssm_catwoe_create();
ssm_catwoe_set_transformation(c, "sick", "healthy", SSM_TRANSFORM_CONCRETE);
RootDefinition* rd = ssm_rootdef_create("Healthcare", SSM_RD_PRIMARY_TASK);
ssm_rootdef_set_pqr(rd, "provide care", "nursing", "improve health");
ConceptualModel* cm = ssm_conceptual_create("RD text");
int a = ssm_conceptual_add_activity(cm, "Assess", "...");
SSMCycle* cycle = ssm_cycle_create();
ssm_cycle_advance(cycle);
double accom = ssm_accommodation_index(cycle);
```

## Lean 4 Theorems
- `accommodation_symmetric`: accommodation(w1,w2) = accommodation(w2,w1)
- `well_formulated_requires_all_elements`: RD requires complete CATWOE
- `SSMStage`: inductive 7-stage cycle type
- `InquiringSystem`: Churchman's 5 epistemologies

## CATWOE Analysis

| Component | Question | Example |
|-----------|----------|---------|
| C - Customers | Who benefits/suffers? | Patients, community |
| A - Actors | Who performs T? | Nurses, doctors, admin |
| T - Transformation | What input becomes what output? | Sick -> Healthy |
| W - Weltanschauung | What worldview makes T meaningful? | Healthcare is a right |
| O - Owners | Who can stop T? | Hospital Board, Government |
| E - Environment | What constraints are given? | Budget, law, staffing |

## 5E Evaluation

| Criterion | Measures | Threshold |
|-----------|----------|-----------|
| E1 Efficacy | Does the output happen? | >= 0.50 |
| E2 Efficiency | Is resource use minimal? | >= 0.40 |
| E3 Effectiveness | Does it achieve higher purpose? | >= 0.60 |
| E4 Ethicality | Is it morally acceptable? | >= 0.70 |
| E5 Elegance | Is it aesthetically pleasing? | >= 0.30 |

## Weltanschauung Types

| Type | Description |
|------|-------------|
| SSM_W_EXPLICIT | Formally articulated worldview |
| SSM_W_IMPLICIT | Unstated assumptions |
| SSM_W_DOMINANT | Prevailing organizational W |
| SSM_W_ALTERNATIVE | Marginalized or competing W |
| SSM_W_EMERGENT | New W arising from inquiry |

## 7-Stage Learning Cycle

| Stage | Name | Key Activity |
|-------|------|-------------|
| 1 | Problem situation | Identify the messy situation |
| 2 | Expression | Rich picture, stakeholders, issues |
| 3 | Root definitions | CATWOE, PQR formulation |
| 4 | Conceptual models | Minimum necessary activities |
| 5 | Comparison | Model vs. reality gap analysis |
| 6 | Changes | Desirable and feasible changes |
| 7 | Action | Implement to improve |

## Comparision Methods

| Method | Description |
|--------|-------------|
| SSM_COMPARE_INFORMAL | Open discussion |
| SSM_COMPARE_QUESTIONS | Formal question-answering |
| SSM_COMPARE_MATRIX | Scenario-based comparison |
| SSM_COMPARE_SCRIPT | Dynamic operational gaming |

## Mathematical Constants (30+)
```c
SSM_PI, SSM_E, SSM_EPSILON          // Math
SSM_ACCOMMODATION_THRESHOLD  0.70   // Key threshold
SSM_EFFICACY_MIN  0.50              // Viability minima
SSM_GAP_CRITICAL  0.70              // Gap severity
SSM_MAX_ITERATIONS  10              // Learning cycle
```

## References
- Checkland, P. (1981). *Systems Thinking, Systems Practice*. Wiley.
- Checkland, P. & Scholes, J. (1990). *Soft Systems Methodology in Action*. Wiley.
- Checkland, P. & Poulter, J. (2006). *Learning for Action*. Wiley.
- Churchman, C.W. (1971). *The Design of Inquiring Systems*. Basic Books.
- Ulrich, W. (1983). *Critical Heuristics of Social Planning*. Haupt.
- Flood, R.L. & Jackson, M.C. (1991). *Creative Problem Solving*. Wiley.
- Mingers, J. & Rosenhead, J. (2004). *Rational Analysis for a Problematic World*. Wiley.


## Module Status: COMPLETE

| Level | Status |
|-------|--------|
| L1 Definitions | Complete: CATWOE, RootDefinition, ConceptualModel |
| L2 Core Concepts | Complete: Rich pictures, worldview, SSM 7-stage |
| L3 Math Structures | Complete: Activity graph, dependency topology |
| L4 Fundamental Laws | Complete: W-led vs C-led cycle, boundary inclusion |
| L5 Algorithms | Complete: Topological sort, coverage, comparison |
| L6 Canonical Problems | Complete: Library system, NHS waiting times |
| L7 Applications | Complete (3): UK Prison, NHS, community policing |
| L8 Advanced Topics | Complete (1): Multi-worldview negotiation |
| L9 Research Frontiers | Partial: Digital SSM, AI-assisted rich pictures |

### Course Alignment: Cambridge 4F3, CMU 24-654, Oxford C20, Princeton MAE 546