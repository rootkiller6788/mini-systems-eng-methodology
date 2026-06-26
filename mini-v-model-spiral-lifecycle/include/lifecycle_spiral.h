#ifndef LIFECYCLE_SPIRAL_H
#define LIFECYCLE_SPIRAL_H

#include "lifecycle_core.h"

/* ============================================================================
 * Spiral Model (Boehm, 1988) ? Risk-Driven Iterative Development
 *
 * Each spiral cycle progresses through four phases:
 *   1. Determine Objectives ? What to achieve this cycle
 *   2. Identify & Resolve Risks ? Risk analysis, prototyping
 *   3. Development & Test ? Build and verify the product increment
 *   4. Plan Next Cycle ? Review, commit to next iteration
 *
 * The radius of the spiral represents cumulative cost.
 * The angular dimension represents progress through cycles.
 *
 * Based on:
 *   Boehm, B.W. ? "A Spiral Model of Software Development and Enhancement" (1988)
 *   Boehm, B.W. ? "Anchoring the Software Process" (1996)
 *   Boehm et al. ? "The Incremental Commitment Spiral Model" (2014)
 * ============================================================================ */

/* --- Spiral Cycle --- */
typedef struct {
    int         cycle_number;         /* 0, 1, 2, ... */
    char*       cycle_name;           /* e.g., "Concept", "Prototype", "Build 1" */
    char*       objectives;           /* What this cycle aims to achieve */
    char*       constraints;          /* Constraints on this cycle */
    char**      alternatives;         /* Alternative approaches considered */
    int         n_alternatives;

    /* Progress through spiral phases */
    double      phase_progress[SPIRAL_NUM_PHASES];

    /* Risk assessment for this cycle */
    int         n_risks_identified;
    int         n_risks_resolved;
    double      cycle_risk_exposure;

    /* Development */
    char*       prototype_type;       /* None, Mockup, Breadboard, Operational */
    char*       deliverables;
    double      development_effort;   /* Person-days */

    /* Planning */
    char*       next_cycle_plan;
    double      committed_budget;
    double      cumulative_cost;      /* Radius of spiral */
    bool        is_complete;
    bool        is_approved;
} SpiralCycle;

/* --- Spiral Model System --- */
typedef struct {
    LifecycleSystem base;

    SpiralCycle* cycles;
    int         n_cycles;
    int         cycle_capacity;

    /* Spiral parameters */
    double      risk_threshold;       /* Risk exposure that triggers prototyping */
    double      cycle_duration_days;  /* Typical cycle length */
    int         max_cycles;           /* Maximum planned cycles */
    double      cumulative_cost;      /* Total cost (radius) */

    /* Anchor points (Boehm, 1996) */
    bool        lco_anchor;           /* Life Cycle Objectives */
    bool        lca_anchor;           /* Life Cycle Architecture */
    bool        ioc_anchor;           /* Initial Operational Capability */

    /* Prototyping */
    int         n_prototypes_built;
    double      prototyping_effort;
    bool        uses_prototyping;

    /* Metrics */
    double      risk_reduction_per_cycle;
    double      cost_growth_rate;
    double      average_cycle_completion;
    int         current_cycle;
} SpiralSystem;

/* --- Spiral Model API --- */
SpiralSystem* spiral_create(const char* name, int max_cycles,
                             double risk_threshold);
void spiral_free(SpiralSystem* sp);

/* Cycle management */
SpiralCycle* spiral_add_cycle(SpiralSystem* sp, const char* name,
                               const char* objectives, const char* constraints);
void spiral_set_cycle_progress(SpiralSystem* sp, int cycle,
                                SpiralPhase phase, double progress);
SpiralCycle* spiral_get_current_cycle(SpiralSystem* sp);
int spiral_completed_cycles(SpiralSystem* sp);

/* Risk-driven decisions */
bool spiral_should_prototype(SpiralSystem* sp, double risk_exposure);
bool spiral_should_continue(SpiralSystem* sp);
bool spiral_is_anchored(SpiralSystem* sp, const char* anchor_name);

/* Planning */
void spiral_plan_next_cycle(SpiralSystem* sp, const char* plan,
                             double budget);
double spiral_estimate_remaining_cost(SpiralSystem* sp);
double spiral_estimate_remaining_cycles(SpiralSystem* sp);

/* Progress metrics */
double spiral_cumulative_cost(SpiralSystem* sp);
double spiral_risk_reduction_rate(SpiralSystem* sp);
double spiral_progress_by_angle(SpiralSystem* sp); /* Angular progress */

void spiral_print(SpiralSystem* sp);
void spiral_print_cycle(SpiralCycle* cycle);

#endif
