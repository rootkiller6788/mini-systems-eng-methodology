#ifndef CST_ETHICS_H
#define CST_ETHICS_H
#include "cst_core.h"

/* Ethical Systems Practice — integrating ethics into systems intervention
 * Based on: Ulrich (1983), Churchman (1979), Midgley (2000)
 *
 * Key principles:
 * 1. Beneficence — the intervention should benefit those affected
 * 2. Non-maleficence — avoid harm
 * 3. Justice — fair distribution of benefits and burdens
 * 4. Autonomy — respect for self-determination
 * 5. Transparency — openness about methods and assumptions */

typedef enum {
    CST_ETHIC_BENEFICENCE = 0, CST_ETHIC_NONMALEFICENCE = 1,
    CST_ETHIC_JUSTICE = 2, CST_ETHIC_AUTONOMY = 3,
    CST_ETHIC_TRANSPARENCY = 4
} CSTEthicalPrinciple;

typedef struct {
    CSTEthicalPrinciple principle;
    char* name; char* description;
    double score;        /* 0-1: how well this principle is upheld */
    bool is_violated;    /* True if score < 0.3 */
    char** concerns; int n_concerns;
} CSTEthicalAssessment;

typedef struct {
    CSTSystem* sys;
    CSTEthicalAssessment* assessments;
    int n_assessments;
    double overall_ethics_score;
    double beneficence, nonmaleficence, justice, autonomy, transparency;
    bool has_ethical_violations;
    int n_violations;
    char** recommendations; int n_recommendations;
} CSTEthicsReport;

CSTEthicsReport* cst_ethics_create(CSTSystem* sys);
void cst_ethics_free(CSTEthicsReport* er);
void cst_ethics_assess_all(CSTEthicsReport* er);
void cst_ethics_assess_principle(CSTEthicsReport* er, CSTEthicalPrinciple p, double score);
double cst_ethics_aggregate_score(const CSTEthicsReport* er);
bool cst_ethics_is_acceptable(const CSTEthicsReport* er);
int cst_ethics_count_violations(const CSTEthicsReport* er);
void cst_ethics_generate_recommendations(CSTEthicsReport* er);
void cst_ethics_print(const CSTEthicsReport* er);
const char* cst_principle_name(CSTEthicalPrinciple p);
double cst_ethics_violation_severity(const CSTEthicsReport* er);
bool cst_ethics_needs_urgent_review(const CSTEthicsReport* er);
int cst_ethics_principle_rank(const CSTEthicsReport* er, CSTEthicalPrinciple p);
double cst_ethics_compliance_index(const CSTEthicsReport* er);
void cst_ethics_report_card(const CSTEthicsReport* er);
void cst_ethics_audit_stakeholders(const CSTEthicsReport* er);
double cst_ethics_rawlsian_difference(const CSTEthicsReport* er);
bool cst_ethics_satisfies_rawls(const CSTEthicsReport* er);
double cst_ethics_capability_approach(const CSTEthicsReport* er);
void cst_ethics_care_perspective(const CSTEthicsReport* er);

#endif /* CST_ETHICS_H */