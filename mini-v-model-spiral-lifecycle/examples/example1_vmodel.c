#include <stdio.h>
#include <stdlib.h>
#include "lifecycle_core.h"
#include "lifecycle_vmodel.h"
#include "lifecycle_gates.h"

/* Example 1: V-Model — Satellite System Lifecycle
 * Demonstrates the complete V-Model from concept through verification. */

int main(void) {
    printf("=== Example 1: V-Model Lifecycle — Satellite System ===\n\n");

    VModelSystem* sat = vm_create("EarthObservingSatellite");

    printf("--- Phase 1: Decomposition (Left Side) ---\n");
    vm_set_level_completion(sat, V_LEVEL_CONCEPT, 1.0);
    printf("Concept (ConOps): %.0f%%\n", 100.0 * vm_get_level_completion(sat, V_LEVEL_CONCEPT));

    vm_set_level_completion(sat, V_LEVEL_REQUIREMENTS, 1.0);
    printf("Requirements (SRR): %.0f%%\n", 100.0 * vm_get_level_completion(sat, V_LEVEL_REQUIREMENTS));

    vm_set_level_completion(sat, V_LEVEL_ARCHITECTURE, 0.8);
    printf("Architecture (PDR): %.0f%%\n", 100.0 * vm_get_level_completion(sat, V_LEVEL_ARCHITECTURE));

    vm_set_level_completion(sat, V_LEVEL_DESIGN, 0.5);
    printf("Design (CDR): %.0f%%\n\n", 100.0 * vm_get_level_completion(sat, V_LEVEL_DESIGN));

    printf("--- Phase 2: Integration & Verification (Right Side) ---\n");
    vm_set_verify_completion(sat, V_VERIFY_UNIT, 0.9);
    printf("Unit Test: %.0f%%\n", 100.0 * vm_get_verify_completion(sat, V_VERIFY_UNIT));

    vm_set_verify_completion(sat, V_VERIFY_INTEGRATION, 0.6);
    printf("Integration Test: %.0f%%\n", 100.0 * vm_get_verify_completion(sat, V_VERIFY_INTEGRATION));

    vm_set_verify_completion(sat, V_VERIFY_SYSTEM, 0.3);
    printf("System Test: %.0f%%\n\n", 100.0 * vm_get_verify_completion(sat, V_VERIFY_SYSTEM));

    printf("--- Reviews ---\n");
    vm_complete_review(sat, "SRR_PASSED");
    vm_complete_review(sat, "PDR_PASSED");
    vm_complete_review(sat, "CDR_PASSED");
    printf("SRR: %s  PDR: %s  CDR: %s\n",
           vm_is_review_passed(sat, "SRR") ? "PASS" : "PENDING",
           vm_is_review_passed(sat, "PDR") ? "PASS" : "PENDING",
           vm_is_review_passed(sat, "CDR") ? "PASS" : "PENDING");

    printf("\n--- V-Model Summary ---\n");
    vm_print(sat);
    vm_print_v_shape(sat);

    printf("Symmetry Index: %.3f (%.0f%% balanced)\n",
           vm_symmetry_index(sat), 100.0 * vm_symmetry_index(sat));
    printf("Key insight: Verification planning begins during decomposition.\n");

    vm_free(sat);
    return 0;
}
