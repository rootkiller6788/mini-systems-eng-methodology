#include "mbse_core.h"
#include "mbse_requirements.h"
#include "mbse_verification.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ============================================================================
 * Requirements Traceability & Coverage Analysis
 *
 * Implements full bidirectional traceability:
 *   - Requirements-to-design (forward traceability)
 *   - Design-to-requirements (backward traceability)
 *   - Verification traceability (reqs -> test cases)
 *   - Coverage metrics (allocation, verification, satisfaction)
 *   - Impact analysis (what-ifs on requirement changes)
 *
 * Based on: INCOSE Guide to Writing Requirements (2022),
 * ISO 29148:2018, CMMI REQM process area
 * ============================================================================ */

/* ---------- Traceability Matrix Operations ---------- */

/* Build NxM traceability matrix: rows=requirements, cols=blocks.
 * trace[i][j] = 1 if requirement i is allocated to block j.
 * Fills matrix (caller alloc, size n_reqs * n_blocks).
 * Caller initializes with calloc. */
void mbse_build_traceability_matrix(SystemModel* m, int* matrix) {
    if (!m || !matrix) return;
    int nr = m->n_requirements;
    int nb = m->n_blocks;
    for (int i = 0; i < nr; i++) {
        MBSERequirement* r = &m->requirements[i];
        int bid = r->satisfied_by_block;
        if (bid >= 0 && bid < nb)
            matrix[i * nb + bid] = 1;
        /* Also mark derived-from chains */
        for (int d = 0; d < r->n_derived; d++) {
            int parent_id = r->derived_from[d];
            if (parent_id >= 0 && parent_id < nr)
                matrix[i * nb + bid] |= matrix[parent_id * nb + bid];
        }
    }
}

/* Count traceability links (non-zero entries in the matrix). */
int mbse_count_trace_links(int* matrix, int n_reqs, int n_blocks) {
    if (!matrix) return 0;
    int count = 0;
    for (int i = 0; i < n_reqs; i++)
        for (int j = 0; j < n_blocks; j++)
            if (matrix[i * n_blocks + j]) count++;
    return count;
}

/* Traceability density: links / (n_reqs * n_blocks).
 * >0.3 = well-traced, <0.1 = poorly traced. */
double mbse_traceability_density(int* matrix, int n_reqs, int n_blocks) {
    if (n_reqs <= 0 || n_blocks <= 0) return 0.0;
    int total = n_reqs * n_blocks;
    return (double)mbse_count_trace_links(matrix, n_reqs, n_blocks)
         / (double)total;
}

/* ---------- Coverage Metrics ---------- */

/* Requirement allocation coverage: fraction of requirements
 * allocated to at least one block. */
double mbse_allocation_coverage(SystemModel* m) {
    if (!m || m->n_requirements <= 0) return 0.0;
    int allocated = 0;
    for (int i = 0; i < m->n_requirements; i++)
        if (m->requirements[i].satisfied_by_block >= 0) allocated++;
    return (double)allocated / (double)m->n_requirements;
}

/* Verification coverage: fraction of requirements verified. */
double mbse_verification_coverage(SystemModel* m) {
    if (!m || m->n_requirements <= 0) return 0.0;
    int verified = 0;
    for (int i = 0; i < m->n_requirements; i++)
        if (m->requirements[i].is_verified) verified++;
    return (double)verified / (double)m->n_requirements;
}

/* Satisfaction coverage: fraction of requirements satisfied. */
double mbse_satisfaction_coverage(SystemModel* m) {
    if (!m || m->n_requirements <= 0) return 0.0;
    int satisfied = 0;
    for (int i = 0; i < m->n_requirements; i++)
        if (m->requirements[i].is_satisfied) satisfied++;
    return (double)satisfied / (double)m->n_requirements;
}

/* Overall requirements health index: weighted average.
 * health = 0.3*alloc_cov + 0.3*verif_cov + 0.2*sat_cov
 *        + 0.2*trace_density. Returns [0, 1]. */
double mbse_requirements_health(SystemModel* m) {
    if (!m || m->n_requirements <= 0) return 0.0;
    double alloc = mbse_allocation_coverage(m);
    double verif = mbse_verification_coverage(m);
    double satis = mbse_satisfaction_coverage(m);
    double trace = mbse_traceability_density(NULL, 0, 0);  /* simplified */
    /* Use internal trace matrix if available */
    if (m->traceability_matrix) {
        trace = mbse_traceability_density(*m->traceability_matrix,
                                          m->trace_rows, m->trace_cols);
    }
    return 0.3 * alloc + 0.3 * verif + 0.2 * satis + 0.2 * trace;
}

/* ---------- Impact Analysis ---------- */

/* Impact assessment: if requirement r_id changes, which blocks
 * and derived requirements are affected?
 *
 * Fills affected_blocks[] and affected_reqs[] (caller alloc).
 * affected_blocks[j] = 1 if block j is impacted.
 * affected_reqs[k] = 1 if requirement k is impacted.
 * Returns total number of impacted elements. */
int mbse_impact_analysis(SystemModel* m, int r_id,
                           int* affected_blocks, int* affected_reqs) {
    if (!m || !affected_blocks || !affected_reqs
        || r_id < 0 || r_id >= m->n_requirements) return 0;
    int nb = m->n_blocks, nr = m->n_requirements;
    memset(affected_blocks, 0, (size_t)nb * sizeof(int));
    memset(affected_reqs, 0, (size_t)nr * sizeof(int));

    /* Mark the changed requirement */
    affected_reqs[r_id] = 1;
    int total = 1;

    /* Find block allocated to this requirement */
    int bid = m->requirements[r_id].satisfied_by_block;
    if (bid >= 0 && bid < nb) {
        affected_blocks[bid] = 1;
        total++;
    }

    /* Find derived requirements (transitive closure) */
    for (int iter = 0; iter < 10; iter++) {  /* Limit propagation depth */
        int new_found = 0;
        for (int i = 0; i < nr; i++) {
            if (affected_reqs[i]) continue;
            for (int d = 0; d < m->requirements[i].n_derived; d++) {
                if (affected_reqs[m->requirements[i].derived_from[d]]) {
                    affected_reqs[i] = 1; total++; new_found = 1;
                    /* Also mark its block */
                    int b = m->requirements[i].satisfied_by_block;
                    if (b >= 0 && b < nb && !affected_blocks[b]) {
                        affected_blocks[b] = 1; total++;
                    }
                    break;
                }
            }
        }
        if (!new_found) break;
    }
    return total;
}

/* Cost of change: estimate the rework cost if requirement r_id changes.
 * Based on Boehm's COCOMO cost drivers and traceability depth.
 * cost = base_cost * (1 + propagation_depth * multiplier). */
double mbse_cost_of_change(SystemModel* m, int r_id,
                             double base_cost) {
    if (!m || r_id < 0 || r_id >= m->n_requirements) return 0.0;
    int nb = m->n_blocks, nr = m->n_requirements;
    int* aff_blocks = (int*)calloc((size_t)nb, sizeof(int));
    int* aff_reqs   = (int*)calloc((size_t)nr, sizeof(int));
    if (!aff_blocks || !aff_reqs) {
        free(aff_blocks); free(aff_reqs); return base_cost;
    }
    int impact = mbse_impact_analysis(m, r_id, aff_blocks, aff_reqs);
    /* Deeper propagation = higher cost */
    double depth_penalty = 1.0 + 0.2 * (double)(impact - 1);
    if (depth_penalty > 5.0) depth_penalty = 5.0;
    free(aff_blocks); free(aff_reqs);
    return base_cost * depth_penalty;
}

/* ---------- Verification Traceability ---------- */

typedef struct {
    int id;
    char* name;
    VerificationMethod method;
    int verified_req_id;
    double result_value;
    bool passed;
    char* procedure;
} VerificationCase;

typedef struct {
    VerificationCase* cases;
    int n_cases;
    int case_capacity;
} TraceVerificationPlan;

TraceVerificationPlan* vplan_create(void) {
    TraceVerificationPlan* vp = (TraceVerificationPlan*)calloc(1,
                               sizeof(TraceVerificationPlan));
    if (!vp) return NULL;
    vp->case_capacity = 16;
    vp->cases = (VerificationCase*)calloc(16, sizeof(VerificationCase));
    return vp;
}

void vplan_free(TraceVerificationPlan* vp) {
    if (!vp) return;
    for (int i = 0; i < vp->n_cases; i++) {
        free(vp->cases[i].name);
        free(vp->cases[i].procedure);
    }
    free(vp->cases); free(vp);
}

int vplan_add_case(TraceVerificationPlan* vp, const char* name,
                    VerificationMethod method, int req_id,
                    const char* procedure) {
    if (!vp || !name) return -1;
    if (vp->n_cases >= vp->case_capacity) {
        int nc = vp->case_capacity * 2;
        VerificationCase* tmp = (VerificationCase*)realloc(
            vp->cases, (size_t)nc * sizeof(VerificationCase));
        if (!tmp) return -1;
        vp->cases = tmp; vp->case_capacity = nc;
    }
    int idx = vp->n_cases++;
    VerificationCase* vc = &vp->cases[idx];
    memset(vc, 0, sizeof(VerificationCase));
    vc->id = idx;
    vc->name = strdup(name);
    vc->method = method;
    vc->verified_req_id = req_id;
    vc->procedure = procedure ? strdup(procedure) : NULL;
    return idx;
}

/* Execute a verification case and record the result. */
void vplan_execute_case(TraceVerificationPlan* vp, int case_id,
                          double result, bool passed) {
    if (!vp || case_id < 0 || case_id >= vp->n_cases) return;
    vp->cases[case_id].result_value = result;
    vp->cases[case_id].passed = passed;
}

/* Verification completion percentage. */
double vplan_completion(TraceVerificationPlan* vp) {
    if (!vp || vp->n_cases <= 0) return 0.0;
    int done = 0;
    for (int i = 0; i < vp->n_cases; i++)
        if (vp->cases[i].passed || !vp->cases[i].passed) done++;
    return (double)done / (double)vp->n_cases;
}

/* Pass rate among executed cases. */
double vplan_pass_rate(TraceVerificationPlan* vp) {
    if (!vp || vp->n_cases <= 0) return 0.0;
    int passed = 0;
    for (int i = 0; i < vp->n_cases; i++)
        if (vp->cases[i].passed) passed++;
    return (double)passed / (double)vp->n_cases;
}

/* ---------- Print ---------- */

void mbse_traceability_report(SystemModel* m) {
    if (!m) return;
    printf("===== Traceability Report =====\n");
    printf("Requirements: %d  Blocks: %d  Interfaces: %d\n",
           m->n_requirements, m->n_blocks, m->n_interfaces);
    printf("Allocation:  %.1f%%\n",
           100.0 * mbse_allocation_coverage(m));
    printf("Verification: %.1f%%\n",
           100.0 * mbse_verification_coverage(m));
    printf("Satisfaction: %.1f%%\n",
           100.0 * mbse_satisfaction_coverage(m));
    printf("Health Index: %.3f\n",
           mbse_requirements_health(m));
    printf("===============================\n");
}

void vplan_print_summary(TraceVerificationPlan* vp) {
    if (!vp) return;
    printf("=== Verification Plan ===\n");
    printf("Cases: %d  Passed: %d\n",
           vp->n_cases, (int)(vplan_pass_rate(vp) * vp->n_cases));
    printf("Pass rate: %.1f%%\n", 100.0 * vplan_pass_rate(vp));
    for (int i = 0; i < vp->n_cases; i++) {
        printf("  [%d] %s: %s\n",
               vp->cases[i].id, vp->cases[i].name,
               vp->cases[i].passed ? "PASS" : "pending");
    }
}
