#ifndef SSM_TYPES_H
#define SSM_TYPES_H

#include <stdbool.h>
#include <stddef.h>

/* ============================================================================
 * Soft Systems Methodology (SSM) — Core Type Definitions
 *
 * Based on:  Peter Checkland — Systems Thinking, Systems Practice (1981)
 *            Checkland & Scholes — SSM in Action (1990)
 *            Checkland & Poulter — Learning for Action (2006)
 *
 * SSM deals with "soft" problems — messy, ill-structured situations
 * involving human activity systems where multiple worldviews clash.
 * ============================================================================ */

/* ── Weltanschauung (Worldview) Types ───────────── */
typedef enum {
    SSM_W_EXPLICIT     = 0,
    SSM_W_IMPLICIT     = 1,
    SSM_W_DOMINANT     = 2,
    SSM_W_ALTERNATIVE  = 3,
    SSM_W_EMERGENT     = 4
} WeltanschauungType;

typedef struct {
    int                id;
    char*              name;
    char*              description;
    char**             assumptions;
    int                n_assumptions;
    int                assumption_capacity;
    char**             implications;
    int                n_implications;
    int                implication_capacity;
    WeltanschauungType type;
    double             influence_weight;
    double             conflict_score;
} Weltanschauung;

/* ── CATWOE Components ────────────────────────── */
typedef enum {
    SSM_TRANSFORM_CONCRETE    = 0,
    SSM_TRANSFORM_ABSTRACT    = 1,
    SSM_TRANSFORM_SOCIAL      = 2,
    SSM_TRANSFORM_POLITICAL   = 3,
    SSM_TRANSFORM_COGNITIVE   = 4
} TransformationType;

typedef struct {
    char*              customers;
    int                n_customer_types;
    char**             customer_list;

    char*              actors;
    int                n_actor_types;
    char**             actor_list;

    char*              transformation_input;
    char*              transformation_output;
    TransformationType transform_type;
    double             transform_measurability;

    char*              weltanschauung;
    Weltanschauung*    w_ref;

    char*              owners;
    int                n_owner_types;
    char**             owner_list;

    char**             environment_constraints;
    int                n_constraints;
    int                constraint_capacity;

    double             efficacy_criterion;
    double             efficiency_criterion;
    double             effectiveness_criterion;
} CATWOE;

/* ── Root Definition ──────────────────────────── */
typedef enum {
    SSM_RD_PRIMARY_TASK   = 0,
    SSM_RD_ISSUE_BASED    = 1,
    SSM_RD_EXPLORATORY    = 2,
    SSM_RD_CRITICAL       = 3
} RootDefType;

typedef struct {
    int           id;
    char*         name;
    char*         root_definition;
    RootDefType   type;
    CATWOE*       catwoe;

    char*         what_to_do;
    char*         how_to_do;
    char*         why_do_it;

    double        efficacy;
    double        efficiency;
    double        effectiveness;
    double        ethicality;
    double        elegance;

    double        coherence_score;
    int           relevance_rank;
} RootDefinition;

/* ── Conceptual Model ─────────────────────────── */
typedef struct {
    int           id;
    char*         name;
    char*         description;
    int           level;
    int*          depends_on;
    int           n_dependencies;
    int           dep_capacity;
    int*          enables;
    int           n_enables;
    int           en_capacity;
    double        monitoring_frequency;
    char**        performance_indicators;
    int           n_indicators;
    double*       indicator_targets;
} ConceptualActivity;

typedef struct {
    char*                root_definition_text;
    ConceptualActivity** activities;
    int                  n_activities;
    int                  activity_capacity;
    double               logical_coverage;
    double               structural_complexity;
    int                  max_level;
} ConceptualModel;

/* ── Stakeholder View (Multi-Perspective Analysis) ──── */
typedef struct {
    char*        concern_name;
    double       intensity;        /* strength of concern [0-1] */
} StakeholderConcern;

typedef struct {
    char*                stakeholder_name;
    StakeholderConcern*  concerns;
    int                  n_concerns;
} StakeholderView;

/* ── Problem Situation ────────────────────────── */
typedef struct {
    char*              situation_name;
    char*              rich_description;
    char**             stakeholders;
    int                n_stakeholders;
    int                stakeholder_capacity;
    StakeholderView**  stakeholder_views;  /* enriched perspectives */
    int                n_views;
    char**             issues;
    int                n_issues;
    int                issue_capacity;
    char**             structures;
    int                n_structures;
    Weltanschauung**   worldviews;
    int                n_worldviews;
    int                w_capacity;
    double             complexity_score;
    double             conflict_level;
} ProblemSituation;

/* ── Comparison & Gap Analysis ────────────────── */
typedef enum {
    SSM_COMPARE_INFORMAL    = 0,
    SSM_COMPARE_QUESTIONS   = 1,
    SSM_COMPARE_MATRIX      = 2,
    SSM_COMPARE_SCRIPT      = 3
} ComparisonMethod;

typedef struct {
    int           activity_id;
    char*         gap_description;
    double        gap_severity;
    bool          exists_in_reality;
    bool          works_well;
    char*         recommendation;
} ActivityGap;

typedef struct {
    char*              model_name;
    char*              situation_name;
    ComparisonMethod   method;
    ActivityGap**      gaps;
    int                n_gaps;
    int                gap_capacity;
    double             overall_alignment;
    char***            agenda_items;
    int*               agenda_priorities;
    int                n_agenda;
} Comparison;

/* ── SSM Learning Cycle ───────────────────────── */
typedef enum {
    SSM_STAGE_1_SITUATION    = 0,
    SSM_STAGE_2_EXPRESSION   = 1,
    SSM_STAGE_3_ROOT_DEFS    = 2,
    SSM_STAGE_4_MODELS       = 3,
    SSM_STAGE_5_COMPARISON   = 4,
    SSM_STAGE_6_CHANGES      = 5,
    SSM_STAGE_7_ACTION       = 6
} SSMStage;

typedef struct {
    SSMStage            current_stage;
    int                 iteration_count;
    ProblemSituation*   situation;
    Weltanschauung**    worldviews;
    int                 n_worldviews;
    RootDefinition**    root_definitions;
    int                 n_root_defs;
    ConceptualModel**   conceptual_models;
    int                 n_models;
    Comparison**        comparisons;
    int                 n_comparisons;
    char**              learning_log;
    int                 n_log_entries;
    double              accommodation_index;
} SSMCycle;

#endif /* SSM_TYPES_H */
