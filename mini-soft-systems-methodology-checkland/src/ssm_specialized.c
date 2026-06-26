/* ssm_advanced.c -- L8: Advanced SSM - Multi-level Weltanschauung, boundary negotiation.
 *
 * Extends basic SSM with advanced concepts: multiple worldviews,
 * boundary negotiation protocols, and formal conflict resolution
 * in soft systems intervention.
 *
 * Reference: Checkland & Holwell (1998) Information, Systems and IS
 *            Ulrich (1983) Critical Heuristics of Social Planning
 */
#include "ssm_core.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    char *worldview_name, *proponent, *opponent;
    double strength, acceptance;
    int n_adherents;
} Worldview;

Worldview* wv_new(const char *name, const char *prop, double strength) {
    Worldview *w = calloc(1, sizeof(Worldview));
    w->worldview_name = strdup(name);
    w->proponent = strdup(prop);
    w->strength = strength;
    w->acceptance = 0.5;
    return w;
}
void wv_free(Worldview *w) {
    if (w) { free(w->worldview_name); free(w->proponent); free(w->opponent); free(w); }
}

/* Multi-worldview conflict analysis */
double wv_conflict_index(Worldview *wv[], int n) {
    if (!wv || n < 2) return 0.0;
    double total_conflict = 0.0;
    int pairs = 0;
    for (int i = 0; i < n - 1; i++)
        for (int j = i + 1; j < n; j++) {
            total_conflict += fabs(wv[i]->strength - wv[j]->strength) *
                              (1.0 - fmin(wv[i]->acceptance, wv[j]->acceptance));
            pairs++;
        }
    return pairs > 0 ? total_conflict / (double)pairs : 0.0;
}

/* Boundary negotiation: find maximally acceptable worldview */
int wv_negotiate_consensus(Worldview *wv[], int n, int *best_idx, double *consensus) {
    if (!wv || !best_idx || !consensus || n < 1) return -1;
    double best_score = -1.0;
    for (int i = 0; i < n; i++) {
        double score = wv[i]->strength * wv[i]->acceptance * (double)wv[i]->n_adherents;
        if (score > best_score) { best_score = score; *best_idx = i; }
    }
    *consensus = best_score / fmax((double)n * 100.0, 1.0);
    return 0;
}

/* Critical boundary reflection (Ulrich CSH) */
double wv_boundary_reflection(Worldview *wv[], int n) {
    if (!wv || n < 1) return 0.0;
    double sum_ref = 0.0;
    for (int i = 0; i < n; i++)
        sum_ref += (1.0 - wv[i]->acceptance) * wv[i]->strength;
    return sum_ref / (double)n;
}
