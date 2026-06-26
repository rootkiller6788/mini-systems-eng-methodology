#ifndef MBSE_VERIFICATION_H
#define MBSE_VERIFICATION_H
#include "mbse_core.h"
/* ===================================================================
 * V&V planning follows ISO/IEC/IEEE 15288:2023 processes:
 *
 * Six verification methods per INCOSE:
 *   Test, Analysis, Demonstration, Inspection,
 *   Simulation, Certification
 *
 * Each requirement can be assigned one or more verification
 * methods. The verification plan tracks pass/fail/pending
 * status and computes a pass rate and risk score.
 *
 * Risk Score = (failed + pending) / total_items
 * Acceptable threshold: >=80% coverage, >=90% pass rate
 *
 * Automated procedure generation creates verification
 * procedure templates based on the assigned method type.
 * =================================================================== */
typedef struct { int req_id, method; char* procedure, *date; double pass_criteria, actual_value; bool passed; } VerificationItem;
/*
 * V&V Planning & Execution:
 *   - Verification plan with test/analysis/demo/inspection items
 *   - Pass/fail tracking and pass rate computation
 *   - Automated procedure generation per verification method
 *   - Risk scoring based on failed+pending items
 *   - ISO 15288 compliant verification reporting
 */
typedef struct { int id, n_items, item_capacity, n_passed, n_failed, n_pending; char* name; VerificationItem* items; double pass_rate; } VerificationPlan;
typedef struct { int n_reqs, n_verified, n_methods[6]; double verification_coverage, risk_score; bool is_acceptable; } VerificationReport;
VerificationPlan* mbse_vv_plan_create(const char* name);
void mbse_vv_plan_free(VerificationPlan* vp);
int mbse_vv_add_item(VerificationPlan* vp, int req_id, int method, double criteria);
void mbse_vv_record_result(VerificationPlan* vp, int idx, bool passed, double actual);
double mbse_vv_pass_rate(VerificationPlan* vp);
void mbse_vv_plan_print(VerificationPlan* vp);
VerificationReport* mbse_vv_generate_report(VerificationPlan* vp, SystemModel* m);
void mbse_vv_report_free(VerificationReport* vr);
int mbse_vv_recommend_method(RequirementType t);
void mbse_vv_report_print(VerificationReport* vr);
double mbse_vv_risk_score(VerificationPlan* vp);
#endif
