#ifndef HSM_TIME_H
#define HSM_TIME_H
#include "hsm_core.h"

/* ============================================================================
 * Time Dimension — Lifecycle Phase Management
 *
 * Hall's 7-phase lifecycle (time dimension of the 3D morphology):
 *   1. Program Planning — needs analysis, feasibility, policy
 *   2. Project Planning — preliminary design, resource allocation
 *   3. System Development — detailed design, prototyping, testing
 *   4. Production — manufacturing, construction, deployment preparation
 *   5. Distribution — installation, commissioning, training
 *   6. Operations — operation, maintenance, performance monitoring
 *   7. Retirement — decommissioning, disposal, lessons learned
 *
 * Each phase has: entry/exit criteria, deliverables, reviews, metrics.
 * ============================================================================ */

typedef struct {
    HSMPhase phase;
    double start_date;         /* Planned start (days from project start) */
    double end_date;           /* Planned end */
    double actual_start;
    double actual_end;
    double budget_allocated;
    double budget_spent;
    char* entry_criteria;
    char* exit_criteria;
    char* key_deliverables;
    bool gate_passed;          /* Phase gate review passed */
    double risk_level;
} HSMPhasePlan;

/* Phase transition network */
typedef struct {
    HSMPhasePlan phases[HSM_NUM_PHASES];
    double project_start_date;
    double project_end_date;
    double total_budget;
    double contingency;        /* Budget contingency percentage */
} HSMLifecycle;

/* Phase dependency */
typedef struct {
    int n_dependencies;
    HSMPhase* predecessors;    /* Phases that must complete first */
    HSMPhase* successors;      /* Phases that follow */
} HSMPhaseDependency;

/* Phase metrics */
typedef struct {
    double schedule_variance;
    double cost_variance;
    double completion_percent;
    int n_activities;
    int n_completed;
    int n_deliverables_met;
    int n_deliverables_total;
} HSMPhaseMetrics;

/* API */
HSMLifecycle* hsm_lifecycle_create(void);
void hsm_lifecycle_free(HSMLifecycle* lc);
void hsm_set_phase_plan(HSMLifecycle* lc, HSMPhase phase,
                         double start, double end, double budget,
                         const char* entry, const char* exit,
                         const char* deliverables);
void hsm_phase_gate_pass(HSMLifecycle* lc, HSMPhase phase);
double hsm_lifecycle_budget_burned(HSMLifecycle* lc);
double hsm_lifecycle_schedule_slippage(HSMLifecycle* lc);
HSMPhaseMetrics hsm_phase_metrics(HSMLifecycle* lc, HSMPhase phase);
int hsm_current_phase(HSMLifecycle* lc, double current_time);
bool hsm_is_phase_active(HSMLifecycle* lc, HSMPhase phase, double current_time);
void hsm_print_lifecycle(HSMLifecycle* lc);
void hsm_print_phase_gantt(HSMLifecycle* lc);
#endif
