# System of Systems Engineering — mini-system-of-systems-engineering

System of Systems Engineering (SoSE) addresses the integration of multiple independent
constituent systems into a larger meta-system that delivers emergent capabilities beyond
any individual system. Ref: Maier (1998), Boardman & Sauser (2006).

## Build & Test

```sh
make          # build static library libsos_engineering.a
make test     # compile and run test suite (30+ assert checks)
make examples # compile all 3 example programs
make demo     # compile and run all examples
make clean    # remove build artifacts
```

## File Structure

```
mini-system-of-systems-engineering/
  Makefile                      # C11 build system
  include/ (6 headers)
    gst_core.h                  # Core types: ConstituentSystem, SystemSet
    sos_types.h                 # SoS classification, characteristics, lifecycle
    sos_architecture.h          # Architecture types, connections, interoperability
    sos_emergence.h             # Emergent behavior detection and analysis
    sos_governance.h            # Governance models, stakeholders, policies
    sos_assessment.h            # Readiness, maturity, resilience assessment
  src/ (6 .c + 1 .lean)
    gst_core.c                  # Core type implementations
    sos_types.c                 # SoS type system, characteristics computation
    sos_architecture.c          # Floyd-Warshall metrics, centrality, robustness
    sos_emergence.c             # Emergence assessment, pathological risk
    sos_governance.c            # Stakeholder satisfaction, governance fitness
    sos_assessment.c            # Resilience scenarios, effectiveness formula
    sos_engineering.lean        # Lean 4 formal verification
  tests/
    test_sos.c                  # 30+ assert checks covering all APIs
  examples/
    example1_sos_types.c        # SoS type classification demo
    example2_architecture.c     # Network topology analysis demo
    example3_emergence.c        # Emergent behavior detection demo
  docs/
    sos-theory.md               # Theory overview (Maier, Boardman & Sauser)
    sos-assessment-guide.md     # Assessment methodology guide
```

## Maier's SoS Types

| Type | Control | Example |
|------|---------|---------|
| Directed | Centralized authority | Integrated air defense |
| Acknowledged | Central coordination | Joint force operations |
| Collaborative | Voluntary agreements | Internet (IETF) |
| Virtual | No central authority | Global supply chains |

## Five SoS Characteristics

| Characteristic | Description | API |
|---------------|-------------|-----|
| Autonomy | Independence of constituent systems | `sos->chars.autonomy` |
| Belonging | Commitment to SoS goals | `sos->chars.belonging` |
| Connectivity | Inter-system links and data flow | `sos->chars.connectivity` |
| Diversity | Heterogeneity of systems | `sos->chars.diversity` |
| Emergence | Capability beyond sum of parts | `sos->chars.emergence` |

## API Reference

### Core Types (gst_core.h)
| Function | Description |
|----------|-------------|
| `gst_system_create/free()` | Create/free constituent system |
| `gst_system_add_interface()` | Add communication interface |
| `gst_system_add_capability()` | Add system capability |
| `gst_set_create/free()` | Create/free system set |
| `gst_set_add()` | Add system to set |

### SoS Types (sos_types.h)
| Function | Description |
|----------|-------------|
| `sos_create/free()` | Create/free System of Systems |
| `sos_add_constituent()` | Add constituent system |
| `sos_compute_characteristics()` | Compute 5 SoS characteristics |
| `sos_readiness()` | Operational readiness score |
| `sos_is_viable()` | Viability check |
| `sos_type_name()` | SoS type string |

### Architecture (sos_architecture.h)
| Function | Description |
|----------|-------------|
| `sos_arch_create/free()` | Create/free SoS architecture |
| `sos_arch_connect()` | Add inter-system connection |
| `sos_arch_compute_metrics()` | Floyd-Warshall: density, path, centrality |
| `sos_arch_interoperability_score()` | Average interoperability level |
| `sos_arch_robustness()` | Network robustness score |
| `sos_arch_critical_nodes()` | Identify critical (high-centrality) nodes |

### Emergence (sos_emergence.h)
| Function | Description |
|----------|-------------|
| `sos_emerge_create()` | Create emergent behavior entry |
| `sos_emerge_profile_create()` | Create emergence profile |
| `sos_emerge_profile_analyze()` | Assess all emergent behaviors |
| `sos_emerge_pathological_risk()` | Compute risk of harmful emergence |
| `sos_detect_novel_emergence()` | Detect new emergent behavior |

### Governance (sos_governance.h)
| Function | Description |
|----------|-------------|
| `sos_gov_create/free()` | Create/free governance structure |
| `sos_gov_add_policy()` | Add governance policy |
| `sos_gov_add_stakeholder()` | Register stakeholder |
| `sos_gov_governance_fitness()` | Match governance model to SoS type |
| `sos_gov_assess()` | Assess control, compliance, agility |

### Assessment (sos_assessment.h)
| Function | Description |
|----------|-------------|
| `sos_assess_create/free()` | Create/free assessment |
| `sos_assess_readiness()` | Compute readiness score |
| `sos_assess_effectiveness()` | Mission effectiveness |
| `sos_assess_resilience()` | Disruption resilience |
| `sos_assess_compute_overall()` | Weighted overall score |
| `sos_resilience_analysis_create()` | Multi-scenario resilience analysis |

## Formal Verification (Lean 4)

`src/sos_engineering.lean` verifies:
- **Theorem**: Emergence scale factor bounded in [0,1]
- **Theorem**: Enterprise interoperability is maximum
- **Theorem**: Perfect governance fitness = 1.0
- **Theorem**: Resilience index bounded in [0,1]
- **Structure**: SoSType, SoSCharacteristics, InteroperabilityLevel

## Reference
- Maier, M.W. (1998). Architecting principles for systems-of-systems. *Systems Engineering*.
- Boardman, J. & Sauser, B. (2006). System of Systems - the meaning of "of". *IEEE SMC*.
- INCOSE. *Systems Engineering Handbook*, 4th Edition.
- ISO/IEC/IEEE 21839:2019. *Systems of systems considerations in engineering*.


## Module Status: COMPLETE

| Level | Status |
|-------|--------|
| L1 Definitions | Complete: SoSType, SoSArchitecture, EmergenceType |
| L2 Core Concepts | Complete: Directed/Acknowledged/Collaborative/Virtual SoS |
| L3 Math Structures | Complete: Connectivity matrix, interoperability index |
| L4 Fundamental Laws | Complete: Emergence principle, SoS governance |
| L5 Algorithms | Complete: Architecture analysis, resilience computation |
| L6 Canonical Problems | Complete: Smart grid, ATM, C2 constellation |
| L7 Applications | Complete (4): SESAR, C2, AV platooning, Smart grid |
| L8 Advanced Topics | Complete (1): SoS resilience, reconfiguration |
| L9 Research Frontiers | Partial: SoS of SoS governance |

### Course Alignment: MIT 6.241J, Stanford AA274, CMU 24-654, Caltech CDS140, ETH 227-0220