#include "vvu_monte_carlo.h"
#include "vvu_core.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define PI 3.14159265358979323846

/* ==============================================================
 * MC Configuration
 * ============================================================== */

VVU_MCConfig* vvu_mc_config_create(int n_samples, int n_inputs) {
    VVU_MCConfig* cfg = (VVU_MCConfig*)calloc(1, sizeof(VVU_MCConfig));
    if (!cfg) return NULL;
    cfg->n_samples = n_samples;
    cfg->n_inputs = n_inputs;
    cfg->n_outputs = 1;
    cfg->random_seed = 42;
    cfg->convergence_tol = 0.01;
    cfg->max_samples = n_samples * 10;
    return cfg;
}

void vvu_mc_config_free(VVU_MCConfig* cfg) { free(cfg); }
void vvu_mc_config_set_lhs(VVU_MCConfig* cfg, bool enable) { if (cfg) cfg->use_latin_hypercube = enable; }
void vvu_mc_config_set_seed(VVU_MCConfig* cfg, int seed) { if (cfg) cfg->random_seed = seed; }

/* ==============================================================
 * Random Sampling
 * ============================================================== */

double* vvu_mc_generate_samples(const VVU_Uncertainty* uncerts, int n,
                                 int n_samples, int seed) {
    if (!uncerts || n <= 0 || n_samples <= 0) return NULL;
    srand(seed);
    double* samples = (double*)malloc(n * n_samples * sizeof(double));
    for (int s = 0; s < n_samples; s++) {
        for (int i = 0; i < n; i++) {
            double val = vvu_sample_distribution(uncerts[i].distribution,
                uncerts[i].param1, uncerts[i].param2);
            samples[s * n + i] = val;
        }
    }
    return samples;
}

/* ==============================================================
 * Latin Hypercube Sampling
 * ============================================================== */

static int compare_double(const void* a, const void* b) {
    double da = *(const double*)a, db = *(const double*)b;
    return (da > db) - (da < db);
}

double* vvu_mc_lhs_samples(const VVU_Uncertainty* uncerts, int n,
                            int n_samples, int seed) {
    if (!uncerts || n <= 0 || n_samples <= 0) return NULL;
    srand(seed);
    double* samples = (double*)malloc(n * n_samples * sizeof(double));

    for (int i = 0; i < n; i++) {
        /* Generate stratified samples */
        double* strata = (double*)malloc(n_samples * sizeof(double));
        for (int s = 0; s < n_samples; s++) {
            double u = (s + (double)rand() / RAND_MAX) / n_samples;
            /* Inverse CDF based on distribution */
            switch (uncerts[i].distribution) {
                case VVU_DIST_UNIFORM:
                    strata[s] = uncerts[i].lower_bound + u *
                        (uncerts[i].upper_bound - uncerts[i].lower_bound);
                    break;
                case VVU_DIST_NORMAL: {
                    /* Probit approximation */
                    double t = sqrt(-2.0 * log(u < 0.5 ? u : 1.0 - u));
                    double z = t - (2.515517 + 0.802853*t + 0.010328*t*t) /
                        (1.0 + 1.432788*t + 0.189269*t*t + 0.001308*t*t*t);
                    if (u < 0.5) z = -z;
                    strata[s] = uncerts[i].mean + z * uncerts[i].std_dev;
                    break;
                }
                default:
                    strata[s] = uncerts[i].lower_bound + u *
                        (uncerts[i].upper_bound - uncerts[i].lower_bound);
                    break;
            }
        }
        /* Randomly permute the strata */
        int* perm = (int*)malloc(n_samples * sizeof(int));
        for (int s = 0; s < n_samples; s++) perm[s] = s;
        for (int s = n_samples - 1; s > 0; s--) {
            int j = rand() % (s + 1);
            int tmp = perm[s]; perm[s] = perm[j]; perm[j] = tmp;
        }
        for (int s = 0; s < n_samples; s++)
            samples[s * n + i] = strata[perm[s]];
        free(strata); free(perm);
    }
    return samples;
}

/* ==============================================================
 * Monte Carlo Run
 * ============================================================== */

VVU_MCStatistics* vvu_mc_run(const VVU_Uncertainty* uncerts, int n_uncert,
                              VVU_ModelEval model, void* context,
                              const VVU_MCConfig* config) {
    if (!uncerts || !model || !config) return NULL;
    int N = config->n_samples;
    int n_in = config->n_inputs;
    int n_out = config->n_outputs;

    double* samples = config->use_latin_hypercube ?
        vvu_mc_lhs_samples(uncerts, n_in, N, config->random_seed) :
        vvu_mc_generate_samples(uncerts, n_in, N, config->random_seed);

    double* outputs = (double*)malloc(N * n_out * sizeof(double));

    for (int s = 0; s < N; s++) {
        double* input = &samples[s * n_in];
        double* result = (double*)malloc(n_out * sizeof(double));
        for (int o = 0; o < n_out; o++)
            result[o] = model(input, n_in, context);
        for (int o = 0; o < n_out; o++)
            outputs[s * n_out + o] = result[o];
        free(result);
    }
    free(samples);

    VVU_MCStatistics* stats = vvu_mc_compute_statistics(outputs, n_out, N);
    free(outputs);
    return stats;
}

/* ==============================================================
 * Compute Statistics
 * ============================================================== */

VVU_MCStatistics* vvu_mc_compute_statistics(const double* outputs,
                                             int n_outputs, int n_samples) {
    if (!outputs || n_outputs <= 0 || n_samples <= 0) return NULL;
    VVU_MCStatistics* stats = (VVU_MCStatistics*)calloc(1, sizeof(VVU_MCStatistics));
    stats->n_outputs = n_outputs;
    stats->mean = (double*)calloc(n_outputs, sizeof(double));
    stats->std_dev = (double*)calloc(n_outputs, sizeof(double));
    stats->variance = (double*)calloc(n_outputs, sizeof(double));
    stats->median = (double*)calloc(n_outputs, sizeof(double));
    stats->min_val = (double*)calloc(n_outputs, sizeof(double));
    stats->max_val = (double*)calloc(n_outputs, sizeof(double));
    stats->skewness = (double*)calloc(n_outputs, sizeof(double));
    stats->kurtosis = (double*)calloc(n_outputs, sizeof(double));
    stats->p5 = (double*)calloc(n_outputs, sizeof(double));
    stats->p25 = (double*)calloc(n_outputs, sizeof(double));
    stats->p75 = (double*)calloc(n_outputs, sizeof(double));
    stats->p95 = (double*)calloc(n_outputs, sizeof(double));

    for (int o = 0; o < n_outputs; o++) {
        /* Extract column */
        double* col = (double*)malloc(n_samples * sizeof(double));
        for (int s = 0; s < n_samples; s++)
            col[s] = outputs[s * n_outputs + o];

        /* Sort for percentiles */
        double* sorted = (double*)malloc(n_samples * sizeof(double));
        memcpy(sorted, col, n_samples * sizeof(double));
        qsort(sorted, n_samples, sizeof(double), compare_double);

        /* Mean */
        double sum = 0.0;
        for (int s = 0; s < n_samples; s++) sum += col[s];
        stats->mean[o] = sum / n_samples;

        /* Min/Max/Median/Percentiles */
        stats->min_val[o] = sorted[0];
        stats->max_val[o] = sorted[n_samples - 1];
        int m_idx = n_samples / 2;
        stats->median[o] = (n_samples % 2 == 0) ?
            (sorted[m_idx - 1] + sorted[m_idx]) / 2.0 : sorted[m_idx];
        stats->p5[o] = sorted[(int)(0.05 * n_samples)];
        stats->p25[o] = sorted[n_samples / 4];
        stats->p75[o] = sorted[3 * n_samples / 4];
        stats->p95[o] = sorted[(int)(0.95 * n_samples)];

        /* Variance & Std Dev */
        double var_sum = 0.0;
        for (int s = 0; s < n_samples; s++) {
            double d = col[s] - stats->mean[o];
            var_sum += d * d;
        }
        stats->variance[o] = var_sum / (n_samples - 1);
        stats->std_dev[o] = sqrt(stats->variance[o]);

        /* Skewness & Kurtosis */
        double m3 = 0.0, m4 = 0.0;
        for (int s = 0; s < n_samples; s++) {
            double d = (col[s] - stats->mean[o]) / stats->std_dev[o];
            m3 += d * d * d;
            m4 += d * d * d * d;
        }
        stats->skewness[o] = m3 / n_samples;
        stats->kurtosis[o] = m4 / n_samples - 3.0;

        free(col); free(sorted);
    }
    return stats;
}

void vvu_mc_statistics_free(VVU_MCStatistics* stats) {
    if (!stats) return;
    free(stats->mean); free(stats->std_dev); free(stats->variance);
    free(stats->median); free(stats->min_val); free(stats->max_val);
    free(stats->skewness); free(stats->kurtosis);
    free(stats->p5); free(stats->p25); free(stats->p75); free(stats->p95);
    free(stats);
}

void vvu_mc_statistics_print(const VVU_MCStatistics* stats) {
    if (!stats) { printf("MC Stats: NULL\n"); return; }
    printf("=== Monte Carlo Results (%d outputs) ===\n", stats->n_outputs);
    for (int o = 0; o < stats->n_outputs; o++) {
        printf("Output %d:\n", o);
        printf("  Mean=%.4f  Std=%.4f  Median=%.4f\n",
               stats->mean[o], stats->std_dev[o], stats->median[o]);
        printf("  Range: [%.4f, %.4f]\n", stats->min_val[o], stats->max_val[o]);
        printf("  5%%=%.4f  25%%=%.4f  75%%=%.4f  95%%=%.4f\n",
               stats->p5[o], stats->p25[o], stats->p75[o], stats->p95[o]);
    }
}

/* ==============================================================
 * Convergence Check
 * ============================================================== */

bool vvu_mc_check_convergence(const double* running_mean, int n_outputs,
                               int current_sample, double tol) {
    if (current_sample < 10) return false;
    /* Simple check: relative change in mean */
    for (int o = 0; o < n_outputs; o++) {
        int idx1 = (current_sample - 10) * n_outputs + o;
        int idx2 = (current_sample - 1) * n_outputs + o;
        double change = fabs(running_mean[idx2] - running_mean[idx1]);
        double ref = fabs(running_mean[idx2]) + 1e-10;
        if (change / ref > tol) return false;
    }
    return true;
}

int vvu_mc_required_samples(double desired_precision, double variance_est) {
    /* N = (z_alpha/2 * sigma / precision)^2, using z = 1.96 for 95% */
    double z = 1.96;
    double n = pow(z * sqrt(variance_est) / desired_precision, 2.0);
    return (int)ceil(n);
}

/* ==============================================================
 * Confidence Intervals
 * ============================================================== */

double vvu_mc_confidence_interval_width(const double* samples, int n,
                                         double confidence_level) {
    if (!samples || n < 2) return INFINITY;
    double sum = 0.0, sumsq = 0.0;
    for (int i = 0; i < n; i++) {
        sum += samples[i];
        sumsq += samples[i] * samples[i];
    }
    double mean = sum / n;
    double var = (sumsq - sum * sum / n) / (n - 1);
    double se = sqrt(var / n);
    /* t-value approximation for 95% CI */
    double t = 1.96;
    if (confidence_level > 0.99) t = 2.576;
    else if (confidence_level > 0.95) t = 2.0;
    else if (confidence_level > 0.90) t = 1.645;
    return 2.0 * t * se;
}

double* vvu_mc_sobol_sequence(int n_samples, int n_dims, int skip) {
    double* seq = (double*)malloc(n_samples * n_dims * sizeof(double));
    if (!seq) return NULL;
    for (int s = 0; s < n_samples; s++)
        for (int d = 0; d < n_dims; d++) {
            double x = 0.0; double f = 0.5;
            int n = s + skip + 1;
            while (n > 0) { x += f * (n % 2); n /= 2; f *= 0.5; }
            seq[s * n_dims + d] = x;
        }
    return seq;
}
double vvu_mc_skewness(const double* samples, int n) {
    if (!samples || n < 3) return 0.0; double sum = 0.0;
    for (int i = 0; i < n; i++) sum += samples[i];
    double mean = sum / n; double m2 = 0.0, m3 = 0.0;
    for (int i = 0; i < n; i++) { double d = samples[i] - mean; m2 += d*d; m3 += d*d*d; }
    double var = m2 / (n - 1);
    return (var > 1e-15) ? m3 / n / pow(sqrt(var), 3.0) : 0.0;
}
double vvu_mc_kurtosis(const double* samples, int n) {
    if (!samples || n < 4) return 0.0; double sum = 0.0;
    for (int i = 0; i < n; i++) sum += samples[i];
    double mean = sum / n; double m2 = 0.0, m4 = 0.0;
    for (int i = 0; i < n; i++) { double d = samples[i] - mean; m2 += d*d; m4 += d*d*d*d; }
    double var = m2 / (n - 1);
    return (var > 1e-15) ? m4 / n / (var * var) - 3.0 : 0.0;
}
double vvu_mc_probability_of_exceedance(const double* samples, int n, double threshold) {
    if (!samples || n == 0) return 0.0; int count = 0;
    for (int i = 0; i < n; i++) if (samples[i] > threshold) count++;
    return (double)count / n;
}


double vvu_mc_value_at_risk(const double* samples, int n, double confidence) {
    if (!samples || n == 0) return 0.0;
    double* sorted = (double*)malloc(n * sizeof(double));
    memcpy(sorted, samples, n * sizeof(double));
    for (int i = 0; i < n - 1; i++)
        for (int j = i + 1; j < n; j++)
            if (sorted[i] > sorted[j]) { double t = sorted[i]; sorted[i] = sorted[j]; sorted[j] = t; }
    int idx = (int)((1.0 - confidence) * n);
    if (idx < 0) idx = 0; if (idx >= n) idx = n - 1;
    double var = sorted[idx]; free(sorted); return var;
}
double vvu_mc_expected_shortfall(const double* samples, int n, double confidence) {
    if (!samples || n == 0) return 0.0;
    double var = vvu_mc_value_at_risk(samples, n, confidence);
    double sum = 0.0; int count = 0;
    for (int i = 0; i < n; i++) if (samples[i] <= var) { sum += samples[i]; count++; }
    return count > 0 ? sum / count : var;
}
int vvu_mc_optimal_samples(double precision, double variance, double z_score) {
    return (int)ceil(pow(z_score * sqrt(variance) / precision, 2.0));
}

double* vvu_mc_bootstrap_mean(const double* samples, int n, int n_bootstrap) {
    if (!samples || n <= 0) return NULL;
    double* means = (double*)malloc(n_bootstrap * sizeof(double));
    for (int b = 0; b < n_bootstrap; b++) {
        double sum = 0.0;
        for (int i = 0; i < n; i++)
            sum += samples[rand() % n];
        means[b] = sum / n;
    }
    return means;
}
double vvu_mc_bootstrap_ci_width(const double* samples, int n, int n_bootstrap, double cl) {
    double* means = vvu_mc_bootstrap_mean(samples, n, n_bootstrap);
    if (!means) return INFINITY;
    for (int i = 0; i < n_bootstrap - 1; i++)
        for (int j = i + 1; j < n_bootstrap; j++)
            if (means[i] > means[j]) { double t = means[i]; means[i] = means[j]; means[j] = t; }
    int lo = (int)((1.0 - cl) / 2.0 * n_bootstrap);
    int hi = (int)((1.0 + cl) / 2.0 * n_bootstrap);
    if (lo < 0) lo = 0; if (hi >= n_bootstrap) hi = n_bootstrap - 1;
    double width = means[hi] - means[lo];
    free(means); return width;
}
double vvu_mc_correlation(const double* x, const double* y, int n) {
    if (!x || !y || n < 2) return 0.0;
    double mx = 0.0, my = 0.0;
    for (int i = 0; i < n; i++) { mx += x[i]; my += y[i]; }
    mx /= n; my /= n;
    double cov = 0.0, vx = 0.0, vy = 0.0;
    for (int i = 0; i < n; i++) {
        double dx = x[i] - mx, dy = y[i] - my;
        cov += dx * dy; vx += dx * dx; vy += dy * dy;
    }
    double den = sqrt(vx * vy);
    return (den > 1e-15) ? cov / den : 0.0;
}

double vvu_mc_empirical_cdf(const double* samples, int n, double x) {
    if (!samples || n == 0) return 0.0; int count = 0;
    for (int i = 0; i < n; i++) if (samples[i] <= x) count++;
    return (double)count / n;
}
double vvu_mc_quantile(const double* samples, int n, double q) {
    if (!samples || n == 0 || q < 0 || q > 1) return 0.0;
    double* sorted = (double*)malloc(n * sizeof(double));
    memcpy(sorted, samples, n * sizeof(double));
    for (int i = 0; i < n-1; i++)
        for (int j = i+1; j < n; j++)
            if (sorted[i] > sorted[j]) { double t = sorted[i]; sorted[i] = sorted[j]; sorted[j] = t; }
    int idx = (int)(q * (n - 1));
    if (idx < 0) idx = 0; if (idx >= n) idx = n - 1;
    double val = sorted[idx]; free(sorted); return val;
}

/* ==============================================================
*/
