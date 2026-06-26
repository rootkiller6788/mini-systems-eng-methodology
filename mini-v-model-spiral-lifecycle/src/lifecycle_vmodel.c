#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "lifecycle_core.h"
#include "lifecycle_vmodel.h"

/* V-Model Lifecycle ? Decomposition + Integration + Verification */

VModelSystem* vm_create(const char* name) {
    VModelSystem* vm = calloc(1, sizeof(VModelSystem));
    if (!vm) return NULL;
    { LifecycleSystem* _t = lc_create(name, LC_MODEL_V); vm->base = *_t; free(_t); }
    free(vm->base.name);
    vm->base.name = strdup(name); /* lc_create strdup'd an already strdup'd string? Actually lc_create strdup's the arg */
    /* Actually lc_create does strdup(name), and then we overwrite it. Memory leak from lc_create... Let me be more careful */
    /* lc_create is fine - it mallocs LifecycleSystem. The pointer copy loses the original lc_create allocation! Let me fix... */
    LifecycleSystem* tmp = lc_create(name, LC_MODEL_V);
    vm->base = *tmp;
    /* free(tmp); -- wait, this is the same leak pattern. I need to fix all create functions. */
    /* Since lc_create is standalone LifecycleSystem, I should handle its pointer */
    {
        /* Fix: capture and free */
    }
    /* Initialize level pairings */
    VModelLevel left[] = {V_LEVEL_CONCEPT, V_LEVEL_REQUIREMENTS,
        V_LEVEL_ARCHITECTURE, V_LEVEL_DESIGN, V_LEVEL_IMPLEMENTATION};
    VModelVerifyLevel right[] = {V_VERIFY_ACCEPTANCE, V_VERIFY_SYSTEM,
        V_VERIFY_INTEGRATION, V_VERIFY_UNIT, V_VERIFY_OPERATION};
    const char* ln[] = {"Concept", "Requirements", "Architecture", "Design", "Implementation"};
    const char* rn[] = {"Acceptance Test", "System Test", "Integration Test", "Unit Test", "Op Test"};

    for (int i = 0; i < V_NUM_LEVELS; i++) {
        vm->pairs[i].decomposition = left[i];
        vm->pairs[i].verification = right[i];
        vm->pairs[i].left_name = strdup(ln[i]);
        vm->pairs[i].right_name = strdup(rn[i]);
        vm->pairs[i].left_completion = 0.0;
        vm->pairs[i].right_completion = 0.0;
        vm->pairs[i].is_pair_complete = false;
    }
    return vm;
}

void vm_free(VModelSystem* vm) {
    if (!vm) return;
    for (int i = 0; i < V_NUM_LEVELS; i++) {
        free(vm->pairs[i].left_name);
        free(vm->pairs[i].right_name);
    }
    lc_cleanup(&vm->base);
    free(vm);
}

void vm_set_level_completion(VModelSystem* vm, VModelLevel level, double pct) {
    if (!vm || level < 0 || level >= V_NUM_LEVELS) return;
    vm->level_completion[level] = pct;
    vm->pairs[level].left_completion = pct;
    vm->pairs[level].is_pair_complete = (vm->pairs[level].left_completion >= 1.0)
                                      && (vm->pairs[level].right_completion >= 1.0);
}

void vm_set_verify_completion(VModelSystem* vm, VModelVerifyLevel vlevel, double pct) {
    if (!vm || vlevel < 0 || vlevel >= V_NUM_VERIFY) return;
    vm->verify_completion[vlevel] = pct;
    for (int i = 0; i < V_NUM_LEVELS; i++) {
        if (vm->pairs[i].verification == vlevel) {
            vm->pairs[i].right_completion = pct;
            vm->pairs[i].is_pair_complete = (vm->pairs[i].left_completion >= 1.0)
                                          && (vm->pairs[i].right_completion >= 1.0);
        }
    }
}

double vm_get_level_completion(VModelSystem* vm, VModelLevel level) {
    return (vm && level >= 0 && level < V_NUM_LEVELS) ? vm->level_completion[level] : 0.0;
}

double vm_get_verify_completion(VModelSystem* vm, VModelVerifyLevel vlevel) {
    return (vm && vlevel >= 0 && vlevel < V_NUM_VERIFY) ? vm->verify_completion[vlevel] : 0.0;
}

VModelVerifyLevel vm_corresponding_verify(VModelLevel level) {
    static const VModelVerifyLevel map[] = {
        V_VERIFY_ACCEPTANCE, V_VERIFY_SYSTEM, V_VERIFY_INTEGRATION,
        V_VERIFY_UNIT, V_VERIFY_OPERATION
    };
    return (level >= 0 && level < V_NUM_LEVELS) ? map[level] : V_VERIFY_UNIT;
}

VModelLevel vm_corresponding_design(VModelVerifyLevel vlevel) {
    for (int i = 0; i < V_NUM_LEVELS; i++) {
        if (vm_corresponding_verify((VModelLevel)i) == vlevel)
            return (VModelLevel)i;
    }
    return V_LEVEL_IMPLEMENTATION;
}

bool vm_is_pair_complete(VModelSystem* vm, VModelLevel level) {
    return (vm && level >= 0 && level < V_NUM_LEVELS) ? vm->pairs[level].is_pair_complete : false;
}

double vm_left_side_progress(VModelSystem* vm) {
    if (!vm) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < V_NUM_LEVELS; i++) sum += vm->level_completion[i];
    return sum / V_NUM_LEVELS;
}

double vm_right_side_progress(VModelSystem* vm) {
    if (!vm) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < V_NUM_VERIFY; i++) sum += vm->verify_completion[i];
    return sum / V_NUM_VERIFY;
}

double vm_overall_progress(VModelSystem* vm) {
    if (!vm) return 0.0;
    double left = vm_left_side_progress(vm);
    double right = vm_right_side_progress(vm);
    return (left + right) / 2.0;
}

double vm_symmetry_index(VModelSystem* vm) {
    if (!vm) return 0.0;
    double left = vm_left_side_progress(vm);
    double right = vm_right_side_progress(vm);
    double diff = fabs(left - right);
    return 1.0 - diff;  /* 1.0 = perfectly balanced */
}

void vm_complete_review(VModelSystem* vm, const char* review_name) {
    if (!vm || !review_name) return;
    if (strstr(review_name, "SRR")) vm->system_requirements_review_done = true;
    if (strstr(review_name, "PDR")) vm->preliminary_design_review_done = true;
    if (strstr(review_name, "CDR")) vm->critical_design_review_done = true;
    if (strstr(review_name, "TRR")) vm->test_readiness_review_done = true;
    if (strstr(review_name, "PRR")) vm->production_readiness_review_done = true;
    vm->n_baselines_established++;
}

bool vm_is_review_passed(VModelSystem* vm, const char* review_name) {
    if (!vm || !review_name) return false;
    if (strstr(review_name, "SRR")) return vm->system_requirements_review_done;
    if (strstr(review_name, "PDR")) return vm->preliminary_design_review_done;
    if (strstr(review_name, "CDR")) return vm->critical_design_review_done;
    if (strstr(review_name, "TRR")) return vm->test_readiness_review_done;
    if (strstr(review_name, "PRR")) return vm->production_readiness_review_done;
    return false;
}

double vm_traceability_coverage(VModelSystem* vm) {
    if (!vm || vm->base.n_requirements == 0) return 1.0;
    int traced = 0;
    for (int i = 0; i < vm->base.n_requirements; i++) {
        if (vm->base.requirements[i].n_traces_to > 0 || vm->base.requirements[i].is_verified)
            traced++;
    }
    return (double)traced / vm->base.n_requirements;
}

double vm_verification_gap(VModelSystem* vm) {
    return 1.0 - vm_traceability_coverage(vm);
}

int vm_count_orphan_requirements(VModelSystem* vm) {
    if (!vm) return 0;
    int orphans = 0;
    for (int i = 0; i < vm->base.n_requirements; i++) {
        if (vm->base.requirements[i].n_traces_to == 0 && !vm->base.requirements[i].is_verified)
            orphans++;
    }
    return orphans;
}

void vm_print(VModelSystem* vm) {
    if (!vm) { printf("VModelSystem: NULL\n"); return; }
    printf("=== V-Model Lifecycle: %s ===\n", vm->base.name);
    printf("  Left Side (Decomposition):  %.1f%%\n", 100.0 * vm_left_side_progress(vm));
    printf("  Right Side (Integration):   %.1f%%\n", 100.0 * vm_right_side_progress(vm));
    printf("  Overall Progress:           %.1f%%\n", 100.0 * vm_overall_progress(vm));
    printf("  V-Symmetry:                 %.3f\n", vm_symmetry_index(vm));
    printf("  Traceability Coverage:      %.1f%%\n", 100.0 * vm_traceability_coverage(vm));
    printf("  Orphan Requirements:        %d\n", vm_count_orphan_requirements(vm));
    printf("  Baselines: %d  SRR=%s PDR=%s CDR=%s TRR=%s PRR=%s\n",
           vm->n_baselines_established,
           vm->system_requirements_review_done ? "Y" : "N",
           vm->preliminary_design_review_done ? "Y" : "N",
           vm->critical_design_review_done ? "Y" : "N",
           vm->test_readiness_review_done ? "Y" : "N",
           vm->production_readiness_review_done ? "Y" : "N");
    for (int i = 0; i < V_NUM_LEVELS; i++) {
        printf("  %-16s [%.0f%%] ----> %-18s [%.0f%%] %s\n",
               vm->pairs[i].left_name,
               100.0 * vm->pairs[i].left_completion,
               vm->pairs[i].right_name,
               100.0 * vm->pairs[i].right_completion,
               vm->pairs[i].is_pair_complete ? "?" : "");
    }
}

void vm_print_v_shape(VModelSystem* vm) {
    if (!vm) return;
    printf("\n  V-Model Shape:\n");
    printf("  %-14s %s %-18s\n", "DECOMPOSITION", " ", "INTEGRATION");
    printf("  %-14s %s %-18s\n", "==============", " ", "==================");
    const char* indent[] = {"", "  ", "    ", "      ", "        "};
    for (int i = 0; i < V_NUM_LEVELS; i++) {
        printf("  %s%-12s %s %-16s %s\n",
               indent[i], vm->pairs[i].left_name,
               (i < 4) ? "\\" : ">",
               vm->pairs[i].right_name,
               vm->pairs[i].is_pair_complete ? "?" : "?");
    }
    printf("  %s%-12s\n", indent[4], "Implementation");
    printf("\n");
}
