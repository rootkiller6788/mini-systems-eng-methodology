#include "../include/hsm_core.h"
#include <stdlib.h>
#include <math.h>

/* Verification metrics for Hall's methodology.
 * Tracks verification coverage, defect density, and requirement satisfaction. */

typedef struct {
    int     n_requirements;
    int     n_verified;
    int     n_failed;
    double* defect_density;  /* defects per requirement */
    double  verification_coverage;
    double  defect_removal_efficiency;
} VerificationMetrics;

VerificationMetrics* vm_create(int n_reqs) {
    VerificationMetrics* vm = calloc(1, sizeof(VerificationMetrics));
    if (!vm) return NULL;
    vm->n_requirements = n_reqs;
    vm->defect_density = calloc(n_reqs, sizeof(double));
    return vm;
}

void vm_free(VerificationMetrics* vm) {
    if (!vm) return;
    free(vm->defect_density);
    free(vm);
}

int vm_record_verification(VerificationMetrics* vm, int req_idx, int passed,
                            double defects_found) {
    if (!vm || req_idx < 0 || req_idx >= vm->n_requirements) return -1;
    if (passed) vm->n_verified++; else vm->n_failed++;
    vm->defect_density[req_idx] = defects_found;
    vm->verification_coverage = (double)vm->n_verified /
                                (double)(vm->n_verified + vm->n_failed + 1);
    return 0;
}

double vm_defect_removal_efficiency(VerificationMetrics* vm) {
    if (!vm || vm->n_requirements == 0) return 0.0;
    double total_defects = 0.0;
    for (int i = 0; i < vm->n_requirements; i++)
        total_defects += vm->defect_density[i];
    double removed = (double)vm->n_verified * total_defects / vm->n_requirements;
    vm->defect_removal_efficiency = total_defects > 1e-10
        ? removed / total_defects : 1.0;
    return vm->defect_removal_efficiency;
}

int vm_is_ready_for_release(VerificationMetrics* vm) {
    if (!vm) return 0;
    return (vm->verification_coverage > 0.95 &&
            vm->defect_removal_efficiency > 0.90) ? 1 : 0;
}
