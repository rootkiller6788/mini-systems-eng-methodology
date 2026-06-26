#include "hsm_core.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Phase names and descriptions */
const char* hsm_phase_name(HSMPhase phase) {
    switch (phase) {
        case HSM_PHASE_PROGRAM_PLANNING: return "Program Planning";
        case HSM_PHASE_PROJECT_PLANNING: return "Project Planning";
        case HSM_PHASE_SYSTEM_DEVELOPMENT: return "System Development";
        case HSM_PHASE_PRODUCTION: return "Production";
        case HSM_PHASE_DISTRIBUTION: return "Distribution";
        case HSM_PHASE_OPERATIONS: return "Operations";
        case HSM_PHASE_RETIREMENT: return "Retirement";
        default: return "Unknown";
    }
}
const char* hsm_step_name(HSMLogicStep step) {
    switch (step) {
        case HSM_STEP_PROBLEM_DEFINITION: return "Problem Definition";
        case HSM_STEP_VALUE_SYSTEM: return "Value System Design";
        case HSM_STEP_SYNTHESIS: return "Systems Synthesis";
        case HSM_STEP_ANALYSIS: return "Systems Analysis";
        case HSM_STEP_OPTIMIZATION: return "Optimization";
        case HSM_STEP_DECISION_MAKING: return "Decision Making";
        case HSM_STEP_PLANNING_ACTION: return "Planning for Action";
        default: return "Unknown";
    }
}
const char* hsm_discipline_name(HSMDiscipline disc) {
    switch (disc) {
        case HSM_DISC_SYSTEMS_ENG: return "Systems Engineering";
        case HSM_DISC_ELECTRICAL: return "Electrical Engineering";
        case HSM_DISC_MECHANICAL: return "Mechanical Engineering";
        case HSM_DISC_SOFTWARE: return "Software Engineering";
        case HSM_DISC_MANAGEMENT: return "Project Management";
        case HSM_DISC_MATH: return "Mathematics/OR";
        case HSM_DISC_HUMAN_FACTORS: return "Human Factors";
        case HSM_DISC_SAFETY: return "Safety Engineering";
        case HSM_DISC_QUALITY: return "Quality Assurance";
        default: return "Unknown";
    }
}
const char* hsm_phase_description(HSMPhase phase) {
    switch (phase) {
        case HSM_PHASE_PROGRAM_PLANNING: return "Needs analysis, feasibility study, policy formulation";
        case HSM_PHASE_PROJECT_PLANNING: return "Preliminary design, resource allocation, project planning";
        case HSM_PHASE_SYSTEM_DEVELOPMENT: return "Detailed design, prototyping, integration, testing";
        case HSM_PHASE_PRODUCTION: return "Manufacturing, construction, quality control";
        case HSM_PHASE_DISTRIBUTION: return "Deployment, installation, commissioning, training";
        case HSM_PHASE_OPERATIONS: return "Operation, maintenance, performance monitoring, support";
        case HSM_PHASE_RETIREMENT: return "Decommissioning, disposal, lessons learned, archiving";
        default: return "";
    }
}
const char* hsm_step_description(HSMLogicStep step) {
    switch (step) {
        case HSM_STEP_PROBLEM_DEFINITION: return "Define needs, environment, constraints, stakeholders";
        case HSM_STEP_VALUE_SYSTEM: return "Select objectives, criteria, metrics, weights";
        case HSM_STEP_SYNTHESIS: return "Generate alternative solutions, brainstorm, research";
        case HSM_STEP_ANALYSIS: return "Model, simulate, evaluate alternatives; sensitivity analysis";
        case HSM_STEP_OPTIMIZATION: return "Refine each alternative to best performance level";
        case HSM_STEP_DECISION_MAKING: return "Multi-criteria selection of best alternative";
        case HSM_STEP_PLANNING_ACTION: return "Implementation plan, schedule, resources, risk mitigation";
        default: return "";
    }
}

/* System lifecycle */
HSMSystem* hsm_create(const char* name, const char* mission) {
    HSMSystem* sys = calloc(1, sizeof(HSMSystem));
    if (!sys) return NULL;
    sys->name = strdup(name);
    sys->mission = strdup(mission);
    sys->matrix = calloc(HSM_NUM_PHASES, sizeof(HSMActivity*));
    sys->matrix_counts = calloc(HSM_NUM_PHASES * HSM_NUM_STEPS, sizeof(int));
    sys->req_capacity = 64;
    sys->requirements = malloc(64 * sizeof(HSMRequirement));
    return sys;
}
void hsm_free(HSMSystem* sys) {
    if (!sys) return;
    free(sys->name); free(sys->mission);
    if (sys->matrix) {
        for (int p = 0; p < HSM_NUM_PHASES; p++) {
            if (sys->matrix[p]) {
                for (int s = 0; s < HSM_NUM_STEPS; s++) {
                    int idx = p * HSM_NUM_STEPS + s;
                    int base = 0;
                    for (int pp = 0; pp < p; pp++) base += sys->matrix_counts[pp * HSM_NUM_STEPS + s];
                    for (int a = 0; a < sys->matrix_counts[idx]; a++) {
                        free(sys->matrix[p][base + a].description);
                        free(sys->matrix[p][base + a].deliverables);
                        free(sys->matrix[p][base + a].tools_used);
                    }
                }
                free(sys->matrix[p]);
            }
        }
        free(sys->matrix);
    }
    free(sys->matrix_counts);
    for (int i = 0; i < sys->n_requirements; i++) {
        free(sys->requirements[i].id);
        free(sys->requirements[i].description);
        free(sys->requirements[i].category);
        free(sys->requirements[i].verification_method);
        free(sys->requirements[i].parent_id);
    }
    free(sys->requirements);
    free(sys);
}

/* Activity management */
HSMActivity* hsm_add_activity(HSMSystem* sys, HSMPhase phase, HSMLogicStep step,
                                HSMDiscipline disc, const char* desc) {
    if (!sys || !desc) return NULL;
    int cell = phase * HSM_NUM_STEPS + step;
    int n = sys->matrix_counts[cell];
    /* Expand activity array for this cell */
    int new_size = (n + 1) * sizeof(HSMActivity);
    if (!sys->matrix[phase]) {
        sys->matrix[phase] = calloc(HSM_NUM_STEPS, sizeof(HSMActivity));
        /* Need flat storage per cell */
        free(sys->matrix[phase]);
        sys->matrix[phase] = NULL;
    }
    /* Simple realloc-based approach */
    HSMActivity* new_arr = realloc(sys->matrix[phase], (n + 1) * HSM_NUM_STEPS * sizeof(HSMActivity)
                                     > 0 ? (n + 1) * sizeof(HSMActivity) : 0);
    if (!new_arr && n > 0) return NULL;
    sys->matrix[phase] = new_arr ? new_arr : malloc(sizeof(HSMActivity));
    HSMActivity* act = &sys->matrix[phase][n];
    memset(act, 0, sizeof(HSMActivity));
    act->phase = phase; act->step = step; act->discipline = disc;
    act->description = strdup(desc);
    sys->matrix_counts[cell]++;
    return act;
}
void hsm_set_activity_progress(HSMActivity* act, double p) { if (act) { if (p < 0) p = 0; if (p > 1) p = 1; act->progress = p; } }
void hsm_set_activity_effort(HSMActivity* act, double est, double actual) { if (act) { act->effort_estimate = est; act->effort_actual = actual; } }

double hsm_phase_completion(HSMSystem* sys, HSMPhase phase) {
    if (!sys) return 0.0;
    double total = 0.0; int count = 0;
    for (int s = 0; s < HSM_NUM_STEPS; s++) {
        int cell = phase * HSM_NUM_STEPS + s;
        for (int a = 0; a < sys->matrix_counts[cell]; a++) {
            total += sys->matrix[phase][a].progress; count++;
        }
    }
    return count > 0 ? total / count : 0.0;
}
double hsm_step_completion(HSMSystem* sys, HSMLogicStep step) {
    if (!sys) return 0.0;
    double total = 0.0; int count = 0;
    for (int p = 0; p < HSM_NUM_PHASES; p++) {
        int cell = p * HSM_NUM_STEPS + step;
        for (int a = 0; a < sys->matrix_counts[cell]; a++) {
            total += sys->matrix[p][a].progress; count++;
        }
    }
    return count > 0 ? total / count : 0.0;
}
double hsm_overall_completion(HSMSystem* sys) {
    if (!sys) return 0.0;
    double total = 0.0; int count = 0;
    for (int p = 0; p < HSM_NUM_PHASES; p++)
        for (int s = 0; s < HSM_NUM_STEPS; s++) {
            int cell = p * HSM_NUM_STEPS + s;
            for (int a = 0; a < sys->matrix_counts[cell]; a++) {
                total += sys->matrix[p][a].progress; count++;
            }
        }
    return count > 0 ? total / count : 0.0;
}
/* Requirements */
HSMRequirement* hsm_add_requirement(HSMSystem* sys, const char* id, const char* desc, const char* cat, double pri) {
    if (!sys || !id || !desc) return NULL;
    if (sys->n_requirements >= sys->req_capacity) { sys->req_capacity *= 2; sys->requirements = realloc(sys->requirements, sys->req_capacity * sizeof(HSMRequirement)); }
    HSMRequirement* r = &sys->requirements[sys->n_requirements++];
    memset(r, 0, sizeof(HSMRequirement));
    r->id = strdup(id); r->description = strdup(desc); r->category = strdup(cat); r->priority = pri;
    return r;
}
void hsm_verify_requirement(HSMRequirement* r, bool s) { if (r) r->is_verified = s; }
void hsm_validate_requirement(HSMRequirement* r, bool s) { if (r) r->is_validated = s; }
double hsm_requirement_coverage(HSMSystem* sys) {
    if (!sys || sys->n_requirements == 0) return 0.0;
    int v = 0; for (int i = 0; i < sys->n_requirements; i++) if (sys->requirements[i].is_verified) v++;
    return (double)v / sys->n_requirements;
}
void hsm_print_system(HSMSystem* sys) {
    if (!sys) return;
    printf("=== HSM System: %s ===\n", sys->name);
    printf("Mission: %s\n", sys->mission);
    printf("Completion: %.1f%%\n", hsm_overall_completion(sys) * 100.0);
    printf("Requirements: %d (%.1f%% verified)\n", sys->n_requirements, hsm_requirement_coverage(sys) * 100.0);
}
void hsm_print_phase_report(HSMSystem* sys, HSMPhase phase) {
    if (!sys) return;
    printf("=== Phase: %s (%.1f%% complete) ===\n", hsm_phase_name(phase), hsm_phase_completion(sys, phase) * 100.0);
    for (int s = 0; s < HSM_NUM_STEPS; s++) {
        int cell = phase * HSM_NUM_STEPS + s;
        printf("  Step %d (%s): %d activities\n", s + 1, hsm_step_name(s), sys->matrix_counts[cell]);
        for (int a = 0; a < sys->matrix_counts[cell]; a++) {
            HSMActivity* act = &sys->matrix[phase][a];
            printf("    [%s] %.0f%% %s\n", hsm_discipline_name(act->discipline), act->progress * 100.0, act->description);
        }
    }
}
void hsm_print_morphology_matrix(HSMSystem* sys) {
    if (!sys) return;
    printf("=== HSM Morphology Matrix (Phase x Step) ===\n");
    printf("%-22s", "Phase/Step");
    for (int s = 0; s < HSM_NUM_STEPS; s++) printf(" S%d", s + 1);
    printf("\n");
    for (int p = 0; p < HSM_NUM_PHASES; p++) {
        printf("%-22s", hsm_phase_name(p));
        for (int s = 0; s < HSM_NUM_STEPS; s++) {
            int cell = p * HSM_NUM_STEPS + s;
            printf(" %2d", sys->matrix_counts[cell]);
        }
        printf("\n");
    }
}




