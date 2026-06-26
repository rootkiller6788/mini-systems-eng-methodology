#ifndef CST_INTERVENTION_H
#define CST_INTERVENTION_H
#include "cst_core.h"

/* Systemic Intervention (Midgley, 2000)
 * A methodology that integrates boundary critique, methodological pluralism,
 * and ethical reflection into a unified intervention framework.
 *
 * Three phases:
 * 1. Boundary critique — surface what is included/excluded
 * 2. Judgment — evaluate boundaries ethically
 * 3. Action — implement changes with ongoing reflection */

typedef enum {
    CST_INTERV_DIAGNOSIS = 0, CST_INTERV_DESIGN = 1,
    CST_INTERV_IMPLEMENT = 2, CST_INTERV_EVALUATE = 3,
    CST_INTERV_ITERATE = 4
} CSTInterventionPhase;

typedef struct {
    char* name; char* description;
    char** participants; int n_participants;
    char** methods_used; int n_methods;
    double progress;     /* 0-1 */
    double success_prob; /* 0-1: estimated likelihood of success */
    double ethical_score; /* 0-1 */
    CSTInterventionPhase phase;
    int iteration_count;
} CSTIntervention;

typedef struct {
    CSTSystem* sys;
    CSTIntervention* interventions;
    int n_interventions;
    double overall_success_rate;
    double ethical_adequacy;
    double stakeholder_satisfaction;
    int n_completed;
} CSTInterventionPortfolio;

CSTInterventionPortfolio* cst_intervention_create(CSTSystem* sys);
void cst_intervention_free(CSTInterventionPortfolio* ip);
int cst_intervention_add(CSTInterventionPortfolio* ip, const char* name, const char* desc);
void cst_intervention_add_participant(CSTInterventionPortfolio* ip, int idx, const char* name);
void cst_intervention_add_method(CSTInterventionPortfolio* ip, int idx, const char* method);
void cst_intervention_advance(CSTInterventionPortfolio* ip, int idx);
void cst_intervention_evaluate(CSTInterventionPortfolio* ip, int idx);
double cst_intervention_ethical_score(const CSTInterventionPortfolio* ip);
double cst_intervention_midgley_index(const CSTInterventionPortfolio* ip);
bool cst_intervention_is_systemic(const CSTInterventionPortfolio* ip, int idx);
int cst_intervention_best_candidate(const CSTInterventionPortfolio* ip);
void cst_intervention_print(const CSTInterventionPortfolio* ip);
void cst_intervention_print_one(const CSTInterventionPortfolio* ip, int idx);
double cst_intervention_success_rate(const CSTInterventionPortfolio* ip);
int cst_intervention_count_completed(const CSTInterventionPortfolio* ip);
bool cst_intervention_all_completed(const CSTInterventionPortfolio* ip);
void cst_intervention_portfolio_summary(const CSTInterventionPortfolio* ip);
void cst_intervention_start(CSTInterventionPortfolio* ip, int idx);
void cst_intervention_terminate(CSTInterventionPortfolio* ip, int idx);
bool cst_intervention_is_active(const CSTInterventionPortfolio* ip, int idx);
void cst_intervention_risk_assessment(const CSTInterventionPortfolio* ip, int idx);
void cst_intervention_learn_from_failure(CSTInterventionPortfolio* ip, int idx);

#endif /* CST_INTERVENTION_H */