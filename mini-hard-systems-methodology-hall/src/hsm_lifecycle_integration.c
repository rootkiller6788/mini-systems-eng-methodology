#include "../include/hsm_core.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Lifecycle Integration — Hall's Phase 7: Planning for Action
 *
 * Integrates verification, validation, and deployment planning
 * across the full system lifecycle per Hall (1969).
 * Computes readiness levels and transition criteria between phases. */

typedef struct {
    int     phase;               /* 1-7 per Hall's methodology */
    double  completion_pct;      /* 0-100 */
    int     n_deliverables;
    int     n_completed;
    int     is_gate_passed;
    double  risk_level;          /* residual risk at this phase */
} LifecyclePhase;

typedef struct {
    LifecyclePhase* phases;
    int             n_phases;
    double          overall_completion;
    double          cumulative_risk;
    int             current_phase;
} LifecyclePlan;

LifecyclePlan* lp_create(int n_phases) {
    LifecyclePlan* lp = calloc(1, sizeof(LifecyclePlan));
    if (!lp) return NULL;
    lp->n_phases = n_phases > 0 ? n_phases : 7;
    lp->phases = calloc(lp->n_phases, sizeof(LifecyclePhase));
    lp->current_phase = 0;
    return lp;
}

void lp_free(LifecyclePlan* lp) {
    if (!lp) return;
    free(lp->phases);
    free(lp);
}

int lp_set_phase(LifecyclePlan* lp, int idx, int deliverables, double risk) {
    if (!lp || idx < 0 || idx >= lp->n_phases) return -1;
    lp->phases[idx].phase = idx + 1;
    lp->phases[idx].n_deliverables = deliverables;
    lp->phases[idx].risk_level = risk;
    return 0;
}

int lp_advance_phase(LifecyclePlan* lp) {
    if (!lp || lp->current_phase >= lp->n_phases) return -1;
    lp->phases[lp->current_phase].is_gate_passed = 1;
    lp->current_phase++;
    return lp->current_phase;
}

double lp_overall_completion(LifecyclePlan* lp) {
    if (!lp || lp->n_phases == 0) return 0.0;
    double total = 0.0;
    for (int i = 0; i < lp->n_phases; i++)
        total += lp->phases[i].completion_pct;
    lp->overall_completion = total / lp->n_phases;
    return lp->overall_completion;
}
