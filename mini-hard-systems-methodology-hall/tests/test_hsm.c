#include "hsm_core.h"
#include "hsm_time.h"
#include "hsm_logic.h"
#include "hsm_morphology.h"
#include "hsm_requirements.h"
#include "hsm_trade.h"
#include "hsm_verification.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <math.h>

// removed - using standard assert()
#define T(n) printf("  %s... ",n)

int main(void) {
    printf("=== Test Suite: Hard Systems Methodology (Hall) ===\n\n");

    T("hsm_create"); HSMSystem* s = hsm_create("TestSat", "Deploy broadband"); assert(s!=NULL);
    T("phase_name"); assert(strlen(hsm_phase_name(HSM_PHASE_SYSTEM_DEVELOPMENT)) > 0);
    T("step_name"); assert(strlen(hsm_step_name(HSM_STEP_OPTIMIZATION)) > 0);
    T("discipline_name"); assert(strlen(hsm_discipline_name(HSM_DISC_SOFTWARE)) > 0);

    T("add_activity"); HSMActivity* a = hsm_add_activity(s, HSM_PHASE_SYSTEM_DEVELOPMENT, HSM_STEP_SYNTHESIS, HSM_DISC_SOFTWARE, "Architecture design"); assert(a!=NULL);
    T("activity_progress"); hsm_set_activity_progress(a, 0.5); assert(a->progress == 0.5);
    T("activity_effort"); hsm_set_activity_effort(a, 100.0, 120.0); assert(a->effort_estimate == 100.0);
    T("phase_completion"); double pc = hsm_phase_completion(s, HSM_PHASE_SYSTEM_DEVELOPMENT); assert(pc > 0);
    T("overall_completion"); double oc = hsm_overall_completion(s); assert(oc >= 0 && oc <= 1.0);

    T("add_requirement"); HSMRequirement* r = hsm_add_requirement(s, "REQ-001", "Shall provide >100Mbps", "Performance", 0.9); assert(r!=NULL);
    T("verify_req"); hsm_verify_requirement(r, true); assert(r->is_verified);
    T("req_coverage"); double rc = hsm_requirement_coverage(s); assert(rc > 0);

    /* Lifecycle */
    T("lifecycle_create"); HSMLifecycle* lc = hsm_lifecycle_create(); assert(lc!=NULL);
    T("set_phase_plan"); hsm_set_phase_plan(lc, HSM_PHASE_SYSTEM_DEVELOPMENT, 90, 180, 500000, "SRR complete", "CDR passed", "Prototype"); assert(lc->phases[HSM_PHASE_SYSTEM_DEVELOPMENT].budget_allocated == 500000);
    T("current_phase"); int cp = hsm_current_phase(lc, 120); assert(cp == HSM_PHASE_SYSTEM_DEVELOPMENT);
    T("budget_burned"); double bb = hsm_lifecycle_budget_burned(lc); assert(bb >= 0);
    hsm_lifecycle_free(lc);

    /* Logic */
    T("logic_create"); HSMLogicActivity* la = hsm_logic_create(HSM_STEP_DECISION_MAKING); assert(la!=NULL);
    T("add_alternative"); hsm_logic_add_alternative(la, "Option A", 100, 200, 0.1); hsm_logic_add_alternative(la, "Option B", 150, 250, 0.2); assert(la->n_alternatives == 2);
    T("add_criterion"); hsm_logic_add_criterion(la, "Cost", 0.4); hsm_logic_add_criterion(la, "Performance", 0.6); assert(la->n_criteria == 2);
    T("logic_select"); int best = hsm_logic_select_best(la); assert(best >= 0);
    hsm_logic_free(la);

    /* Decision matrix */
    T("dm_create"); double w[]={0.3,0.7}; HSMDecisionMatrix* dm = hsm_dm_create(3, 2, w); assert(dm!=NULL);
    T("dm_compute"); hsm_dm_set_score(dm,0,0,0.8);hsm_dm_set_score(dm,0,1,0.6);hsm_dm_set_score(dm,1,0,0.5);hsm_dm_set_score(dm,1,1,0.9);hsm_dm_set_score(dm,2,0,0.9);hsm_dm_set_score(dm,2,1,0.4); hsm_dm_compute_ranking(dm); assert(hsm_dm_best(dm) >= 0);
    hsm_dm_free(dm);

    /* Morphology */
    T("morphology_compute"); HSMMorphologyMetrics mm = hsm_morphology_compute(s); assert(mm.overall_completion >= 0);
    T("morphology_count"); int na = hsm_morphology_count_activities(s); assert(na > 0);
    T("morphology_completed"); int nc = hsm_morphology_count_completed(s); assert(nc >= 0);

    /* Trade study */
    T("trade_create"); TradeStudy* ts = hsm_trade_create(); assert(ts!=NULL);
    T("trade_add_criterion"); hsm_trade_add_criterion(ts, "Speed", 0.5, true, 10, 100); assert(ts->n_criteria == 1);
    T("trade_add_alt"); hsm_trade_add_alternative(ts, "Design1", "Fast cheap", 100, 12, 0.1); hsm_trade_add_alternative(ts, "Design2", "Slow expensive", 200, 24, 0.3); assert(ts->n_alternatives == 2);
    T("trade_score"); hsm_trade_set_score(ts, 0, 0, 0.9); hsm_trade_set_score(ts, 1, 0, 0.5); hsm_trade_compute(ts); assert(hsm_trade_best(ts) == 0);
    T("trade_dominance"); double dom = hsm_trade_dominance(ts, 0, 1); assert(dom >= -1 && dom <= 1);
    T("npv"); double npv = hsm_net_present_value((double[]){-100,30,40,50}, 4, 0.1); assert(npv > -200);
    T("roi"); double roi = hsm_return_on_investment(150, 100); assert(fabs(roi - 0.5) < 0.01);
    hsm_trade_free(ts);

    /* V&V */
    T("vv_create"); HSMVVFramework* vv = hsm_vv_create(); assert(vv!=NULL);
    T("vv_init_review"); hsm_vv_init_review(vv, REVIEW_CDR, HSM_PHASE_SYSTEM_DEVELOPMENT, "PDR passed", "Design baselined"); assert(vv->reviews[REVIEW_CDR].entry_criteria != NULL);
    T("vv_action_item"); int ai = hsm_vv_add_action_item(vv, "Fix thermal issue", "John", 30); assert(ai >= 0);
    T("vv_close_item"); hsm_vv_close_action_item(vv, 0); assert(vv->action_items[0].resolved);
    T("vv_progress"); double vp = hsm_vv_progress(vv); assert(vp > 0);
    hsm_vv_free(vv);

    /* Requirements */
    T("req_traceability"); double ti = hsm_req_traceability_index(s); assert(ti >= 0);

    /* Verification record */
    T("verify_create"); HSMVerificationRecord* vr = hsm_verify_create_record(VERIFY_TEST, "Speed test", 100.0); assert(vr!=NULL);
    T("verify_execute"); hsm_verify_execute(vr, 120.0); assert(vr->passed);
    hsm_verify_free_record(vr);

    hsm_free(s);
    printf("\n=== All asserts passed ===\n"); return 0;
}
