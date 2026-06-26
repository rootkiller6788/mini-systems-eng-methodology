#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "lifecycle_core.h"
#include "lifecycle_spiral.h"

/* Spiral Model (Boehm, 1988) ? Risk-Driven Iterative Development */

SpiralSystem* spiral_create(const char* name, int max_cycles, double risk_threshold) {
    SpiralSystem* sp = calloc(1, sizeof(SpiralSystem));
    if (!sp) return NULL;
    { LifecycleSystem* _t = lc_create(name, LC_MODEL_SPIRAL); sp->base = *_t; free(_t); }
    sp->cycle_capacity = max_cycles > 0 ? max_cycles : 8;
    sp->cycles = calloc((size_t)sp->cycle_capacity, sizeof(SpiralCycle));
    sp->n_cycles = 0;
    sp->max_cycles = max_cycles > 0 ? max_cycles : 8;
    sp->risk_threshold = risk_threshold > 0 ? risk_threshold : 0.3;
    sp->cycle_duration_days = 30.0;
    sp->cumulative_cost = 0.0;
    sp->lco_anchor = false;
    sp->lca_anchor = false;
    sp->ioc_anchor = false;
    sp->n_prototypes_built = 0;
    sp->prototyping_effort = 0.0;
    sp->uses_prototyping = false;
    sp->risk_reduction_per_cycle = 0.0;
    sp->cost_growth_rate = 0.0;
    sp->average_cycle_completion = 0.0;
    sp->current_cycle = 0;
    return sp;
}

void spiral_free(SpiralSystem* sp) {
    if (!sp) return;
    if (sp->cycles) {
        for (int i = 0; i < sp->n_cycles; i++) {
            free(sp->cycles[i].cycle_name);
            free(sp->cycles[i].objectives);
            free(sp->cycles[i].constraints);
            free(sp->cycles[i].prototype_type);
            free(sp->cycles[i].deliverables);
            free(sp->cycles[i].next_cycle_plan);
            if (sp->cycles[i].alternatives) {
                for (int j = 0; j < sp->cycles[i].n_alternatives; j++)
                    free(sp->cycles[i].alternatives[j]);
                free(sp->cycles[i].alternatives);
            }
        }
        free(sp->cycles);
    }
    lc_cleanup(&sp->base);
    free(sp);
}

SpiralCycle* spiral_add_cycle(SpiralSystem* sp, const char* name,
                               const char* objectives, const char* constraints) {
    if (!sp || sp->n_cycles >= sp->cycle_capacity) return NULL;
    SpiralCycle* sc = &sp->cycles[sp->n_cycles];
    sc->cycle_number = sp->n_cycles;
    sc->cycle_name = strdup(name ? name : "Unnamed");
    sc->objectives = strdup(objectives ? objectives : "");
    sc->constraints = strdup(constraints ? constraints : "");
    sc->alternatives = NULL;
    sc->n_alternatives = 0;
    for (int i = 0; i < SPIRAL_NUM_PHASES; i++) sc->phase_progress[i] = 0.0;
    sc->n_risks_identified = 0;
    sc->n_risks_resolved = 0;
    sc->cycle_risk_exposure = 0.0;
    sc->prototype_type = strdup("None");
    sc->deliverables = strdup("");
    sc->development_effort = 0.0;
    sc->next_cycle_plan = strdup("");
    sc->committed_budget = 0.0;
    sc->cumulative_cost = 0.0;
    sc->is_complete = false;
    sc->is_approved = false;
    sp->n_cycles++;
    sp->current_cycle = sc->cycle_number;
    return sc;
}

void spiral_set_cycle_progress(SpiralSystem* sp, int cycle, SpiralPhase phase, double progress) {
    if (!sp || cycle < 0 || cycle >= sp->n_cycles || phase < 0 || phase >= SPIRAL_NUM_PHASES) return;
    sp->cycles[cycle].phase_progress[phase] = progress;
    /* Update cumulative cost as cycle progresses */
    if (phase == SPIRAL_DEVELOPMENT && progress > 0) {
        sp->cycles[cycle].cumulative_cost = sp->cumulative_cost + progress * 100.0;
        sp->cumulative_cost = sp->cycles[cycle].cumulative_cost;
    }
}

SpiralCycle* spiral_get_current_cycle(SpiralSystem* sp) {
    if (!sp || sp->n_cycles == 0) return NULL;
    return &sp->cycles[sp->current_cycle];
}

int spiral_completed_cycles(SpiralSystem* sp) {
    if (!sp) return 0;
    int count = 0;
    for (int i = 0; i < sp->n_cycles; i++)
        if (sp->cycles[i].is_complete) count++;
    return count;
}

bool spiral_should_prototype(SpiralSystem* sp, double risk_exposure) {
    return sp && risk_exposure > sp->risk_threshold;
}

bool spiral_should_continue(SpiralSystem* sp) {
    if (!sp) return false;
    if (sp->n_cycles >= sp->max_cycles) return false;
    if (sp->current_cycle < sp->n_cycles && sp->cycles[sp->current_cycle].is_complete)
        return (sp->current_cycle + 1) < sp->max_cycles;
    return sp->n_cycles < sp->max_cycles;
}

bool spiral_is_anchored(SpiralSystem* sp, const char* anchor_name) {
    if (!sp || !anchor_name) return false;
    if (strstr(anchor_name, "LCO")) return sp->lco_anchor;
    if (strstr(anchor_name, "LCA")) return sp->lca_anchor;
    if (strstr(anchor_name, "IOC")) return sp->ioc_anchor;
    return false;
}

void spiral_plan_next_cycle(SpiralSystem* sp, const char* plan, double budget) {
    if (!sp || sp->n_cycles == 0) return;
    SpiralCycle* sc = &sp->cycles[sp->n_cycles - 1];
    free(sc->next_cycle_plan);
    sc->next_cycle_plan = strdup(plan ? plan : "");
    sc->committed_budget = budget;
    sc->is_approved = true;
}

double spiral_estimate_remaining_cost(SpiralSystem* sp) {
    if (!sp || sp->n_cycles == 0) return 0.0;
    double avg_cost = sp->cumulative_cost / sp->n_cycles;
    int remaining = sp->max_cycles - spiral_completed_cycles(sp);
    return avg_cost * remaining;
}

double spiral_estimate_remaining_cycles(SpiralSystem* sp) {
    if (!sp) return 0.0;
    return (double)(sp->max_cycles - spiral_completed_cycles(sp));
}

double spiral_cumulative_cost(SpiralSystem* sp) {
    return sp ? sp->cumulative_cost : 0.0;
}

double spiral_risk_reduction_rate(SpiralSystem* sp) {
    if (!sp || sp->n_cycles < 2) return 0.0;
    double initial = sp->cycles[0].cycle_risk_exposure;
    double current = sp->cycles[sp->n_cycles - 1].cycle_risk_exposure;
    if (initial < 1e-10) return 0.0;
    return (initial - current) / initial;
}

double spiral_progress_by_angle(SpiralSystem* sp) {
    if (!sp || sp->max_cycles == 0) return 0.0;
    return (double)spiral_completed_cycles(sp) / sp->max_cycles;
}

void spiral_print(SpiralSystem* sp) {
    if (!sp) { printf("SpiralSystem: NULL\n"); return; }
    printf("=== Spiral Model: %s ===\n", sp->base.name);
    printf("  Cycles: %d/%d  Cumulative Cost: %.0f\n",
           sp->n_cycles, sp->max_cycles, sp->cumulative_cost);
    printf("  Risk Threshold: %.2f  Prototypes: %d\n",
           sp->risk_threshold, sp->n_prototypes_built);
    printf("  Anchors: LCO=%s LCA=%s IOC=%s\n",
           sp->lco_anchor ? "Y" : "N",
           sp->lca_anchor ? "Y" : "N",
           sp->ioc_anchor ? "Y" : "N");
    printf("  Risk Reduction: %.1f%%  Remaining Cost: %.0f\n",
           100.0 * spiral_risk_reduction_rate(sp),
           spiral_estimate_remaining_cost(sp));
    for (int i = 0; i < sp->n_cycles; i++) {
        SpiralCycle* sc = &sp->cycles[i];
        printf("  Cycle %d [%s]: Obj=%.0f%% Risk=%.0f%% Dev=%.0f%% Plan=%.0f%% Cost=%.0f %s\n",
               sc->cycle_number, sc->cycle_name,
               100.0 * sc->phase_progress[SPIRAL_OBJECTIVES],
               100.0 * sc->phase_progress[SPIRAL_RISK_ANALYSIS],
               100.0 * sc->phase_progress[SPIRAL_DEVELOPMENT],
               100.0 * sc->phase_progress[SPIRAL_PLANNING],
               sc->cumulative_cost,
               sc->is_complete ? "?" : "?");
    }
}

void spiral_print_cycle(SpiralCycle* cycle) {
    if (!cycle) { printf("SpiralCycle: NULL\n"); return; }
    printf("=== Spiral Cycle %d: %s ===\n", cycle->cycle_number, cycle->cycle_name);
    printf("  Objectives: %s\n", cycle->objectives);
    printf("  Progress: Obj=%.0f%% Risk=%.0f%% Dev=%.0f%% Plan=%.0f%%\n",
           100.0 * cycle->phase_progress[0], 100.0 * cycle->phase_progress[1],
           100.0 * cycle->phase_progress[2], 100.0 * cycle->phase_progress[3]);
    printf("  Risks: %d identified, %d resolved  Cost: %.0f\n",
           cycle->n_risks_identified, cycle->n_risks_resolved, cycle->cumulative_cost);
    printf("  Complete: %s  Approved: %s\n",
           cycle->is_complete ? "YES" : "NO", cycle->is_approved ? "YES" : "NO");
}

/* ====================================================================
 * Extended Spiral Model Functions
 * ==================================================================== */

/* Compute net present value of spiral investment */
double spiral_npv(SpiralSystem* sp, double discount_rate) {
    if (!sp || sp->n_cycles == 0) return 0.0;
    double npv = 0.0;
    for (int i = 0; i < sp->n_cycles; i++) {
        double cost = sp->cycles[i].committed_budget;
        double discount = pow(1.0 + discount_rate, (double)i);
        npv -= cost / discount;
    }
    return npv;
}

/* Return on investment for spiral development */
double spiral_roi(SpiralSystem* sp, double total_benefit, double discount_rate) {
    double npv = spiral_npv(sp, discount_rate);
    if (fabs(npv) < 1e-10) return 0.0;
    return (total_benefit + npv) / fabs(npv);
}

/* Cycle efficiency: progress per unit cost */
double spiral_cycle_efficiency(SpiralSystem* sp, int cycle_idx) {
    if (!sp || cycle_idx < 0 || cycle_idx >= sp->n_cycles) return 0.0;
    SpiralCycle* sc = &sp->cycles[cycle_idx];
    double avg_progress = 0.0;
    for (int i = 0; i < SPIRAL_NUM_PHASES; i++) avg_progress += sc->phase_progress[i];
    avg_progress /= SPIRAL_NUM_PHASES;
    if (sc->committed_budget < 1e-10) return 0.0;
    return avg_progress / sc->committed_budget;
}

/* Risk reduction efficiency (exposure reduced per $1000 spent) */
double spiral_risk_reduction_efficiency(SpiralSystem* sp) {
    if (!sp || sp->n_cycles < 2) return 0.0;
    double initial = sp->cycles[0].cycle_risk_exposure;
    double current = sp->cycles[sp->n_cycles-1].cycle_risk_exposure;
    double reduced = initial - current;
    double total_cost = sp->cumulative_cost;
    if (total_cost < 1e-10) return 0.0;
    return reduced * 1000.0 / total_cost;
}

/* Detect if spiral is converging (progress per cycle is increasing) */
bool spiral_is_converging(SpiralSystem* sp) {
    if (!sp || sp->n_cycles < 3) return false;
    double eff0 = spiral_cycle_efficiency(sp, sp->n_cycles - 3);
    double eff1 = spiral_cycle_efficiency(sp, sp->n_cycles - 2);
    double eff2 = spiral_cycle_efficiency(sp, sp->n_cycles - 1);
    return (eff1 > eff0) && (eff2 > eff1);
}

