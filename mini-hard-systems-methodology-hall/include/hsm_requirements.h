#ifndef HSM_REQUIREMENTS_H
#define HSM_REQUIREMENTS_H
#include "hsm_core.h"

/* ============================================================================
 * Requirements Engineering — Capture, Analyze, Verify, Validate
 *
 * Requirements form the foundation of hard systems engineering.
 * Hall emphasized the critical importance of problem definition (Step 1)
 * and value system design (Step 2) in getting requirements right.
 * ============================================================================ */

typedef enum {
    REQ_FUNCTIONAL = 0,
    REQ_PERFORMANCE = 1,
    REQ_INTERFACE = 2,
    REQ_CONSTRAINT = 3,
    REQ_SAFETY = 4,
    REQ_SECURITY = 5,
    REQ_RELIABILITY = 6,
    REQ_MAINTAINABILITY = 7,
    REQ_ENVIRONMENTAL = 8,
    REQ_NUM_CATEGORIES = 9
} ReqCategory;

typedef struct {
    char* category_name;
    double weight;               /* Importance weight for this category */
    int n_requirements;
    int n_verified;
    int n_validated;
} ReqCategorySummary;

typedef struct {
    HSMRequirement** all_requirements;
    int n_total;
    int n_verified;
    int n_validated;
    double verification_progress;   /* 0.0 - 1.0 */
    double validation_progress;
    ReqCategorySummary summaries[REQ_NUM_CATEGORIES];
} ReqDatabase;

/* API */
const char* hsm_req_category_name(ReqCategory cat);
ReqDatabase hsm_req_summarize(HSMSystem* sys);
void hsm_req_print_summary(HSMSystem* sys);
double hsm_req_traceability_index(HSMSystem* sys);

/* Requirements metrics */
int hsm_req_count_by_category(HSMSystem* sys, ReqCategory cat);
int hsm_req_count_verified(HSMSystem* sys);
int hsm_req_count_validated(HSMSystem* sys);
double hsm_req_verification_coverage(HSMSystem* sys);
double hsm_req_validation_coverage(HSMSystem* sys);

/* Requirements quality */
bool hsm_req_is_well_formed(HSMRequirement* req);
int hsm_req_well_formed_count(HSMSystem* sys);
double hsm_req_ambiguity_score(HSMSystem* sys);
void hsm_req_find_orphans(HSMSystem* sys, char*** orphan_ids, int* n);
#endif
