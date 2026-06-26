/* vvu_app2.c -- L7-2: Fukushima Daiichi safety V&V failure + autonomous vehicle V&V (ISO 26262).
 * Reference: IAEA Fukushima Report (2015), ISO 26262:2018
 */
#include "vvu_core.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    double design_basis_event, actual_event;
    double safety_margin, backup_readiness;
    int n_barriers, n_failed_barriers;
    double detection_time_h, response_time_h;
} NuclearSafety;

NuclearSafety* nuke_new(double design, double actual) {
    NuclearSafety *n = calloc(1, sizeof(NuclearSafety));
    n->design_basis_event = design;
    n->actual_event = actual;
    n->n_barriers = 5;
    return n;
}
void nuke_free(NuclearSafety *n) { free(n); }

int nuke_safety_margin(NuclearSafety *n, double *margin) {
    if (!n || !margin) return -1;
    *margin = n->design_basis_event / fmax(n->actual_event, 1e-6);
    if (*margin > 1.0) *margin = 1.0;
    return 0;
}

int nuke_barrier_integrity(NuclearSafety *n, double *integrity) {
    if (!n || !integrity) return -1;
    *integrity = n->n_barriers > 0 ?
        (double)(n->n_barriers - n->n_failed_barriers) / (double)n->n_barriers : 0.0;
    return 0;
}

int nuke_response_adequacy(NuclearSafety *n, double *adequacy) {
    if (!n || !adequacy) return -1;
    double threshold = 4.0;
    *adequacy = threshold / fmax(n->response_time_h, 0.01);
    if (*adequacy > 1.0) *adequacy = 1.0;
    return 0;
}

int nuke_fukushima_lesson(NuclearSafety *n, double *beyond_design_score) {
    if (!n || !beyond_design_score) return -1;
    *beyond_design_score = n->actual_event / fmax(n->design_basis_event, 1.0);
    return 0;
}

void nuke_print(NuclearSafety *n) {
    double margin, integrity, adequacy, beyond;
    nuke_safety_margin(n, &margin);
    nuke_barrier_integrity(n, &integrity);
    nuke_response_adequacy(n, &adequacy);
    nuke_fukushima_lesson(n, &beyond);
    printf("=== Fukushima V&V Analysis ===\n");
    printf("DesignBasis=%.1f Actual=%.1f BeyondDesign=%.2fx\n",
           n->design_basis_event, n->actual_event, beyond);
    printf("Barriers=%d/%d Margin=%.3f Response=%.3f\n",
           n->n_barriers - n->n_failed_barriers, n->n_barriers, margin, adequacy);
    printf("Lesson: Beyond-design-basis events require %s V&V\n",
           beyond > 1.5 ? "EXPANDED" : "standard");
}
