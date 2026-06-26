#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "lifecycle_core.h"

/* ============================================================================
 * Verification & Validation Metrics
 * Pure computation functions operating on lifecycle system data.
 * No custom structs needed — uses LifecycleSystem fields directly.
 * ============================================================================ */

/* V&V Coverage: what fraction of requirements have been verified.
 * This is the primary metric for system readiness. */
double vv_coverage(LifecycleSystem* lc) {
    return lc_requirement_verification_coverage(lc);
}

/* V&V Gap: number of requirements lacking verification.
 * Every gap represents a risk at system acceptance. */
int vv_gap_count(LifecycleSystem* lc) {
    if (!lc) return 0;
    int gap = 0;
    for (int i = 0; i < lc->n_requirements; i++)
        if (!lc->requirements[i].is_verified) gap++;
    return gap;
}

/* Estimate verification effort (person-days) using parametric model.
 * Effort = A * n_reqs^B * (1 + complexity_factor * gap_ratio) */
double vv_estimate_effort(LifecycleSystem* lc, double complexity) {
    if (!lc || lc->n_requirements == 0) return 0.0;
    double A = 1.5, B = 1.05;
    double base = A * pow((double)lc->n_requirements, B);
    double gap_ratio = (double)vv_gap_count(lc) / (double)lc->n_requirements;
    return base * (1.0 + complexity * gap_ratio);
}

/* Verification debt: accumulated cost of unverified requirements.
 * Each unverified requirement will need testing effort later. */
double vv_verification_debt(LifecycleSystem* lc, double cost_per_test) {
    if (!lc) return 0.0;
    return (double)vv_gap_count(lc) * cost_per_test;
}

/* Defect containment: fraction of defects found before system integration.
 * Higher is better — defects are cheaper to fix earlier in the lifecycle. */
double vv_defect_containment(LifecycleSystem* lc, int defects_found_early) {
    if (!lc || lc->n_defects_found == 0) return 0.0;
    return (double)defects_found_early / (double)lc->n_defects_found;
}

/* Test effectiveness: defects found per test executed.
 * Low effectiveness suggests tests need improvement. */
double vv_test_effectiveness(LifecycleSystem* lc, int n_tests_executed) {
    if (!lc || n_tests_executed <= 0) return 0.0;
    return (double)lc->n_defects_found / (double)n_tests_executed;
}

/* V&V Progress: percentage of verification activities completed.
 * Based on verification coverage weighted by requirement priority. */
double vv_progress_weighted(LifecycleSystem* lc) {
    if (!lc || lc->n_requirements == 0) return 0.0;
    double weighted_verified = 0.0, weighted_total = 0.0;
    for (int i = 0; i < lc->n_requirements; i++) {
        double p = lc->requirements[i].priority;
        if (p < 0.0) p = 0.0;
        weighted_total += p;
        if (lc->requirements[i].is_verified) weighted_verified += p;
    }
    return weighted_total > 1e-12 ? weighted_verified / weighted_total : 0.0;
}

/* Verify requirements at a specific V-model level.
 * Returns number of requirements verified at that level. */
int vv_verify_at_level(LifecycleSystem* lc, VModelLevel level) {
    if (!lc) return 0;
    int count = 0;
    for (int i = 0; i < lc->n_requirements; i++) {
        if (lc->requirements[i].allocated_to == level) {
            lc->requirements[i].is_verified = true;
            count++;
        }
    }
    return count;
}

/* Validate requirements: check that the right system was built.
 * Validation = "did we build the right thing?" (vs verification = "did we build it right?") */
int vv_validate_requirements(LifecycleSystem* lc, bool* acceptance_criteria_met, int n_criteria) {
    if (!lc || !acceptance_criteria_met || n_criteria <= 0) return 0;
    int passed = 0;
    for (int i = 0; i < n_criteria; i++) {
        if (acceptance_criteria_met[i]) {
            if (i < lc->n_requirements) lc->requirements[i].is_validated = true;
            passed++;
        }
    }
    lc->validation_coverage = lc->n_requirements > 0 ?
        (double)passed / (double)lc->n_requirements : 0.0;
    return passed;
}

/* V&V Schedule variance: are we on track to complete verification?
 * Returns days ahead (positive) or behind (negative) schedule. */
double vv_schedule_variance(LifecycleSystem* lc, double planned_verification_days,
                              double actual_verification_days) {
    if (!lc) return 0.0;
    double planned_progress = lc->elapsed_days / (planned_verification_days + 1e-12);
    if (planned_progress > 1.0) planned_progress = 1.0;
    double expected_verified = planned_progress * (double)lc->n_requirements;
    int actually_verified = lc->n_requirements - vv_gap_count(lc);
    return (double)actually_verified - expected_verified;
}

/* V&V Completion Estimate: days until all verification is complete.
 * Based on current verification rate. */
double vv_estimate_completion_days(LifecycleSystem* lc) {
    if (!lc || lc->elapsed_days < 1e-10) return 365.0;
    int verified = lc->n_requirements - vv_gap_count(lc);
    if (verified <= 0) return 365.0;
    double rate = (double)verified / lc->elapsed_days;
    int remaining = vv_gap_count(lc);
    if (remaining <= 0) return 0.0;
    return (double)remaining / rate;
}

/* Requirements volatility: how stable are requirements during verification.
 * High volatility during V&V is a major schedule risk. */
double vv_requirements_volatility(LifecycleSystem* lc, int n_changed_since_baseline) {
    if (!lc || lc->n_requirements == 0) return 0.0;
    return (double)n_changed_since_baseline / (double)lc->n_requirements;
}

/* V&V Maturity Score (0-100): composite metric for verification process quality.
 * Factors: coverage, automation, pass rate, traceability */
double vv_maturity_score(LifecycleSystem* lc, double automation_pct,
                          double pass_rate, double traceability_pct) {
    if (!lc) return 0.0;
    double cov = vv_coverage(lc);
    double score = 25.0 * cov + 25.0 * automation_pct +
                   25.0 * pass_rate + 25.0 * traceability_pct;
    return (score > 100.0) ? 100.0 : score;
}

/* Print V&V status report */
void vv_print_status(LifecycleSystem* lc) {
    if (!lc) return;
    printf("=== V&V Status Report ===\n");
    printf("  Coverage: %.1f%%  Gap: %d requirements\n",
           100.0 * vv_coverage(lc), vv_gap_count(lc));
    printf("  Weighted progress: %.1f%%\n", 100.0 * vv_progress_weighted(lc));
    printf("  Defects: %d found, %d resolved\n",
           lc->n_defects_found, lc->n_defects_resolved);
    printf("  Validation coverage: %.1f%%\n", 100.0 * lc->validation_coverage);
    printf("  Estimated completion: %.0f days\n", vv_estimate_completion_days(lc));
}

