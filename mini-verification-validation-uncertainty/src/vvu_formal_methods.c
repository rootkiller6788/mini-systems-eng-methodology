#include "../include/vvu_core.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Formal Verification Methods — systematic V&V framework
 *
 * Traceability matrix analysis, requirements coverage metrics,
 * verification evidence management, and formal proof integration. */

typedef struct {
    int     n_requirements;
    int     n_tests;
    int*    trace_matrix;  /* [n_reqs * n_tests] 1=covered, 0=not */
    double  coverage;
    int     n_covered;
    int     n_uncovered;
} TraceabilityMatrix;

typedef struct {
    char*   test_id;
    char*   procedure;
    int     n_steps;
    double  pass_rate;
    int     n_iterations;
} VerificationCase;

typedef struct {
    VerificationCase* cases;
    int               n_cases;
    int               n_passed;
    int               n_failed;
    double            overall_pass_rate;
} VerificationSuite;

TraceabilityMatrix* tm_create(int n_reqs, int n_tests) {
    TraceabilityMatrix* tm = calloc(1, sizeof(TraceabilityMatrix));
    if (!tm) return NULL;
    tm->n_requirements = n_reqs;
    tm->n_tests = n_tests;
    tm->trace_matrix = calloc(n_reqs * n_tests, sizeof(int));
    return tm;
}

void tm_free(TraceabilityMatrix* tm) {
    if (!tm) return;
    free(tm->trace_matrix);
    free(tm);
}

int tm_link(TraceabilityMatrix* tm, int req_idx, int test_idx) {
    if (!tm || req_idx < 0 || req_idx >= tm->n_requirements ||
        test_idx < 0 || test_idx >= tm->n_tests) return -1;
    tm->trace_matrix[req_idx * tm->n_tests + test_idx] = 1;
    return 0;
}

double tm_compute_coverage(TraceabilityMatrix* tm) {
    if (!tm || tm->n_requirements == 0) return 0.0;
    tm->n_covered = 0;
    for (int i = 0; i < tm->n_requirements; i++) {
        int covered = 0;
        for (int j = 0; j < tm->n_tests; j++)
            if (tm->trace_matrix[i * tm->n_tests + j]) { covered = 1; break; }
        if (covered) tm->n_covered++;
    }
    tm->n_uncovered = tm->n_requirements - tm->n_covered;
    tm->coverage = (double)tm->n_covered / (double)tm->n_requirements;
    return tm->coverage;
}

VerificationSuite* vs_create(void) {
    VerificationSuite* vs = calloc(1, sizeof(VerificationSuite));
    return vs;
}

void vs_free(VerificationSuite* vs) {
    if (!vs) return;
    for (int i = 0; i < vs->n_cases; i++) {
        free(vs->cases[i].test_id);
        free(vs->cases[i].procedure);
    }
    free(vs->cases);
    free(vs);
}

int vs_add_case(VerificationSuite* vs, const char* id, const char* proc, int steps) {
    if (!vs || !id || !proc) return -1;
    vs->cases = realloc(vs->cases, (vs->n_cases + 1) * sizeof(VerificationCase));
    VerificationCase* vc = &vs->cases[vs->n_cases];
    vc->test_id = strdup(id);
    vc->procedure = strdup(proc);
    vc->n_steps = steps;
    vc->pass_rate = 0.0;
    vc->n_iterations = 0;
    vs->n_cases++;
    return vs->n_cases - 1;
}

int vs_record_result(VerificationSuite* vs, int case_idx, int passed) {
    if (!vs || case_idx < 0 || case_idx >= vs->n_cases) return -1;
    VerificationCase* vc = &vs->cases[case_idx];
    vc->n_iterations++;
    if (passed) vs->n_passed++; else vs->n_failed++;
    vc->pass_rate = (double)vs->n_passed / (double)vc->n_iterations;
    vs->overall_pass_rate = (vs->n_cases > 0)
        ? (double)vs->n_passed / (double)(vs->n_passed + vs->n_failed) : 0.0;
    return 0;
}
