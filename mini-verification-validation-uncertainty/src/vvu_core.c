#include "vvu_core.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define PI 3.14159265358979323846
#define INIT_CAP 32

static int vvu_req_count_priority(const VVU_Project* proj, VVU_ReqPriority priority) {
    if (!proj) return 0;
    int count = 0;
    for (int i = 0; i < proj->n_requirements; i++)
        if (proj->requirements[i].priority == priority) count++;
    return count;
}

/* ==============================================================
 * Project Lifecycle
 * ============================================================== */

VVU_Project* vvu_project_create(const char* name, const char* system_name) {
    VVU_Project* proj = (VVU_Project*)calloc(1, sizeof(VVU_Project));
    if (!proj) return NULL;
    proj->name = strdup(name ? name : "untitled");
    proj->system_under_test = strdup(system_name ? system_name : "unknown");
    proj->req_capacity = INIT_CAP;
    proj->requirements = (VVU_Requirement*)calloc(INIT_CAP, sizeof(VVU_Requirement));
    proj->verif_capacity = INIT_CAP;
    proj->verif_tests = (VVU_VerificationTest*)calloc(INIT_CAP, sizeof(VVU_VerificationTest));
    proj->valid_capacity = INIT_CAP;
    proj->valid_assessments = (VVU_ValidationAssessment*)calloc(INIT_CAP, sizeof(VVU_ValidationAssessment));
    proj->uncert_capacity = INIT_CAP;
    proj->uncertainties = (VVU_Uncertainty*)calloc(INIT_CAP, sizeof(VVU_Uncertainty));
    proj->overall_status = VVU_NOT_STARTED;
    return proj;
}

void vvu_project_free(VVU_Project* proj) {
    if (!proj) return;
    free(proj->name);
    free(proj->system_under_test);
    for (int i = 0; i < proj->n_requirements; i++) {
        free(proj->requirements[i].id);
        free(proj->requirements[i].description);
        free(proj->requirements[i].parent_id);
        if (proj->requirements[i].child_ids) {
            for (int j = 0; j < proj->requirements[i].n_children; j++)
                free(proj->requirements[i].child_ids[j]);
            free(proj->requirements[i].child_ids);
        }
        free(proj->requirements[i].test_method);
        free(proj->requirements[i].validated_by);
    }
    free(proj->requirements);
    for (int i = 0; i < proj->n_verif_tests; i++) {
        free(proj->verif_tests[i].id);
        free(proj->verif_tests[i].description);
        free(proj->verif_tests[i].req_id);
        if (proj->verif_tests[i].dependency_ids) {
            for (int j = 0; j < proj->verif_tests[i].n_dependencies; j++)
                free(proj->verif_tests[i].dependency_ids[j]);
            free(proj->verif_tests[i].dependency_ids);
        }
    }
    free(proj->verif_tests);
    for (int i = 0; i < proj->n_valid_assessments; i++) {
        free(proj->valid_assessments[i].id);
        free(proj->valid_assessments[i].stakeholder);
        free(proj->valid_assessments[i].concern);
        free(proj->valid_assessments[i].evidence);
        free(proj->valid_assessments[i].method);
        free(proj->valid_assessments[i].reviewer_scores);
    }
    free(proj->valid_assessments);
    for (int i = 0; i < proj->n_uncertainties; i++)
        free(proj->uncertainties[i].name);
    free(proj->uncertainties);
    free(proj);
}

/* ==============================================================
 * Requirement Management
 * ============================================================== */

static void req_cap_check(VVU_Project* proj) {
    if (proj->n_requirements >= proj->req_capacity) {
        proj->req_capacity *= 2;
        proj->requirements = (VVU_Requirement*)realloc(proj->requirements,
            proj->req_capacity * sizeof(VVU_Requirement));
        memset(proj->requirements + proj->req_capacity/2, 0,
               (proj->req_capacity/2) * sizeof(VVU_Requirement));
    }
}

int vvu_req_add(VVU_Project* proj, const char* id, const char* desc,
                VVU_ReqPriority priority) {
    if (!proj || !id) return -1;
    req_cap_check(proj);
    int idx = proj->n_requirements;
    VVU_Requirement* r = &proj->requirements[idx];
    memset(r, 0, sizeof(VVU_Requirement));
    r->id = strdup(id);
    r->description = strdup(desc ? desc : "");
    r->priority = priority;
    r->verification_status = VVU_NOT_STARTED;
    r->validation_status = VVU_NOT_STARTED;
    r->satisfaction_score = 0.0;
    proj->n_requirements++;
    return idx;
}

int vvu_req_find(VVU_Project* proj, const char* id) {
    if (!proj || !id) return -1;
    for (int i = 0; i < proj->n_requirements; i++)
        if (strcmp(proj->requirements[i].id, id) == 0) return i;
    return -1;
}

void vvu_req_set_parent(VVU_Project* proj, int idx, const char* parent_id) {
    if (!proj || idx < 0 || idx >= proj->n_requirements) return;
    free(proj->requirements[idx].parent_id);
    proj->requirements[idx].parent_id = strdup(parent_id);
}

void vvu_req_set_score(VVU_Project* proj, int idx, double score) {
    if (!proj || idx < 0 || idx >= proj->n_requirements) return;
    proj->requirements[idx].satisfaction_score = vvu_clamp(score, 0.0, 1.0);
}

void vvu_req_verify(VVU_Project* proj, int idx, bool passed) {
    if (!proj || idx < 0 || idx >= proj->n_requirements) return;
    proj->requirements[idx].verification_status =
        passed ? VVU_PASSED : VVU_FAILED;
    proj->requirements[idx].is_verified = passed;
}

void vvu_req_validate(VVU_Project* proj, int idx, bool validated) {
    if (!proj || idx < 0 || idx >= proj->n_requirements) return;
    proj->requirements[idx].validation_status =
        validated ? VVU_PASSED : VVU_FAILED;
    proj->requirements[idx].is_validated = validated;
}

/* ==============================================================
 * Verification Test Management
 * ============================================================== */

static void verif_cap_check(VVU_Project* proj) {
    if (proj->n_verif_tests >= proj->verif_capacity) {
        proj->verif_capacity *= 2;
        proj->verif_tests = (VVU_VerificationTest*)realloc(proj->verif_tests,
            proj->verif_capacity * sizeof(VVU_VerificationTest));
        memset(proj->verif_tests + proj->verif_capacity/2, 0,
               (proj->verif_capacity/2) * sizeof(VVU_VerificationTest));
    }
}

int vvu_verif_add(VVU_Project* proj, const char* id, const char* desc,
                  const char* req_id, double threshold, double tolerance) {
    if (!proj || !id) return -1;
    verif_cap_check(proj);
    int idx = proj->n_verif_tests;
    VVU_VerificationTest* t = &proj->verif_tests[idx];
    memset(t, 0, sizeof(VVU_VerificationTest));
    t->id = strdup(id);
    t->description = strdup(desc ? desc : "");
    t->req_id = strdup(req_id ? req_id : "");
    t->pass_threshold = threshold;
    t->tolerance = tolerance;
    t->status = VVU_NOT_STARTED;
    proj->n_verif_tests++;
    return idx;
}

void vvu_verif_run(VVU_Project* proj, int idx, double measured) {
    if (!proj || idx < 0 || idx >= proj->n_verif_tests) return;
    VVU_VerificationTest* t = &proj->verif_tests[idx];
    t->measured_value = measured;
    t->n_runs++;
    double diff = fabs(measured - t->pass_threshold);
    if (diff <= t->tolerance) {
        t->n_passes++;
        t->status = VVU_PASSED;
    } else {
        t->status = VVU_FAILED;
    }
}

void vvu_verif_add_dependency(VVU_Project* proj, int idx, const char* dep_id) {
    if (!proj || idx < 0 || idx >= proj->n_verif_tests || !dep_id) return;
    VVU_VerificationTest* t = &proj->verif_tests[idx];
    t->n_dependencies++;
    t->dependency_ids = (char**)realloc(t->dependency_ids,
        t->n_dependencies * sizeof(char*));
    t->dependency_ids[t->n_dependencies - 1] = strdup(dep_id);
}

/* ==============================================================
 * Validation Assessment Management
 * ============================================================== */

static void valid_cap_check(VVU_Project* proj) {
    if (proj->n_valid_assessments >= proj->valid_capacity) {
        proj->valid_capacity *= 2;
        proj->valid_assessments = (VVU_ValidationAssessment*)realloc(
            proj->valid_assessments,
            proj->valid_capacity * sizeof(VVU_ValidationAssessment));
        memset(proj->valid_assessments + proj->valid_capacity/2, 0,
               (proj->valid_capacity/2) * sizeof(VVU_ValidationAssessment));
    }
}

int vvu_valid_add(VVU_Project* proj, const char* stakeholder,
                  const char* concern, const char* method) {
    if (!proj || !stakeholder) return -1;
    valid_cap_check(proj);
    int idx = proj->n_valid_assessments;
    VVU_ValidationAssessment* a = &proj->valid_assessments[idx];
    memset(a, 0, sizeof(VVU_ValidationAssessment));
    a->id = (char*)malloc(32);
    snprintf(a->id, 32, "VAL-%03d", idx + 1);
    a->stakeholder = strdup(stakeholder);
    a->concern = strdup(concern ? concern : "");
    a->method = strdup(method ? method : "");
    a->status = VVU_NOT_STARTED;
    proj->n_valid_assessments++;
    return idx;
}

void vvu_valid_score(VVU_Project* proj, int idx, double alignment,
                     double confidence) {
    if (!proj || idx < 0 || idx >= proj->n_valid_assessments) return;
    VVU_ValidationAssessment* a = &proj->valid_assessments[idx];
    a->alignment_score = vvu_clamp(alignment, 0.0, 1.0);
    a->confidence = vvu_clamp(confidence, 0.0, 1.0);
    a->status = (a->alignment_score >= 0.7) ? VVU_PASSED : VVU_FAILED;
}

/* ==============================================================
 * Uncertainty Management
 * ============================================================== */

static void uncert_cap_check(VVU_Project* proj) {
    if (proj->n_uncertainties >= proj->uncert_capacity) {
        proj->uncert_capacity *= 2;
        proj->uncertainties = (VVU_Uncertainty*)realloc(proj->uncertainties,
            proj->uncert_capacity * sizeof(VVU_Uncertainty));
        memset(proj->uncertainties + proj->uncert_capacity/2, 0,
               (proj->uncert_capacity/2) * sizeof(VVU_Uncertainty));
    }
}

int vvu_uncert_add(VVU_Project* proj, const char* name,
                   VVU_UncertaintyType type, VVU_DistType dist,
                   double nominal, double param1, double param2) {
    if (!proj || !name) return -1;
    uncert_cap_check(proj);
    int idx = proj->n_uncertainties;
    VVU_Uncertainty* u = &proj->uncertainties[idx];
    memset(u, 0, sizeof(VVU_Uncertainty));
    u->name = strdup(name);
    u->type = type;
    u->distribution = dist;
    u->nominal = nominal;
    u->param1 = param1;
    u->param2 = param2;
    /* Derive mean and std_dev from distribution */
    switch (dist) {
        case VVU_DIST_UNIFORM:
            u->lower_bound = param1; u->upper_bound = param2;
            u->mean = (param1 + param2) / 2.0;
            u->std_dev = (param2 - param1) / sqrt(12.0);
            break;
        case VVU_DIST_NORMAL:
            u->mean = param1; u->std_dev = param2;
            u->lower_bound = param1 - 3.0 * param2;
            u->upper_bound = param1 + 3.0 * param2;
            break;
        case VVU_DIST_TRIANGULAR:
            u->lower_bound = nominal - param1;
            u->upper_bound = nominal + param2;
            u->mean = (u->lower_bound + u->upper_bound + nominal) / 3.0;
            break;
        default:
            u->lower_bound = nominal - fabs(param1);
            u->upper_bound = nominal + fabs(param2);
            u->mean = nominal;
            u->std_dev = fabs(param1);
            break;
    }
    u->variance = u->std_dev * u->std_dev;
    proj->n_uncertainties++;
    return idx;
}

void vvu_uncert_set_bounds(VVU_Project* proj, int idx, double lo, double hi) {
    if (!proj || idx < 0 || idx >= proj->n_uncertainties) return;
    proj->uncertainties[idx].lower_bound = lo;
    proj->uncertainties[idx].upper_bound = hi;
}

void vvu_uncert_set_correlation(VVU_Project* proj, int idx,
                                int correlated_with, double coeff) {
    if (!proj || idx < 0 || idx >= proj->n_uncertainties) return;
    proj->uncertainties[idx].is_correlated = true;
    proj->uncertainties[idx].correlated_with = correlated_with;
    proj->uncertainties[idx].correlation_coeff = vvu_clamp(coeff, -1.0, 1.0);
}

/* ==============================================================
 * Analysis
 * ============================================================== */

void vvu_compute_coverage(VVU_Project* proj) {
    if (!proj) return;
    int n = proj->n_requirements;
    if (n == 0) { proj->overall_verif_coverage = 0; proj->overall_valid_coverage = 0; return; }
    int vcount = 0, vacount = 0;
    for (int i = 0; i < n; i++) {
        if (proj->requirements[i].is_verified) vcount++;
        if (proj->requirements[i].is_validated) vacount++;
    }
    proj->overall_verif_coverage = (double)vcount / n;
    proj->overall_valid_coverage = (double)vacount / n;
}

void vvu_compute_confidence(VVU_Project* proj) {
    if (!proj) return;
    double vconf = proj->overall_verif_coverage;
    double dconf = proj->overall_valid_coverage;
    double uconf = 1.0;
    /* Uncertainty reduces confidence */
    for (int i = 0; i < proj->n_uncertainties; i++)
        uconf *= (1.0 - proj->uncertainties[i].std_dev / fmax(fabs(proj->uncertainties[i].nominal), 1e-6));
    uconf = vvu_clamp(uconf, 0.0, 1.0);
    /* Composite: weighted average */
    proj->overall_confidence = 0.4 * vconf + 0.3 * dconf + 0.3 * uconf;
}

double vvu_verif_pass_rate(const VVU_Project* proj) {
    if (!proj || proj->n_verif_tests == 0) return 0.0;
    int passed = 0;
    for (int i = 0; i < proj->n_verif_tests; i++)
        if (proj->verif_tests[i].status == VVU_PASSED) passed++;
    return (double)passed / proj->n_verif_tests;
}

double vvu_valid_alignment_mean(const VVU_Project* proj) {
    if (!proj || proj->n_valid_assessments == 0) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < proj->n_valid_assessments; i++)
        sum += proj->valid_assessments[i].alignment_score;
    return sum / proj->n_valid_assessments;
}

int vvu_req_count_by_status(const VVU_Project* proj, VVU_Status status) {
    if (!proj) return 0;
    int count = 0;
    for (int i = 0; i < proj->n_requirements; i++)
        if (proj->requirements[i].verification_status == status) count++;
    return count;
}

/* ==============================================================
 * Display Functions
 * ============================================================== */

void vvu_project_print(const VVU_Project* proj) {
    if (!proj) { printf("Project: NULL\n"); return; }
    printf("=== V&V Project: %s ===\n", proj->name);
    printf("System: %s\n", proj->system_under_test);
    printf("Requirements: %d | Verification Tests: %d | Validations: %d | Uncertainties: %d\n",
           proj->n_requirements, proj->n_verif_tests,
           proj->n_valid_assessments, proj->n_uncertainties);
    printf("Verif Coverage: %.1f%% | Valid Coverage: %.1f%% | Confidence: %.1f%%\n",
           proj->overall_verif_coverage * 100,
           proj->overall_valid_coverage * 100,
           proj->overall_confidence * 100);
    vvu_project_summary(proj);
}

void vvu_requirement_print(const VVU_Project* proj, int idx) {
    if (!proj || idx < 0 || idx >= proj->n_requirements) return;
    const VVU_Requirement* r = &proj->requirements[idx];
    printf("Req[%d] %s (P%d): %s\n", idx, r->id, r->priority, r->description);
    printf("  Verified: %s | Validated: %s | Score: %.2f\n",
           r->is_verified ? "Y" : "N", r->is_validated ? "Y" : "N",
           r->satisfaction_score);
}

void vvu_verification_print(const VVU_Project* proj, int idx) {
    if (!proj || idx < 0 || idx >= proj->n_verif_tests) return;
    const VVU_VerificationTest* t = &proj->verif_tests[idx];
    printf("Test[%d] %s: %s\n", idx, t->id, t->description);
    printf("  Req: %s | Status: %d | Threshold: %.4f +/- %.4f\n",
           t->req_id, t->status, t->pass_threshold, t->tolerance);
    printf("  Measured: %.4f | Runs: %d | Passes: %d\n",
           t->measured_value, t->n_runs, t->n_passes);
}

void vvu_validation_print(const VVU_Project* proj, int idx) {
    if (!proj || idx < 0 || idx >= proj->n_valid_assessments) return;
    const VVU_ValidationAssessment* a = &proj->valid_assessments[idx];
    printf("Valid[%d] %s: Stakeholder=%s\n", idx, a->id, a->stakeholder);
    printf("  Concern: %s | Alignment: %.2f | Confidence: %.2f\n",
           a->concern, a->alignment_score, a->confidence);
}

void vvu_uncertainty_print(const VVU_Project* proj, int idx) {
    if (!proj || idx < 0 || idx >= proj->n_uncertainties) return;
    const VVU_Uncertainty* u = &proj->uncertainties[idx];
    printf("Uncert[%d] %s: type=%d dist=%d nominal=%.4f\n",
           idx, u->name, u->type, u->distribution, u->nominal);
    printf("  Range: [%.4f, %.4f] mean=%.4f sigma=%.4f\n",
           u->lower_bound, u->upper_bound, u->mean, u->std_dev);
}

void vvu_project_summary(const VVU_Project* proj) {
    if (!proj) return;
    int mandatory = 0, mandatory_verified = 0;
    for (int i = 0; i < proj->n_requirements; i++) {
        if (proj->requirements[i].priority == VVU_REQ_MANDATORY) {
            mandatory++;
            if (proj->requirements[i].is_verified) mandatory_verified++;
        }
    }
    printf("=== Summary ===\n");
    printf("Status: %d | Pass Rate: %.1f%% | Alignment: %.2f\n",
           proj->overall_status, vvu_verif_pass_rate(proj) * 100,
           vvu_valid_alignment_mean(proj));
    printf("Mandatory Verified: %d/%d\n", mandatory_verified, mandatory);
}

void vvu_traceability_matrix(const VVU_Project* proj) {
    if (!proj || proj->n_requirements == 0) return;
    printf("=== Traceability Matrix ===\n");
    printf("%-12s %-12s %-12s %s\n", "Req ID", "Parent", "Priority", "Verified");
    for (int i = 0; i < proj->n_requirements; i++) {
        VVU_Requirement* r = &proj->requirements[i];
        printf("%-12s %-12s P%-11d %s\n",
               r->id, r->parent_id ? r->parent_id : "NONE",
               r->priority, r->is_verified ? "YES" : "NO");
    }
}

/* ==============================================================
 * Utility Functions
 * ============================================================== */

double vvu_clamp(double x, double lo, double hi) {
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

double vvu_gaussian_pdf(double x, double mu, double sigma) {
    if (sigma <= 0) return (x == mu) ? INFINITY : 0.0;
    double z = (x - mu) / sigma;
    return exp(-0.5 * z * z) / (sigma * sqrt(2.0 * PI));
}

double vvu_gaussian_cdf(double x, double mu, double sigma) {
    if (sigma <= 0) return (x >= mu) ? 1.0 : 0.0;
    double z = (x - mu) / (sigma * sqrt(2.0));
    return 0.5 * (1.0 + erf(z));
}

double vvu_uniform_random(double lo, double hi) {
    return lo + (double)rand() / RAND_MAX * (hi - lo);
}

double vvu_box_muller(void) {
    double u1 = (double)rand() / RAND_MAX;
    double u2 = (double)rand() / RAND_MAX;
    return sqrt(-2.0 * log(u1 + 1e-15)) * cos(2.0 * PI * u2);
}

double vvu_sample_distribution(VVU_DistType dist, double p1, double p2) {
    switch (dist) {
        case VVU_DIST_UNIFORM:
            return vvu_uniform_random(p1, p2);
        case VVU_DIST_NORMAL:
            return p1 + vvu_box_muller() * p2;
        case VVU_DIST_TRIANGULAR: {
            double r = (double)rand() / RAND_MAX;
            double mode = (p1 + p2) / 2.0;
            if (r < (mode - p1) / (p2 - p1))
                return p1 + sqrt(r * (p2 - p1) * (mode - p1));
            else
                return p2 - sqrt((1 - r) * (p2 - p1) * (p2 - mode));
        }
        case VVU_DIST_EXPONENTIAL:
            return -log((double)rand() / RAND_MAX + 1e-15) / p1;
        default:
            return p1;
    }
}

VVU_Status vvu_project_overall_status(const VVU_Project* proj) {
    if (!proj) return VVU_NOT_STARTED;
    if (proj->n_requirements == 0) return VVU_NOT_STARTED;
    int verified = 0, validated = 0;
    for (int i = 0; i < proj->n_requirements; i++) {
        if (proj->requirements[i].is_verified) verified++;
        if (proj->requirements[i].is_validated) validated++;
    }
    if (verified == proj->n_requirements && validated == proj->n_requirements)
        return VVU_PASSED;
    if (verified == 0 && validated == 0) return VVU_NOT_STARTED;
    return VVU_IN_PROGRESS;
}
double vvu_req_satisfaction_mean(const VVU_Project* proj) {
    if (!proj || proj->n_requirements == 0) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < proj->n_requirements; i++)
        sum += proj->requirements[i].satisfaction_score;
    return sum / proj->n_requirements;
}
bool vvu_req_has_test(const VVU_Project* proj, int req_idx) {
    if (!proj || req_idx < 0 || req_idx >= proj->n_requirements) return false;
    const char* rid = proj->requirements[req_idx].id;
    for (int i = 0; i < proj->n_verif_tests; i++)
        if (strcmp(proj->verif_tests[i].req_id, rid) == 0) return true;
    return false;
}
double vvu_uncert_coefficient_of_variation(const VVU_Project* proj, int idx) {
    if (!proj || idx < 0 || idx >= proj->n_uncertainties) return 0.0;
    double nom = fabs(proj->uncertainties[idx].nominal);
    return (nom > 1e-10) ? proj->uncertainties[idx].std_dev / nom : 0.0;
}


int vvu_req_count_children(const VVU_Project* proj, int idx) {
    if (!proj || idx < 0 || idx >= proj->n_requirements) return 0;
    return proj->requirements[idx].n_children;
}
double vvu_req_critical_mass(const VVU_Project* proj) {
    if (!proj || proj->n_requirements == 0) return 0.0;
    int critical = 0;
    for (int i = 0; i < proj->n_requirements; i++)
        if (proj->requirements[i].priority <= VVU_REQ_CRITICAL) critical++;
    return (double)critical / proj->n_requirements;
}
bool vvu_is_fully_verified(const VVU_Project* proj) {
    if (!proj || proj->n_requirements == 0) return true;
    for (int i = 0; i < proj->n_requirements; i++)
        if (!proj->requirements[i].is_verified) return false;
    return true;
}
bool vvu_is_fully_validated(const VVU_Project* proj) {
    if (!proj || proj->n_requirements == 0) return true;
    for (int i = 0; i < proj->n_requirements; i++)
        if (!proj->requirements[i].is_validated) return false;
    return true;
}
double vvu_test_redundancy_ratio(const VVU_Project* proj) {
    if (!proj || proj->n_verif_tests == 0 || proj->n_requirements == 0) return 0.0;
    return (double)proj->n_verif_tests / proj->n_requirements;
}

int vvu_req_dependency_depth(const VVU_Project* proj, int idx) {
    if (!proj || idx < 0 || idx >= proj->n_requirements) return 0;
    if (!proj->requirements[idx].parent_id) return 0;
    int parent = vvu_req_find(proj, proj->requirements[idx].parent_id);
    return (parent >= 0) ? 1 + vvu_req_dependency_depth(proj, parent) : 0;
}
int vvu_req_leaf_count(const VVU_Project* proj) {
    if (!proj) return 0; int count = 0;
    for (int i = 0; i < proj->n_requirements; i++)
        if (proj->requirements[i].n_children == 0) count++;
    return count;
}
int vvu_req_depth_max(const VVU_Project* proj) {
    if (!proj) return 0; int max_d = 0;
    for (int i = 0; i < proj->n_requirements; i++) {
        int d = vvu_req_dependency_depth(proj, i);
        if (d > max_d) max_d = d;
    }
    return max_d;
}

double vvu_uncert_pooled_variance(const VVU_Project* proj) {
    if (!proj || proj->n_uncertainties == 0) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < proj->n_uncertainties; i++)
        sum += proj->uncertainties[i].variance;
    return sum;
}
int vvu_req_count_verified_priority(const VVU_Project* proj, VVU_ReqPriority p) {
    if (!proj) return 0; int c = 0;
    for (int i = 0; i < proj->n_requirements; i++)
        if (proj->requirements[i].priority == p && proj->requirements[i].is_verified) c++;
    return c;
}
double vvu_verif_pass_rate_priority(const VVU_Project* proj, VVU_ReqPriority p) {
    int total = vvu_req_count_priority(proj, p);
    int passed = vvu_req_count_verified_priority(proj, p);
    return total > 0 ? (double)passed / total : 0.0;
}


int vvu_req_find_by_status(const VVU_Project* proj, VVU_Status status, int* indices, int max_n) {
    if(!proj||!indices||max_n<=0)return 0;int count=0;
    for(int i=0;i<proj->n_requirements&&count<max_n;i++)
        if(proj->requirements[i].verification_status==status)indices[count++]=i;
    return count;
}
double vvu_uncert_total_variance(const VVU_Project* proj) {
    if(!proj||proj->n_uncertainties==0)return 0;double s=0;
    for(int i=0;i<proj->n_uncertainties;i++)s+=proj->uncertainties[i].variance;
    return s;
}
int vvu_req_count_passed(const VVU_Project* proj) {
    if(!proj)return 0;int c=0;
    for(int i=0;i<proj->n_requirements;i++)if(proj->requirements[i].is_verified)c++;
    return c;
}
int vvu_req_count_failed(const VVU_Project* proj) {
    if(!proj)return 0;int c=0;
    for(int i=0;i<proj->n_requirements;i++)if(!proj->requirements[i].is_verified&&proj->requirements[i].verification_status!=VVU_NOT_STARTED)c++;
    return c;
}
bool vvu_uncert_has_correlation(const VVU_Project* proj) {
    if(!proj)return false;
    for(int i=0;i<proj->n_uncertainties;i++)if(proj->uncertainties[i].is_correlated)return true;
    return false;
}
int vvu_uncert_count_by_type(const VVU_Project* proj, VVU_UncertaintyType t) {
    if(!proj)return 0;int c=0;
    for(int i=0;i<proj->n_uncertainties;i++)if(proj->uncertainties[i].type==t)c++;
    return c;
}

/* Convert status enum to string for reporting */
const char* vvu_status_to_string(VVU_Status status) {
    switch (status) {
        case VVU_NOT_STARTED: return "NOT_STARTED";
        case VVU_IN_PROGRESS: return "IN_PROGRESS";
        case VVU_PASSED: return "PASSED";
        case VVU_FAILED: return "FAILED";
        case VVU_CONDITIONAL: return "CONDITIONAL";
        case VVU_NOT_APPLICABLE: return "N/A";
        default: return "UNKNOWN";
    }
}

/* Get requirement priority as string */
const char* vvu_priority_to_string(VVU_ReqPriority p) {
    switch (p) {
        case VVU_REQ_MANDATORY: return "MANDATORY";
        case VVU_REQ_CRITICAL: return "CRITICAL";
        case VVU_REQ_IMPORTANT: return "IMPORTANT";
        case VVU_REQ_DESIRABLE: return "DESIRABLE";
        case VVU_REQ_OPTIONAL: return "OPTIONAL";
        default: return "UNKNOWN";
    }
}

/* V&V readiness: compute fraction of requirements with defined verification.
 * "No requirement is complete without a verification method." — INCOSE */
double vvu_readiness_index(int n_reqs, const int* has_verification_method) {
    if (!has_verification_method || n_reqs <= 0) return 0.0;
    int count = 0;
    for (int i = 0; i < n_reqs; i++) if (has_verification_method[i]) count++;
    return (double)count / (double)n_reqs;
}

/* Validation confidence: stakeholder agreement weighted by influence.
 * Higher confidence → lower risk of requirement misunderstanding. */
double vvu_validation_confidence(int n_stakeholders, const double* agreement,
                                  const double* influence) {
    if (!agreement || !influence || n_stakeholders <= 0) return 0.0;
    double weighted_sum = 0.0, total_influence = 0.0;
    for (int i = 0; i < n_stakeholders; i++) {
        weighted_sum += agreement[i] * influence[i];
        total_influence += influence[i];
    }
    return (total_influence > 1e-10) ? weighted_sum / total_influence : 0.0;
}

































































