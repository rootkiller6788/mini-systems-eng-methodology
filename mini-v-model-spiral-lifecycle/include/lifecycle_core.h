#ifndef LIFECYCLE_CORE_H
#define LIFECYCLE_CORE_H

#include <stdbool.h>
#include <stddef.h>

/* ============================================================================
 * Systems Engineering Lifecycle Models ? Core Types
 *
 * V-Model (Forsberg & Mooz, 1991):
 *   Decomposition (left side) ? Implementation (bottom) ? Integration (right side)
 *   Each decomposition level has a corresponding verification level.
 *
 * Spiral Model (Boehm, 1988):
 *   Iterative risk-driven development in cycles.
 *   Each cycle: Objectives ? Risk Analysis ? Development ? Planning.
 *
 * Based on:
 *   Forsberg, K. & Mooz, H. ? "Application of the Vee Model" (1991)
 *   Boehm, B.W. ? "A Spiral Model of Software Development" (1988)
 *   INCOSE ? Systems Engineering Handbook
 *   ISO/IEC 15288 ? System Life Cycle Processes
 * ============================================================================ */

/* --- Lifecycle Model Types --- */
typedef enum {
    LC_MODEL_V = 0,
    LC_MODEL_SPIRAL = 1,
    LC_MODEL_HYBRID = 2,        /* V-Model with spiral iterations */
    LC_MODEL_AGILE = 3,
    LC_MODEL_WATERFALL = 4,
    LC_MODEL_INCREMENTAL = 5
} LifecycleModelType;

/* --- V-Model Decomposition Levels (Left Side) --- */
typedef enum {
    V_LEVEL_CONCEPT = 0,         /* Stakeholder needs, concept of operations */
    V_LEVEL_REQUIREMENTS = 1,    /* System requirements specification */
    V_LEVEL_ARCHITECTURE = 2,    /* System architecture, functional allocation */
    V_LEVEL_DESIGN = 3,          /* Detailed design, component specification */
    V_LEVEL_IMPLEMENTATION = 4,  /* Coding, fabrication, procurement */
    V_NUM_LEVELS = 5
} VModelLevel;

/* --- V-Model Verification Levels (Right Side) --- */
typedef enum {
    V_VERIFY_UNIT = 0,           /* Unit/component testing */
    V_VERIFY_INTEGRATION = 1,    /* Integration testing */
    V_VERIFY_SYSTEM = 2,         /* System verification */
    V_VERIFY_ACCEPTANCE = 3,     /* Acceptance testing, validation */
    V_VERIFY_OPERATION = 4,      /* Operational test & evaluation */
    V_NUM_VERIFY = 5
} VModelVerifyLevel;

/* --- Spiral Model Cycle Phases --- */
typedef enum {
    SPIRAL_OBJECTIVES = 0,       /* Determine objectives, alternatives, constraints */
    SPIRAL_RISK_ANALYSIS = 1,    /* Evaluate alternatives, identify/ resolve risks */
    SPIRAL_DEVELOPMENT = 2,      /* Develop and verify next-level product */
    SPIRAL_PLANNING = 3,         /* Plan next cycle, commit to next phase */
    SPIRAL_NUM_PHASES = 4
} SpiralPhase;

/* --- Lifecycle Stage --- */
typedef enum {
    STAGE_PRE_CONCEPT = 0,       /* Pre-phase A: Conceptual studies */
    STAGE_CONCEPT = 1,           /* Phase A: Concept & technology development */
    STAGE_PRELIMINARY = 2,       /* Phase B: Preliminary design */
    STAGE_DETAILED = 3,          /* Phase C: Detailed design & development */
    STAGE_PRODUCTION = 4,        /* Phase D: Production & deployment */
    STAGE_OPERATIONS = 5,        /* Phase E: Operations & sustainment */
    STAGE_DISPOSAL = 6,          /* Phase F: Disposal/decommissioning */
    STAGE_NUM = 7
} LifecycleStage;

/* --- Stage Gate Decision --- */
typedef enum {
    GATE_GO = 0,                 /* Proceed to next stage */
    GATE_NO_GO = 1,              /* Do not proceed */
    GATE_RECYCLE = 2,            /* Repeat current stage with changes */
    GATE_CONDITIONAL = 3         /* Proceed with conditions */
} GateDecision;

/* --- Risk Level --- */
typedef enum {
    RISK_NEGLIGIBLE = 0,
    RISK_LOW = 1,
    RISK_MODERATE = 2,
    RISK_HIGH = 3,
    RISK_CRITICAL = 4,
    RISK_CATASTROPHIC = 5
} RiskLevel;

/* --- Risk Item --- */
typedef struct {
    char*       id;
    char*       description;
    RiskLevel   probability;      /* Likelihood of occurrence */
    RiskLevel   impact;           /* Consequence severity */
    double      risk_exposure;    /* probability * impact (numeric) */
    char*       mitigation;
    char*       trigger;          /* Early warning indicator */
    bool        is_resolved;
    bool        is_accepted;      /* Risk accepted as-is */
    double      mitigation_cost;
    double      residual_exposure; /* After mitigation */
} RiskItem;

/* --- Requirement with Traceability --- */
typedef struct {
    char*       id;
    char*       description;
    char*       source;           /* Where requirement originates */
    VModelLevel allocated_to;     /* Which V-level owns this requirement */
    char**      traces_to;        /* Forward traceability (children) */
    int         n_traces_to;
    char**      traces_from;      /* Backward traceability (parents) */
    int         n_traces_from;
    bool        is_verified;      /* Verification completed */
    bool        is_validated;     /* Validation completed */
    double      priority;
    char*       verification_method;
    VModelVerifyLevel verify_at;  /* At what level is this verified */
} TraceableRequirement;

/* --- Deliverable / Work Product --- */
typedef struct {
    char*       name;
    char*       description;
    LifecycleStage produced_in;
    char*       format;           /* Document, Model, Code, Hardware, etc. */
    bool        is_delivered;
    bool        is_approved;
    double      completion_pct;
    char**      reviewed_by;
    int         n_reviewers;
} Deliverable;

/* --- Generic Lifecycle System --- */
typedef struct {
    char*               name;
    LifecycleModelType  model_type;
    LifecycleStage      current_stage;
    double              overall_progress;    /* 0.0 to 1.0 */
    double              total_budget;
    double              spent_budget;
    double              total_schedule_days;
    double              elapsed_days;

    /* Requirements */
    TraceableRequirement* requirements;
    int                 n_requirements;
    int                 req_capacity;

    /* Risks */
    RiskItem*           risks;
    int                 n_risks;
    int                 risk_capacity;

    /* Deliverables */
    Deliverable*        deliverables;
    int                 n_deliverables;
    int                 deliv_capacity;

    /* Risk metrics */
    double              total_risk_exposure;
    double              residual_risk_exposure;
    RiskLevel           overall_risk_level;

    /* V&V metrics */
    double              verification_coverage;  /* % requirements verified */
    double              validation_coverage;
    int                 n_defects_found;
    int                 n_defects_resolved;
} LifecycleSystem;

/* --- Core API --- */
LifecycleSystem* lc_create(const char* name, LifecycleModelType model);
void lc_free(LifecycleSystem* lc);
void lc_cleanup(LifecycleSystem* lc);  /* Clean internals, don't free shell */

/* Requirement management */
TraceableRequirement* lc_add_requirement(LifecycleSystem* lc, const char* id,
    const char* desc, const char* source, VModelLevel level,
    VModelVerifyLevel verify_at, double priority);
void lc_add_trace(LifecycleSystem* lc, const char* from_id, const char* to_id);
void lc_verify_requirement(LifecycleSystem* lc, const char* id, bool status);
void lc_validate_requirement(LifecycleSystem* lc, const char* id, bool status);
double lc_requirement_verification_coverage(LifecycleSystem* lc);

/* Deliverable management */
Deliverable* lc_add_deliverable(LifecycleSystem* lc, const char* name,
    const char* desc, LifecycleStage stage, const char* format);
void lc_approve_deliverable(LifecycleSystem* lc, const char* name);
double lc_deliverable_completion(LifecycleSystem* lc, LifecycleStage stage);

/* Status */
const char* lc_stage_name(LifecycleStage stage);
const char* lc_vlevel_name(VModelLevel level);
const char* lc_vverify_name(VModelVerifyLevel level);
const char* lc_spiral_phase_name(SpiralPhase phase);
const char* lc_model_name(LifecycleModelType model);
double lc_earned_value(LifecycleSystem* lc, double planned_value);
void lc_print_status(LifecycleSystem* lc);

#endif
