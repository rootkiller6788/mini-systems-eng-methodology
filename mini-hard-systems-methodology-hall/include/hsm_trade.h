#ifndef HSM_TRADE_H
#define HSM_TRADE_H
#include "hsm_core.h"

/* ============================================================================
 * Trade Studies — Multi-Criteria Decision Analysis
 *
 * Trade studies are central to Hall's Optimization (Step 5) and
 * Decision Making (Step 6). They formalize the comparison of
 * alternatives against multiple weighted criteria.
 * ============================================================================ */

typedef struct {
    char* name;
    double weight;             /* 0.0 - 1.0, weights sum to 1.0 */
    bool is_higher_better;     /* true = maximize, false = minimize */
    double threshold;          /* Minimum acceptable value */
    double goal;               /* Desired target value */
} TradeCriterion;

typedef struct {
    char* name;
    char* description;
    double* scores;            /* Score per criterion */
    double total_score;
    double cost;
    double schedule;
    double risk;
    double benefit;
    bool is_feasible;
} TradeAlternative;

typedef struct {
    TradeCriterion* criteria;
    int n_criteria;
    TradeAlternative* alternatives;
    int n_alternatives;
    int alt_capacity;
    int* ranking;              /* Sorted indices (0 = best) */
    double* sensitivity;       /* Sensitivity to weight changes */
} TradeStudy;

/* Utility functions */
typedef double (*UtilityFunc)(double value, double min, double max, bool higher_better);
double hsm_utility_linear(double x, double min, double max, bool higher);
double hsm_utility_exponential(double x, double min, double max, bool higher);
double hsm_utility_s_curve(double x, double min, double max, bool higher);

/* API */
TradeStudy* hsm_trade_create(void);
void hsm_trade_free(TradeStudy* ts);
int hsm_trade_add_criterion(TradeStudy* ts, const char* name, double weight,
                              bool higher_better, double threshold, double goal);
int hsm_trade_add_alternative(TradeStudy* ts, const char* name,
                                const char* desc, double cost, double schedule,
                                double risk);
void hsm_trade_set_score(TradeStudy* ts, int alt, int crit, double score);
void hsm_trade_set_scores(TradeStudy* ts, int alt, double* scores);
void hsm_trade_compute(TradeStudy* ts);
int hsm_trade_best(TradeStudy* ts);
void hsm_trade_sensitivity_analysis(TradeStudy* ts);
double hsm_trade_dominance(TradeStudy* ts, int alt_a, int alt_b);
void hsm_trade_print(TradeStudy* ts);
void hsm_trade_print_ranking(TradeStudy* ts);

/* Pareto frontier */
int* hsm_trade_pareto_frontier(TradeStudy* ts, int cost_idx,
                                 int benefit_idx, int* n_pareto);
void hsm_trade_print_pareto(TradeStudy* ts, int cost_idx, int benefit_idx);

/* Cost-Benefit Analysis */
double hsm_cost_benefit_ratio(TradeStudy* ts, int alt_idx);
double hsm_net_present_value(double* cashflows, int n, double discount_rate);
double hsm_return_on_investment(double benefit, double cost);
#endif
