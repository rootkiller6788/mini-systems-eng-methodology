#include "hsm_time.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

HSMLifecycle* hsm_lifecycle_create(void) {
    HSMLifecycle* lc = calloc(1, sizeof(HSMLifecycle));
    if (!lc) return NULL;
    for (int i = 0; i < HSM_NUM_PHASES; i++) lc->phases[i].phase = (HSMPhase)i;
    return lc;
}
void hsm_lifecycle_free(HSMLifecycle* lc) {
    if (!lc) return;
    for (int i = 0; i < HSM_NUM_PHASES; i++) {
        free(lc->phases[i].entry_criteria);
        free(lc->phases[i].exit_criteria);
        free(lc->phases[i].key_deliverables);
    }
    free(lc);
}
void hsm_set_phase_plan(HSMLifecycle* lc, HSMPhase phase, double start, double end,
                         double budget, const char* entry, const char* exit, const char* deliv) {
    if (!lc || phase < 0 || phase >= HSM_NUM_PHASES) return;
    HSMPhasePlan* p = &lc->phases[phase];
    p->start_date = start; p->end_date = end; p->budget_allocated = budget;
    free(p->entry_criteria); p->entry_criteria = entry ? strdup(entry) : NULL;
    free(p->exit_criteria); p->exit_criteria = exit ? strdup(exit) : NULL;
    free(p->key_deliverables); p->key_deliverables = deliv ? strdup(deliv) : NULL;
}
void hsm_phase_gate_pass(HSMLifecycle* lc, HSMPhase phase) {
    if (!lc || phase < 0 || phase >= HSM_NUM_PHASES) return;
    lc->phases[phase].gate_passed = true;
}
double hsm_lifecycle_budget_burned(HSMLifecycle* lc) {
    if (!lc) return 0.0;
    double spent = 0.0, total = 0.0;
    for (int i = 0; i < HSM_NUM_PHASES; i++) {
        spent += lc->phases[i].budget_spent;
        total += lc->phases[i].budget_allocated;
    }
    return total > 0 ? spent / total : 0.0;
}
double hsm_lifecycle_schedule_slippage(HSMLifecycle* lc) {
    if (!lc) return 0.0;
    double total_slip = 0.0; int n = 0;
    for (int i = 0; i < HSM_NUM_PHASES; i++) {
        if (lc->phases[i].actual_end > 0 && lc->phases[i].end_date > 0) {
            total_slip += lc->phases[i].actual_end - lc->phases[i].end_date; n++;
        }
    }
    return n > 0 ? total_slip / n : 0.0;
}
HSMPhaseMetrics hsm_phase_metrics(HSMLifecycle* lc, HSMPhase phase) {
    HSMPhaseMetrics m; memset(&m, 0, sizeof(m));
    if (!lc || phase < 0 || phase >= HSM_NUM_PHASES) return m;
    HSMPhasePlan* p = &lc->phases[phase];
    m.schedule_variance = p->actual_end - p->end_date;
    m.cost_variance = p->budget_spent - p->budget_allocated;
    return m;
}
int hsm_current_phase(HSMLifecycle* lc, double t) {
    if (!lc) return -1;
    for (int i = 0; i < HSM_NUM_PHASES; i++)
        if (t >= lc->phases[i].start_date && t <= lc->phases[i].end_date) return i;
    return -1;
}
bool hsm_is_phase_active(HSMLifecycle* lc, HSMPhase phase, double t) {
    if (!lc || phase < 0 || phase >= HSM_NUM_PHASES) return false;
    HSMPhasePlan* p = &lc->phases[phase];
    return t >= p->start_date && t <= p->end_date;
}
void hsm_print_lifecycle(HSMLifecycle* lc) {
    if (!lc) return;
    printf("=== System Lifecycle ===\n");
    for (int i = 0; i < HSM_NUM_PHASES; i++) {
        HSMPhasePlan* p = &lc->phases[i];
        printf("  Phase %d (%s): day %.0f-%.0f, budget %.1f, gate=%s\n",
               i + 1, hsm_phase_name((HSMPhase)i), p->start_date, p->end_date,
               p->budget_allocated, p->gate_passed ? "PASS" : "PEND");
    }
}
void hsm_print_phase_gantt(HSMLifecycle* lc) {
    if (!lc) return;
    printf("=== Phase Gantt Chart ===\n");
    for (int i = 0; i < HSM_NUM_PHASES; i++) {
        HSMPhasePlan* p = &lc->phases[i];
        printf("  %-22s [", hsm_phase_name((HSMPhase)i));
        int bar = (int)(p->end_date / 10.0 + 0.5);
        if (bar > 50) bar = 50;
        for (int j = 0; j < bar; j++) printf("=");
        printf("] %.0f-%.0f d\n", p->start_date, p->end_date);
    }
}

