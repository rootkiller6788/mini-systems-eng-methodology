#ifndef VVU_SENSITIVITY_H
#define VVU_SENSITIVITY_H

#include "vvu_core.h"
#include "vvu_monte_carlo.h"

/* ==============================================================
 * vvu_sensitivity.h - Sensitivity Analysis Methods
 *
 * Sensitivity analysis determines how variation in model inputs
 * affects model outputs. Essential for:
 *   - Identifying critical parameters
 *   - Prioritizing uncertainty reduction efforts
 *   - Model simplification (fix insensitive parameters)
 *
 * Methods implemented:
 *   1. Local (one-at-a-time / OAT): dY/dXi at nominal point
 *   2. Morris screening: elementary effects for factor screening
 *   3. Sobol' variance decomposition: ANOVA for nonlinear models
 *
 * References:
 *   Saltelli et al. (2008) Global Sensitivity Analysis: The Primer
 *   Morris (1991) Factorial Sampling Plans
 *   Sobol' (1993) Sensitivity Estimates for Nonlinear Models
 * ============================================================== */

/* --- Sensitivity Method --- */
typedef enum {
    VVU_SENS_LOCAL = 0,        /* One-at-a-time perturbation */
    VVU_SENS_MORRIS = 1,       /* Morris elementary effects */
    VVU_SENS_SOBOL = 2,        /* Sobol' variance decomposition */
    VVU_SENS_FAST = 3,         /* Fourier Amplitude Sensitivity Test */
    VVU_SENS_REGRESSION = 4    /* Standardized regression coefficients */
} VVU_SensitivityMethod;

/* --- Local Sensitivity Result --- */
typedef struct {
    int input_index;
    char* input_name;
    double nominal_value;
    double perturbation;
    double sensitivity;           /* dY/dX_i */
    double normalized_sensitivity; /* (dY/dX) * (X/Y) */
    double ranking;               /* Importance rank */
} VVU_LocalSensitivity;

/* --- Morris Screening Result --- */
typedef struct {
    int input_index;
    char* input_name;
    double mu_star;              /* Mean of absolute elementary effects */
    double mu;                   /* Mean of elementary effects */
    double sigma;                /* Std dev of elementary effects */
    double ranking;              /* 1 = most important */
} VVU_MorrisResult;

/* --- Factor Prioritization --- */
typedef struct {
    char** factor_names;
    double* importance_scores;
    int n_factors;
    int* ranking;                /* 0 = most important */
    double total_sensitivity;
} VVU_FactorRanking;

/* --- Sensitivity Analysis Configuration --- */
typedef struct {
    VVU_SensitivityMethod method;
    int n_trajectories;          /* For Morris: number of trajectories */
    int n_levels;                /* For Morris: grid levels */
    int n_base_samples;          /* For Sobol': base samples */
    double perturbation_fraction; /* For local: delta fraction */
    bool compute_second_order;   /* For Sobol': Sij interactions */
} VVU_SensitivityConfig;

/* --- API: Configuration --- */
VVU_SensitivityConfig* vvu_sens_config_create(VVU_SensitivityMethod method);
void vvu_sens_config_free(VVU_SensitivityConfig* cfg);
void vvu_sens_config_set_trajectories(VVU_SensitivityConfig* cfg, int n);
void vvu_sens_config_set_levels(VVU_SensitivityConfig* cfg, int n);

/* --- API: Local Sensitivity --- */
VVU_LocalSensitivity* vvu_sens_local(VVU_ModelEval model, void* context,
    const double* nominal, int n_inputs, double delta);
void vvu_sens_local_free(VVU_LocalSensitivity* ls, int n);
void vvu_sens_local_print(const VVU_LocalSensitivity* ls, int n);
int vvu_sens_most_sensitive(const VVU_LocalSensitivity* ls, int n);

/* --- API: Morris Screening --- */
VVU_MorrisResult* vvu_sens_morris(VVU_ModelEval model, void* context,
    const VVU_Uncertainty* inputs, int n_inputs,
    const VVU_SensitivityConfig* cfg);
void vvu_sens_morris_free(VVU_MorrisResult* mr, int n);
void vvu_sens_morris_print(const VVU_MorrisResult* mr, int n);

/* --- API: Factor Ranking --- */
VVU_FactorRanking* vvu_sens_rank_factors(const VVU_MorrisResult* morris,
                                          int n_factors);
void vvu_sens_ranking_free(VVU_FactorRanking* fr);
void vvu_sens_ranking_print(const VVU_FactorRanking* fr);
int vvu_sens_top_factor(const VVU_FactorRanking* fr);

/* --- API: Utility --- */
double vvu_sens_partial_derivative(VVU_ModelEval model, void* context,
    const double* x, int n, int idx, double h);
double vvu_sens_total_sensitivity_index(const double* first_order,
                                         int n_inputs, int output_idx);
bool vvu_sens_is_influential(double sensitivity, double threshold);

#endif
