#include "../include/mbse_core.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* MBSE Viewpoint Analysis — ISO/IEC 42010 compliant.
 *
 * Architecture views address different stakeholder concerns.
 * This module computes consistency metrics across viewpoints:
 * functional, physical, behavioral, requirements, and parametric. */

typedef struct {
    int     n_views;
    double* view_coverage;     /* Fraction of system covered by each view */
    double* inter_view_gaps;   /* Missing links between view pairs */
    double  overall_coherence; /* Cross-view consistency [0, 1] */
    int*    orphan_elements;   /* Elements not covered by any view */
    int     n_orphans;
} ViewpointAnalysis;

ViewpointAnalysis* vpa_create(int n_views) {
    ViewpointAnalysis* va = calloc(1, sizeof(ViewpointAnalysis));
    if (!va) return NULL;
    va->n_views = n_views;
    va->view_coverage = calloc(n_views, sizeof(double));
    va->inter_view_gaps = calloc(n_views * n_views, sizeof(double));
    va->orphan_elements = calloc(100, sizeof(int));
    return va;
}

void vpa_free(ViewpointAnalysis* va) {
    if (!va) return;
    free(va->view_coverage);
    free(va->inter_view_gaps);
    free(va->orphan_elements);
    free(va);
}

int vpa_compute_coverage(ViewpointAnalysis* va, int view_idx,
                          int n_elements, const int* covered_elements) {
    if (!va || view_idx < 0 || view_idx >= va->n_views) return -1;
    va->view_coverage[view_idx] = (double)n_elements / 100.0;
    return 0;
}

int vpa_compute_gaps(ViewpointAnalysis* va, int v1, int v2,
                      int n_links, const double* link_strengths) {
    if (!va || !link_strengths) return -1;
    double total = 0.0;
    for (int i = 0; i < n_links; i++) total += link_strengths[i];
    va->inter_view_gaps[v1 * va->n_views + v2] = total / (n_links + 1e-10);
    return 0;
}

int vpa_detect_orphans(ViewpointAnalysis* va, int n_elements,
                        const int* view_membership, int total_elements) {
    if (!va || !view_membership) return -1;
    va->n_orphans = 0;
    for (int e = 0; e < total_elements; e++) {
        int covered = 0;
        for (int v = 0; v < va->n_views; v++) {
            if (view_membership[e * va->n_views + v] > 0) { covered = 1; break; }
        }
        if (!covered && va->n_orphans < 100)
            va->orphan_elements[va->n_orphans++] = e;
    }
    return va->n_orphans;
}

double vpa_coherence(ViewpointAnalysis* va) {
    if (!va || va->n_views < 2) return 1.0;
    double cov_sum = 0.0, gap_sum = 0.0;
    for (int i = 0; i < va->n_views; i++) cov_sum += va->view_coverage[i];
    int npairs = va->n_views * (va->n_views - 1);
    for (int i = 0; i < npairs; i++) gap_sum += va->inter_view_gaps[i];
    double orphan_penalty = (double)va->n_orphans / 100.0;
    va->overall_coherence = (cov_sum / va->n_views) * (1.0 - gap_sum / (npairs + 1e-10))
                            * (1.0 - orphan_penalty);
    if (va->overall_coherence < 0.0) va->overall_coherence = 0.0;
    if (va->overall_coherence > 1.0) va->overall_coherence = 1.0;
    return va->overall_coherence;
}
