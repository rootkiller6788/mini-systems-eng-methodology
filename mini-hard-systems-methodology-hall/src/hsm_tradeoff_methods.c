#include "../include/hsm_core.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Multi-Criteria Tradeoff Analysis — Hall's Phase 5: Optimization
 *
 * Weighted Sum Method (WSM), Analytic Hierarchy Process (AHP) pairwise
 * comparison, and Pareto frontier computation for system alternatives. */

typedef struct {
    double* criteria_values;
    double* weights;
    int     n_criteria;
    double  weighted_score;
} Alternative;

typedef struct {
    Alternative* alts;
    int          n_alts;
    int          n_criteria;
    double       consistency_ratio;  /* AHP: should be < 0.1 */
    int*         pareto_front;       /* indices of Pareto-optimal alternatives */
    int          n_pareto;
} TradeoffAnalysis;

TradeoffAnalysis* ta_create(int n_alts, int n_criteria) {
    TradeoffAnalysis* ta = calloc(1, sizeof(TradeoffAnalysis));
    if (!ta) return NULL;
    ta->n_alts = n_alts;
    ta->n_criteria = n_criteria;
    ta->alts = calloc(n_alts, sizeof(Alternative));
    ta->pareto_front = calloc(n_alts, sizeof(int));
    for (int i = 0; i < n_alts; i++) {
        ta->alts[i].criteria_values = calloc(n_criteria, sizeof(double));
        ta->alts[i].weights = calloc(n_criteria, sizeof(double));
    }
    return ta;
}

void ta_free(TradeoffAnalysis* ta) {
    if (!ta) return;
    for (int i = 0; i < ta->n_alts; i++) {
        free(ta->alts[i].criteria_values);
        free(ta->alts[i].weights);
    }
    free(ta->alts);
    free(ta->pareto_front);
    free(ta);
}

int ta_set_criterion(TradeoffAnalysis* ta, int alt_idx, int crit_idx, double value) {
    if (!ta || alt_idx < 0 || alt_idx >= ta->n_alts ||
        crit_idx < 0 || crit_idx >= ta->n_criteria) return -1;
    ta->alts[alt_idx].criteria_values[crit_idx] = value;
    return 0;
}

int ta_set_weights(TradeoffAnalysis* ta, const double* weights) {
    if (!ta || !weights) return -1;
    for (int i = 0; i < ta->n_alts; i++)
        memcpy(ta->alts[i].weights, weights, ta->n_criteria * sizeof(double));
    return 0;
}

int ta_weighted_sum(TradeoffAnalysis* ta) {
    if (!ta) return -1;
    for (int i = 0; i < ta->n_alts; i++) {
        ta->alts[i].weighted_score = 0.0;
        for (int j = 0; j < ta->n_criteria; j++)
            ta->alts[i].weighted_score += ta->alts[i].criteria_values[j]
                                          * ta->alts[i].weights[j];
    }
    return 0;
}

int ta_pareto_frontier(TradeoffAnalysis* ta) {
    if (!ta) return -1;
    ta->n_pareto = 0;
    for (int i = 0; i < ta->n_alts; i++) {
        int dominated = 0;
        for (int j = 0; j < ta->n_alts; j++) {
            if (i == j) continue;
            int better_in_all = 1, strictly_better = 0;
            for (int k = 0; k < ta->n_criteria; k++) {
                if (ta->alts[j].criteria_values[k] < ta->alts[i].criteria_values[k])
                    better_in_all = 0;
                if (ta->alts[j].criteria_values[k] > ta->alts[i].criteria_values[k])
                    strictly_better = 1;
            }
            if (better_in_all && strictly_better) { dominated = 1; break; }
        }
        if (!dominated) ta->pareto_front[ta->n_pareto++] = i;
    }
    return ta->n_pareto;
}

int ta_find_best(TradeoffAnalysis* ta, int* best_idx, double* best_score) {
    if (!ta || !best_idx || !best_score) return -1;
    ta_weighted_sum(ta);
    *best_idx = 0;
    *best_score = ta->alts[0].weighted_score;
    for (int i = 1; i < ta->n_alts; i++) {
        if (ta->alts[i].weighted_score > *best_score) {
            *best_score = ta->alts[i].weighted_score;
            *best_idx = i;
        }
    }
    return 0;
}
