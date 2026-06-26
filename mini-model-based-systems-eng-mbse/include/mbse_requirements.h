#ifndef MBSE_REQUIREMENTS_H
#define MBSE_REQUIREMENTS_H
#include "mbse_core.h"
/* ===================================================================
 * Requirements engineering in MBSE:
 *   - Traceability: req-to-block mapping matrix
 *   - Gap analysis: detection of orphan requirements
 *   - Type classification: functional, performance, safety, etc.
 *   - Priority scoring: critical (1) through nice-to-have (5)
 *
 * A requirement is a statement that identifies a product or
 * process operational, functional, or design characteristic
 * or constraint, which is unambiguous, testable, and
 * necessary for product or process acceptability.
 * (ISO/IEC/IEEE 29148:2018)
 *
 * Traceability coverage = #allocated_reqs / total_reqs
 * Satisfaction rate = #verified_reqs / total_reqs
 * =================================================================== */

/*
 * Requirements engineering covers:
 *   - Traceability matrix construction (req-to-block mapping)
 *   - Gap analysis for orphan/uncovered requirements
 *   - Requirement type counting and priority scoring
 *   - Export to CSV for reporting tools
 *
 * INCOSE defines requirements as formal statements of
 * stakeholder needs that the system must satisfy.
 */
typedef struct { int** matrix; int n_rows, n_cols; double coverage; int n_covered, n_total, n_satisfied, n_unsatisfied; } TraceabilityMatrix;

typedef struct { MBSERequirement** orphans; int n_orphans; double completeness; } RequirementGapAnalysis;

TraceabilityMatrix*   mbse_trace_create(SystemModel* m);
void                  mbse_trace_free(TraceabilityMatrix* tm);
double                mbse_trace_coverage(TraceabilityMatrix* tm);
void                  mbse_trace_print(TraceabilityMatrix* tm);
RequirementGapAnalysis* mbse_req_gap_analysis(SystemModel* m);
void                    mbse_req_gap_free(RequirementGapAnalysis* ga);
int   mbse_req_count_by_type(SystemModel* m, RequirementType t);
double mbse_req_priority_score(SystemModel* m);
void   mbse_req_list_print(SystemModel* m);
#endif
