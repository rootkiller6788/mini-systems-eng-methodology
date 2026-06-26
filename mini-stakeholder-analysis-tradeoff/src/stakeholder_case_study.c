/* stakeholder_app2.c -- L7-2: Nuclear waste site selection MCDA (Keeney 1987) + renewable energy portfolio.
 *
 * Multi-Criteria Decision Analysis for complex stakeholder-intensive
 * problems. Based on Keeney''s classic nuclear waste siting study.
 * Implements weighted sum model, sensitivity analysis, and
 * stakeholder preference aggregation.
 *
 * Reference: Keeney & Raiffa (1976) Decisions with Multiple Objectives
 *            Keeney (1987) Operations Research 35(6)
 */
#include "stakeholder_model.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    double env_impact, cost_b, public_accept, safety_index;
    double transport_risk, seismic_risk;
    int n_sites, n_criteria;
    double *scores;
    char **site_names;
} WasteSiteSelection;

WasteSiteSelection* waste_new(int n_sites, int n_criteria) {
    WasteSiteSelection *w = calloc(1, sizeof(WasteSiteSelection));
    w->n_sites = n_sites; w->n_criteria = n_criteria;
    w->scores = calloc((size_t)(n_sites * n_criteria), sizeof(double));
    w->site_names = calloc((size_t)n_sites, sizeof(char*));
    return w;
}
void waste_free(WasteSiteSelection *w) {
    if (!w) return;
    free(w->scores);
    for (int i = 0; i < w->n_sites; i++) free(w->site_names[i]);
    free(w->site_names); free(w);
}

int waste_set_score(WasteSiteSelection *w, int site, int criterion, double val) {
    if (!w || site < 0 || site >= w->n_sites || criterion < 0 || criterion >= w->n_criteria)
        return -1;
    w->scores[site * w->n_criteria + criterion] = val;
    return 0;
}

int waste_weighted_sum(WasteSiteSelection *w, const double *weights, double *results) {
    if (!w || !weights || !results) return -1;
    for (int i = 0; i < w->n_sites; i++) {
        results[i] = 0.0;
        for (int j = 0; j < w->n_criteria; j++)
            results[i] += weights[j] * w->scores[i * w->n_criteria + j];
    }
    return 0;
}

int waste_select_best(WasteSiteSelection *w, const double *weights, int *best) {
    if (!w || !weights || !best || w->n_sites < 1) return -1;
    double *results = calloc((size_t)w->n_sites, sizeof(double));
    waste_weighted_sum(w, weights, results);
    *best = 0;
    for (int i = 1; i < w->n_sites; i++)
        if (results[i] > results[*best]) *best = i;
    free(results);
    return 0;
}

int waste_sensitivity(WasteSiteSelection *w, int criterion, const double *base_w,
                       double *robustness) {
    if (!w || !base_w || !robustness) return -1;
    *robustness = 0.0;
    for (int delta = -5; delta <= 5; delta++) {
        double *wt = calloc((size_t)w->n_criteria, sizeof(double));
        for (int j = 0; j < w->n_criteria; j++)
            wt[j] = (j == criterion) ? base_w[j] + 0.05 * (double)delta : base_w[j];
        int best;
        waste_select_best(w, wt, &best);
        if (delta == 0) { int base_best; waste_select_best(w, base_w, &base_best);
            *robustness += (best == base_best) ? 1.0 : 0.0; }
        free(wt);
    }
    *robustness /= 11.0;
    return 0;
}
