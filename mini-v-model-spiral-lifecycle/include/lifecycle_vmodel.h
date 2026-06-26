#ifndef LIFECYCLE_VMODEL_H
#define LIFECYCLE_VMODEL_H

#include "lifecycle_core.h"

/* ============================================================================
 * V-Model Lifecycle ? Decomposition ? Integration ? Verification
 *
 * The V-Model (Forsberg & Mooz, 1991) represents the systems engineering
 * lifecycle as a V-shape:
 *
 *   Left side (Decomposition):         Right side (Integration):
 *     Concept ????????????????????? Acceptance Test
 *       ?                               ?
 *     Requirements ??????????????? System Verification
 *       ?                               ?
 *     Architecture ??????????????? Integration Test
 *       ?                               ?
 *     Design ????????????????????? Unit Test
 *       ?                               ?
 *     Implementation (Code/Build)
 *
 * Each decomposition level has a corresponding verification level.
 * Traceability flows from requirements ? design ? implementation ? tests.
 * ============================================================================ */

/* --- V-Model Level Pair (Left + Right sides) --- */
typedef struct {
    VModelLevel      decomposition;   /* Left side: design level */
    VModelVerifyLevel verification;   /* Right side: test level */
    char*            left_name;       /* e.g., "Architecture" */
    char*            right_name;      /* e.g., "Integration Test" */
    double           left_completion;
    double           right_completion;
    bool             is_pair_complete;
} VLevelPair;

/* --- V-Model State --- */
typedef struct {
    LifecycleSystem base;

    /* Level completions */
    double level_completion[V_NUM_LEVELS];     /* Left side */
    double verify_completion[V_NUM_VERIFY];    /* Right side */

    /* Level pairings (predefined) */
    VLevelPair pairs[V_NUM_LEVELS];

    /* Traceability matrix: requirements at each level */
    int    n_reqs_at_level[V_NUM_LEVELS];
    int    n_reqs_verified_at_level[V_NUM_VERIFY];

    /* V-model specific metrics */
    double left_side_progress;    /* Decomposition progress */
    double right_side_progress;   /* Integration/verification progress */
    double v_symmetry;            /* How balanced the V is (1.0 = perfect) */
    double traceability_gap;      /* Requirements without test coverage */

    /* Reviews and baselines */
    int    n_baselines_established;
    bool   system_requirements_review_done;   /* SRR */
    bool   preliminary_design_review_done;    /* PDR */
    bool   critical_design_review_done;       /* CDR */
    bool   test_readiness_review_done;        /* TRR */
    bool   production_readiness_review_done;  /* PRR */
} VModelSystem;

/* --- V-Model API --- */
VModelSystem* vm_create(const char* name);
void vm_free(VModelSystem* vm);

/* Level management */
void vm_set_level_completion(VModelSystem* vm, VModelLevel level, double pct);
void vm_set_verify_completion(VModelSystem* vm, VModelVerifyLevel vlevel, double pct);
double vm_get_level_completion(VModelSystem* vm, VModelLevel level);
double vm_get_verify_completion(VModelSystem* vm, VModelVerifyLevel vlevel);

/* Relationship mapping */
VModelVerifyLevel vm_corresponding_verify(VModelLevel level);
VModelLevel vm_corresponding_design(VModelVerifyLevel vlevel);
bool vm_is_pair_complete(VModelSystem* vm, VModelLevel level);

/* Progress */
double vm_left_side_progress(VModelSystem* vm);
double vm_right_side_progress(VModelSystem* vm);
double vm_overall_progress(VModelSystem* vm);
double vm_symmetry_index(VModelSystem* vm);  /* 1.0 = perfectly balanced V */

/* Review gates */
void vm_complete_review(VModelSystem* vm, const char* review_name);
bool vm_is_review_passed(VModelSystem* vm, const char* review_name);

/* Traceability */
double vm_traceability_coverage(VModelSystem* vm);
double vm_verification_gap(VModelSystem* vm);
int vm_count_orphan_requirements(VModelSystem* vm);  /* No test coverage */

void vm_print(VModelSystem* vm);
void vm_print_v_shape(VModelSystem* vm);  /* ASCII art V-diagram */

#endif
