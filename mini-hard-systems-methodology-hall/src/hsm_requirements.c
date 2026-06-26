#include "hsm_requirements.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

const char* hsm_req_category_name(ReqCategory cat) {
    switch (cat) {
        case REQ_FUNCTIONAL: return "Functional";
        case REQ_PERFORMANCE: return "Performance"; case REQ_INTERFACE: return "Interface";
        case REQ_CONSTRAINT: return "Constraint"; case REQ_SAFETY: return "Safety";
        case REQ_SECURITY: return "Security"; case REQ_RELIABILITY: return "Reliability";
        case REQ_MAINTAINABILITY: return "Maintainability"; case REQ_ENVIRONMENTAL: return "Environmental";
        default: return "Unknown";
    }
}
ReqDatabase hsm_req_summarize(HSMSystem* sys) {
    ReqDatabase db; memset(&db, 0, sizeof(db));
    if (!sys) return db;
    db.all_requirements = NULL; db.n_total = sys->n_requirements;
    for (int i = 0; i < sys->n_requirements; i++) {
        if (sys->requirements[i].is_verified) db.n_verified++;
        if (sys->requirements[i].is_validated) db.n_validated++;
    }
    db.verification_progress = db.n_total > 0 ? (double)db.n_verified / db.n_total : 0.0;
    db.validation_progress = db.n_total > 0 ? (double)db.n_validated / db.n_total : 0.0;
    return db;
}
void hsm_req_print_summary(HSMSystem* sys) {
    ReqDatabase db = hsm_req_summarize(sys);
    printf("=== Requirements Summary ===\n");
    printf("Total: %d, Verified: %d (%.0f%%), Validated: %d (%.0f%%)\n",
           db.n_total, db.n_verified, db.verification_progress * 100, db.n_validated, db.validation_progress * 100);
}
double hsm_req_traceability_index(HSMSystem* sys) {
    if (!sys || sys->n_requirements == 0) return 0.0;
    int traced = 0;
    for (int i = 0; i < sys->n_requirements; i++)
        if (sys->requirements[i].verification_method && strlen(sys->requirements[i].verification_method) > 0) traced++;
    return (double)traced / sys->n_requirements;
}
int hsm_req_count_by_category(HSMSystem* sys, ReqCategory cat) {
    if (!sys) return 0; const char* cn = hsm_req_category_name(cat); int n = 0;
    for (int i = 0; i < sys->n_requirements; i++) if (sys->requirements[i].category && strcmp(sys->requirements[i].category, cn) == 0) n++;
    return n;
}
int hsm_req_count_verified(HSMSystem* sys) { if(!sys)return 0;int n=0;for(int i=0;i<sys->n_requirements;i++)if(sys->requirements[i].is_verified)n++;return n; }
int hsm_req_count_validated(HSMSystem* sys) { if(!sys)return 0;int n=0;for(int i=0;i<sys->n_requirements;i++)if(sys->requirements[i].is_validated)n++;return n; }
double hsm_req_verification_coverage(HSMSystem* sys) { return sys&&sys->n_requirements>0?(double)hsm_req_count_verified(sys)/sys->n_requirements:0.0; }
double hsm_req_validation_coverage(HSMSystem* sys) { return sys&&sys->n_requirements>0?(double)hsm_req_count_validated(sys)/sys->n_requirements:0.0; }
bool hsm_req_is_well_formed(HSMRequirement* req) { return req && req->id && req->description && strlen(req->id) > 0 && strlen(req->description) > 10; }
int hsm_req_well_formed_count(HSMSystem* sys) { if(!sys)return 0;int n=0;for(int i=0;i<sys->n_requirements;i++)if(hsm_req_is_well_formed(&sys->requirements[i]))n++;return n; }
double hsm_req_ambiguity_score(HSMSystem* sys) { if(!sys||sys->n_requirements==0)return 0.0;return 1.0-(double)hsm_req_well_formed_count(sys)/sys->n_requirements; }
void hsm_req_find_orphans(HSMSystem* sys, char*** ids, int* n) { if(!sys||!ids||!n)return;*n=0;*ids=NULL;for(int i=0;i<sys->n_requirements;i++)if(!sys->requirements[i].parent_id||strlen(sys->requirements[i].parent_id)==0)(*n)++;if(*n>0){*ids=malloc(*n*sizeof(char*));int idx=0;for(int i=0;i<sys->n_requirements;i++)if(!sys->requirements[i].parent_id||strlen(sys->requirements[i].parent_id)==0)(*ids)[idx++]=sys->requirements[i].id;} }

/* ── Matrix decomposition helpers ── */
static void _householder_vector(const double* x, int n, double* v, double* beta) {
    double sigma = 0.0;
    for (int i = 1; i < n; i++) sigma += x[i] * x[i];
    v[0] = 1.0;
    for (int i = 1; i < n; i++) v[i] = x[i];
    if (sigma < 1e-15) { *beta = 0.0; return; }
    double mu = sqrt(x[0] * x[0] + sigma);
    if (x[0] <= 0) v[0] = x[0] - mu;
    else v[0] = -sigma / (x[0] + mu);
    *beta = 2.0 * v[0] * v[0] / (sigma + v[0] * v[0]);
    double inv_v0 = 1.0 / v[0];
    for (int i = 1; i < n; i++) v[i] *= inv_v0;
}
static void _apply_householder_left(double* A, int m, int n, const double* v, double beta) {
    if (beta < 1e-15) return;
    for (int j = 0; j < n; j++) {
        double s = 0.0;
        for (int i = 0; i < m; i++) s += v[i] * A[i * n + j];
        s *= beta;
        for (int i = 0; i < m; i++) A[i * n + j] -= s * v[i];
    }
}
static void _qr_decompose(double* A, int m, int n, double* Q, double* R) {
    int k = (m < n) ? m : n;
    for (int i = 0; i < m * n; i++) { Q[i] = A[i]; R[i] = 0.0; }
    double* v = (double*)calloc((size_t)m, sizeof(double));
    for (int j = 0; j < k; j++) {
        double beta;
        for (int i = j; i < m; i++) v[i - j] = Q[i * n + j];
        _householder_vector(v, m - j, v, &beta);
        for (int i = j; i < m; i++) R[i * n + j] = (i == j) ? Q[j * n + j] : v[i - j];
        _apply_householder_left(Q + j * n + j, m - j, n - j, v, beta);
    }
    free(v);
}
/* ── Singular value estimates ── */
double _estimate_max_singular(const double* A, int m, int n, int iters) {
    if (m <= 0 || n <= 0 || !A || iters <= 0) return 0.0;
    double* v = (double*)calloc((size_t)n, sizeof(double));
    double* u = (double*)calloc((size_t)m, sizeof(double));
    for (int i = 0; i < n; i++) v[i] = 1.0 / sqrt((double)n);
    for (int iter = 0; iter < iters; iter++) {
        for (int i = 0; i < m; i++) {
            u[i] = 0.0;
            for (int j = 0; j < n; j++) u[i] += A[i * n + j] * v[j];
        }
        double norm_u = 0.0;
        for (int i = 0; i < m; i++) norm_u += u[i] * u[i];
        norm_u = sqrt(norm_u);
        if (norm_u < 1e-15) { free(v); free(u); return 0.0; }
        for (int i = 0; i < m; i++) u[i] /= norm_u;
        for (int j = 0; j < n; j++) {
            v[j] = 0.0;
            for (int i = 0; i < m; i++) v[j] += A[i * n + j] * u[i];
        }
        double norm_v = 0.0;
        for (int j = 0; j < n; j++) norm_v += v[j] * v[j];
        norm_v = sqrt(norm_v);
        if (norm_v < 1e-15) { free(v); free(u); return 0.0; }
        for (int j = 0; j < n; j++) v[j] /= norm_v;
    }
    free(v); free(u);
    return 1.0;
}
