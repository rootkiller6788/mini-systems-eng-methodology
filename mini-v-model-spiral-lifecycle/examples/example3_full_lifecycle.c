#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "lifecycle_core.h"
#include "lifecycle_vmodel.h"
#include "lifecycle_spiral.h"
#include "lifecycle_risk.h"
#include "lifecycle_metrics.h"
#include "lifecycle_gates.h"

/* Example 3: Full Lifecycle Integration — Autonomous Vehicle
 * Demonstrates V-Model with spiral iterations, gates, EVM, and risk. */

int main(void) {
    printf("=== Example 3: Full Lifecycle — Autonomous Vehicle ===\n\n");

    VModelSystem* av = vm_create("AutonomousVehicle");
    SpiralSystem* spr = spiral_create("AV_Spiral", 4, 0.3);
    RiskManagementPlan* risk = risk_create("AV_Risk");
    LifecycleDashboard* dash = dash_create("AV_Dashboard");
    GateReviewSystem* gates = gates_create("AV_Gates");

    /* 1. Setup requirements */
    printf("--- 1. Requirements Traceability ---\n");
    lc_add_requirement(&av->base, "AV-001", "Detect pedestrians within 100m",
                       "Safety Analysis", V_LEVEL_REQUIREMENTS, V_VERIFY_SYSTEM, 1.0);
    lc_add_requirement(&av->base, "AV-002", "Emergency stop within 2s",
                       "Regulations", V_LEVEL_REQUIREMENTS, V_VERIFY_SYSTEM, 1.0);
    lc_add_requirement(&av->base, "AV-003", "Lane keeping on highways",
                       "Product Spec", V_LEVEL_ARCHITECTURE, V_VERIFY_INTEGRATION, 0.8);
    lc_add_trace(&av->base, "AV-001", "AV-003");
    printf("Requirements: %d  Traced: %.0f%%\n",
           av->base.n_requirements, 100.0 * vm_traceability_coverage(av));

    /* 2. V-Model progress */
    printf("\n--- 2. V-Model Progress ---\n");
    vm_set_level_completion(av, V_LEVEL_CONCEPT, 1.0);
    vm_set_level_completion(av, V_LEVEL_REQUIREMENTS, 1.0);
    vm_set_level_completion(av, V_LEVEL_ARCHITECTURE, 0.7);
    vm_set_verify_completion(av, V_VERIFY_UNIT, 0.4);
    vm_set_verify_completion(av, V_VERIFY_INTEGRATION, 0.2);
    vm_print(av);

    /* 3. Spiral cycles */
    printf("--- 3. Spiral Cycles ---\n");
    spiral_add_cycle(spr, "Concept", "Feasibility", "Budget $500K");
    spiral_set_cycle_progress(spr, 0, SPIRAL_OBJECTIVES, 1.0);
    spiral_set_cycle_progress(spr, 0, SPIRAL_RISK_ANALYSIS, 1.0);
    spiral_set_cycle_progress(spr, 0, SPIRAL_DEVELOPMENT, 1.0);
    spiral_set_cycle_progress(spr, 0, SPIRAL_PLANNING, 1.0);

    spiral_add_cycle(spr, "Prototype", "Sensor integration", "Must pass NCAP");
    spiral_set_cycle_progress(spr, 1, SPIRAL_OBJECTIVES, 1.0);
    spiral_set_cycle_progress(spr, 1, SPIRAL_DEVELOPMENT, 0.5);
    printf("Cycles: %d  Progress angle: %.0f%%\n",
           spr->n_cycles, 100.0 * spiral_progress_by_angle(spr));

    /* 4. Risk management */
    printf("\n--- 4. Risk Management ---\n");
    risk_add_risk(risk, "R-A", "Sensor fusion failure in rain",
                  RISK_HIGH, RISK_CRITICAL, "Redundant sensors + weather testing", "False positive rate");
    risk_add_risk(risk, "R-B", "Regulatory certification delay",
                  RISK_MODERATE, RISK_HIGH, "Early engagement with NHTSA", "6-month delay");
    risk_add_risk(risk, "R-C", "Software regression after update",
                  RISK_LOW, RISK_MODERATE, "CI/CD + regression test suite", "Test failures");
    risk_identify_top_risks(risk, 3);
    risk_apply_mitigation(risk, "R-A", 50.0);
    risk_record_trend(risk);
    risk_print_top_risks(risk);

    /* 5. EVM Dashboard */
    printf("\n--- 5. Earned Value ---\n");
    dash_set_budget(dash, 5000.0);
    dash_record_progress(dash, 2000.0, 1800.0, 2100.0);
    dash_add_measure(dash, "MTBF", 10000.0, 8000.0, 12000.0);
    dash_update_measure(dash, 0, 9500.0);
    dash_record_defect(dash, false); dash_record_defect(dash, true);
    dash_print(dash);
    dash_print_evm(dash);
    printf("Health: %s\n", dash_health_assessment(dash));

    /* 6. Gates */
    printf("\n--- 6. Stage Gates ---\n");
    TechnicalReview* sr = gates_configure_review(gates, REVIEW_SRR, STAGE_CONCEPT);
    gates_add_entry_criterion(sr, "System requirements documented");
    gates_add_action_item(sr, "Clarify ODD definition", FINDING_MINOR, "Systems Team", 3.0);
    gates_resolve_action_item(sr, 0, "ODD defined as urban + highway");
    gates_conduct_review(gates, REVIEW_SRR, GATE_GO, "Requirements are complete and clear");
    gates_configure_gate(gates, STAGE_CONCEPT, STAGE_PRELIMINARY);
    gates_pass_gate(gates, STAGE_CONCEPT);
    gates_print(gates);

    /* Cleanup */
    gates_free(gates);
    dash_free(dash);
    risk_free(risk);
    spiral_free(spr);
    vm_free(av);

    printf("\nKey insight: Modern SE integrates V-Model structure with spiral iteration,\n");
    printf("risk-driven decisions, earned value tracking, and formal stage gates.\n");
    return 0;
}
