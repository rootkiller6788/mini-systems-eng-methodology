#ifndef HSM_VERIFICATION_H
#define HSM_VERIFICATION_H
#include "hsm_core.h"

/* ============================================================================
 * Verification & Validation — Hall's Gate Reviews
 *
 * Hall emphasized formal reviews at each phase transition:
 *   - System Requirements Review (SRR)
 *   - Preliminary Design Review (PDR)
 *   - Critical Design Review (CDR)
 *   - Test Readiness Review (TRR)
 *   - Production Readiness Review (PRR)
 *   - Operational Readiness Review (ORR)
 *
 * Verification: "Did we build the system right?" (meets specifications)
 * Validation: "Did we build the right system?" (meets user needs)
 * ============================================================================ */

typedef enum {
    REVIEW_SRR = 0,    /* System Requirements Review */
    REVIEW_PDR = 1,    /* Preliminary Design Review */
    REVIEW_CDR = 2,    /* Critical Design Review */
    REVIEW_TRR = 3,    /* Test Readiness Review */
    REVIEW_PRR = 4,    /* Production Readiness Review */
    REVIEW_ORR = 5,    /* Operational Readiness Review */
    REVIEW_DRR = 6,    /* Disposal Readiness Review */
    REVIEW_NUM = 7
} ReviewType;

typedef struct {
    ReviewType type;
    HSMPhase associated_phase;
    char* entry_criteria;
    char* exit_criteria;
    bool passed;
    int n_action_items;
    int n_closed_items;
    double review_date;
} HSMReview;

typedef struct {
    char* description;
    bool resolved;
    char* assigned_to;
    double due_date;
} HSMActionItem;

typedef struct {
    HSMReview reviews[REVIEW_NUM];
    HSMActionItem* action_items;
    int n_action_items;
    int item_capacity;
    double overall_vv_progress;  /* 0.0 - 1.0 */
} HSMVVFramework;

/* Verification methods */
typedef enum {
    VERIFY_TEST = 0,
    VERIFY_INSPECTION = 1,
    VERIFY_ANALYSIS = 2,
    VERIFY_DEMONSTRATION = 3,
    VERIFY_SIMULATION = 4,
    VERIFY_CERTIFICATION = 5
} VerificationMethod;

typedef struct {
    VerificationMethod method;
    char* description;
    double confidence;         /* 0.0 - 1.0 */
    bool completed;
    double result;             /* Measured result */
    double specification;      /* Required specification */
    bool passed;
} HSMVerificationRecord;

/* API */
HSMVVFramework* hsm_vv_create(void);
void hsm_vv_free(HSMVVFramework* vv);
void hsm_vv_init_review(HSMVVFramework* vv, ReviewType type,
                          HSMPhase phase, const char* entry,
                          const char* exit);
void hsm_vv_pass_review(HSMVVFramework* vv, ReviewType type);
int hsm_vv_add_action_item(HSMVVFramework* vv, const char* desc,
                             const char* assignee, double due);
void hsm_vv_close_action_item(HSMVVFramework* vv, int idx);
double hsm_vv_progress(HSMVVFramework* vv);

/* Verification records */
HSMVerificationRecord* hsm_verify_create_record(VerificationMethod method,
                                                  const char* desc,
                                                  double spec);
void hsm_verify_free_record(HSMVerificationRecord* vr);
bool hsm_verify_execute(HSMVerificationRecord* vr, double measured);
void hsm_verify_print(HSMVerificationRecord* vr);

void hsm_vv_print(HSMVVFramework* vv);
void hsm_vv_print_reviews(HSMVVFramework* vv);
#endif
