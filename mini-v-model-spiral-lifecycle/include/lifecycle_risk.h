#ifndef LIFECYCLE_RISK_H
#define LIFECYCLE_RISK_H

#include "lifecycle_core.h"

/* ============================================================================
 * Risk Management ? Identification, Analysis, Mitigation, Tracking
 *
 * Based on:
 *   Boehm ? Software Risk Management (1989)
 *   INCOSE ? Risk Management Process (ISO 15288)
 *   NASA ? Risk-Informed Decision Making Handbook
 *   DoD ? Risk Management Guide (MIL-STD-882E)
 * ============================================================================ */

/* --- Risk Matrix Entry --- */
typedef struct {
    RiskLevel probability;
    RiskLevel impact;
    double    exposure;            /* P ? I value */
    int       priority;            /* 1=highest priority */
} RiskMatrixEntry;

/* --- Risk Trend --- */
typedef struct {
    double*   exposure_history;
    int       n_points;
    int       capacity;
    double    trend_slope;         /* + = increasing risk, - = decreasing */
    bool      is_improving;
    bool      is_worsening;
} RiskTrend;

/* --- Risk Management Plan --- */
typedef struct {
    LifecycleSystem base;

    /* Risk registers */
    RiskItem*        watch_list;        /* Risks being monitored */
    int              n_watch;
    int              watch_capacity;

    /* Risk matrix */
    RiskMatrixEntry** risk_matrix;      /* [probability][impact] entries */
    int              matrix_size;       /* Typically 5?5 or 6?6 */

    /* Top risks */
    int*             top_risk_indices;
    int              n_top_risks;       /* Usually top 5-10 */

    /* Trends */
    RiskTrend*       trends;
    int              n_trends;

    /* Burn-down */
    double           initial_exposure;
    double           target_exposure;
    double           risk_burndown_rate;

    /* Risk budget */
    double           risk_budget;        /* Management reserve */
    double           risk_budget_spent;
    double           risk_budget_remaining;
} RiskManagementPlan;

/* --- Risk Management API --- */
RiskManagementPlan* risk_create(const char* name);
void risk_free(RiskManagementPlan* rmp);

/* Risk identification */
RiskItem* risk_add_risk(RiskManagementPlan* rmp, const char* id,
    const char* desc, RiskLevel prob, RiskLevel impact,
    const char* mitigation, const char* trigger);

/* Risk analysis */
double risk_compute_exposure(RiskLevel prob, RiskLevel impact);
void risk_update_exposure(RiskManagementPlan* rmp);
RiskLevel risk_exposure_to_level(double exposure);
void risk_identify_top_risks(RiskManagementPlan* rmp, int top_n);
double risk_total_exposure(RiskManagementPlan* rmp);

/* Risk mitigation */
void risk_apply_mitigation(RiskManagementPlan* rmp, const char* risk_id,
                            double cost);
void risk_accept_risk(RiskManagementPlan* rmp, const char* risk_id);
bool risk_is_acceptable(RiskManagementPlan* rmp, const char* risk_id,
                         double tolerance);
double risk_residual_exposure(RiskManagementPlan* rmp);

/* Risk tracking */
void risk_record_trend(RiskManagementPlan* rmp);
double risk_trend_slope(RiskManagementPlan* rmp);
bool risk_is_improving(RiskManagementPlan* rmp);
bool risk_is_worsening(RiskManagementPlan* rmp);

/* Risk budget */
void risk_set_budget(RiskManagementPlan* rmp, double budget);
double risk_budget_burn_rate(RiskManagementPlan* rmp);
double risk_budget_efficiency(RiskManagementPlan* rmp);
    /* Exposure reduced per budget spent */

void risk_print(RiskManagementPlan* rmp);
void risk_print_top_risks(RiskManagementPlan* rmp);
void risk_print_matrix(RiskManagementPlan* rmp);

#endif
