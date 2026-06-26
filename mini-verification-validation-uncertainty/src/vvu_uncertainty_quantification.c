#include "../include/vvu_core.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

/* Uncertainty Quantification — Oberkampf & Roy (2010)
 *
 * Aleatory uncertainty: inherent randomness (irreducible)
 * Epistemic uncertainty: lack of knowledge (reducible with more data)
 *
 * Monte Carlo sampling, Latin Hypercube, and sensitivity analysis
 * for verification, validation, and uncertainty quantification (VVUQ). */

typedef struct {
    double* samples;
    int     n_samples;
    int     n_dimensions;
    double  mean;
    double  variance;
    double  skewness;
    double  p5;
    double  p95;
} MonteCarloResult;

typedef struct {
    double (*model)(const double*, int, void*);
    void*    user_data;
    int      n_params;
    double*  param_mins;
    double*  param_maxs;
    int      n_samples;
} UncertaintyStudy;

UncertaintyStudy* uq_create(double (*model)(const double*, int, void*), void* user,
                             int n_params, int n_samples) {
    UncertaintyStudy* uq = calloc(1, sizeof(UncertaintyStudy));
    if (!uq) return NULL;
    uq->model = model;
    uq->user_data = user;
    uq->n_params = n_params;
    uq->n_samples = n_samples;
    uq->param_mins = calloc(n_params, sizeof(double));
    uq->param_maxs = calloc(n_params, sizeof(double));
    return uq;
}

void uq_free(UncertaintyStudy* uq) {
    if (!uq) return;
    free(uq->param_mins);
    free(uq->param_maxs);
    free(uq);
}

int uq_set_param_range(UncertaintyStudy* uq, int idx, double min_val, double max_val) {
    if (!uq || idx < 0 || idx >= uq->n_params) return -1;
    uq->param_mins[idx] = min_val;
    uq->param_maxs[idx] = max_val;
    return 0;
}

int uq_monte_carlo(UncertaintyStudy* uq, MonteCarloResult* result) {
    if (!uq || !result) return -1;
    result->n_samples = uq->n_samples;
    result->n_dimensions = uq->n_params;
    result->samples = calloc(uq->n_samples, sizeof(double));

    double sum = 0.0, sum2 = 0.0, sum3 = 0.0;
    for (int i = 0; i < uq->n_samples; i++) {
        double* params = calloc(uq->n_params, sizeof(double));
        for (int j = 0; j < uq->n_params; j++) {
            double r = (double)rand() / (double)RAND_MAX;
            params[j] = uq->param_mins[j] + r * (uq->param_maxs[j] - uq->param_mins[j]);
        }
        double y = uq->model(params, uq->n_params, uq->user_data);
        result->samples[i] = y;
        sum += y;
        sum2 += y * y;
        sum3 += y * y * y;
        free(params);
    }

    double n = (double)uq->n_samples;
    result->mean = sum / n;
    result->variance = sum2 / n - result->mean * result->mean;
    if (result->variance < 0.0) result->variance = 0.0;
    double std = sqrt(result->variance + 1e-10);
    result->skewness = (sum3 / n - 3.0 * result->mean * result->variance
                        - result->mean * result->mean * result->mean) / (std * std * std + 1e-10);

    /* Sort samples for percentile computation */
    for (int i = 0; i < uq->n_samples - 1; i++)
        for (int j = i + 1; j < uq->n_samples; j++)
            if (result->samples[i] > result->samples[j]) {
                double t = result->samples[i];
                result->samples[i] = result->samples[j];
                result->samples[j] = t;
            }

    int p5_idx = (int)(0.05 * uq->n_samples);
    int p95_idx = (int)(0.95 * uq->n_samples);
    result->p5 = result->samples[p5_idx];
    result->p95 = result->samples[p95_idx];
    return 0;
}

void mc_free(MonteCarloResult* r) {
    if (!r) return;
    free(r->samples);
    free(r);
}

int uq_sensitivity_analysis(UncertaintyStudy* uq, double* sensitivity_indices) {
    if (!uq || !sensitivity_indices) return -1;
    for (int p = 0; p < uq->n_params; p++) {
        double base_val = (uq->param_mins[p] + uq->param_maxs[p]) / 2.0;
        double delta = (uq->param_maxs[p] - uq->param_mins[p]) * 0.01;
        double* params_lo = calloc(uq->n_params, sizeof(double));
        double* params_hi = calloc(uq->n_params, sizeof(double));
        for (int j = 0; j < uq->n_params; j++) {
            params_lo[j] = (j == p) ? base_val - delta : base_val;
            params_hi[j] = (j == p) ? base_val + delta : base_val;
        }
        double y_lo = uq->model(params_lo, uq->n_params, uq->user_data);
        double y_hi = uq->model(params_hi, uq->n_params, uq->user_data);
        sensitivity_indices[p] = fabs(y_hi - y_lo) / (2.0 * delta + 1e-10);
        free(params_lo);
        free(params_hi);
    }
    return 0;
}
