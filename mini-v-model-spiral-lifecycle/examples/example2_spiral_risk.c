#include <stdio.h>
#include <stdlib.h>
#include "lifecycle_core.h"
#include "lifecycle_spiral.h"
#include "lifecycle_risk.h"

/* Example 2: Spiral Model + Risk Management — Medical Device
 * Demonstrates risk-driven iterative development with the Spiral Model. */

int main(void) {
    printf("=== Example 2: Spiral Model + Risk — Medical Device ===\n\n");

    SpiralSystem* device = spiral_create("ImplantablePacemaker", 6, 0.25);
    RiskManagementPlan* risk = risk_create("PacemakerRisk");

    printf("Risk threshold: %.2f (prototype if risk exceeds this)\n\n", device->risk_threshold);

    /* Cycle 0: Concept */
    printf("--- Cycle 0: Concept Exploration ---\n");
    spiral_add_cycle(device, "Concept", "Feasibility study and stakeholder needs",
                     "Regulatory: FDA Class III");
    risk_add_risk(risk, "R-001", "Battery lifetime insufficient",
                  RISK_HIGH, RISK_CRITICAL, "Alternative battery chemistry", "Life test < 5 years");
    risk_add_risk(risk, "R-002", "EMI interference with other devices",
                  RISK_MODERATE, RISK_HIGH, "Shielding and validation", "EMC test failures");
    risk_add_risk(risk, "R-003", "Biocompatibility of encapsulation",
                  RISK_LOW, RISK_CRITICAL, "Material testing per ISO 10993", "Rejection in animal tests");

    spiral_set_cycle_progress(device, 0, SPIRAL_OBJECTIVES, 1.0);
    spiral_set_cycle_progress(device, 0, SPIRAL_RISK_ANALYSIS, 1.0);
    if (spiral_should_prototype(device, risk_total_exposure(risk)))
        printf("Decision: BUILD PROTOTYPE (risk %.3f > threshold %.3f)\n",
               risk_total_exposure(risk), device->risk_threshold);
    spiral_set_cycle_progress(device, 0, SPIRAL_DEVELOPMENT, 1.0);
    spiral_set_cycle_progress(device, 0, SPIRAL_PLANNING, 1.0);
    spiral_plan_next_cycle(device, "Develop functional prototype", 200.0);

    /* Mitigate risks */
    risk_apply_mitigation(risk, "R-001", 30.0);
    risk_apply_mitigation(risk, "R-002", 15.0);
    printf("Risk budget spent: %.0fK  Remaining: %.0fK\n",
           risk->risk_budget_spent, risk->risk_budget_remaining);

    /* Cycle 1 */
    printf("\n--- Cycle 1: Prototype Development ---\n");
    spiral_add_cycle(device, "Prototype", "Build and test functional prototype",
                     "Must demonstrate 90%% function");
    spiral_set_cycle_progress(device, 1, SPIRAL_OBJECTIVES, 1.0);
    spiral_set_cycle_progress(device, 1, SPIRAL_RISK_ANALYSIS, 0.8);
    spiral_set_cycle_progress(device, 1, SPIRAL_DEVELOPMENT, 0.6);

    risk_record_trend(risk);
    risk_record_trend(risk);
    risk_identify_top_risks(risk, 3);

    printf("\n--- Risk Status ---\n");
    risk_print(risk);
    risk_print_top_risks(risk);

    printf("\n--- Spiral Status ---\n");
    spiral_print(device);
    printf("Estimated remaining cost: %.0fK\n", spiral_estimate_remaining_cost(device));
    printf("Risk trend: %s\n", risk_is_improving(risk) ? "IMPROVING" :
           risk_is_worsening(risk) ? "WORSENING" : "STABLE");

    printf("\nKey insight: Risk drives iteration. Each cycle reduces uncertainty.\n");

    risk_free(risk);
    spiral_free(device);
    return 0;
}
