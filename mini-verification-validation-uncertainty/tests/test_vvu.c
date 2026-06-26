#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "vvu_core.h"
#include "vvu_monte_carlo.h"
#include "vvu_metrics.h"
#include "vvu_report.h"

#define EPSILON 1e-6
static int tests_run = 0, tests_passed = 0;
#define TEST(n) do { tests_run++; printf("  TEST %s... ", n); fflush(stdout); } while(0)
#define PASS() do { tests_passed++; printf("PASSED\n"); fflush(stdout); } while(0)
#define ASSERT_NEAR(a,b,eps) do { double _d=fabs((double)(a)-(double)(b)); if(_d>(eps)){printf("FAIL: %.10f != %.10f\n",(double)(a),(double)(b));assert(_d<=(eps));}} while(0)

/* --- Project Tests --- */
void test_project_create(void) {
    TEST("project_create");
    VVU_Project* p = vvu_project_create("TestProj", "TestSystem");
    assert(p); assert(strcmp(p->name, "TestProj") == 0);
    assert(strcmp(p->system_under_test, "TestSystem") == 0);
    assert(p->n_requirements == 0);
    vvu_project_free(p); PASS();
}

void test_req_add(void) {
    TEST("req_add");
    VVU_Project* p = vvu_project_create("test", "sys");
    int r0 = vvu_req_add(p, "REQ-001", "System shall boot in <2s", VVU_REQ_MANDATORY);
    assert(r0 == 0); assert(p->n_requirements == 1);
    int r1 = vvu_req_add(p, "REQ-002", "UI shall respond <100ms", VVU_REQ_CRITICAL);
    assert(r1 == 1);
    int found = vvu_req_find(p, "REQ-001");
    assert(found == 0);
    assert(vvu_req_find(p, "NONEXIST") == -1);
    vvu_project_free(p); PASS();
}

void test_req_verify(void) {
    TEST("req_verify");
    VVU_Project* p = vvu_project_create("test", "sys");
    vvu_req_add(p, "REQ-001", "desc", VVU_REQ_MANDATORY);
    assert(!p->requirements[0].is_verified);
    vvu_req_verify(p, 0, true);
    assert(p->requirements[0].is_verified);
    assert(p->requirements[0].verification_status == VVU_PASSED);
    vvu_req_validate(p, 0, true);
    assert(p->requirements[0].is_validated);
    vvu_project_free(p); PASS();
}

void test_verif_add(void) {
    TEST("verif_add");
    VVU_Project* p = vvu_project_create("test", "sys");
    vvu_req_add(p, "REQ-001", "Boot time", VVU_REQ_MANDATORY);
    int t0 = vvu_verif_add(p, "TEST-001", "Boot time measurement", "REQ-001", 2.0, 0.1);
    assert(t0 == 0); assert(p->n_verif_tests == 1);
    vvu_verif_run(p, 0, 1.95);
    assert(p->verif_tests[0].status == VVU_PASSED);
    assert(p->verif_tests[0].n_runs == 1);
    vvu_verif_run(p, 0, 3.0); /* This should fail */
    assert(p->verif_tests[0].status == VVU_FAILED);
    vvu_project_free(p); PASS();
}

void test_valid_add(void) {
    TEST("valid_add");
    VVU_Project* p = vvu_project_create("test", "sys");
    int a0 = vvu_valid_add(p, "Operator-X", "Ease of use", "User Study");
    assert(a0 == 0); assert(p->n_valid_assessments == 1);
    vvu_valid_score(p, 0, 0.85, 0.9);
    ASSERT_NEAR(p->valid_assessments[0].alignment_score, 0.85, EPSILON);
    ASSERT_NEAR(p->valid_assessments[0].confidence, 0.9, EPSILON);
    assert(p->valid_assessments[0].status == VVU_PASSED);
    vvu_valid_score(p, 0, 0.5, 0.6);
    assert(p->valid_assessments[0].status == VVU_FAILED);
    vvu_project_free(p); PASS();
}

void test_uncert_add(void) {
    TEST("uncert_add");
    VVU_Project* p = vvu_project_create("test", "sys");
    int u0 = vvu_uncert_add(p, "mass", VVU_UNCERT_PARAMETER, VVU_DIST_NORMAL, 100.0, 100.0, 5.0);
    assert(u0 == 0); assert(p->n_uncertainties == 1);
    ASSERT_NEAR(p->uncertainties[0].mean, 100.0, EPSILON);
    ASSERT_NEAR(p->uncertainties[0].std_dev, 5.0, EPSILON);
    vvu_uncert_set_bounds(p, 0, 85.0, 115.0);
    ASSERT_NEAR(p->uncertainties[0].lower_bound, 85.0, EPSILON);
    vvu_uncert_set_correlation(p, 0, -1, 0.5);
    assert(p->uncertainties[0].is_correlated);
    vvu_project_free(p); PASS();
}

void test_coverage(void) {
    TEST("coverage");
    VVU_Project* p = vvu_project_create("test", "sys");
    vvu_req_add(p, "R1", "d1", VVU_REQ_MANDATORY);
    vvu_req_add(p, "R2", "d2", VVU_REQ_CRITICAL);
    vvu_req_add(p, "R3", "d3", VVU_REQ_DESIRABLE);
    vvu_req_verify(p, 0, true); vvu_req_verify(p, 1, true);
    vvu_compute_coverage(p);
    ASSERT_NEAR(p->overall_verif_coverage, 2.0/3.0, EPSILON);
    VVU_CoverageMetrics cm = vvu_compute_coverage_metrics(p);
    assert(cm.verified_requirements == 2);
    assert(cm.verified_requirements <= 2);
    vvu_project_free(p); PASS();
}

void test_all_critical(void) {
    TEST("all_critical");
    VVU_Project* p = vvu_project_create("test", "sys");
    vvu_req_add(p, "R1", "d1", VVU_REQ_MANDATORY);
    vvu_req_add(p, "R2", "d2", VVU_REQ_CRITICAL);
    assert(!p->requirements[0].is_verified);
    vvu_req_verify(p, 0, true); vvu_req_verify(p, 1, true);
    assert(p->requirements[0].is_verified && p->requirements[1].is_verified);
    vvu_project_free(p); PASS();
}

void test_quality(void) {
    TEST("quality");
    VVU_Project* p = vvu_project_create("test", "sys");
    vvu_req_add(p, "R1", "d1", VVU_REQ_MANDATORY);
    vvu_verif_add(p, "T1", "test", "R1", 10.0, 0.5);
    vvu_verif_add(p, "T2", "test2", "R1", 20.0, 1.0);
    vvu_verif_run(p, 0, 10.2); /* Pass */
    vvu_verif_run(p, 1, 25.0); /* Fail */
    VVU_QualityMetrics qm = vvu_compute_quality_metrics(p);
    assert(qm.passed_tests == 1); assert(qm.failed_tests == 1);
    ASSERT_NEAR(qm.pass_rate, 0.5, EPSILON);
    vvu_project_free(p); PASS();
}

void test_confidence(void) {
    TEST("confidence");
    VVU_Project* p = vvu_project_create("test", "sys");
    vvu_req_add(p, "R1", "d1", VVU_REQ_MANDATORY);
    vvu_req_verify(p, 0, true);
    vvu_compute_coverage(p); vvu_compute_confidence(p);
    VVU_ConfidenceMetrics cm = vvu_compute_confidence_metrics(p, 30);
    assert(cm.overall_confidence >= 0.0 && cm.overall_confidence <= 1.0);
    vvu_project_free(p); PASS();
}

void test_coverage_metrics(void) {
    TEST("coverage_metrics");
    VVU_Project* p = vvu_project_create("test", "sys");
    vvu_req_add(p, "R1", "Critical safety req", VVU_REQ_CRITICAL);
    vvu_req_add(p, "R2", "Nice to have", VVU_REQ_DESIRABLE);
    vvu_req_verify(p, 0, true);
    VVU_CoverageMetrics cm = vvu_compute_coverage_metrics(p);
    assert(cm.verified_requirements == 1);
    assert(cm.requirements_coverage == 0.5);
    vvu_project_free(p); PASS();
}

void test_monte_carlo(void) {
    TEST("monte_carlo");
    VVU_Uncertainty u; memset(&u, 0, sizeof(u));
    u.name = "test"; u.distribution = VVU_DIST_UNIFORM;
    u.param1 = 0.0; u.param2 = 10.0;
    u.lower_bound = 0.0; u.upper_bound = 10.0;
    double* samples = vvu_mc_generate_samples(&u, 1, 1000, 42);
    assert(samples);
    double sum = 0.0;
    for (int i = 0; i < 1000; i++) sum += samples[i];
    double mean = sum / 1000.0;
    assert(mean > 3.0 && mean < 7.0); /* Should be ~5.0 */
    free(samples); PASS();
}

void test_lhs(void) {
    TEST("lhs");
    VVU_Uncertainty u; memset(&u, 0, sizeof(u));
    u.name = "test"; u.distribution = VVU_DIST_UNIFORM;
    u.param1 = 0.0; u.param2 = 10.0;
    u.lower_bound = 0.0; u.upper_bound = 10.0;
    double* lhs_samples = vvu_mc_lhs_samples(&u, 1, 100, 42);
    assert(lhs_samples);
    double min_v = lhs_samples[0], max_v = lhs_samples[0];
    for (int i = 0; i < 100; i++) {
        if (lhs_samples[i] < min_v) min_v = lhs_samples[i];
        if (lhs_samples[i] > max_v) max_v = lhs_samples[i];
    }
    assert(min_v > -0.01); assert(max_v < 10.01);
    free(lhs_samples); PASS();
}

void test_mc_stats(void) {
    TEST("mc_stats");
    double data[20];
    for (int i = 0; i < 20; i++) data[i] = (double)i;
    VVU_MCStatistics* stats = vvu_mc_compute_statistics(data, 1, 20);
    assert(stats); assert(stats->n_outputs == 1);
    ASSERT_NEAR(stats->mean[0], 9.5, EPSILON);
    ASSERT_NEAR(stats->min_val[0], 0.0, EPSILON);
    ASSERT_NEAR(stats->max_val[0], 19.0, EPSILON);
    vvu_mc_statistics_free(stats); PASS();
}

void test_ci_width(void) {
    TEST("ci_width");
    double data[30];
    for (int i = 0; i < 30; i++) data[i] = 5.0 + (double)i * 0.1;
    double w = vvu_mc_confidence_interval_width(data, 30, 0.95);
    assert(w > 0.0); assert(isfinite(w));
    PASS();
}

void test_distribution_sampling(void) {
    TEST("distribution_sampling");
    srand(42);
    double u = vvu_sample_distribution(VVU_DIST_UNIFORM, 0.0, 1.0);
    assert(u >= 0.0 && u <= 1.0);
    double n = vvu_sample_distribution(VVU_DIST_NORMAL, 5.0, 1.0);
    assert(n > -10.0 && n < 20.0); /* Wide bound for random */
    double e = vvu_sample_distribution(VVU_DIST_EXPONENTIAL, 2.0, 0.0);
    assert(e > 0.0);
    PASS();
}

void test_confidence_bounds(void) {
    TEST("confidence_bounds");
    /* Verify confidence interval calculation sanity */
    VVU_Project* p = vvu_project_create("test", "sys");
    vvu_req_add(p, "R1", "d1", VVU_REQ_MANDATORY);
    vvu_req_verify(p, 0, true);
    VVU_ConfidenceMetrics cm = vvu_compute_confidence_metrics(p, 100);
    double lo = cm.overall_confidence - cm.margin_of_error;
    double hi = cm.overall_confidence + cm.margin_of_error;
    assert(lo < hi);
    assert(cm.overall_confidence >= 0.0 && cm.overall_confidence <= 1.0);
    vvu_project_free(p); PASS();
}

int main(void) {
    printf("\n=== V&V Test Suite ===\n\n");
    test_project_create();
    test_req_add();
    test_req_verify();
    test_verif_add();
    test_valid_add();
    test_uncert_add();
    test_coverage();
    test_all_critical();
    test_quality();
    test_confidence();
    test_coverage_metrics();
    test_monte_carlo();
    test_lhs();
    test_mc_stats();
    test_ci_width();
    test_distribution_sampling();
    test_confidence_bounds();
    printf("\n=== Results: %d/%d tests passed ===\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
