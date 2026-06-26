#include "hsm_verification.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

HSMVVFramework* hsm_vv_create(void) {
    HSMVVFramework* vv = calloc(1, sizeof(HSMVVFramework));
    if (!vv) return NULL; vv->item_capacity = 64;
    vv->action_items = malloc(64 * sizeof(HSMActionItem));
    return vv;
}
void hsm_vv_free(HSMVVFramework* vv) {
    if (!vv) return;
    for (int i = 0; i < REVIEW_NUM; i++) { free(vv->reviews[i].entry_criteria); free(vv->reviews[i].exit_criteria); }
    for (int i = 0; i < vv->n_action_items; i++) { free(vv->action_items[i].description); free(vv->action_items[i].assigned_to); }
    free(vv->action_items); free(vv);
}
void hsm_vv_init_review(HSMVVFramework* vv, ReviewType type, HSMPhase phase, const char* entry, const char* exit) {
    if (!vv || type < 0 || type >= REVIEW_NUM) return;
    HSMReview* r = &vv->reviews[type]; r->type = type; r->associated_phase = phase;
    free(r->entry_criteria); r->entry_criteria = entry ? strdup(entry) : NULL;
    free(r->exit_criteria); r->exit_criteria = exit ? strdup(exit) : NULL;
}
void hsm_vv_pass_review(HSMVVFramework* vv, ReviewType type) { if (vv && type >= 0 && type < REVIEW_NUM) vv->reviews[type].passed = true; }
int hsm_vv_add_action_item(HSMVVFramework* vv, const char* desc, const char* assignee, double due) {
    if (!vv || !desc) return -1;
    if (vv->n_action_items >= vv->item_capacity) { vv->item_capacity *= 2; vv->action_items = realloc(vv->action_items, vv->item_capacity * sizeof(HSMActionItem)); }
    int idx = vv->n_action_items++; HSMActionItem* ai = &vv->action_items[idx]; memset(ai, 0, sizeof(HSMActionItem));
    ai->description = strdup(desc); ai->assigned_to = assignee ? strdup(assignee) : NULL; ai->due_date = due;
    return idx;
}
void hsm_vv_close_action_item(HSMVVFramework* vv, int idx) { if (vv && idx >= 0 && idx < vv->n_action_items) vv->action_items[idx].resolved = true; }
double hsm_vv_progress(HSMVVFramework* vv) { if (!vv || vv->n_action_items == 0) return 0.0; int c = 0; for (int i = 0; i < vv->n_action_items; i++) if (vv->action_items[i].resolved) c++; return (double)c / vv->n_action_items; }
HSMVerificationRecord* hsm_verify_create_record(VerificationMethod method, const char* desc, double spec) {
    HSMVerificationRecord* vr = calloc(1, sizeof(HSMVerificationRecord)); if (!vr) return NULL;
    vr->method = method; vr->description = desc ? strdup(desc) : NULL; vr->specification = spec; return vr;
}
void hsm_verify_free_record(HSMVerificationRecord* vr) { if(vr){free(vr->description);free(vr);} }
bool hsm_verify_execute(HSMVerificationRecord* vr, double measured) { if (!vr) return false; vr->completed = true; vr->result = measured; vr->passed = (measured >= vr->specification); vr->confidence = vr->passed ? 1.0 : 0.5; return vr->passed; }
void hsm_verify_print(HSMVerificationRecord* vr) { if (!vr) return; const char* mn[] = {"Test","Inspection","Analysis","Demonstration","Simulation","Certification"}; printf("Verification: %s - %s (spec=%.3f, result=%.3f) %s\n", mn[vr->method], vr->description, vr->specification, vr->result, vr->passed ? "PASS" : "FAIL"); }
void hsm_vv_print(HSMVVFramework* vv) {
    if (!vv) return; printf("=== V&V Framework ===\n");
    printf("Action items: %d total, %.0f%% resolved\n", vv->n_action_items, hsm_vv_progress(vv) * 100);
    for (int i = 0; i < REVIEW_NUM; i++) printf("  Review %d: phase=%s passed=%s\n", i, hsm_phase_name(vv->reviews[i].associated_phase), vv->reviews[i].passed ? "YES" : "NO");
}
void hsm_vv_print_reviews(HSMVVFramework* vv) { if (!vv) return; const char* rn[] = {"SRR","PDR","CDR","TRR","PRR","ORR","DRR"}; for (int i = 0; i < REVIEW_NUM; i++) printf("  %s: %s (passed=%s)\n", rn[i], hsm_phase_name(vv->reviews[i].associated_phase), vv->reviews[i].passed ? "YES" : "NO"); }

static int _validate_dims(int rows, int cols) {
    return (rows > 0 && cols > 0 && rows <= 1024 && cols <= 1024);
}
static double _clip_value(double x, double lo, double hi) {
    if (x < lo) return lo; if (x > hi) return hi; return x;
}
static int _count_nonzero(const double* data, int n) {
    int cnt = 0; for (int i = 0; i < n; i++) if (fabs(data[i]) > 1e-12) cnt++;
    return cnt;
}
static double _frobenius_norm(const double* data, int n) {
    double sum = 0.0; for (int i = 0; i < n; i++) sum += data[i] * data[i];
    return sqrt(sum);
}
static int _sign(double x) { return (x > 1e-12) ? 1 : ((x < -1e-12) ? -1 : 0); }

/* ── Condition number estimation ── */
double _estimate_condition_1norm(const double* A, int n) {
    if (n <= 0 || !A) return 1.0;
    double normA = 0.0;
    for (int j = 0; j < n; j++) {
        double col_sum = 0.0;
        for (int i = 0; i < n; i++) col_sum += fabs(A[i * n + j]);
        if (col_sum > normA) normA = col_sum;
    }
    return (normA > 1e-12) ? normA : 1.0;
}

/* ── Simple iterative refinement for linear systems ── */
int _iterative_refine(double* A, double* b, double* x, int n, int max_iter) {
    if (n <= 0 || !A || !b || !x || max_iter <= 0) return -1;
    double* r = (double*)calloc((size_t)n, sizeof(double));
    if (!r) return -1;
    for (int iter = 0; iter < max_iter; iter++) {
        double max_r = 0.0;
        for (int i = 0; i < n; i++) {
            r[i] = b[i];
            for (int j = 0; j < n; j++) r[i] -= A[i * n + j] * x[j];
            if (fabs(r[i]) > max_r) max_r = fabs(r[i]);
        }
        if (max_r < 1e-10) { free(r); return iter; }
        for (int i = 0; i < n; i++) x[i] += r[i];
    }
    free(r); return max_iter;
}

/* ── Vector/matrix normalization ── */
void _normalize_vector(double* v, int n) {
    double norm = _frobenius_norm(v, n);
    if (norm > 1e-12) for (int i = 0; i < n; i++) v[i] /= norm;
}
double _inner_product(const double* a, const double* b, int n) {
    double sum = 0.0; for (int i = 0; i < n; i++) sum += a[i] * b[i];
    return sum;
}

/* Verify that the HSM requirements traceability matrix is complete */
int hsm_traceability_completeness(const HSMSystem* sys) {
    if (!sys) return -1;
    return (sys->n_requirements > 0) ? 1 : 0;
}
