#include "hsm_core.h"
#include "hsm_time.h"
#include "hsm_morphology.h"
#include "hsm_trade.h"
#include "hsm_verification.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("============================================\n");
    printf("  Hall HSM: Satellite Systems Engineering\n");
    printf("============================================\n\n");

    HSMSystem* sat = hsm_create("CommSat", "Deploy LEO communication satellite");

    hsm_add_activity(sat, HSM_PHASE_PROGRAM_PLANNING, HSM_STEP_PROBLEM_DEFINITION, HSM_DISC_SYSTEMS_ENG, "Define coverage needs");
    hsm_add_activity(sat, HSM_PHASE_PROJECT_PLANNING, HSM_STEP_SYNTHESIS, HSM_DISC_ELECTRICAL, "Select frequency band");
    hsm_add_activity(sat, HSM_PHASE_SYSTEM_DEVELOPMENT, HSM_STEP_SYNTHESIS, HSM_DISC_SOFTWARE, "Design flight software");
    hsm_add_activity(sat, HSM_PHASE_SYSTEM_DEVELOPMENT, HSM_STEP_ANALYSIS, HSM_DISC_MECHANICAL, "Thermal analysis");
    hsm_add_activity(sat, HSM_PHASE_PRODUCTION, HSM_STEP_PLANNING_ACTION, HSM_DISC_MANAGEMENT, "Manufacturing schedule");
    hsm_add_activity(sat, HSM_PHASE_OPERATIONS, HSM_STEP_ANALYSIS, HSM_DISC_QUALITY, "Performance monitoring");

    printf("--- Morphology Matrix ---\n");
    hsm_print_morphology_matrix(sat);
    printf("\n--- Phase Report: System Development ---\n");
    hsm_print_phase_report(sat, HSM_PHASE_SYSTEM_DEVELOPMENT);

    HSMLifecycle* lc = hsm_lifecycle_create();
    hsm_set_phase_plan(lc, HSM_PHASE_PROGRAM_PLANNING, 0, 90, 200000, "Mission need", "Feasibility approved", "CONOPS");
    hsm_set_phase_plan(lc, HSM_PHASE_PROJECT_PLANNING, 90, 180, 500000, "CONOPS approved", "PDR passed", "Prelim design");
    hsm_set_phase_plan(lc, HSM_PHASE_SYSTEM_DEVELOPMENT, 180, 540, 2000000, "PDR passed", "CDR passed", "Detailed design");
    hsm_set_phase_plan(lc, HSM_PHASE_PRODUCTION, 540, 720, 5000000, "CDR passed", "TRR passed", "Satellite");
    hsm_set_phase_plan(lc, HSM_PHASE_DISTRIBUTION, 720, 780, 1000000, "TRR passed", "ORR passed", "Launch");
    hsm_set_phase_plan(lc, HSM_PHASE_OPERATIONS, 780, 1800, 3000000, "ORR passed", "EOL review", "Services");
    printf("\n--- Lifecycle ---\n");
    hsm_print_lifecycle(lc);
    hsm_lifecycle_free(lc);

    TradeStudy* ts = hsm_trade_create();
    hsm_trade_add_criterion(ts, "Data Rate", 0.4, true, 100, 1000);
    hsm_trade_add_criterion(ts, "Power (W)", 0.3, false, 0, 500);
    hsm_trade_add_criterion(ts, "Cost ($M)", 0.3, false, 0, 200);
    hsm_trade_add_alternative(ts, "Ka-Band", "High throughput", 150, 12, 0.2);
    hsm_trade_add_alternative(ts, "Ku-Band", "Medium throughput", 100, 10, 0.1);
    hsm_trade_add_alternative(ts, "S-Band", "Low throughput, robust", 80, 8, 0.05);
    double s0[] = {0.9, 0.6, 0.4};
    double s1[] = {0.6, 0.7, 0.6};
    double s2[] = {0.3, 0.9, 0.8};
    hsm_trade_set_scores(ts, 0, s0);
    hsm_trade_set_scores(ts, 1, s1);
    hsm_trade_set_scores(ts, 2, s2);
    hsm_trade_compute(ts);
    printf("\n--- Trade Study ---\n");
    hsm_trade_print(ts);
    printf("Best: "); hsm_trade_print_ranking(ts);
    hsm_trade_free(ts);

    printf("\n============================================\n");
    hsm_free(sat);
    return 0;
}
