#include "hsm_core.h"
#include "hsm_morphology.h"
#include "hsm_knowledge.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("============================================\n");
    printf("  Hall Morphology: 3D Matrix Analysis\n");
    printf("============================================\n\n");

    HSMSystem* sys = hsm_create("TrafficSystem", "Intelligent traffic management");

    hsm_add_activity(sys, HSM_PHASE_PROGRAM_PLANNING, HSM_STEP_PROBLEM_DEFINITION, HSM_DISC_SYSTEMS_ENG, "Problem analysis");
    hsm_add_activity(sys, HSM_PHASE_PROJECT_PLANNING, HSM_STEP_ANALYSIS, HSM_DISC_MATH, "Mathematical modeling");
    hsm_add_activity(sys, HSM_PHASE_SYSTEM_DEVELOPMENT, HSM_STEP_SYNTHESIS, HSM_DISC_SOFTWARE, "Software architecture");

    printf("--- Morphology Matrix ---\n");
    hsm_print_morphology_matrix(sys);

    HSMMorphologyMetrics mm = hsm_morphology_compute(sys);
    hsm_morphology_print_metrics(&mm);

    HSMKnowledgeBase* kb = hsm_kb_create();
    hsm_kb_init_profile(kb, HSM_DISC_SYSTEMS_ENG, "INCOSE SE Handbook v4", "ISO 15288");
    hsm_kb_init_profile(kb, HSM_DISC_MATH, "Operations Research", "None");
    HSMTool* t1 = hsm_kb_add_tool(kb, HSM_DISC_SYSTEMS_ENG, "Requirements Analysis", "Capture needs", "Analysis");
    hsm_kb_set_tool_applicability(t1, HSM_PHASE_PROJECT_PLANNING, HSM_STEP_PROBLEM_DEFINITION, 1.0);
    HSMTool* t2 = hsm_kb_add_tool(kb, HSM_DISC_SYSTEMS_ENG, "FFBD", "Functional Flow Block Diagram", "Design");
    hsm_kb_set_tool_applicability(t2, HSM_PHASE_SYSTEM_DEVELOPMENT, HSM_STEP_SYNTHESIS, 1.0);
    HSMTool* t3 = hsm_kb_add_tool(kb, HSM_DISC_MATH, "Monte Carlo", "Uncertainty analysis", "Analysis");
    hsm_kb_set_tool_applicability(t3, HSM_PHASE_SYSTEM_DEVELOPMENT, HSM_STEP_ANALYSIS, 0.8);

    printf("\n--- Knowledge Base Coverage ---\n");
    hsm_kb_print_coverage_matrix(kb);
    printf("\n--- Discipline: Systems Engineering ---\n");
    hsm_kb_print_discipline(kb, HSM_DISC_SYSTEMS_ENG);

    int phases[10], steps[10];
    int n_crit = hsm_morphology_critical_cells(sys, phases, steps, 10);
    printf("\n--- Critical Cells (%d) ---\n", n_crit);
    for (int i = 0; i < n_crit; i++)
        printf("  %s / %s\n", hsm_phase_name((HSMPhase)phases[i]), hsm_step_name((HSMLogicStep)steps[i]));

    hsm_kb_free(kb);
    hsm_free(sys);
    printf("\n============================================\n");
    return 0;
}
