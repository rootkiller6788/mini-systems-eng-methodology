#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "lifecycle_core.h"
#include "lifecycle_risk.h"

/* Risk Management ? Analysis, Mitigation, Tracking */

RiskManagementPlan* risk_create(const char* name) {
    RiskManagementPlan* rmp = calloc(1, sizeof(RiskManagementPlan));
    if (!rmp) return NULL;
    { LifecycleSystem* _t = lc_create(name, LC_MODEL_V); rmp->base = *_t; free(_t); }
    rmp->watch_capacity = 32;
    rmp->watch_list = calloc((size_t)rmp->watch_capacity, sizeof(RiskItem));
    rmp->n_watch = 0;
    rmp->matrix_size = 6;
    rmp->risk_matrix = calloc((size_t)rmp->matrix_size, sizeof(RiskMatrixEntry*));
    for (int i = 0; i < rmp->matrix_size; i++)
        rmp->risk_matrix[i] = calloc((size_t)rmp->matrix_size, sizeof(RiskMatrixEntry));
    rmp->top_risk_indices = NULL;
    rmp->n_top_risks = 0;
    rmp->trends = NULL;
    rmp->n_trends = 0;
    rmp->initial_exposure = 0.0;
    rmp->target_exposure = 0.0;
    rmp->risk_burndown_rate = 0.0;
    rmp->risk_budget = 100.0;
    rmp->risk_budget_spent = 0.0;
    rmp->risk_budget_remaining = 100.0;
    return rmp;
}

void risk_free(RiskManagementPlan* rmp) {
    if (!rmp) return;
    free(rmp->watch_list);
    if (rmp->risk_matrix) {
        for (int i = 0; i < rmp->matrix_size; i++) free(rmp->risk_matrix[i]);
        free(rmp->risk_matrix);
    }
    free(rmp->top_risk_indices);
    if (rmp->trends) {
        for (int i = 0; i < rmp->n_trends; i++) free(rmp->trends[i].exposure_history);
        free(rmp->trends);
    }
    lc_cleanup(&rmp->base);
    free(rmp);
}

RiskItem* risk_add_risk(RiskManagementPlan* rmp, const char* id,
    const char* desc, RiskLevel prob, RiskLevel impact,
    const char* mitigation, const char* trigger) {
    if (!rmp || !id) return NULL;
    if (rmp->base.n_risks >= rmp->base.risk_capacity) {
        rmp->base.risk_capacity = rmp->base.risk_capacity ? rmp->base.risk_capacity * 2 : 16;
        rmp->base.risks = realloc(rmp->base.risks,
            (size_t)rmp->base.risk_capacity * sizeof(RiskItem));
    }
    RiskItem* r = &rmp->base.risks[rmp->base.n_risks];
    memset(r, 0, sizeof(RiskItem));
    r->id = strdup(id);
    r->description = strdup(desc ? desc : "");
    r->probability = prob;
    r->impact = impact;
    r->risk_exposure = risk_compute_exposure(prob, impact);
    r->mitigation = strdup(mitigation ? mitigation : "");
    r->trigger = strdup(trigger ? trigger : "");
    r->is_resolved = false;
    r->is_accepted = false;
    r->mitigation_cost = 0.0;
    r->residual_exposure = r->risk_exposure;
    rmp->base.n_risks++;
    rmp->base.total_risk_exposure += r->risk_exposure;
    risk_update_exposure(rmp);
    return r;
}

double risk_compute_exposure(RiskLevel prob, RiskLevel impact) {
    return (double)(prob + 1) * (double)(impact + 1) / 36.0;
}

void risk_update_exposure(RiskManagementPlan* rmp) {
    if (!rmp) return;
    double total = 0.0;
    for (int i = 0; i < rmp->base.n_risks; i++)
        total += rmp->base.risks[i].risk_exposure;
    rmp->base.total_risk_exposure = total;
}

RiskLevel risk_exposure_to_level(double exposure) {
    if (exposure < 0.01) return RISK_NEGLIGIBLE;
    if (exposure < 0.1) return RISK_LOW;
    if (exposure < 0.3) return RISK_MODERATE;
    if (exposure < 0.6) return RISK_HIGH;
    if (exposure < 0.9) return RISK_CRITICAL;
    return RISK_CATASTROPHIC;
}

static int cmp_risk_desc(const void* a, const void* b) {
    int ia = *(const int*)a, ib = *(const int*)b;
    /* We need access to the RiskItem array, but we don't have it. Use a global workaround. */
    (void)ia; (void)ib;
    return 0;
}

void risk_identify_top_risks(RiskManagementPlan* rmp, int top_n) {
    if (!rmp || rmp->base.n_risks == 0) return;
    if (top_n > rmp->base.n_risks) top_n = rmp->base.n_risks;
    rmp->top_risk_indices = realloc(rmp->top_risk_indices, (size_t)top_n * sizeof(int));
    rmp->n_top_risks = top_n;
    /* Simple selection: find top N by exposure */
    bool* selected = calloc((size_t)rmp->base.n_risks, sizeof(bool));
    for (int k = 0; k < top_n; k++) {
        double max_exp = -1.0;
        int max_idx = 0;
        for (int i = 0; i < rmp->base.n_risks; i++) {
            if (!selected[i] && rmp->base.risks[i].risk_exposure > max_exp) {
                max_exp = rmp->base.risks[i].risk_exposure;
                max_idx = i;
            }
        }
        rmp->top_risk_indices[k] = max_idx;
        selected[max_idx] = true;
    }
    free(selected);
}

double risk_total_exposure(RiskManagementPlan* rmp) {
    return rmp ? rmp->base.total_risk_exposure : 0.0;
}

void risk_apply_mitigation(RiskManagementPlan* rmp, const char* risk_id, double cost) {
    if (!rmp || !risk_id) return;
    for (int i = 0; i < rmp->base.n_risks; i++) {
        if (strcmp(rmp->base.risks[i].id, risk_id) == 0) {
            rmp->base.risks[i].mitigation_cost += cost;
            rmp->base.risks[i].residual_exposure *= 0.3; /* Mitigation reduces exposure */
            rmp->base.risks[i].is_resolved = (rmp->base.risks[i].residual_exposure < 0.01);
            rmp->risk_budget_spent += cost;
            rmp->risk_budget_remaining = rmp->risk_budget - rmp->risk_budget_spent;
            risk_update_exposure(rmp);
            return;
        }
    }
}

void risk_accept_risk(RiskManagementPlan* rmp, const char* risk_id) {
    if (!rmp || !risk_id) return;
    for (int i = 0; i < rmp->base.n_risks; i++) {
        if (strcmp(rmp->base.risks[i].id, risk_id) == 0) {
            rmp->base.risks[i].is_accepted = true;
            return;
        }
    }
}

bool risk_is_acceptable(RiskManagementPlan* rmp, const char* risk_id, double tolerance) {
    if (!rmp || !risk_id) return true;
    for (int i = 0; i < rmp->base.n_risks; i++) {
        if (strcmp(rmp->base.risks[i].id, risk_id) == 0)
            return (rmp->base.risks[i].residual_exposure <= tolerance);
    }
    return true;
}

double risk_residual_exposure(RiskManagementPlan* rmp) {
    if (!rmp) return 0.0;
    double total = 0.0;
    for (int i = 0; i < rmp->base.n_risks; i++)
        total += rmp->base.risks[i].residual_exposure;
    return total;
}

void risk_record_trend(RiskManagementPlan* rmp) {
    if (!rmp) return;
    /* Simple: just store current total exposure */
    if (rmp->n_trends == 0) {
        rmp->trends = calloc(1, sizeof(RiskTrend));
        rmp->trends[0].capacity = 100;
        rmp->trends[0].exposure_history = calloc(100, sizeof(double));
        rmp->n_trends = 1;
    }
    RiskTrend* rt = &rmp->trends[0];
    if (rt->n_points >= rt->capacity) {
        rt->capacity *= 2;
        rt->exposure_history = realloc(rt->exposure_history,
            (size_t)rt->capacity * sizeof(double));
    }
    rt->exposure_history[rt->n_points++] = rmp->base.total_risk_exposure;
    /* Compute trend */
    if (rt->n_points >= 2) {
        double sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0;
        for (int i = 0; i < rt->n_points; i++) {
            sum_x += i;
            sum_y += rt->exposure_history[i];
            sum_xy += i * rt->exposure_history[i];
            sum_x2 += i * i;
        }
        double n = (double)rt->n_points;
        double denom = n * sum_x2 - sum_x * sum_x;
        if (fabs(denom) > 1e-10) {
            rt->trend_slope = (n * sum_xy - sum_x * sum_y) / denom;
        }
        rt->is_improving = (rt->trend_slope < -0.001);
        rt->is_worsening = (rt->trend_slope > 0.001);
    }
}

double risk_trend_slope(RiskManagementPlan* rmp) {
    if (!rmp || rmp->n_trends == 0) return 0.0;
    return rmp->trends[0].trend_slope;
}

bool risk_is_improving(RiskManagementPlan* rmp) {
    if (!rmp || rmp->n_trends == 0) return false;
    return rmp->trends[0].is_improving;
}

bool risk_is_worsening(RiskManagementPlan* rmp) {
    if (!rmp || rmp->n_trends == 0) return false;
    return rmp->trends[0].is_worsening;
}

void risk_set_budget(RiskManagementPlan* rmp, double budget) {
    if (!rmp) return;
    rmp->risk_budget = budget;
    rmp->risk_budget_remaining = budget - rmp->risk_budget_spent;
}

double risk_budget_burn_rate(RiskManagementPlan* rmp) {
    if (!rmp || rmp->base.elapsed_days < 1e-10) return 0.0;
    return rmp->risk_budget_spent / rmp->base.elapsed_days;
}

double risk_budget_efficiency(RiskManagementPlan* rmp) {
    if (!rmp || rmp->risk_budget_spent < 1e-10) return 0.0;
    double exposure_reduced = rmp->base.total_risk_exposure - risk_residual_exposure(rmp);
    return exposure_reduced / rmp->risk_budget_spent;
}

void risk_print(RiskManagementPlan* rmp) {
    if (!rmp) { printf("RiskManagementPlan: NULL\n"); return; }
    printf("=== Risk Management: %s ===\n", rmp->base.name);
    printf("  Total Risk Exposure: %.4f  Residual: %.4f\n",
           rmp->base.total_risk_exposure, risk_residual_exposure(rmp));
    printf("  Risk Budget: %.0f spent / %.0f total (%.0f remaining)\n",
           rmp->risk_budget_spent, rmp->risk_budget, rmp->risk_budget_remaining);
    printf("  Trend: %.4f slope (%s)\n", risk_trend_slope(rmp),
           risk_is_improving(rmp) ? "IMPROVING" :
           risk_is_worsening(rmp) ? "WORSENING" : "STABLE");
    printf("  Risks: %d total, %d on watch list\n",
           rmp->base.n_risks, rmp->n_watch);
}

void risk_print_top_risks(RiskManagementPlan* rmp) {
    if (!rmp || rmp->n_top_risks == 0) { printf("No top risks identified.\n"); return; }
    printf("=== Top %d Risks ===\n", rmp->n_top_risks);
    for (int i = 0; i < rmp->n_top_risks; i++) {
        RiskItem* r = &rmp->base.risks[rmp->top_risk_indices[i]];
        printf("  %2d. %s: %.4f (P=%d I=%d) %s\n",
               i + 1, r->id, r->risk_exposure, r->probability, r->impact,
               r->is_resolved ? "?" : "?");
    }
}

void risk_print_matrix(RiskManagementPlan* rmp) {
    if (!rmp) return;
    printf("=== Risk Matrix (P x I) ===\n");
    printf("  ");
    for (int i = 0; i < 6; i++) printf(" I%-2d", i);
    printf("\n");
    for (int p = 0; p < 6; p++) {
        printf("P%d", p);
        for (int i = 0; i < 6; i++)
            printf(" %.2f", risk_compute_exposure((RiskLevel)p, (RiskLevel)i));
        printf("\n");
    }
}

/* ====================================================================
 * Extended Risk Management Functions
 * ==================================================================== */

/* Monte Carlo risk exposure (simple triangular distribution estimate) */
double risk_monte_carlo_estimate(RiskManagementPlan* rmp, int n_samples) {
    if (!rmp || n_samples < 1) return 0.0;
    double best_case = rmp->base.total_risk_exposure * 0.5;
    double worst_case = rmp->base.total_risk_exposure * 2.0;
    double most_likely = rmp->base.total_risk_exposure;
    double sum = 0.0;
    for (int i = 0; i < n_samples; i++) {
        double u = (double)rand() / RAND_MAX;
        double estimate;
        if (u < (most_likely - best_case) / (worst_case - best_case + 1e-12))
            estimate = best_case + sqrt(u * (most_likely - best_case) * (worst_case - best_case));
        else
            estimate = worst_case - sqrt((1-u) * (worst_case - most_likely) * (worst_case - best_case));
        sum += estimate;
    }
    return sum / n_samples;
}

/* Risk priority number (RPN = probability * impact * detectability) */
double risk_priority_number(RiskItem* ri, int detectability) {
    if (!ri || detectability < 1 || detectability > 10) return 0.0;
    return ri->risk_exposure * (double)detectability;
}

/* Aggregate risk by category (simplified: by probability level) */
int risk_count_by_probability(RiskManagementPlan* rmp, RiskLevel prob) {
    if (!rmp) return 0;
    int count = 0;
    for (int i = 0; i < rmp->base.n_risks; i++)
        if (rmp->base.risks[i].probability == prob) count++;
    return count;
}

/* Risk retirement rate: number of risks closed per unit time */
double risk_retirement_rate(RiskManagementPlan* rmp) {
    if (!rmp || rmp->base.elapsed_days < 1e-10) return 0.0;
    int closed = 0;
    for (int i = 0; i < rmp->base.n_risks; i++)
        if (rmp->base.risks[i].is_resolved) closed++;
    return (double)closed / rmp->base.elapsed_days;
}

/* Estimate days to retire all current risks */
double risk_days_to_retire(RiskManagementPlan* rmp) {
    double rate = risk_retirement_rate(rmp);
    if (rate < 1e-10) return 365.0;
    int open = rmp->base.n_risks;
    for (int i = 0; i < rmp->base.n_risks; i++)
        if (rmp->base.risks[i].is_resolved) open--;
    return (double)open / rate;
}

/* Risk correlation matrix entry (simplified pairwise check) */
double risk_correlation(RiskManagementPlan* rmp, int idx1, int idx2) {
    if (!rmp || idx1 < 0 || idx1 >= rmp->base.n_risks || idx2 < 0 || idx2 >= rmp->base.n_risks) return 0.0;
    if (idx1 == idx2) return 1.0;
    /* Correlation based on probability and impact similarity */
    double diff_prob = fabs((double)rmp->base.risks[idx1].probability - (double)rmp->base.risks[idx2].probability) / 5.0;
    double diff_impact = fabs((double)rmp->base.risks[idx1].impact - (double)rmp->base.risks[idx2].impact) / 5.0;
    return 1.0 - 0.5 * (diff_prob + diff_impact);
}


/* Risk burn-down: track cumulative risk resolution across spiral iterations.
 * Boehm (1988): each spiral iteration should reduce risk exposure. */
double risk_burndown(int n_iterations, const double* risk_per_iteration,
                     double initial_risk, double* remaining_risk) {
    if (!risk_per_iteration || !remaining_risk || n_iterations <= 0) return -1.0;
    *remaining_risk = initial_risk;
    for (int i = 0; i < n_iterations; i++) {
        *remaining_risk -= risk_per_iteration[i];
        if (*remaining_risk < 0.0) *remaining_risk = 0.0;
    }
    return initial_risk - *remaining_risk;  /* total risk resolved */
}

/* Prototype fidelity progression: fidelity increases with each iteration. */
double fidelity_progression(int iteration, double initial_fidelity, double growth_rate) {
    return initial_fidelity * (1.0 + growth_rate * (double)iteration);
}
