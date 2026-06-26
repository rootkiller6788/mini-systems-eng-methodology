#ifndef HSM_CORE_H
#define HSM_CORE_H
#include <stdbool.h>
#include <stddef.h>

/* ============================================================================
 * Hard Systems Methodology (Hall, 1962/1969) — Core Types
 *
 * Arthur D. Hall's "A Methodology for Systems Engineering" defined a
 * three-dimensional morphology for systems engineering:
 *   - Time dimension: 7 lifecycle phases
 *   - Logic dimension: 7 problem-solving steps
 *   - Knowledge dimension: disciplines and tools
 *
 * The 3D morphology matrix has 7x7xN cells, each representing a specific
 * activity at the intersection of a phase, step, and discipline.
 * ============================================================================ */

/* --- Time Dimension: System Lifecycle Phases --- */
typedef enum {
    HSM_PHASE_PROGRAM_PLANNING = 0,   /* Needs analysis, feasibility */
    HSM_PHASE_PROJECT_PLANNING = 1,   /* Preliminary design, planning */
    HSM_PHASE_SYSTEM_DEVELOPMENT = 2, /* Detailed design, prototyping */
    HSM_PHASE_PRODUCTION = 3,         /* Manufacturing, construction */
    HSM_PHASE_DISTRIBUTION = 4,       /* Deployment, installation */
    HSM_PHASE_OPERATIONS = 5,         /* Operation, maintenance */
    HSM_PHASE_RETIREMENT = 6,         /* Decommissioning, disposal */
    HSM_NUM_PHASES = 7
} HSMPhase;

/* --- Logic Dimension: Problem-Solving Steps --- */
typedef enum {
    HSM_STEP_PROBLEM_DEFINITION = 0,  /* Define needs, environment, constraints */
    HSM_STEP_VALUE_SYSTEM = 1,        /* Select objectives, criteria, metrics */
    HSM_STEP_SYNTHESIS = 2,           /* Generate alternative solutions */
    HSM_STEP_ANALYSIS = 3,            /* Analyze alternatives, model, simulate */
    HSM_STEP_OPTIMIZATION = 4,        /* Optimize each alternative */
    HSM_STEP_DECISION_MAKING = 5,     /* Select best alternative */
    HSM_STEP_PLANNING_ACTION = 6,     /* Plan implementation, schedule */
    HSM_NUM_STEPS = 7
} HSMLogicStep;

/* --- Knowledge Dimension: Disciplines --- */
typedef enum {
    HSM_DISC_SYSTEMS_ENG = 0,
    HSM_DISC_ELECTRICAL = 1,
    HSM_DISC_MECHANICAL = 2,
    HSM_DISC_SOFTWARE = 3,
    HSM_DISC_MANAGEMENT = 4,
    HSM_DISC_MATH = 5,
    HSM_DISC_HUMAN_FACTORS = 6,
    HSM_DISC_SAFETY = 7,
    HSM_DISC_QUALITY = 8,
    HSM_NUM_DISCIPLINES = 9
} HSMDiscipline;

/* --- Activity Cell: One cell in the 3D morphology matrix --- */
typedef struct {
    HSMPhase phase;
    HSMLogicStep step;
    HSMDiscipline discipline;
    char* description;
    double progress;          /* 0.0 = not started, 1.0 = complete */
    double effort_estimate;   /* Person-hours estimated */
    double effort_actual;     /* Person-hours actually spent */
    bool is_critical;         /* On critical path */
    char* deliverables;
    char* tools_used;
} HSMActivity;

/* --- Requirements --- */
typedef struct {
    char* id;
    char* description;
    char* category;           /* Functional, Performance, Interface, etc. */
    double priority;          /* 0.0 = low, 1.0 = critical */
    bool is_verified;
    bool is_validated;
    char* verification_method;
    char* parent_id;          /* Parent requirement (for decomposition) */
} HSMRequirement;

/* --- System Model --- */
typedef struct {
    char* name;
    char* mission;
    HSMActivity** matrix;     /* [phase][step] array of activity lists */
    int* matrix_counts;       /* Number of activities per cell */
    HSMRequirement* requirements;
    int n_requirements;
    int req_capacity;
    double total_budget;
    double total_schedule;
    double risk_level;        /* 0.0 = safe, 1.0 = extreme risk */
} HSMSystem;

/* --- Core API --- */
HSMSystem* hsm_create(const char* name, const char* mission);
void hsm_free(HSMSystem* sys);

const char* hsm_phase_name(HSMPhase phase);
const char* hsm_step_name(HSMLogicStep step);
const char* hsm_discipline_name(HSMDiscipline disc);
const char* hsm_phase_description(HSMPhase phase);
const char* hsm_step_description(HSMLogicStep step);

/* Activity management */
HSMActivity* hsm_add_activity(HSMSystem* sys, HSMPhase phase,
                                HSMLogicStep step, HSMDiscipline disc,
                                const char* desc);
void hsm_set_activity_progress(HSMActivity* act, double progress);
void hsm_set_activity_effort(HSMActivity* act, double estimate, double actual);
double hsm_phase_completion(HSMSystem* sys, HSMPhase phase);
double hsm_step_completion(HSMSystem* sys, HSMLogicStep step);
double hsm_overall_completion(HSMSystem* sys);

/* Requirement management */
HSMRequirement* hsm_add_requirement(HSMSystem* sys, const char* id,
                                      const char* desc, const char* category,
                                      double priority);
void hsm_verify_requirement(HSMRequirement* req, bool status);
void hsm_validate_requirement(HSMRequirement* req, bool status);
double hsm_requirement_coverage(HSMSystem* sys);

void hsm_print_system(HSMSystem* sys);
void hsm_print_phase_report(HSMSystem* sys, HSMPhase phase);
void hsm_print_morphology_matrix(HSMSystem* sys);
#endif
