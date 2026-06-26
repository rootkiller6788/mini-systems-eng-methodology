#include "hsm_core.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ==============================================================
 * HSM Lifecycle Cost Analysis
 *
 * Hall's (1962) methodology emphasizes total lifecycle cost.
 * This implements the standard cost breakdown structure for
 * systems engineering programs.
 * ============================================================== */

/* Compute total lifecycle cost: RDTE + production + O&S + disposal */
double hsm_lifecycle_cost(double rdte, double production, double ops_support,
                           double disposal, int n_years_ops) {
    double npv_ops = 0.0;
    double discount = 0.05;
    for (int y = 0; y < n_years_ops; y++)
        npv_ops += ops_support / pow(1.0 + discount, (double)y);
    return rdte + production + npv_ops + disposal;
}

/* Risk-adjusted cost estimate with optimistic/most-likely/pessimistic */
double hsm_pert_cost_estimate(double optimistic, double most_likely,
                               double pessimistic) {
    return (optimistic + 4.0 * most_likely + pessimistic) / 6.0;
}

/* Schedule variance: SPI = Earned Value / Planned Value */
double hsm_schedule_performance_index(double earned_value, double planned_value) {
    if (fabs(planned_value) < 1e-10) return 1.0;
    return earned_value / planned_value;
}

/* Cost variance: CPI = Earned Value / Actual Cost */
double hsm_cost_performance_index(double earned_value, double actual_cost) {
    if (fabs(actual_cost) < 1e-10) return 1.0;
    return earned_value / actual_cost;
}

/* Estimate at Completion (EAC) based on CPI trend */
double hsm_estimate_at_completion(double budget_at_completion, double cpi) {
    if (fabs(cpi) < 1e-10) return budget_at_completion * 2.0;
    return budget_at_completion / cpi;
}

/* Technical performance measurement: compute margin to threshold */
double hsm_technical_margin(double current_value, double threshold,
                             bool higher_is_better) {
    if (higher_is_better)
        return (current_value - threshold) / (fabs(threshold) + 1e-10) * 100.0;
    else
        return (threshold - current_value) / (fabs(threshold) + 1e-10) * 100.0;
}

/* Requirements volatility index = changed_reqs / total_reqs */
double hsm_requirements_volatility(int n_changed, int n_total) {
    if (n_total < 1) return 0.0;
    return (double)n_changed / (double)n_total;
}

/* System readiness level (SRL) from component TRLs and integration readiness */
double hsm_system_readiness_level(const double* trl_values, int n_components,
                                   const double* irl_matrix, int n_interfaces) {
    if (!trl_values || n_components < 1) return 0.0;
    double sum_trl = 0.0, sum_irl = 0.0;
    for (int i = 0; i < n_components; i++) sum_trl += trl_values[i];
    sum_trl /= (double)n_components;
    for (int i = 0; i < n_interfaces && irl_matrix; i++)
        sum_irl += irl_matrix[i];
    if (n_interfaces > 0) sum_irl /= (double)n_interfaces;
    else sum_irl = 1.0;
    return sum_trl * sum_irl / 9.0; /* Normalized to [0,1] */
}

/* Work breakdown structure: count elements at each level */
int hsm_wbs_element_count(int n_level_1, const int* children_per_element,
                           int n_elements) {
    if (!children_per_element || n_elements < 1) return n_level_1;
    int total = n_level_1;
    for (int i = 0; i < n_elements; i++) total += children_per_element[i];
    return total;
}

/* HSM decision matrix: weighted scoring for alternative selection */
int hsm_decision_matrix(const double* scores, const double* weights,
                         int n_alternatives, int n_criteria, int* best_idx) {
    if (!scores || !weights || !best_idx || n_alternatives < 1) return -1;
    double best_score = -INFINITY;
    *best_idx = 0;
    for (int i = 0; i < n_alternatives; i++) {
        double weighted = 0.0;
        for (int j = 0; j < n_criteria; j++)
            weighted += scores[i*n_criteria+j] * weights[j];
        if (weighted > best_score) { best_score = weighted; *best_idx = i; }
    }
    return 0;
}

/* Program evaluation and review technique: expected duration */
double hsm_pert_duration(double optimistic, double most_likely, double pessimistic) {
    return (optimistic + 4.0 * most_likely + pessimistic) / 6.0;
}

/* HSM phase gate review: check deliverables completeness */
int hsm_phase_gate_check(const bool* deliverables_complete, int n_deliverables) {
    if (!deliverables_complete || n_deliverables < 1) return -1;
    for (int i = 0; i < n_deliverables; i++)
        if (!deliverables_complete[i]) return 0;
    return 1;
}

/* Configuration management: version baseline index */
int hsm_configuration_baseline(const int* item_versions, int n_items,
                                int* baseline_version) {
    if (!item_versions || !baseline_version || n_items < 1) return -1;
    int max_ver = 0;
    for (int i = 0; i < n_items; i++)
        if (item_versions[i] > max_ver) max_ver = item_versions[i];
    *baseline_version = max_ver + 1;
    return 0;
}
/* HSM reliability prediction: MIL-HDBK-217 parts count method */
double hsm_reliability_prediction(const double* failure_rates, const int* quantities,
                                    int n_part_types, double environment_factor) {
    if (!failure_rates || !quantities || n_part_types < 1) return 0.0;
    double total_rate = 0.0;
    for (int i = 0; i < n_part_types; i++)
        total_rate += failure_rates[i] * (double)quantities[i] * environment_factor;
    return total_rate;
}

/* HSM maintainability: MTTR = sum(lambda_i * MTTR_i) / sum(lambda_i) */
double hsm_mean_time_to_repair(const double* failure_rates, const double* repair_times,
                                int n_components) {
    if (!failure_rates || !repair_times || n_components < 1) return 0.0;
    double sum_lambda = 0.0, weighted_repair = 0.0;
    for (int i = 0; i < n_components; i++) {
        sum_lambda += failure_rates[i];
        weighted_repair += failure_rates[i] * repair_times[i];
    }
    return (sum_lambda > 1e-15) ? weighted_repair / sum_lambda : 0.0;
}

/* HSM availability: A = MTBF / (MTBF + MTTR) */
double hsm_inherent_availability(double mtbf, double mttr) {
    if (fabs(mtbf + mttr) < 1e-15) return 0.0;
    return mtbf / (mtbf + mttr);
}
/* HSM trade study: compare alternatives by weighted criteria */
int hsm_trade_study_count(int n_alternatives) { return n_alternatives; }// HSM spare parts provisioning: optimal inventory level
int hsm_spare_parts_level(double mtbf, double lead_time, double confidence) { if(confidence<=0)return 0; return (int)ceil(-log(1.0-confidence)*lead_time/mtbf); }
// HSM obsolescence risk: years until component end-of-life
double hsm_obsolescence_risk(int years_to_eol, int system_life) { return (system_life>0)?1.0-(double)years_to_eol/(double)system_life:1.0; }
