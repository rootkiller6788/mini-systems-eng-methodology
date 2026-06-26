#ifndef VVU_REPORT_H
#define VVU_REPORT_H
#include "vvu_core.h"
#include "vvu_metrics.h"

typedef enum { VVU_FMT_TEXT=0, VVU_FMT_JSON=1, VVU_FMT_CSV=2, VVU_FMT_MARKDOWN=3 } VVU_ReportFormat;

/* V&V Metrics Structures (per INCOSE SE Handbook §6.3) */
typedef struct {
    double requirements_coverage;    /* fraction of reqs with verification */
    double test_coverage;            /* fraction of tests executed */
    double traceability_coverage;    /* fraction of reqs traced to tests */
    int verified_requirements;       /* count of verified requirements */
} VVU_CoverageMetrics;

typedef struct {
    double defect_density;           /* defects per KLOC or per req */
    int passed_tests;                /* count of passed verification tests */
    int failed_tests;                /* count of failed verification tests */
    double pass_rate;                /* fraction of tests passed */
    double requirement_stability;    /* fraction of reqs unchanged */
} VVU_QualityMetrics;

typedef struct {
    double overall_confidence;       /* composite confidence score [0-1] */
    double margin_of_error;          /* statistical margin of error */
    int sample_size;                 /* number of data points used */
} VVU_ConfidenceMetrics;

typedef struct {
    double completion_pct;           /* fraction of V&V activities done */
    double on_track_score;           /* schedule adherence [0-1] */
    int days_remaining;              /* estimated days to completion */
} VVU_ProgressMetrics;

/* Compute metrics from project state */
VVU_CoverageMetrics   vvu_compute_coverage_metrics(const VVU_Project* proj);
VVU_QualityMetrics    vvu_compute_quality_metrics(const VVU_Project* proj);
VVU_ConfidenceMetrics vvu_compute_confidence_metrics(const VVU_Project* proj, int sample_size);
VVU_ProgressMetrics   vvu_compute_progress(const VVU_Project* proj);

typedef struct {
    char* title; char* author; char* date;
    VVU_CoverageMetrics coverage; VVU_QualityMetrics quality;
    VVU_ConfidenceMetrics confidence; VVU_ProgressMetrics progress;
    double pass_rate; double alignment_mean;
    int total_reqs; int total_tests; int total_validations;
    int critical_verified; int critical_total;
    bool gate_ready; char* summary; char** recommendations; int n_recs;
} VVU_Report;

VVU_Report* vvu_report_generate(const VVU_Project* proj, const char* title, const char* author);
void vvu_report_free(VVU_Report* rpt);
void vvu_report_print(const VVU_Report* rpt, VVU_ReportFormat fmt);
void vvu_report_add_recommendation(VVU_Report* rpt, const char* rec);
char* vvu_report_serialize_json(const VVU_Report* rpt);
char* vvu_report_serialize_csv(const VVU_Report* rpt);

typedef struct {
    double* observed; double* expected; int n;
    double chi_squared; double p_value; int degrees_of_freedom;
    bool is_significant; double significance_level;
} VVU_ChiSquaredTest;

typedef struct {
    double* sample_a; double* sample_b; int n_a; int n_b;
    double t_statistic; double p_value; double mean_diff;
    double pooled_std; bool is_significant; double significance_level;
} VVU_TTest;

typedef struct {
    double* x; double* y; int n; double slope; double intercept;
    double r_squared; double f_statistic; double p_value;
    bool is_significant; double significance_level;
} VVU_Regression;

VVU_ChiSquaredTest* vvu_stat_chi_squared(const double* observed, const double* expected, int n, double alpha);
void vvu_stat_chi_free(VVU_ChiSquaredTest* cs);
void vvu_stat_chi_print(const VVU_ChiSquaredTest* cs);

VVU_TTest* vvu_stat_t_test(const double* a, int na, const double* b, int nb, double alpha);
void vvu_stat_t_free(VVU_TTest* tt);
void vvu_stat_t_print(const VVU_TTest* tt);

VVU_Regression* vvu_stat_regression(const double* x, const double* y, int n, double alpha);
void vvu_stat_regression_free(VVU_Regression* reg);
void vvu_stat_regression_print(const VVU_Regression* reg);

double vvu_stat_mean(const double* data, int n);
double vvu_stat_variance(const double* data, int n);
double vvu_stat_stddev(const double* data, int n);
double vvu_stat_median(double* data, int n);
double vvu_stat_percentile(double* data, int n, double p);

#endif
