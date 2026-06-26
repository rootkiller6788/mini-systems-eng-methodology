# mini-verification-validation-uncertainty

## Verification, Validation & Uncertainty for Complex Systems

This module implements the V&V-U trinity for systems engineering:
- **Verification**: "Did we build the system right?" ? conformance to specifications
- **Validation**: "Did we build the right system?" ? stakeholder needs alignment
- **Uncertainty**: "How well do we know what we know?" ? quantification and propagation

### Key Standards
IEEE 1012, ISO 9001, NASA-STD-7009, ASME V&V 10/20

## Architecture

```
mini-verification-validation-uncertainty/
  include/
    vvu_core.h          - Core types (requirement, test, assessment, uncertainty)
    vvu_metrics.h       - Coverage, quality, confidence, progress metrics
    vvu_monte_carlo.h   - Monte Carlo, LHS, statistics, sensitivity
  src/
    vvu_core.c          - Project lifecycle, requirement/test/assessment mgmt
    vvu_metrics.c       - Metrics computation, gap analysis, confidence intervals
    vvu_monte_carlo.c   - Sampling, MC execution, LHS, statistics, convergence
    vvu_formal.lean     - Lean 4 formal verification of V&V concepts
  tests/
    test_vvu.c          - 17 tests, 40+ asserts
  examples/
    example1_traceability.c   - Requirements traceability matrix
    example2_verification.c   - Verification test suite
    example3_monte_carlo.c    - Uncertainty propagation
  docs/
    vv-theory.md              - Theory & practice reference
    course-alignment.md        - Alignment with course curricula
```

## Quick Start

```bash
make          # Build static library libvvu.a
make test     # Build and run tests (17 tests)
make examples # Build and run examples
make clean    # Remove build artifacts
```

Requirements: GCC (C11), Make, ar. No external dependencies beyond libc/libm.

## API Overview

### Core (`vvu_core.h`)
| Function | Description |
|----------|-------------|
| `vvu_project_create(name, system)` | Create V&V project |
| `vvu_req_add(proj, id, desc, pri)` | Add requirement |
| `vvu_verif_add(proj, id, desc, req, thresh, tol)` | Add verification test |
| `vvu_valid_add(proj, stakeholder, concern, method)` | Add validation assessment |
| `vvu_uncert_add(proj, name, type, dist, nominal, p1, p2)` | Add uncertainty quantity |
| `vvu_verif_run(proj, idx, measured)` | Run verification test |
| `vvu_valid_score(proj, idx, align, conf)` | Score validation assessment |
| `vvu_traceability_matrix(proj)` | Print requirement traceability |

### Metrics (`vvu_metrics.h`)
| Function | Description |
|----------|-------------|
| `vvu_compute_coverage_metrics(proj)` | Requirement/test/stakeholder coverage |
| `vvu_compute_quality_metrics(proj)` | Pass rate, defect density, rework |
| `vvu_compute_confidence_metrics(proj, n)` | 95% CI, composite confidence |
| `vvu_all_critical_verified(proj)` | Check all mandatory/critical reqs |
| `vvu_gap_analysis_run(proj, ga)` | Identify V&V gaps |

### Monte Carlo (`vvu_monte_carlo.h`)
| Function | Description |
|----------|-------------|
| `vvu_mc_generate_samples(uncerts, n, N, seed)` | Simple random sampling |
| `vvu_mc_lhs_samples(uncerts, n, N, seed)` | Latin hypercube sampling |
| `vvu_mc_run(uncerts, n, model, ctx, cfg)` | Full Monte Carlo execution |
| `vvu_mc_compute_statistics(outputs, nout, N)` | Compute mean, variance, percentiles |
| `vvu_mc_confidence_interval_width(samples, n, cl)` | CI width at confidence level |
| `vvu_mc_required_samples(precision, var)` | Required N for precision |

## Key Constants

| Constant | Value | Meaning |
|----------|-------|---------|
| `VVU_REQ_MANDATORY` | 0 | Must satisfy for system acceptance |
| `VVU_REQ_CRITICAL` | 1 | Safety/mission critical |
| `VVU_PASSED` | 2 | Test/req passed |
| `VVU_FAILED` | 3 | Test/req failed |
| `VVU_DIST_NORMAL` | 1 | Gaussian distribution |
| `VVU_DIST_UNIFORM` | 0 | Uniform distribution |

## Lean 4 Formalization

`src/vvu_formal.lean` proves:
- `verification_implies_predicate_true`: Verified req has satisfied predicate
- `coverage_bounds`: Coverage ratio always in [0,1]
- `linear_combination_mean`: Mean of linear combination equals combination of means
- `complete_plan_all_pass`: Complete plan + all tests pass = all reqs satisfied

Key theorems: `verification_implies_predicate_true`, `coverage_bounds`

## System Archetypes

1. **Requirements Traceability**: Every requirement linked to tests and stakeholders
2. **Verification Test Suite**: Automated pass/fail with tolerance thresholds
3. **Monte Carlo Propagation**: Uncertainty from inputs to outputs
4. **Gap Analysis**: Identify missing tests, untraced requirements
5. **Confidence Assessment**: Statistical confidence in V&V results

## References

- IEEE 1012-2016: System/Software Verification and Validation
- NASA-STD-7009: Standard for Models and Simulations
- Oberkampf & Roy (2010). Verification and Validation in Scientific Computing
- Saltelli et al. (2008). Global Sensitivity Analysis: The Primer
- DoD VV&A Recommended Practices Guide
- ISO 9001: Quality Management Systems


## Module Status: COMPLETE

| Level | Status |
|-------|--------|
| L1 Definitions | Complete: VVRequirement, TraceLink, UncertaintyBound |
| L2 Core Concepts | Complete: Verification vs validation, traceability, UQ |
| L3 Math Structures | Complete: Monte Carlo sampling, sensitivity indices |
| L4 Fundamental Laws | Complete: V&V independence, uncertainty propagation |
| L5 Algorithms | Complete: MC simulation, sensitivity, Sobol indices |
| L6 Canonical Problems | Complete: Traceability matrix, MC verification, risk |
| L7 Applications | Complete (4): Mars Orbiter, Fukushima, 737 MAX, ISO 26262 |
| L8 Advanced Topics | Partial: Bayesian model calibration |
| L9 Research Frontiers | Partial: AI V&V, formal verification of neural nets |

### Course Alignment: MIT 6.241J, Stanford EE363, Princeton ELE 530, CMU 24-654, Caltech CDS140