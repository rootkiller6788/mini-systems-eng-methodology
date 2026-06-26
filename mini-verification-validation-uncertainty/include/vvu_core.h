#ifndef VVU_CORE_H
#define VVU_CORE_H

#include <stdbool.h>
#include <stddef.h>

/* ==============================================================
 * vvu_core.h - V&V Core Types & Constants
 *
 * Verification, Validation, and Uncertainty Quantification
 * for complex systems engineering.
 *
 * Verification: "Did we build the system right?"
 *   - Requirements traceability
 *   - Compliance testing
 *   - Formal specification conformance
 *
 * Validation: "Did we build the right system?"
 *   - Stakeholder needs alignment
 *   - Operational context fitness
 *   - Face validity / expert review
 *
 * Uncertainty: "How well do we know what we know?"
 *   - Aleatory (inherent randomness)
 *   - Epistemic (knowledge gaps)
 *   - Propagation through system models
 *
 * References:
 *   ISO 9001, IEEE 1012, DoD VV&A Guide
 *   Oberkampf & Roy (2010) Verification and Validation in Scientific Computing
 *   Saltelli et al. (2008) Global Sensitivity Analysis
 * ============================================================== */

/* --- V&V Status --- */
typedef enum {
    VVU_NOT_STARTED = 0,
    VVU_IN_PROGRESS = 1,
    VVU_PASSED = 2,
    VVU_FAILED = 3,
    VVU_CONDITIONAL = 4,
    VVU_NOT_APPLICABLE = 5
} VVU_Status;

/* --- Requirement Priority --- */
typedef enum {
    VVU_REQ_MANDATORY = 0,
    VVU_REQ_CRITICAL = 1,
    VVU_REQ_IMPORTANT = 2,
    VVU_REQ_DESIRABLE = 3,
    VVU_REQ_OPTIONAL = 4
} VVU_ReqPriority;

/* --- Uncertainty Type --- */
typedef enum {
    VVU_UNCERT_NONE = 0,
    VVU_UNCERT_ALEATORY = 1,     /* Inherent randomness (irreducible) */
    VVU_UNCERT_EPISTEMIC = 2,    /* Knowledge gaps (reducible with data) */
    VVU_UNCERT_MODEL_FORM = 3,   /* Model structure uncertainty */
    VVU_UNCERT_PARAMETER = 4,    /* Parameter value uncertainty */
    VVU_UNCERT_MEASUREMENT = 5,  /* Sensor/measurement error */
    VVU_UNCERT_HUMAN = 6         /* Human factors uncertainty */
} VVU_UncertaintyType;

/* --- Distribution Type for Uncertainty --- */
typedef enum {
    VVU_DIST_UNIFORM = 0,
    VVU_DIST_NORMAL = 1,
    VVU_DIST_LOGNORMAL = 2,
    VVU_DIST_TRIANGULAR = 3,
    VVU_DIST_BETA = 4,
    VVU_DIST_WEIBULL = 5,
    VVU_DIST_EXPONENTIAL = 6,
    VVU_DIST_DISCRETE = 7
} VVU_DistType;

/* --- Requirement Descriptor --- */
typedef struct {
    char* id;                    /* e.g., "REQ-001" */
    char* description;          /* Natural language description */
    VVU_ReqPriority priority;
    VVU_Status verification_status;
    VVU_Status validation_status;
    char* parent_id;            /* Parent requirement for traceability */
    int n_children;             /* Number of child requirements */
    char** child_ids;
    double satisfaction_score;  /* 0 to 1 */
    char* test_method;          /* How this requirement is verified */
    char* validated_by;         /* Who/stakeholder validated it */
    bool is_verified;
    bool is_validated;
} VVU_Requirement;

/* --- Verification Test --- */
typedef struct {
    char* id;
    char* description;
    char* req_id;              /* Linked requirement */
    VVU_Status status;
    double pass_threshold;     /* Numeric threshold for pass/fail */
    double measured_value;     /* Actual test result */
    double tolerance;          /* +/- tolerance */
    bool is_automated;
    int n_dependencies;
    char** dependency_ids;
    double execution_time_ms;
    int n_runs;
    int n_passes;
} VVU_VerificationTest;

/* --- Validation Assessment --- */
typedef struct {
    char* id;
    char* stakeholder;
    char* concern;            /* What is being validated */
    double alignment_score;   /* 0 = misaligned, 1 = perfect alignment */
    VVU_Status status;
    char* evidence;           /* Supporting evidence description */
    char* method;             /* Validation method used */
    double confidence;        /* Confidence in assessment (0-1) */
    int n_reviewers;
    double* reviewer_scores;
} VVU_ValidationAssessment;

/* --- Uncertainty Quantity --- */
typedef struct {
    char* name;
    VVU_UncertaintyType type;
    VVU_DistType distribution;
    double nominal;            /* Best estimate / nominal value */
    double lower_bound;        /* Lower bound (or mu - 3*sigma) */
    double upper_bound;        /* Upper bound (or mu + 3*sigma) */
    double param1;             /* Meaning depends on distribution */
    double param2;             /* Meaning depends on distribution */
    double mean;
    double std_dev;
    double variance;
    double skewness;
    double kurtosis;
    bool is_correlated;        /* Correlated with other uncertainties */
    int correlated_with;       /* Index of correlated quantity */
    double correlation_coeff;  /* Pearson correlation coefficient */
} VVU_Uncertainty;

/* --- V&V Project --- */
typedef struct {
    char* name;
    char* system_under_test;
    VVU_Requirement* requirements;
    int n_requirements;
    int req_capacity;
    VVU_VerificationTest* verif_tests;
    int n_verif_tests;
    int verif_capacity;
    VVU_ValidationAssessment* valid_assessments;
    int n_valid_assessments;
    int valid_capacity;
    VVU_Uncertainty* uncertainties;
    int n_uncertainties;
    int uncert_capacity;
    double overall_verif_coverage;   /* 0-1 */
    double overall_valid_coverage;   /* 0-1 */
    double overall_confidence;       /* 0-1 */
    VVU_Status overall_status;
} VVU_Project;

/* --- Core API: Project Lifecycle --- */
VVU_Project* vvu_project_create(const char* name, const char* system_name);
void vvu_project_free(VVU_Project* proj);

/* --- Core API: Requirement Management --- */
int vvu_req_add(VVU_Project* proj, const char* id, const char* desc,
                VVU_ReqPriority priority);
int vvu_req_find(VVU_Project* proj, const char* id);
void vvu_req_set_parent(VVU_Project* proj, int idx, const char* parent_id);
void vvu_req_set_score(VVU_Project* proj, int idx, double score);
void vvu_req_verify(VVU_Project* proj, int idx, bool passed);
void vvu_req_validate(VVU_Project* proj, int idx, bool validated);

/* --- Core API: Verification Test Management --- */
int vvu_verif_add(VVU_Project* proj, const char* id, const char* desc,
                  const char* req_id, double threshold, double tolerance);
void vvu_verif_run(VVU_Project* proj, int idx, double measured);
void vvu_verif_add_dependency(VVU_Project* proj, int idx, const char* dep_id);

/* --- Core API: Validation Assessment Management --- */
int vvu_valid_add(VVU_Project* proj, const char* stakeholder,
                  const char* concern, const char* method);
void vvu_valid_score(VVU_Project* proj, int idx, double alignment,
                     double confidence);

/* --- Core API: Uncertainty Management --- */
int vvu_uncert_add(VVU_Project* proj, const char* name,
                   VVU_UncertaintyType type, VVU_DistType dist,
                   double nominal, double param1, double param2);
void vvu_uncert_set_bounds(VVU_Project* proj, int idx,
                           double lo, double hi);
void vvu_uncert_set_correlation(VVU_Project* proj, int idx,
                                int correlated_with, double coeff);

/* --- Core API: Analysis --- */
void vvu_compute_coverage(VVU_Project* proj);
void vvu_compute_confidence(VVU_Project* proj);
double vvu_verif_pass_rate(const VVU_Project* proj);
double vvu_valid_alignment_mean(const VVU_Project* proj);
int vvu_req_count_by_status(const VVU_Project* proj, VVU_Status status);

/* --- Core API: Display --- */
void vvu_project_print(const VVU_Project* proj);
void vvu_requirement_print(const VVU_Project* proj, int idx);
void vvu_verification_print(const VVU_Project* proj, int idx);
void vvu_validation_print(const VVU_Project* proj, int idx);
void vvu_uncertainty_print(const VVU_Project* proj, int idx);
void vvu_project_summary(const VVU_Project* proj);
void vvu_traceability_matrix(const VVU_Project* proj);

/* --- Core Utility Functions --- */
double vvu_clamp(double x, double lo, double hi);
double vvu_gaussian_pdf(double x, double mu, double sigma);
double vvu_gaussian_cdf(double x, double mu, double sigma);
double vvu_uniform_random(double lo, double hi);
double vvu_box_muller(void);         /* Standard normal random */
double vvu_sample_distribution(VVU_DistType dist, double param1,
                                double param2);

#endif
