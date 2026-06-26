# Course Alignment: MBSE

## MIT 16.842 ¡ª Fundamentals of Systems Engineering

| Topic | Implementation |
|---|---|
| System architecture | mbse_arch_hierarchy_metrics, mbse_arch_modularity |
| Requirements engineering | mbse_req_create, mbse_regap_analysis |
| Trade studies | mbse_trade_create, mbse_trade_compute |
| V&V planning | mbse_vv_plan_create, mbse_vv_generate_report |

## Stanford AA290 ¡ª Systems Engineering

| Topic | Implementation |
|---|---|
| MBSE methodology | SystemModel, full lifecycle |
| SysML modeling | Block, Requirement, Interface types |
| Parametric analysis | mbse_param_model_evaluate_moe |
| Behavioral modeling | StateMachine, ActivityDiagram |

## Caltech CDS 203 ¡ª Model-Based SE

| Topic | Implementation |
|---|---|
| Formal verification | mbse_model.lean (Lean 4 theorems) |
| Traceability matrices | mbse_model_build_traceability |
| Allocation optimization | mbse_arch_allocate_function |
| Consistency checking | mbse_vv_generate_report |

## Learning Objectives
1. Construct system models with blocks, requirements, interfaces
2. Build and analyze traceability matrices
3. Detect orphan requirements via gap analysis
4. Model system behavior with state machines
5. Conduct trade studies with weighted criteria
6. Generate verification plans and reports
7. Understand formal consistency theorems
