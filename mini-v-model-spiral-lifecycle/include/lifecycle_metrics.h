#ifndef LIFECYCLE_METRICS_H
#define LIFECYCLE_METRICS_H

#include "lifecycle_core.h"

/* ============================================================================
 * Lifecycle Metrics ? Progress, Quality, Earned Value, Performance
 *
 * Based on:
 *   ANSI/EIA-748 ? Earned Value Management Systems
 *   INCOSE ? Technical Measurement Guide
 *   SEI ? CMMI Measurement and Analysis Process Area
 * ============================================================================ */

/* --- Earned Value Metrics --- */
typedef struct {
    double planned_value;          /* PV: Budgeted cost of work scheduled */
    double earned_value;           /* EV: Budgeted cost of work performed */
    double actual_cost;            /* AC: Actual cost of work performed */
    double budget_at_completion;   /* BAC: Total budget baseline */
    double estimate_at_completion; /* EAC: Forecast total cost */
    double estimate_to_complete;   /* ETC: Remaining cost */
    double schedule_variance;      /* SV = EV - PV */
    double cost_variance;          /* CV = EV - AC */
    double schedule_performance_index; /* SPI = EV / PV */
    double cost_performance_index;     /* CPI = EV / AC */
    double to_complete_performance_index; /* TCPI = (BAC-EV)/(BAC-AC) */
} EarnedValueMetrics;

/* --- Technical Performance Measures --- */
typedef struct {
    char*    name;
    double   target_value;
    double   current_value;
    double   threshold;            /* Minimum acceptable */
    double   objective;            /* Desired/stretch goal */
    double   margin;               /* (current - threshold) / threshold */
    bool     is_met;
    double*  history;
    int      history_len;
    int      history_cap;
} TechnicalMeasure;

/* --- Quality Metrics --- */
typedef struct {
    int      n_defects_open;
    int      n_defects_closed;
    int      n_defects_total;
    double   defect_density;       /* Defects per requirement */
    double   defect_closure_rate;  /* Defects closed per day */
    double   defect_aging;         /* Average age of open defects */
    int      n_rework_hours;
    double   rework_percentage;    /* % effort spent on rework */
    double   first_pass_yield;     /* % correct on first attempt */
    double   technical_debt;       /* Accumulated shortcuts (hours) */
} QualityMetrics;

/* --- Lifecycle Dashboard --- */
typedef struct {
    LifecycleSystem base;

    /* Earned value */
    EarnedValueMetrics evm;
    bool   evm_enabled;

    /* Technical measures */
    TechnicalMeasure* measures;
    int    n_measures;
    int    meas_capacity;

    /* Quality */
    QualityMetrics quality;

    /* Schedule */
    double schedule_float;         /* Slack in schedule (days) */
    bool   is_on_schedule;
    bool   is_on_budget;

    /* Progress rate */
    double completion_rate;        /* % completed per day */
    double estimated_remaining_days;
    double productivity;           /* EV per person-day */

    /* Health indicators */
    double health_score;           /* 0.0 = critical, 1.0 = excellent */
    int    warning_flags;          /* Bitmask: schedule, cost, quality, risk */
} LifecycleDashboard;

/* --- Metrics API --- */
LifecycleDashboard* dash_create(const char* name);
void dash_free(LifecycleDashboard* dash);

/* Earned Value */
void dash_set_budget(LifecycleDashboard* dash, double bac);
void dash_record_progress(LifecycleDashboard* dash, double pv, double ev,
                           double ac);
EarnedValueMetrics dash_compute_evm(LifecycleDashboard* dash);
double dash_spi(LifecycleDashboard* dash);
double dash_cpi(LifecycleDashboard* dash);
double dash_tcpi(LifecycleDashboard* dash);

/* Technical Measures */
TechnicalMeasure* dash_add_measure(LifecycleDashboard* dash, const char* name,
    double target, double threshold, double objective);
void dash_update_measure(LifecycleDashboard* dash, int idx, double value);
double dash_measure_margin(LifecycleDashboard* dash, int idx);
int dash_measures_met(LifecycleDashboard* dash);
int dash_measures_at_risk(LifecycleDashboard* dash);

/* Quality */
void dash_record_defect(LifecycleDashboard* dash, bool is_closed);
void dash_record_rework(LifecycleDashboard* dash, int hours);
double dash_first_pass_yield(LifecycleDashboard* dash);
double dash_defect_density(LifecycleDashboard* dash);

/* Health */
double dash_health_score(LifecycleDashboard* dash);
bool dash_is_healthy(LifecycleDashboard* dash);
const char* dash_health_assessment(LifecycleDashboard* dash);

void dash_print(LifecycleDashboard* dash);
void dash_print_evm(LifecycleDashboard* dash);

#endif
