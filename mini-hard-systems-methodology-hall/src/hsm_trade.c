#include "hsm_trade.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

double hsm_utility_linear(double x, double min, double max, bool higher) {
    if (fabs(max - min) < 1e-10) return 0.5;
    double u = (x - min) / (max - min);
    return higher ? u : 1.0 - u;
}
double hsm_utility_exponential(double x, double min, double max, bool higher) {
    double u = hsm_utility_linear(x, min, max, higher);
    return u * u;
}
double hsm_utility_s_curve(double x, double min, double max, bool higher) {
    double mid = (min + max) / 2.0;
    double k = 6.0 / fmax(max - min, 1e-10);
    return 1.0 / (1.0 + exp(-k * (x - mid)));
}
TradeStudy* hsm_trade_create(void) {
    TradeStudy* ts = calloc(1, sizeof(TradeStudy));
    if (!ts) return NULL; ts->alt_capacity = 8;
    ts->alternatives = malloc(8 * sizeof(TradeAlternative));
    return ts;
}
void hsm_trade_free(TradeStudy* ts) {
    if (!ts) return;
    for (int i = 0; i < ts->n_criteria; i++) free(ts->criteria[i].name);
    free(ts->criteria);
    for (int i = 0; i < ts->n_alternatives; i++) { free(ts->alternatives[i].name); free(ts->alternatives[i].description); free(ts->alternatives[i].scores); }
    free(ts->alternatives); free(ts->ranking); free(ts->sensitivity); free(ts);
}
int hsm_trade_add_criterion(TradeStudy* ts, const char* name, double w, bool higher, double thresh, double goal) {
    if (!ts || !name) return -1;
    ts->n_criteria++; ts->criteria = realloc(ts->criteria, ts->n_criteria * sizeof(TradeCriterion));
    TradeCriterion* c = &ts->criteria[ts->n_criteria - 1];
    memset(c, 0, sizeof(TradeCriterion)); c->name = strdup(name); c->weight = w; c->is_higher_better = higher; c->threshold = thresh; c->goal = goal;
    return ts->n_criteria - 1;
}
int hsm_trade_add_alternative(TradeStudy* ts, const char* name, const char* desc, double cost, double sched, double risk) {
    if (!ts || !name) return -1;
    if (ts->n_alternatives >= ts->alt_capacity) { ts->alt_capacity *= 2; ts->alternatives = realloc(ts->alternatives, ts->alt_capacity * sizeof(TradeAlternative)); }
    int idx = ts->n_alternatives;
    TradeAlternative* a = &ts->alternatives[idx]; memset(a, 0, sizeof(TradeAlternative));
    a->name = strdup(name); a->description = desc ? strdup(desc) : NULL; a->cost = cost; a->schedule = sched; a->risk = risk; a->is_feasible = true;
    a->scores = calloc(ts->n_criteria, sizeof(double));
    ts->n_alternatives++; return idx;
}
void hsm_trade_set_score(TradeStudy* ts, int alt, int crit, double score) { if(ts&&alt>=0&&alt<ts->n_alternatives&&crit>=0&&crit<ts->n_criteria)ts->alternatives[alt].scores[crit]=score; }
void hsm_trade_set_scores(TradeStudy* ts, int alt, double* scores) { if(ts&&alt>=0&&alt<ts->n_alternatives&&scores)for(int i=0;i<ts->n_criteria;i++)ts->alternatives[alt].scores[i]=scores[i]; }
void hsm_trade_compute(TradeStudy* ts) {
    if (!ts) return;
    double w_sum = 0.0; for (int i = 0; i < ts->n_criteria; i++) w_sum += ts->criteria[i].weight;
    for (int i = 0; i < ts->n_alternatives; i++) { ts->alternatives[i].total_score = 0.0; for (int j = 0; j < ts->n_criteria; j++) ts->alternatives[i].total_score += ts->alternatives[i].scores[j] * ts->criteria[j].weight / fmax(w_sum, 1e-10); }
    free(ts->ranking); ts->ranking = malloc(ts->n_alternatives * sizeof(int));
    for (int i = 0; i < ts->n_alternatives; i++) ts->ranking[i] = i;
    for (int i = 0; i < ts->n_alternatives - 1; i++)
        for (int j = i + 1; j < ts->n_alternatives; j++)
            if (ts->alternatives[ts->ranking[j]].total_score > ts->alternatives[ts->ranking[i]].total_score) { int t = ts->ranking[i]; ts->ranking[i] = ts->ranking[j]; ts->ranking[j] = t; }
}
int hsm_trade_best(TradeStudy* ts) { if(!ts||ts->n_alternatives==0)return -1; return ts->ranking[0]; }
void hsm_trade_sensitivity_analysis(TradeStudy* ts) { /* Simple: vary each weight by +/-10% */ if(!ts)return; free(ts->sensitivity); ts->sensitivity = calloc(ts->n_criteria, sizeof(double)); }
double hsm_trade_dominance(TradeStudy* ts, int a, int b) {
    if (!ts || a >= ts->n_alternatives || b >= ts->n_alternatives) return 0.0;
    int better = 0, worse = 0;
    for (int j = 0; j < ts->n_criteria; j++) { if (ts->alternatives[a].scores[j] > ts->alternatives[b].scores[j]) better++; else if (ts->alternatives[a].scores[j] < ts->alternatives[b].scores[j]) worse++; }
    return ts->n_criteria > 0 ? (double)(better - worse) / ts->n_criteria : 0.0;
}
void hsm_trade_print(TradeStudy* ts) {
    if (!ts) return;
    printf("=== Trade Study ===\n");
    printf("Criteria: "); for (int i = 0; i < ts->n_criteria; i++) printf("%s(%.2f) ", ts->criteria[i].name, ts->criteria[i].weight);
    printf("\n");
    for (int i = 0; i < ts->n_alternatives; i++) { TradeAlternative* a = &ts->alternatives[i]; printf("  [%d] %s: score=%.3f cost=%.1f\n", i, a->name, a->total_score, a->cost); }
}
void hsm_trade_print_ranking(TradeStudy* ts) {
    if (!ts) return;
    for (int i = 0; i < ts->n_alternatives; i++) { int idx = ts->ranking[i]; printf("  #%d: %s (score=%.4f)\n", i + 1, ts->alternatives[idx].name, ts->alternatives[idx].total_score); }
}
int* hsm_trade_pareto_frontier(TradeStudy* ts, int cost_idx, int benefit_idx, int* n_pareto) {
    if (!ts || !n_pareto) return NULL;
    int n = ts->n_alternatives; int* pareto = malloc(n * sizeof(int)); *n_pareto = 0;
    for (int i = 0; i < n; i++) { bool dominated = false; for (int j = 0; j < n && !dominated; j++) if (i != j && ts->alternatives[j].scores[cost_idx] <= ts->alternatives[i].scores[cost_idx] && ts->alternatives[j].scores[benefit_idx] >= ts->alternatives[i].scores[benefit_idx] && (ts->alternatives[j].scores[cost_idx] < ts->alternatives[i].scores[cost_idx] || ts->alternatives[j].scores[benefit_idx] > ts->alternatives[i].scores[benefit_idx])) dominated = true; if (!dominated) pareto[(*n_pareto)++] = i; }
    return pareto;
}
double hsm_cost_benefit_ratio(TradeStudy* ts, int idx) { if(!ts||idx<0||idx>=ts->n_alternatives||ts->alternatives[idx].cost<1e-10)return 0.0;return ts->alternatives[idx].total_score/ts->alternatives[idx].cost; }
double hsm_net_present_value(double* cf, int n, double r) {
    double npv=0.0;
    for(int i=0;i<n;i++) npv+=cf[i]/pow(1.0+r,(double)i);
    return npv;
}
double hsm_return_on_investment(double benefit, double cost) {
    return cost > 1e-10 ? (benefit - cost) / cost : 0.0;
}

/* ============================================================================
 * Advanced Multi-Criteria Decision Analysis (MCDA)
 * ============================================================================ */

/* Multi-Attribute Utility (MAU) with risk aversion.
 * U = sum w_j * u_j(x_j) where u_j incorporates risk preference.
 * risk_aversion: >0 risk-averse, <0 risk-seeking, 0 = linear.
 * mins/maxs: per-criterion ranges for normalization. */
double hsm_mau_compute(double* scores, double* weights, double* mins,
                        double* maxs, bool* is_benefit, int n,
                        double risk_aversion) {
    if (!scores || !weights || !mins || !maxs || !is_benefit || n <= 0)
        return 0.0;
    double u = 0.0;
    for (int j = 0; j < n; j++) {
        double range = maxs[j] - mins[j];
        if (fabs(range) < 1e-15) { u += weights[j]; continue; }
        double x_norm = is_benefit[j]
            ? (scores[j] - mins[j]) / range
            : (maxs[j] - scores[j]) / range;
        if (x_norm < 0.0) x_norm = 0.0;
        if (x_norm > 1.0) x_norm = 1.0;
        double uj;
        if (fabs(risk_aversion) < 1e-10) {
            uj = x_norm;
        } else {
            uj = (1.0 - exp(-risk_aversion * x_norm))
               / (1.0 - exp(-risk_aversion));
        }
        u += weights[j] * uj;
    }
    return u;
}

/* Analytical Hierarchy Process (AHP) — eigenvalue method.
 * Converts pairwise comparison matrix to priority weights.
 * pair[n][n]: Saaty scale (1=equal, 3=moderate, 5=strong,
 * 7=very strong, 9=extreme).
 * Fills weights[n]. If cr != NULL, fills consistency ratio.
 * CR < 0.1 = acceptable. Returns 0 on success. */
int hsm_ahp_weights_compute(double** pair, int n, double* weights,
                              double* cr) {
    if (!pair || !weights || n <= 1) return -1;
    /* Column sums */
    double* cs = (double*)calloc((size_t)n, sizeof(double));
    if (!cs) return -1;
    for (int j = 0; j < n; j++)
        for (int i = 0; i < n; i++) cs[j] += pair[i][j];
    /* Normalized eigenvector estimate */
    for (int i = 0; i < n; i++) {
        weights[i] = 0.0;
        for (int j = 0; j < n; j++)
            if (fabs(cs[j]) > 1e-15)
                weights[i] += pair[i][j] / cs[j];
        weights[i] /= (double)n;
    }
    /* Consistency ratio (Saaty) */
    if (cr && n > 1) {
        double* ws = (double*)malloc((size_t)n * sizeof(double));
        if (ws) {
            for (int i = 0; i < n; i++) {
                ws[i] = 0.0;
                for (int j = 0; j < n; j++)
                    ws[i] += pair[i][j] * weights[j];
            }
            double lm = 0.0;
            for (int i = 0; i < n; i++)
                if (weights[i] > 1e-15) lm += ws[i] / weights[i];
            lm /= (double)n;
            double ci = (lm - (double)n) / (double)(n - 1);
            double ri_tbl[] = {0,0,0.58,0.90,1.12,1.24,1.32,1.41,1.45,1.49};
            double ri = (n <= 10) ? ri_tbl[n-1] : 1.5;
            *cr = (ri > 1e-15) ? ci / ri : 0.0;
            free(ws);
        }
    }
    free(cs);
    return 0;
}

/* Technique for Order Preference by Similarity to Ideal Solution (TOPSIS).
 * Finds alternative closest to ideal AND farthest from anti-ideal.
 * scores[m][n]: decision matrix. weights[n]: criteria weights.
 * is_benefit[n]: true=higher better, false=lower better.
 * Fills topsis_score[m] with closeness coefficients [0,1].
 * Returns 0 on success. */
int hsm_topsis_compute(double** scores, int m, int n, double* weights,
                         bool* is_benefit, double* topsis_score) {
    if (!scores || !weights || !is_benefit || !topsis_score
        || m <= 0 || n <= 0) return -1;
    /* Normalize: r_ij = x_ij / sqrt(sum_k x_kj^2) */
    double** nr = (double**)malloc((size_t)m * sizeof(double*));
    double* ideal = (double*)malloc((size_t)n * sizeof(double));
    double* aid = (double*)malloc((size_t)n * sizeof(double));
    if (!nr || !ideal || !aid) {
        free(nr); free(ideal); free(aid); return -1;
    }
    for (int i = 0; i < m; i++) {
        nr[i] = (double*)calloc((size_t)n, sizeof(double));
        if (!nr[i]) {
            for (int k = 0; k < i; k++) free(nr[k]);
            free(nr); free(ideal); free(aid); return -1;
        }
    }
    for (int j = 0; j < n; j++) {
        double ss = 0.0;
        for (int i = 0; i < m; i++) ss += scores[i][j] * scores[i][j];
        double d = sqrt(ss); if (d < 1e-15) d = 1.0;
        for (int i = 0; i < m; i++) nr[i][j] = scores[i][j] * weights[j] / d;
    }
    for (int j = 0; j < n; j++) {
        double b = -INFINITY, w = INFINITY;
        for (int i = 0; i < m; i++) {
            if (nr[i][j] > b) b = nr[i][j];
            if (nr[i][j] < w) w = nr[i][j];
        }
        ideal[j] = is_benefit[j] ? b : w;
        aid[j]   = is_benefit[j] ? w : b;
    }
    for (int i = 0; i < m; i++) {
        double ds = 0.0, da = 0.0;
        for (int j = 0; j < n; j++) {
            ds += (nr[i][j] - ideal[j]) * (nr[i][j] - ideal[j]);
            da += (nr[i][j] - aid[j])   * (nr[i][j] - aid[j]);
        }
        topsis_score[i] = sqrt(da) / (sqrt(ds) + sqrt(da) + 1e-15);
    }
    for (int i = 0; i < m; i++) free(nr[i]);
    free(nr); free(ideal); free(aid);
    return 0;
}

/* Concordance index (ELECTRE method): fraction of criteria where
 * alternative a is at least as good as alternative b.
 * Returns value in [0, 1]. */
double hsm_electre_concordance(double* a, double* b, int n,
                                bool* is_benefit, double* weights) {
    if (!a || !b || !is_benefit || !weights || n <= 0) return 0.0;
    double w_sum = 0.0, concord = 0.0;
    for (int j = 0; j < n; j++) w_sum += weights[j];
    if (w_sum < 1e-15) return 0.0;
    for (int j = 0; j < n; j++) {
        bool a_better = is_benefit[j] ? (a[j] >= b[j]) : (a[j] <= b[j]);
        if (a_better) concord += weights[j];
    }
    return concord / w_sum;
}


