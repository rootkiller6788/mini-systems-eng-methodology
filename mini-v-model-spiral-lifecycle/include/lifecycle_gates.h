#ifndef LIFECYCLE_GATES_H
#define LIFECYCLE_GATES_H

#include "lifecycle_core.h"

/* ============================================================================
 * Stage Gates, Reviews, and Decision Points
 *
 * Stage gates (Cooper, 1990) are decision points between lifecycle stages.
 * Each gate has entry criteria, review procedures, and exit criteria.
 *
 * Based on:
 *   Cooper, R.G. ? "Stage-Gate Systems" (1990)
 *   NASA ? Systems Engineering Handbook (SP-2016-6105)
 *   ISO/IEC 15288 ? Technical Review Process
 * ============================================================================ */

/* --- Review Type --- */
typedef enum {
    REVIEW_SRR = 0,     /* System Requirements Review */
    REVIEW_SDR = 1,     /* System Design Review */
    REVIEW_PDR = 2,     /* Preliminary Design Review */
    REVIEW_CDR = 3,     /* Critical Design Review */
    REVIEW_TRR = 4,     /* Test Readiness Review */
    REVIEW_PRR = 5,     /* Production Readiness Review */
    REVIEW_SAR = 6,     /* System Acceptance Review */
    REVIEW_ORR = 7,     /* Operational Readiness Review */
    REVIEW_PIR = 8,     /* Post-Implementation Review */
    REVIEW_DRR = 9,     /* Disposal Readiness Review */
    REVIEW_NUM = 10
} ReviewType;

/* --- Review Finding --- */
typedef enum {
    FINDING_NONE = 0,
    FINDING_OBSERVATION = 1,
    FINDING_MINOR = 2,
    FINDING_MAJOR = 3,
    FINDING_CRITICAL = 4,
    FINDING_SHOWSTOPPER = 5
} ReviewFinding;

/* --- Review Action Item --- */
typedef struct {
    char*         description;
    ReviewFinding severity;
    char*         assigned_to;
    bool          is_resolved;
    char*         resolution;
    double        due_days;
} ActionItem;

/* --- Technical Review --- */
typedef struct {
    ReviewType    type;
    char*         name;
    LifecycleStage entry_stage;     /* When review occurs */
    char**        entry_criteria;
    int           n_entry_criteria;
    ActionItem*   action_items;
    int           n_actions;
    int           action_capacity;
    int           n_attendees;
    bool          is_complete;
    bool          is_passed;
    GateDecision  decision;
    char*         decision_rationale;
    double        review_date;      /* Days from project start */
} TechnicalReview;

/* --- Stage Gate --- */
typedef struct {
    LifecycleStage from_stage;
    LifecycleStage to_stage;
    TechnicalReview review;
    bool           is_passed;
    GateDecision   decision;
    char**         deliverables_required;
    int            n_deliverables_required;
    int            n_deliverables_submitted;
    double         gate_date;        /* Scheduled date */
    double         actual_date;      /* Actual completion date */
} StageGate;

/* --- Gate Review System --- */
typedef struct {
    LifecycleSystem base;

    TechnicalReview reviews[REVIEW_NUM];
    bool            review_configured[REVIEW_NUM];

    StageGate       gates[STAGE_NUM];
    bool            gate_configured[STAGE_NUM];

    int             n_reviews_completed;
    int             n_reviews_passed;
    int             n_gates_passed;
    int             n_gates_failed;

    int             n_action_items_open;
    int             n_action_items_total;
    int             n_showstoppers;

    double          average_review_duration;
} GateReviewSystem;

/* --- Gate & Review API --- */
GateReviewSystem* gates_create(const char* name);
void gates_free(GateReviewSystem* grs);

/* Review management */
TechnicalReview* gates_configure_review(GateReviewSystem* grs,
    ReviewType type, LifecycleStage stage);
void gates_add_entry_criterion(TechnicalReview* rev, const char* criterion);
void gates_add_action_item(TechnicalReview* rev, const char* desc,
    ReviewFinding severity, const char* assignee, double due_days);
void gates_resolve_action_item(TechnicalReview* rev, int idx,
    const char* resolution);
void gates_conduct_review(GateReviewSystem* grs, ReviewType type,
    GateDecision decision, const char* rationale);
bool gates_review_passed(GateReviewSystem* grs, ReviewType type);

/* Gate management */
void gates_configure_gate(GateReviewSystem* grs, LifecycleStage from,
    LifecycleStage to);
void gates_pass_gate(GateReviewSystem* grs, LifecycleStage from);
GateDecision gates_get_decision(GateReviewSystem* grs,
    LifecycleStage from_stage);

/* Status */
int gates_open_actions(GateReviewSystem* grs);
int gates_critical_actions(GateReviewSystem* grs);
bool gates_can_proceed(GateReviewSystem* grs, LifecycleStage to_stage);
double gates_review_completion(GateReviewSystem* grs);

/* Names */
const char* gates_review_name(ReviewType type);
const char* gates_finding_name(ReviewFinding finding);

void gates_print(GateReviewSystem* grs);
void gates_print_review(TechnicalReview* rev);

#endif
