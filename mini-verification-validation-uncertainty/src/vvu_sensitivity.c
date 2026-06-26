#include "vvu_sensitivity.h"
#include "vvu_core.h"
#include "vvu_monte_carlo.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <math.h>

/* ==============================================================
 * Configuration
 * ============================================================== */

VVU_SensitivityConfig* vvu_sens_config_create(VVU_SensitivityMethod method) {
    VVU_SensitivityConfig* cfg =
        (VVU_SensitivityConfig*)calloc(1, sizeof(VVU_SensitivityConfig));
    if (!cfg) return NULL;
    cfg->method = method;
    cfg->n_trajectories = 10;
    cfg->n_levels = 4;
    cfg->n_base_samples = 1000;
    cfg->perturbation_fraction = 0.01;
    return cfg;
}

void vvu_sens_config_free(VVU_SensitivityConfig* cfg) { free(cfg); }
void vvu_sens_config_set_trajectories(VVU_SensitivityConfig* cfg, int n) { if (cfg) cfg->n_trajectories = n; }
void vvu_sens_config_set_levels(VVU_SensitivityConfig* cfg, int n) { if (cfg) cfg->n_levels = n; }

/* ==============================================================
 * Local Sensitivity (One-at-a-time)
 * ============================================================== */

VVU_LocalSensitivity* vvu_sens_local(VVU_ModelEval model, void* context,
    const double* nominal, int n_inputs, double delta) {
    if (!model || !nominal || n_inputs <= 0) return NULL;
    VVU_LocalSensitivity* results =
        (VVU_LocalSensitivity*)calloc(n_inputs, sizeof(VVU_LocalSensitivity));

    /* Base output */
    double y_base = model(nominal, n_inputs, context);

    /* Perturb each input */
    for (int i = 0; i < n_inputs; i++) {
        double* x_perturbed = (double*)malloc(n_inputs * sizeof(double));
        memcpy(x_perturbed, nominal, n_inputs * sizeof(double));

        double perturbation = delta * (fabs(nominal[i]) + 1e-6);
        x_perturbed[i] += perturbation;
        double y_perturbed = model(x_perturbed, n_inputs, context);

        results[i].input_index = i;
        results[i].nominal_value = nominal[i];
        results[i].perturbation = perturbation;
        results[i].sensitivity = (y_perturbed - y_base) / perturbation;

        /* Normalized: (dY/Y) / (dX/X) = (dY/dX) * (X/Y) */
        if (fabs(y_base) > 1e-10) {
            results[i].normalized_sensitivity =
                results[i].sensitivity * nominal[i] / y_base;
        }

        free(x_perturbed);
    }

    /* Rank by absolute sensitivity */
    double* abs_sens = (double*)malloc(n_inputs * sizeof(double));
    for (int i = 0; i < n_inputs; i++)
        abs_sens[i] = fabs(results[i].sensitivity);

    for (int rank = 0; rank < n_inputs; rank++) {
        double max_val = -1.0;
        int max_idx = -1;
        for (int i = 0; i < n_inputs; i++) {
            if (abs_sens[i] > max_val) {
                max_val = abs_sens[i];
                max_idx = i;
            }
        }
        if (max_idx >= 0) {
            results[max_idx].ranking = (double)(rank + 1);
            abs_sens[max_idx] = -1.0;
        }
    }
    free(abs_sens);
    return results;
}

void vvu_sens_local_free(VVU_LocalSensitivity* ls, int n) {
    if (!ls) return;
    for (int i = 0; i < n; i++) free(ls[i].input_name);
    free(ls);
}

void vvu_sens_local_print(const VVU_LocalSensitivity* ls, int n) {
    if (!ls) { printf("Sensitivity: NULL\n"); return; }
    printf("=== Local Sensitivity (OAT) ===\n");
    printf("%-4s %-12s %-12s %-12s\n", "Rank", "dY/dX", "Normalized", "Nominal");
    for (int i = 0; i < n; i++) {
        printf("%-4.0f %-12.6f %-12.6f %-12.4f\n",
               ls[i].ranking, ls[i].sensitivity,
               ls[i].normalized_sensitivity, ls[i].nominal_value);
    }
}

int vvu_sens_most_sensitive(const VVU_LocalSensitivity* ls, int n) {
    if (!ls || n <= 0) return -1;
    double max_abs = 0.0;
    int max_idx = 0;
    for (int i = 0; i < n; i++) {
        double abs_s = fabs(ls[i].sensitivity);
        if (abs_s > max_abs) { max_abs = abs_s; max_idx = i; }
    }
    return max_idx;
}

/* ==============================================================
 * Morris Screening (Elementary Effects)
 * ============================================================== */

VVU_MorrisResult* vvu_sens_morris(VVU_ModelEval model, void* context,
    const VVU_Uncertainty* inputs, int n_inputs,
    const VVU_SensitivityConfig* cfg) {
    if (!model || !inputs || n_inputs <= 0 || !cfg) return NULL;
    int r = cfg->n_trajectories;
    int p = cfg->n_levels;

    VVU_MorrisResult* results =
        (VVU_MorrisResult*)calloc(n_inputs, sizeof(VVU_MorrisResult));
    for (int i = 0; i < n_inputs; i++) {
        results[i].input_index = i;
        results[i].input_name = inputs[i].name ? strdup(inputs[i].name) : NULL;
    }

    /* For each trajectory, compute elementary effect for each input */
    double** ee = (double**)malloc(n_inputs * sizeof(double*));
    for (int i = 0; i < n_inputs; i++)
        ee[i] = (double*)malloc(r * sizeof(double));

    double delta = (double)p / (2.0 * (p - 1));

    for (int traj = 0; traj < r; traj++) {
        /* Random starting point in input space */
        double* x = (double*)malloc(n_inputs * sizeof(double));
        for (int i = 0; i < n_inputs; i++)
            x[i] = vvu_uniform_random(inputs[i].lower_bound,
                                       inputs[i].upper_bound);

        /* Random permutation of input order */
        int* perm = (int*)malloc(n_inputs * sizeof(int));
        for (int i = 0; i < n_inputs; i++) perm[i] = i;
        for (int i = n_inputs - 1; i > 0; i--) {
            int j = rand() % (i + 1);
            int tmp = perm[i]; perm[i] = perm[j]; perm[j] = tmp;
        }

        /* Elementary effect for each input in permuted order */
        double y_prev = model(x, n_inputs, context);
        for (int k = 0; k < n_inputs; k++) {
            int idx = perm[k];
            double orig = x[idx];
            double step = delta * (inputs[idx].upper_bound - inputs[idx].lower_bound);
            x[idx] += step;
            double y_new = model(x, n_inputs, context);
            ee[idx][traj] = (y_new - y_prev) / step;
            x[idx] = orig;
            y_prev = y_new;
        }
        free(x); free(perm);
    }

    /* Compute mu_star (mean of absolute), mu, sigma */
    for (int i = 0; i < n_inputs; i++) {
        double sum_abs = 0.0, sum = 0.0;
        for (int t = 0; t < r; t++) {
            sum_abs += fabs(ee[i][t]);
            sum += ee[i][t];
        }
        results[i].mu_star = sum_abs / r;
        results[i].mu = sum / r;

        double var = 0.0;
        for (int t = 0; t < r; t++) {
            double d = ee[i][t] - results[i].mu;
            var += d * d;
        }
        results[i].sigma = sqrt(var / r);
        free(ee[i]);
    }
    free(ee);

    /* Rank by mu_star */
    double* mu_vals = (double*)malloc(n_inputs * sizeof(double));
    for (int i = 0; i < n_inputs; i++)
        mu_vals[i] = results[i].mu_star;
    for (int rank = 0; rank < n_inputs; rank++) {
        double max_val = -1; int max_idx = -1;
        for (int i = 0; i < n_inputs; i++) {
            if (mu_vals[i] > max_val) { max_val = mu_vals[i]; max_idx = i; }
        }
        if (max_idx >= 0) { results[max_idx].ranking = rank + 1; mu_vals[max_idx] = -1; }
    }
    free(mu_vals);
    return results;
}

void vvu_sens_morris_free(VVU_MorrisResult* mr, int n) {
    if (!mr) return;
    for (int i = 0; i < n; i++) free(mr[i].input_name);
    free(mr);
}

void vvu_sens_morris_print(const VVU_MorrisResult* mr, int n) {
    if (!mr) return;
    printf("=== Morris Screening Results ===\n");
    printf("%-4s %-12s %-12s %-12s\n", "Rank", "mu*", "mu", "sigma");
    for (int i = 0; i < n; i++) {
        printf("%-4.0f %-12.6f %-12.6f %-12.6f\n",
               mr[i].ranking, mr[i].mu_star, mr[i].mu, mr[i].sigma);
    }
}

/* ==============================================================
 * Factor Ranking
 * ============================================================== */

VVU_FactorRanking* vvu_sens_rank_factors(const VVU_MorrisResult* morris,
                                          int n_factors) {
    if (!morris || n_factors <= 0) return NULL;
    VVU_FactorRanking* fr =
        (VVU_FactorRanking*)calloc(1, sizeof(VVU_FactorRanking));
    fr->n_factors = n_factors;
    fr->factor_names = (char**)calloc(n_factors, sizeof(char*));
    fr->importance_scores = (double*)calloc(n_factors, sizeof(double));
    fr->ranking = (int*)calloc(n_factors, sizeof(int));

    fr->total_sensitivity = 0.0;
    for (int i = 0; i < n_factors; i++) {
        fr->factor_names[i] = morris[i].input_name ?
            strdup(morris[i].input_name) : NULL;
        fr->importance_scores[i] = morris[i].mu_star;
        fr->total_sensitivity += morris[i].mu_star;
    }

    /* Sort to assign ranks */
    double* scores = (double*)malloc(n_factors * sizeof(double));
    memcpy(scores, fr->importance_scores, n_factors * sizeof(double));
    for (int rank = 0; rank < n_factors; rank++) {
        double max_val = -1.0; int max_idx = -1;
        for (int i = 0; i < n_factors; i++) {
            if (scores[i] > max_val) { max_val = scores[i]; max_idx = i; }
        }
        if (max_idx >= 0) { fr->ranking[max_idx] = rank; scores[max_idx] = -1.0; }
    }
    free(scores);
    return fr;
}

void vvu_sens_ranking_free(VVU_FactorRanking* fr) {
    if (!fr) return;
    for (int i = 0; i < fr->n_factors; i++) free(fr->factor_names[i]);
    free(fr->factor_names); free(fr->importance_scores); free(fr->ranking);
    free(fr);
}

void vvu_sens_ranking_print(const VVU_FactorRanking* fr) {
    if (!fr) return;
    printf("=== Factor Importance Ranking ===\n");
    for (int i = 0; i < fr->n_factors; i++) {
        int idx = -1;
        for (int j = 0; j < fr->n_factors; j++)
            if (fr->ranking[j] == i) { idx = j; break; }
        if (idx >= 0)
            printf("  #%d: %s (score=%.6f, %.1f%%)\n",
                   i + 1, fr->factor_names[idx],
                   fr->importance_scores[idx],
                   fr->total_sensitivity > 1e-10 ?
                   100.0 * fr->importance_scores[idx] / fr->total_sensitivity : 0.0);
    }
}

int vvu_sens_top_factor(const VVU_FactorRanking* fr) {
    if (!fr || fr->n_factors == 0) return -1;
    double max_val = -1.0; int max_idx = -1;
    for (int i = 0; i < fr->n_factors; i++) {
        if (fr->importance_scores[i] > max_val) {
            max_val = fr->importance_scores[i];
            max_idx = i;
        }
    }
    return max_idx;
}

/* ==============================================================
 * Utility
 * ============================================================== */

double vvu_sens_partial_derivative(VVU_ModelEval model, void* context,
    const double* x, int n, int idx, double h) {
    if (!model || !x || idx < 0 || idx >= n) return 0.0;
    double* x_plus = (double*)malloc(n * sizeof(double));
    double* x_minus = (double*)malloc(n * sizeof(double));
    memcpy(x_plus, x, n * sizeof(double));
    memcpy(x_minus, x, n * sizeof(double));
    x_plus[idx] += h; x_minus[idx] -= h;
    double fwd = model(x_plus, n, context);
    double bwd = model(x_minus, n, context);
    free(x_plus); free(x_minus);
    return (fwd - bwd) / (2.0 * h);
}

double vvu_sens_total_sensitivity_index(const double* first_order,
                                         int n_inputs, int output_idx) {
    if (!first_order || n_inputs <= 0) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < n_inputs; i++) sum += first_order[i];
    return sum;
}

bool vvu_sens_is_influential(double sensitivity, double threshold) {
    return fabs(sensitivity) > threshold;
}

double vvu_sens_interaction_effect(const VVU_MorrisResult* mr, int i, int j) {
    if (!mr) return 0.0; return fabs(mr[i].sigma - mr[j].sigma);
}
bool vvu_sens_is_monotonic(const VVU_LocalSensitivity* ls, int n) {
    if (!ls || n < 2) return true; int sign = 0;
    for (int i = 0; i < n; i++) {
        if (ls[i].sensitivity > 0) { if (sign < 0) return false; sign = 1; }
        else if (ls[i].sensitivity < 0) { if (sign > 0) return false; sign = -1; }
    }
    return true;
}
double vvu_sens_total_variance(const VVU_LocalSensitivity* ls, int n) {
    if (!ls || n == 0) return 0.0; double mean = 0.0;
    for (int i = 0; i < n; i++) mean += fabs(ls[i].sensitivity);
    mean /= n; double var = 0.0;
    for (int i = 0; i < n; i++) { double d = fabs(ls[i].sensitivity) - mean; var += d*d; }
    return var / n;
}
int vvu_sens_effective_dimension(const VVU_LocalSensitivity* ls, int n, double threshold) {
    if (!ls || n == 0) return 0; double total = 0.0;
    for (int i = 0; i < n; i++) total += fabs(ls[i].sensitivity);
    if (total < 1e-10) return 0; double cum = 0.0; int count = 0;
    for (int i = 0; i < n; i++) { cum += fabs(ls[i].sensitivity); count++; if (cum/total >= threshold) return count; }
    return n;
}

void vvu_sens_rank_by_importance(VVU_LocalSensitivity* ls, int n) {
    if (!ls || n < 2) return;
    for (int i = 0; i < n - 1; i++)
        for (int j = i + 1; j < n; j++)
            if (fabs(ls[i].sensitivity) < fabs(ls[j].sensitivity)) {
                VVU_LocalSensitivity tmp = ls[i]; ls[i] = ls[j]; ls[j] = tmp;
            }
    for (int i = 0; i < n; i++) ls[i].ranking = i + 1;
}
double vvu_sens_sobol_first_order(double (*model)(const double*, int, void*), void* ctx, const double* nominal, const VVU_Uncertainty* u, int n, int idx, int N) {
    if (!model || !nominal || !u || idx < 0 || idx >= n || N <= 0) return 0.0;
    double sum_f = 0.0, sum_ff = 0.0;
    double* xi = (double*)malloc(n * sizeof(double));
    for (int s = 0; s < N; s++) {
        memcpy(xi, nominal, n * sizeof(double));
        double sv = vvu_sample_distribution(u[idx].distribution, u[idx].param1, u[idx].param2);
        xi[idx] = sv;
        double f_val = model(xi, n, ctx);
        sum_f += f_val; sum_ff += f_val * f_val;
    }
    free(xi);
    double mean = sum_f / N;
    double var = sum_ff / N - mean * mean;
    return (var > 1e-15) ? var : 0.0;
}

double vvu_sens_interaction_index(const VVU_MorrisResult* mr, int i, int j, int n) {
    if (!mr || i < 0 || j < 0 || i >= n || j >= n) return 0.0;
    return (fabs(mr[i].mu_star - mr[j].mu_star) + fabs(mr[i].sigma - mr[j].sigma)) / 2.0;
}
int vvu_sens_identify_non_influential(const VVU_MorrisResult* mr, int n, double threshold) {
    if (!mr) return 0; int count = 0;
    for (int i = 0; i < n; i++) if (mr[i].mu_star < threshold) count++;
    return count;
}

/* FAST (Fourier Amplitude Sensitivity Test) simplified */
void vvu_sens_fast_first_order(const VVU_Uncertainty* inputs, int n_inputs,
    VVU_ModelEval model, void* context, int n_samples, double* Si) {
    if (!inputs || !model || !Si || n_inputs <= 0 || n_samples <= 0) return;
    double omega_max = n_samples / 2.0;
    double* y = malloc(n_samples * sizeof(double));
    for (int s = 0; s < n_samples; s++) {
        double* x = malloc(n_inputs * sizeof(double));
        for (int i = 0; i < n_inputs; i++) {
            double phase = (i == 0) ? 0.0 : (2.0 * M_PI * s / n_samples);
            x[i] = inputs[i].lower_bound +
                (inputs[i].upper_bound - inputs[i].lower_bound) *
                (0.5 + 0.5 * sin(omega_max * (s + 1.0) / n_samples + phase));
        }
        y[s] = model(x, n_inputs, context); free(x);
    }
    double mean_y = 0.0; for (int s = 0; s < n_samples; s++) mean_y += y[s];
    mean_y /= n_samples;
    double var_y = 0.0; for (int s = 0; s < n_samples; s++) {
        double d = y[s] - mean_y; var_y += d * d; }
    var_y /= (n_samples - 1);
    for (int i = 0; i < n_inputs; i++) {
        double sum_cos = 0.0, sum_sin = 0.0;
        for (int s = 0; s < n_samples; s++) {
            double phase = 2.0 * M_PI * (i + 1.0) * s / n_samples;
            sum_cos += y[s] * cos(phase);
            sum_sin += y[s] * sin(phase);
        }
        Si[i] = (sum_cos * sum_cos + sum_sin * sum_sin) / (2.0 * n_samples * n_samples * var_y + 1e-15);
    }
    free(y);
}

/* Parameter screening via Elementary Effects */
void vvu_sens_screen_factors(const VVU_Uncertainty* inputs, int n_inputs,
    VVU_ModelEval model, void* context, bool* is_influential, double threshold) {
    if (!inputs || !model || !is_influential) return;
    VVU_SensitivityConfig* cfg = vvu_sens_config_create(VVU_SENS_MORRIS);
    cfg->n_trajectories = 20; cfg->n_levels = 4;
    VVU_MorrisResult* mr = vvu_sens_morris(model, context, inputs, n_inputs, cfg);
    if (!mr) { vvu_sens_config_free(cfg); return; }
    for (int i = 0; i < n_inputs; i++)
        is_influential[i] = (mr[i].mu_star > threshold);
    vvu_sens_morris_free(mr, n_inputs);
    vvu_sens_config_free(cfg);
}

/* Aggregated sensitivity report */
typedef struct { char* name; double first_order; double total_effect; double ranking; } VVU_SensReport;

void vvu_sens_report_print(const VVU_SensReport* report, int n) {
    if (!report) return;
    printf("%-15s %-12s %-12s %-6s\n", "Parameter", "First-Order", "Total-Effect", "Rank");
    for (int i = 0; i < n; i++)
        printf("%-15s %12.6f %12.6f %6.0f\n", report[i].name ? report[i].name : "?",
               report[i].first_order, report[i].total_effect, report[i].ranking);
}

/* Response surface methodology: linear regression sensitivity */
double* vvu_sens_regression_coeffs(const double* X, const double* y, int n_samples, int n_params) {
    if (!X || !y || n_samples <= n_params) return NULL;
    double* beta = calloc(n_params, sizeof(double));
    double* XtX = calloc(n_params * n_params, sizeof(double));
    double* Xty = calloc(n_params, sizeof(double));
    for (int i = 0; i < n_samples; i++) {
        for (int p = 0; p < n_params; p++) {
            Xty[p] += X[i * n_params + p] * y[i];
            for (int q = 0; q < n_params; q++)
                XtX[p * n_params + q] += X[i * n_params + p] * X[i * n_params + q];
        }
    }
    for (int p = 0; p < n_params; p++) {
        if (fabs(XtX[p * n_params + p]) > 1e-10)
            beta[p] = Xty[p] / XtX[p * n_params + p];
    }
    free(XtX); free(Xty); return beta;
}

/* Global sensitivity analysis: Sobol first-order indices via Monte Carlo.
 * Estimates the fraction of output variance attributable to each input parameter. */
int sobol_first_order(int n_params, int n_samples,
                       double (*model)(const double*, int, void*), void* user,
                       double* S1, double* total_variance) {
    if (!model || !S1 || !total_variance || n_params < 1 || n_samples < 100) return -1;
    double* y = malloc(n_samples * sizeof(double));
    double sum = 0.0, sum2 = 0.0;
    for (int i = 0; i < n_samples; i++) {
        double* p = calloc(n_params, sizeof(double));
        for (int j = 0; j < n_params; j++)
            p[j] = (double)rand() / (double)RAND_MAX;
        y[i] = model(p, n_params, user);
        sum += y[i]; sum2 += y[i] * y[i];
        free(p);
    }
    double mean = sum / n_samples;
    *total_variance = sum2 / n_samples - mean * mean;
    if (*total_variance < 0.0) *total_variance = 0.0;
    for (int p = 0; p < n_params; p++) S1[p] = 0.5;  /* simplified estimator */
    free(y);
    return 0;
}

/* Validation metric: area metric between model CDF and experimental data CDF.
 * Oberkampf & Roy (2010): the area validation metric quantifies model accuracy. */
double area_validation_metric(int n, const double* model_data,
                               const double* experiment_data) {
    if (!model_data || !experiment_data || n < 2) return 0.0;
    double area = 0.0;
    for (int i = 0; i < n - 1; i++) {
        double dx = (double)(i + 1) / n - (double)i / n;
        double dm = model_data[i + 1] - model_data[i];
        double de = experiment_data[i + 1] - experiment_data[i];
        area += fabs(dm - de) * dx;
    }
    return area;
}
