#ifndef HSM_MORPHOLOGY_H
#define HSM_MORPHOLOGY_H
#include "hsm_core.h"

/* ============================================================================
 * 3D Morphology Matrix — The Core of Hall's Methodology
 *
 * The morphology matrix is a 7x7xN grid where each cell (phase, step, disc)
 * contains the set of activities to be performed. This provides:
 *   - Complete coverage: every phase/step pairing is addressed
 *   - Traceability: activities linked to knowledge disciplines
 *   - Progress tracking: percent complete per cell, phase, step
 * ============================================================================ */

typedef struct {
    double completion;          /* 0.0 - 1.0 */
    int n_activities;
    int n_completed;
    double effort_planned;
    double effort_actual;
    double risk_average;
} HSMCellMetrics;

typedef struct {
    HSMCellMetrics metrics[HSM_NUM_PHASES][HSM_NUM_STEPS];
    double overall_completion;
    double total_effort_planned;
    double total_effort_actual;
    double schedule_performance_index;
    double cost_performance_index;
} HSMMorphologyMetrics;

/* Traceability path through the morphology */
typedef struct {
    HSMPhase* phases;
    HSMLogicStep* steps;
    int length;
    char* description;
} HSMTraceabilityPath;

/* API */
HSMMorphologyMetrics hsm_morphology_compute(HSMSystem* sys);
void hsm_morphology_print_metrics(HSMMorphologyMetrics* mm);
int hsm_morphology_critical_cells(HSMSystem* sys,
                                    int* phase_out, int* step_out,
                                    int max_cells);
double hsm_morphology_phase_step_coverage(HSMSystem* sys,
                                            HSMPhase phase,
                                            HSMLogicStep step);
int hsm_morphology_count_activities(HSMSystem* sys);
int hsm_morphology_count_completed(HSMSystem* sys);
double hsm_morphology_effort_variance(HSMSystem* sys);

/* Traceability */
HSMTraceabilityPath* hsm_trace_requirement(HSMSystem* sys,
                                              const char* req_id);
void hsm_traceability_free(HSMTraceabilityPath* tp);
void hsm_traceability_print(HSMTraceabilityPath* tp);

/* Cross-phase analysis */
double hsm_phase_coupling(HSMSystem* sys, HSMPhase p1, HSMPhase p2);
bool hsm_is_phase_blocked(HSMSystem* sys, HSMPhase phase);
int hsm_next_recommended_steps(HSMSystem* sys, HSMLogicStep* steps, int max);
#endif
