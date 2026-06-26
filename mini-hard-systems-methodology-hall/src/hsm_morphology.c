#include "hsm_morphology.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

HSMMorphologyMetrics hsm_morphology_compute(HSMSystem* sys) {
    HSMMorphologyMetrics mm; memset(&mm, 0, sizeof(mm));
    if (!sys) return mm;
    int total_acts = 0, completed = 0;
    for (int p = 0; p < HSM_NUM_PHASES; p++) {
        for (int s = 0; s < HSM_NUM_STEPS; s++) {
            int cell = p * HSM_NUM_STEPS + s;
            HSMCellMetrics* cm = &mm.metrics[p][s];
            for (int a = 0; a < sys->matrix_counts[cell]; a++) {
                HSMActivity* act = &sys->matrix[p][a];
                cm->completion += act->progress;
                cm->effort_planned += act->effort_estimate;
                cm->effort_actual += act->effort_actual;
                cm->n_activities++;
                if (act->progress >= 1.0) cm->n_completed++;
                total_acts++; if (act->progress >= 1.0) completed++;
            }
            if (cm->n_activities > 0) cm->completion /= cm->n_activities;
        }
    }
    mm.overall_completion = total_acts > 0 ? (double)completed / total_acts : 0.0;
    for (int p = 0; p < HSM_NUM_PHASES; p++) for (int s = 0; s < HSM_NUM_STEPS; s++) { mm.total_effort_planned += mm.metrics[p][s].effort_planned; mm.total_effort_actual += mm.metrics[p][s].effort_actual; }
    mm.schedule_performance_index = mm.total_effort_planned > 0 ? mm.total_effort_planned / fmax(mm.total_effort_actual, 1.0) : 1.0;
    mm.cost_performance_index = mm.total_effort_planned > 0 ? mm.total_effort_planned / fmax(mm.total_effort_actual, 1.0) : 1.0;
    return mm;
}
void hsm_morphology_print_metrics(HSMMorphologyMetrics* mm) {
    if (!mm) return;
    printf("=== Morphology Metrics ===\n");
    printf("Overall completion: %.1f%%\n", mm->overall_completion * 100.0);
    printf("Effort planned: %.0f, actual: %.0f\n", mm->total_effort_planned, mm->total_effort_actual);
    printf("SPI: %.3f, CPI: %.3f\n", mm->schedule_performance_index, mm->cost_performance_index);
}
int hsm_morphology_critical_cells(HSMSystem* sys, int* phase_out, int* step_out, int max) {
    if (!sys || !phase_out || !step_out || max <= 0) return 0;
    HSMMorphologyMetrics mm = hsm_morphology_compute(sys);
    /* Find cells with lowest completion */
    double lowest[10]; int low_p[10], low_s[10], n_low = 0;
    for (int p = 0; p < HSM_NUM_PHASES; p++)
        for (int s = 0; s < HSM_NUM_STEPS; s++)
            if (mm.metrics[p][s].n_activities > 0 && mm.metrics[p][s].completion < 0.5) {
                if (n_low < max) { low_p[n_low] = p; low_s[n_low] = s; lowest[n_low] = mm.metrics[p][s].completion; n_low++; }
            }
    for (int i = 0; i < n_low && i < max; i++) { phase_out[i] = low_p[i]; step_out[i] = low_s[i]; }
    return n_low < max ? n_low : max;
}
double hsm_morphology_phase_step_coverage(HSMSystem* sys, HSMPhase phase, HSMLogicStep step) {
    if (!sys) return 0.0;
    int cell = phase * HSM_NUM_STEPS + step;
    return sys->matrix_counts[cell] > 0 ? 1.0 : 0.0;
}
int hsm_morphology_count_activities(HSMSystem* sys) { if(!sys)return 0; int n=0;for(int p=0;p<HSM_NUM_PHASES;p++)for(int s=0;s<HSM_NUM_STEPS;s++)n+=sys->matrix_counts[p*HSM_NUM_STEPS+s];return n; }
int hsm_morphology_count_completed(HSMSystem* sys) { if(!sys)return 0; int n=0;for(int p=0;p<HSM_NUM_PHASES;p++)for(int s=0;s<HSM_NUM_STEPS;s++){int cell=p*HSM_NUM_STEPS+s;for(int a=0;a<sys->matrix_counts[cell];a++)if(sys->matrix[p][a].progress>=1.0)n++;}return n; }
double hsm_morphology_effort_variance(HSMSystem* sys) {
    if (!sys) return 0.0;
    double mean = 0.0; int count = 0;
    for (int p = 0; p < HSM_NUM_PHASES; p++)
        for (int s = 0; s < HSM_NUM_STEPS; s++)
            for (int a = 0; a < sys->matrix_counts[p*HSM_NUM_STEPS+s]; a++) {
                mean += sys->matrix[p][a].effort_actual - sys->matrix[p][a].effort_estimate; count++;
            }
    if (count == 0) return 0.0;
    mean /= count;
    double var = 0.0;
    for (int p = 0; p < HSM_NUM_PHASES; p++)
        for (int s = 0; s < HSM_NUM_STEPS; s++)
            for (int a = 0; a < sys->matrix_counts[p*HSM_NUM_STEPS+s]; a++) {
                double d = (sys->matrix[p][a].effort_actual - sys->matrix[p][a].effort_estimate) - mean; var += d * d;
            }
    return var / count;
}
double hsm_phase_coupling(HSMSystem* sys, HSMPhase p1, HSMPhase p2) {
    if (!sys) return 0.0;
    int shared = 0;
    for (int s = 0; s < HSM_NUM_STEPS; s++)
        if (sys->matrix_counts[p1*HSM_NUM_STEPS+s] > 0 && sys->matrix_counts[p2*HSM_NUM_STEPS+s] > 0) shared++;
    return (double)shared / HSM_NUM_STEPS;
}
bool hsm_is_phase_blocked(HSMSystem* sys, HSMPhase phase) {
    if (!sys || phase == 0) return false;
    HSMMorphologyMetrics mm = hsm_morphology_compute(sys);
    return mm.metrics[phase - 1][HSM_STEP_DECISION_MAKING].completion < 1.0;
}
int hsm_next_recommended_steps(HSMSystem* sys, HSMLogicStep* steps, int max) {
    if (!sys || !steps || max <= 0) return 0;
    int n = 0;
    for (int s = 0; s < HSM_NUM_STEPS && n < max; s++) {
        bool any_incomplete = false;
        for (int p = 0; p < HSM_NUM_PHASES && !any_incomplete; p++)
            for (int a = 0; a < sys->matrix_counts[p*HSM_NUM_STEPS+s]; a++)
                if (sys->matrix[p][a].progress < 1.0) { any_incomplete = true; break; }
        if (any_incomplete) steps[n++] = (HSMLogicStep)s;
    }
    return n;
}

/* Morphological completeness: verify all functions have at least one solution.
 * Returns the number of functions with zero solutions (should be 0 for complete). */
int morph_check_complete(int n_functions, const int* solution_counts) {
    if (!solution_counts || n_functions <= 0) return n_functions;
    int incomplete = 0;
    for (int i = 0; i < n_functions; i++)
        if (solution_counts[i] <= 0) incomplete++;
    return incomplete;
}
