#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "lifecycle_core.h"
#include "lifecycle_metrics.h"

/* Lifecycle Metrics ? Earned Value, Technical Measures, Quality */

LifecycleDashboard* dash_create(const char* name) {
    LifecycleDashboard* dash = calloc(1, sizeof(LifecycleDashboard));
    if (!dash) return NULL;
    { LifecycleSystem* _t = lc_create(name, LC_MODEL_V); dash->base = *_t; free(_t); }
    dash->evm_enabled = false;
    dash->meas_capacity = 16;
    dash->measures = calloc((size_t)dash->meas_capacity, sizeof(TechnicalMeasure));
    dash->n_measures = 0;
    dash->schedule_float = 10.0;
    dash->is_on_schedule = true;
    dash->is_on_budget = true;
    dash->completion_rate = 0.0;
    dash->estimated_remaining_days = 365.0;
    dash->productivity = 1.0;
    dash->health_score = 1.0;
    dash->warning_flags = 0;
    return dash;
}

void dash_free(LifecycleDashboard* dash) {
    if (!dash) return;
    if (dash->measures) {
        for (int i = 0; i < dash->n_measures; i++)
            free(dash->measures[i].history);
        free(dash->measures);
    }
    lc_cleanup(&dash->base);
    free(dash);
}

void dash_set_budget(LifecycleDashboard* dash, double bac) {
    if (!dash) return;
    dash->evm.budget_at_completion = bac;
    dash->evm_enabled = (bac > 0);
}

void dash_record_progress(LifecycleDashboard* dash, double pv, double ev, double ac) {
    if (!dash) return;
    dash->evm.planned_value = pv;
    dash->evm.earned_value = ev;
    dash->evm.actual_cost = ac;
    dash->evm.schedule_variance = ev - pv;
    dash->evm.cost_variance = ev - ac;
    double pv_clamp = (fabs(pv) < 1e-10) ? 1e-10 : pv;
    double ac_clamp = (fabs(ac) < 1e-10) ? 1e-10 : ac;
    dash->evm.schedule_performance_index = ev / pv_clamp;
    dash->evm.cost_performance_index = ev / ac_clamp;
    double bac = dash->evm.budget_at_completion;
    if (fabs(bac - ac) > 1e-10)
        dash->evm.to_complete_performance_index = (bac - ev) / (bac - ac);
    dash->is_on_schedule = (dash->evm.schedule_variance >= -0.01 * pv);
    dash->is_on_budget = (dash->evm.cost_variance >= -0.01 * ac);
    dash->base.overall_progress = (fabs(bac) > 1e-10) ? ev / bac : 0.0;
}

EarnedValueMetrics dash_compute_evm(LifecycleDashboard* dash) {
    EarnedValueMetrics zero = {0};
    return dash ? dash->evm : zero;
}

double dash_spi(LifecycleDashboard* dash) { return dash ? dash->evm.schedule_performance_index : 0.0; }
double dash_cpi(LifecycleDashboard* dash) { return dash ? dash->evm.cost_performance_index : 0.0; }
double dash_tcpi(LifecycleDashboard* dash) { return dash ? dash->evm.to_complete_performance_index : 0.0; }

TechnicalMeasure* dash_add_measure(LifecycleDashboard* dash, const char* name,
    double target, double threshold, double objective) {
    if (!dash || !name) return NULL;
    if (dash->n_measures >= dash->meas_capacity) {
        dash->meas_capacity *= 2;
        dash->measures = realloc(dash->measures,
            (size_t)dash->meas_capacity * sizeof(TechnicalMeasure));
    }
    TechnicalMeasure* tm = &dash->measures[dash->n_measures];
    tm->name = strdup(name);
    tm->target_value = target;
    tm->current_value = 0.0;
    tm->threshold = threshold;
    tm->objective = objective;
    tm->margin = 0.0;
    tm->is_met = false;
    tm->history_cap = 50;
    tm->history = calloc(50, sizeof(double));
    tm->history_len = 0;
    dash->n_measures++;
    return tm;
}

void dash_update_measure(LifecycleDashboard* dash, int idx, double value) {
    if (!dash || idx < 0 || idx >= dash->n_measures) return;
    TechnicalMeasure* tm = &dash->measures[idx];
    tm->current_value = value;
    tm->margin = fabs(tm->threshold) > 1e-10 ? (value - tm->threshold) / fabs(tm->threshold) : 0.0;
    tm->is_met = (value >= tm->threshold);
    if (tm->history_len >= tm->history_cap) {
        tm->history_cap *= 2;
        tm->history = realloc(tm->history, (size_t)tm->history_cap * sizeof(double));
    }
    tm->history[tm->history_len++] = value;
}

double dash_measure_margin(LifecycleDashboard* dash, int idx) {
    return (dash && idx >= 0 && idx < dash->n_measures) ? dash->measures[idx].margin : 0.0;
}

int dash_measures_met(LifecycleDashboard* dash) {
    if (!dash) return 0;
    int count = 0;
    for (int i = 0; i < dash->n_measures; i++)
        if (dash->measures[i].is_met) count++;
    return count;
}

int dash_measures_at_risk(LifecycleDashboard* dash) {
    if (!dash) return 0;
    int count = 0;
    for (int i = 0; i < dash->n_measures; i++)
        if (!dash->measures[i].is_met && dash->measures[i].margin < 0.1) count++;
    return count;
}

void dash_record_defect(LifecycleDashboard* dash, bool is_closed) {
    if (!dash) return;
    dash->quality.n_defects_total++;
    if (is_closed) {
        dash->quality.n_defects_closed++;
        dash->quality.n_defects_open--;
        if (dash->quality.n_defects_open < 0) dash->quality.n_defects_open = 0;
    } else {
        dash->quality.n_defects_open++;
    }
    if (dash->base.n_requirements > 0)
        dash->quality.defect_density = (double)dash->quality.n_defects_total / dash->base.n_requirements;
}

void dash_record_rework(LifecycleDashboard* dash, int hours) {
    if (!dash) return;
    dash->quality.n_rework_hours += hours;
}

double dash_first_pass_yield(LifecycleDashboard* dash) {
    if (!dash || dash->quality.n_defects_total == 0) return 1.0;
    return 1.0 - (double)dash->quality.n_defects_open / (dash->quality.n_defects_total + 1);
}

double dash_defect_density(LifecycleDashboard* dash) {
    return dash ? dash->quality.defect_density : 0.0;
}

double dash_health_score(LifecycleDashboard* dash) {
    if (!dash) return 0.0;
    double score = 1.0;
    if (dash->evm.schedule_performance_index < 0.85) { score -= 0.2; dash->warning_flags |= 1; }
    if (dash->evm.cost_performance_index < 0.85) { score -= 0.2; dash->warning_flags |= 2; }
    if (dash->quality.defect_density > 0.5) { score -= 0.2; dash->warning_flags |= 4; }
    double at_risk = dash->n_measures > 0 ? (double)dash_measures_at_risk(dash) / dash->n_measures : 0;
    if (at_risk > 0.3) { score -= 0.2; dash->warning_flags |= 8; }
    if (score < 0.0) score = 0.0;
    dash->health_score = score;
    return score;
}

bool dash_is_healthy(LifecycleDashboard* dash) {
    return dash ? dash_health_score(dash) >= 0.6 : false;
}

const char* dash_health_assessment(LifecycleDashboard* dash) {
    double s = dash_health_score(dash);
    if (s >= 0.9) return "Excellent ? On track, all indicators green";
    if (s >= 0.7) return "Good ? Minor concerns, monitor closely";
    if (s >= 0.5) return "Caution ? Significant issues, corrective action needed";
    if (s >= 0.3) return "Warning ? Critical problems, immediate action required";
    return "Critical ? Project in jeopardy, replan or terminate";
}

void dash_print(LifecycleDashboard* dash) {
    if (!dash) { printf("LifecycleDashboard: NULL\n"); return; }
    printf("=== Lifecycle Dashboard: %s ===\n", dash->base.name);
    printf("  Health: %.2f ? %s\n", dash_health_score(dash), dash_health_assessment(dash));
    printf("  Progress: %.1f%%  SPI=%.2f  CPI=%.2f\n",
           100.0 * dash->base.overall_progress,
           dash_spi(dash), dash_cpi(dash));
    printf("  Schedule: %s  Budget: %s\n",
           dash->is_on_schedule ? "ON TRACK" : "BEHIND",
           dash->is_on_budget ? "ON BUDGET" : "OVER BUDGET");
    printf("  Defects: %d open / %d total (%.2f per req)\n",
           dash->quality.n_defects_open, dash->quality.n_defects_total,
           dash_defect_density(dash));
    printf("  Technical Measures: %d/%d met, %d at risk\n",
           dash_measures_met(dash), dash->n_measures,
           dash_measures_at_risk(dash));
}

void dash_print_evm(LifecycleDashboard* dash) {
    if (!dash) { printf("EVM: NULL\n"); return; }
    printf("=== Earned Value Metrics ===\n");
    printf("  PV=%.1f  EV=%.1f  AC=%.1f  BAC=%.1f\n",
           dash->evm.planned_value, dash->evm.earned_value,
           dash->evm.actual_cost, dash->evm.budget_at_completion);
    printf("  SV=%.1f  CV=%.1f  SPI=%.2f  CPI=%.2f  TCPI=%.2f\n",
           dash->evm.schedule_variance, dash->evm.cost_variance,
           dash->evm.schedule_performance_index,
           dash->evm.cost_performance_index,
           dash->evm.to_complete_performance_index);
}

/* ====================================================================
 * Extended Metrics Functions
 * ==================================================================== */

double dash_schedule_variance_days(LifecycleDashboard* dash) {
    if (!dash) return 0.0;
    return dash->evm.schedule_variance;
}

double dash_cost_variance_dollars(LifecycleDashboard* dash) {
    if (!dash) return 0.0;
    return dash->evm.cost_variance;
}

double dash_estimate_at_completion(LifecycleDashboard* dash) {
    if (!dash || fabs(dash->evm.cost_performance_index) < 1e-12) return 0.0;
    return dash->evm.budget_at_completion / dash->evm.cost_performance_index;
}

double dash_variance_at_completion(LifecycleDashboard* dash) {
    if (!dash) return 0.0;
    double eac = dash_estimate_at_completion(dash);
    return dash->evm.budget_at_completion - eac;
}

int dash_earned_schedule_days(LifecycleDashboard* dash, double planned_days) {
    if (!dash) return 0;
    return (int)(planned_days * dash->evm.schedule_performance_index);
}

double dash_to_complete_performance_index(LifecycleDashboard* dash) {
    return dash_tcpi(dash);
}

bool dash_is_recoverable(LifecycleDashboard* dash) {
    if (!dash) return false;
    /* Recoverable if TCPI < 1.10 (achievable with reasonable effort) */
    return dash_tcpi(dash) < 1.10;
}

const char* dash_trend_analysis(LifecycleDashboard* dash) {
    if (!dash) return "No data";
    double spi = dash_spi(dash), cpi = dash_cpi(dash);
    if (spi > 1.0 && cpi > 1.0) return "Ahead of schedule and under budget";
    if (spi > 1.0 && cpi < 1.0) return "Ahead of schedule but over budget";
    if (spi < 1.0 && cpi > 1.0) return "Behind schedule but under budget";
    if (spi < 0.8 || cpi < 0.8) return "Critical -- major corrective action needed";
    return "Behind schedule and over budget -- needs attention";
}

int dash_estimate_remaining_weeks(LifecycleDashboard* dash, int total_weeks) {
    if (!dash || total_weeks <= 0) return 0;
    double progress = dash->base.overall_progress;
    if (progress < 0.01) return total_weeks;
    double elapsed = total_weeks * progress / 1.0;
    if (fabs(dash->evm.schedule_performance_index) < 1e-12) return total_weeks;
    return (int)((total_weeks - elapsed) / dash->evm.schedule_performance_index);
}


/* Additional metrics and dashboards */
double dash_budget_efficiency(LifecycleDashboard* dash) {
    if (!dash || fabs(dash->evm.actual_cost) < 1e-12) return 0.0;
    return dash->evm.earned_value / dash->evm.actual_cost;
}

double dash_completion_rate_per_day(LifecycleDashboard* dash) {
    if (!dash || dash->base.elapsed_days < 1e-10) return 0.0;
    return dash->base.overall_progress / dash->base.elapsed_days;
}

double dash_defect_discovery_rate(LifecycleDashboard* dash) {
    if (!dash || dash->base.elapsed_days < 1e-10) return 0.0;
    return (double)dash->quality.n_defects_total / dash->base.elapsed_days;
}

int dash_forecast_defects_total(LifecycleDashboard* dash) {
    if (!dash) return 0;
    double rate = dash_defect_discovery_rate(dash);
    double remaining_days = dash->base.total_schedule_days - dash->base.elapsed_days;
    if (remaining_days < 0) remaining_days = 0;
    return dash->quality.n_defects_total + (int)(rate * remaining_days);
}

bool dash_is_late(LifecycleDashboard* dash) {
    return dash && dash->evm.schedule_variance < -0.05 * dash->evm.planned_value;
}

bool dash_is_over_budget(LifecycleDashboard* dash) {
    return dash && dash->evm.cost_variance < -0.05 * dash->evm.actual_cost;
}

int dash_warning_count(LifecycleDashboard* dash) {
    if (!dash) return 0;
    int count = 0;
    for (int i = 0; i < 4; i++) if (dash->warning_flags & (1 << i)) count++;
    return count;
}


/* Earned Value Management (EVM) for lifecycle progress tracking.
 * SPI = EV/PV (schedule), CPI = EV/AC (cost).
 * ANSI/EIA-748 standard for systems engineering projects. */
int evm_compute(double planned_value, double earned_value, double actual_cost,
                 double* spi, double* cpi, int* on_track) {
    if (!spi || !cpi || !on_track) return -1;
    *spi = (planned_value > 1e-10) ? earned_value / planned_value : 1.0;
    *cpi = (actual_cost > 1e-10) ? earned_value / actual_cost : 1.0;
    *on_track = (*spi >= 0.9 && *cpi >= 0.9) ? 1 : 0;
    return 0;
}

/* Technical debt accumulation across lifecycle phases.
 * Each phase can add or resolve technical debt. */
double technical_debt(int n_phases, const double* debt_added, const double* debt_resolved) {
    if (!debt_added || !debt_resolved || n_phases <= 0) return 0.0;
    double total = 0.0;
    for (int i = 0; i < n_phases; i++)
        total += debt_added[i] - debt_resolved[i];
    return (total > 0.0) ? total : 0.0;
}
