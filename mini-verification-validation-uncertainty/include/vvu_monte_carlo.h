#ifndef VVU_MONTE_CARLO_H
#define VVU_MONTE_CARLO_H

#include "vvu_core.h"

/* ==============================================================
 * vvu_monte_carlo.h - Monte Carlo Simulation Engine
 *
 * Monte Carlo methods propagate uncertainty through system models
 * by repeated random sampling:
 *
 *   1. Sample input distributions N times
 *   2. Run model for each sample
 *   3. Collect output statistics
 *
 * Convergence: O(1/sqrt(N)) for Monte Carlo
 * Latin Hypercube Sampling (LHS): improved convergence
 *
 * References:
 *   Metropolis & Ulam (1949) The Monte Carlo Method
 *   McKay et al. (1979) Latin Hypercube Sampling
 *   Robert & Casella (2004) Monte Carlo Statistical Methods
 * ============================================================== */

/* --- Model evaluation function signature --- */
typedef double (*VVU_ModelEval)(const double* inputs, int n_inputs,
                                 void* context);

/* --- MC Configuration --- */
typedef struct {
    int n_samples;               /* Number of Monte Carlo samples */
    int n_inputs;                /* Number of input uncertainties */
    int n_outputs;               /* Number of output quantities */
    bool use_latin_hypercube;    /* Enable LHS for better coverage */
    bool use_antithetic;         /* Antithetic variates for variance reduction */
    int random_seed;             /* Random seed for reproducibility */
    double convergence_tol;      /* Stop if statistics converge */
    int max_samples;             /* Maximum samples (adaptive mode) */
} VVU_MCConfig;

/* --- MC Result Statistics --- */
typedef struct {
    double* mean;
    double* std_dev;
    double* variance;
    double* median;
    double* p5;                  /* 5th percentile */
    double* p25;                 /* 25th percentile */
    double* p75;                 /* 75th percentile */
    double* p95;                 /* 95th percentile */
    double* min_val;
    double* max_val;
    double* skewness;
    double* kurtosis;
    int n_outputs;
} VVU_MCStatistics;

/* --- Sensitivity Indices (Sobol') --- */
typedef struct {
    int n_inputs;
    double* first_order;         /* Main effect (Si) */
    double* total_effect;        /* Total effect (STi) */
    double* second_order;        /* Interaction effects (Sij) */
    int n_second_order;
} VVU_SobolIndices;

/* --- API: Monte Carlo --- */
VVU_MCConfig* vvu_mc_config_create(int n_samples, int n_inputs);
void vvu_mc_config_free(VVU_MCConfig* cfg);
void vvu_mc_config_set_lhs(VVU_MCConfig* cfg, bool enable);
void vvu_mc_config_set_seed(VVU_MCConfig* cfg, int seed);

/* Generate random samples from uncertainty distributions */
double* vvu_mc_generate_samples(const VVU_Uncertainty* uncerts, int n,
                                 int n_samples, int seed);

/* Latin Hypercube Sampling */
double* vvu_mc_lhs_samples(const VVU_Uncertainty* uncerts, int n,
                            int n_samples, int seed);

/* Run Monte Carlo simulation */
VVU_MCStatistics* vvu_mc_run(const VVU_Uncertainty* uncerts, int n_uncert,
                              VVU_ModelEval model, void* context,
                              const VVU_MCConfig* config);
void vvu_mc_statistics_free(VVU_MCStatistics* stats);

/* Compute statistics from raw output samples */
VVU_MCStatistics* vvu_mc_compute_statistics(const double* outputs,
                                             int n_outputs, int n_samples);
void vvu_mc_statistics_print(const VVU_MCStatistics* stats);

/* --- API: Sensitivity Analysis --- */
VVU_SobolIndices* vvu_mc_sobol(const VVU_Uncertainty* uncerts, int n_uncert,
                                VVU_ModelEval model, void* context,
                                int n_base_samples);
void vvu_mc_sobol_free(VVU_SobolIndices* si);
void vvu_mc_sobol_print(const VVU_SobolIndices* si);

/* --- API: Convergence --- */
bool vvu_mc_check_convergence(const double* running_mean, int n_outputs,
                               int current_sample, double tol);
int vvu_mc_required_samples(double desired_precision, double variance_est);

/* --- API: Utility --- */
double* vvu_mc_percentiles(double* sorted_data, int n_samples, int n_outputs,
                            const double* percentiles, int n_percentiles);
double vvu_mc_confidence_interval_width(const double* samples, int n,
                                         double confidence_level);

#endif
