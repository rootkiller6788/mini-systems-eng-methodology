#ifndef HSM_LOGIC_H
#define HSM_LOGIC_H
#include "hsm_core.h"

/* ============================================================================
 * Logic Dimension — The 7 Problem-Solving Steps
 *
 * Each step is applied iteratively within each phase:
 *   1. Problem Definition — needs analysis, environment, constraints
 *   2. Value System Design — objectives tree, metrics, criteria weights
 *   3. Systems Synthesis — generate alternative solutions
 *   4. Systems Analysis — model, simulate, evaluate alternatives
 *   5. Optimization — refine each alternative to best performance
 *   6. Decision Making — select best alternative (multi-criteria)
 *   7. Planning for Action — implementation plan, schedule, resources
 * ============================================================================ */

typedef struct {
    char* description;
    double score;              /* Weighted evaluation score */
    double cost_estimate;
    double benefit_estimate;
    double risk_level;
    bool is_selected;
} HSMAlternative;

typedef struct {
    HSMLogicStep step;
    HSMAlternative* alternatives;
    int n_alternatives;
    int alt_capacity;
    double* criteria_weights;
    char** criteria_names;
    int n_criteria;
    int best_alternative;      /* Index of selected alternative */
    double decision_confidence;
} HSMLogicActivity;

/* Decision matrix */
typedef struct {
    double** scores;           /* [alternative][criterion] */
    int n_alternatives;
    int n_criteria;
    double* weights;
    double* total_scores;      /* Weighted sum per alternative */
    int* ranking;              /* Rank order (0 = best) */
} HSMDecisionMatrix;

/* API */
HSMLogicActivity* hsm_logic_create(HSMLogicStep step);
void hsm_logic_free(HSMLogicActivity* la);
int hsm_logic_add_alternative(HSMLogicActivity* la, const char* desc,
                                double cost, double benefit, double risk);
void hsm_logic_add_criterion(HSMLogicActivity* la, const char* name,
                               double weight);
void hsm_logic_set_score(HSMLogicActivity* la, int alt_idx,
                           int crit_idx, double score);
int hsm_logic_select_best(HSMLogicActivity* la);
void hsm_logic_print_alternatives(HSMLogicActivity* la);

/* Decision matrix operations */
HSMDecisionMatrix* hsm_dm_create(int n_alts, int n_criteria,
                                   double* weights);
void hsm_dm_free(HSMDecisionMatrix* dm);
void hsm_dm_set_score(HSMDecisionMatrix* dm, int alt, int crit, double s);
void hsm_dm_compute_ranking(HSMDecisionMatrix* dm);
int hsm_dm_best(HSMDecisionMatrix* dm);
double hsm_dm_consistency_ratio(HSMDecisionMatrix* dm);
void hsm_dm_print(HSMDecisionMatrix* dm);

/* Analytic Hierarchy Process (AHP) helper */
double* hsm_ahp_weights(double** pairwise, int n);
double hsm_ahp_consistency(double** pairwise, double* weights, int n);
#endif
