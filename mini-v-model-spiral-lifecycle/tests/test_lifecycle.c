#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "lifecycle_core.h"
#include "lifecycle_vmodel.h"
#include "lifecycle_spiral.h"
#include "lifecycle_risk.h"
#include "lifecycle_metrics.h"
#include "lifecycle_gates.h"
#define EPS 1e-9

int main(void) {
    printf("=== V-Model & Spiral Lifecycle Tests ===\n");

    LifecycleSystem* lc = lc_create("TestProj", LC_MODEL_V);
    assert(lc); assert(lc->current_stage == STAGE_PRE_CONCEPT);
    lc_add_requirement(lc, "R1", "Sys operate", "Stake", V_LEVEL_REQUIREMENTS, V_VERIFY_SYSTEM, 1.0);
    lc_add_requirement(lc, "R2", "Sys safe", "Safety", V_LEVEL_ARCHITECTURE, V_VERIFY_SYSTEM, 0.9);
    assert(lc->n_requirements == 2);
    lc_add_trace(lc, "R1", "R2"); lc_verify_requirement(lc, "R1", true);
    double cov = lc_requirement_verification_coverage(lc); assert(cov == 0.5);
    lc_add_deliverable(lc, "SRS", "Req Spec", STAGE_CONCEPT, "Doc");
    lc_approve_deliverable(lc, "SRS");
    assert(lc_deliverable_completion(lc, STAGE_CONCEPT) == 1.0);
    assert(strcmp(lc_stage_name(STAGE_PRODUCTION), "Production") == 0);
    assert(lc_earned_value(lc, 100.0) == 0.0);
    lc_free(lc);

    VModelSystem* vm = vm_create("SatSys"); assert(vm);
    vm_set_level_completion(vm, V_LEVEL_REQUIREMENTS, 1.0);
    assert(fabs(vm_get_level_completion(vm, V_LEVEL_REQUIREMENTS) - 1.0) < EPS);
    vm_set_verify_completion(vm, V_VERIFY_SYSTEM, 0.8);
    assert(vm_corresponding_verify(V_LEVEL_CONCEPT) == V_VERIFY_ACCEPTANCE);
    double sym = vm_symmetry_index(vm); assert(sym >= 0 && sym <= 1);
    assert(vm_overall_progress(vm) >= 0);
    vm_complete_review(vm, "SRR_PASSED"); assert(vm_is_review_passed(vm, "SRR") == true);
    vm_complete_review(vm, "CDR_COMPLETE"); assert(vm->n_baselines_established == 2);
    double tc = vm_traceability_coverage(vm); assert(tc >= 0);
    vm_free(vm);

    SpiralSystem* sp = spiral_create("MedDevice", 5, 0.3); assert(sp); assert(sp->max_cycles == 5);
    SpiralCycle* c0 = spiral_add_cycle(sp, "Concept", "Feasibility", "Budget");
    assert(c0); assert(sp->n_cycles == 1);
    spiral_set_cycle_progress(sp, 0, SPIRAL_OBJECTIVES, 1.0);
    spiral_set_cycle_progress(sp, 0, SPIRAL_RISK_ANALYSIS, 0.8);
    assert(spiral_should_prototype(sp, 0.5) == true);
    assert(spiral_should_continue(sp) == true);
    spiral_plan_next_cycle(sp, "Build prototype", 50.0);
    double rem = spiral_estimate_remaining_cycles(sp); assert(rem > 0);
    spiral_free(sp);

    RiskManagementPlan* rmp = risk_create("RiskPlan"); assert(rmp);
    RiskItem* ri = risk_add_risk(rmp, "R1", "Budget overrun", RISK_HIGH, RISK_MODERATE, "Contingency", "Burn rate");
    assert(ri); assert(rmp->base.n_risks == 1);
    double exp = risk_compute_exposure(RISK_HIGH, RISK_MODERATE); assert(exp > 0);
    risk_identify_top_risks(rmp, 1); assert(rmp->n_top_risks == 1);
    risk_apply_mitigation(rmp, "R1", 10.0); assert(rmp->risk_budget_spent == 10.0);
    risk_set_budget(rmp, 50.0); assert(fabs(rmp->risk_budget_remaining - 40.0) < EPS);
    assert(risk_residual_exposure(rmp) >= 0);
    risk_free(rmp);

    LifecycleDashboard* dash = dash_create("Dash1"); assert(dash);
    dash_set_budget(dash, 1000.0); dash_record_progress(dash, 500.0, 450.0, 480.0);
    assert(dash_spi(dash) < 1.0); assert(dash_cpi(dash) < 1.0);
    dash_add_measure(dash, "Mass", 50.0, 50.0, 45.0);
    dash_update_measure(dash, 0, 52.0); assert(dash->measures[0].is_met == true);
    dash_record_defect(dash, false); dash_record_defect(dash, true);
    assert(dash->quality.n_defects_total == 2);
    double h = dash_health_score(dash); assert(h >= 0 && h <= 1);
    dash_free(dash);

    GateReviewSystem* grs = gates_create("GateSys"); assert(grs);
    TechnicalReview* rev = gates_configure_review(grs, REVIEW_SRR, STAGE_CONCEPT); assert(rev);
    gates_add_entry_criterion(rev, "Stakeholder needs documented");
    gates_add_action_item(rev, "Update CONOPS", FINDING_MAJOR, "Alice", 5.0);
    gates_resolve_action_item(rev, 0, "Updated v2"); assert(rev->action_items[0].is_resolved);
    gates_conduct_review(grs, REVIEW_SRR, GATE_CONDITIONAL, "Proceed");
    assert(gates_review_passed(grs, REVIEW_SRR));
    gates_configure_gate(grs, STAGE_CONCEPT, STAGE_PRELIMINARY);
    gates_pass_gate(grs, STAGE_CONCEPT); assert(grs->n_gates_passed == 1);
    assert(grs->base.current_stage == STAGE_PRELIMINARY);
    gates_free(grs);

    printf("All tests passed.\n");
    return 0;
}
