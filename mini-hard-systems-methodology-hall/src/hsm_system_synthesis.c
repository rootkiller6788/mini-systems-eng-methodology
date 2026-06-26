#include "../include/hsm_core.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* System Synthesis — Hall Phase 3: Generate Alternatives
 *
 * Morphological analysis generates all feasible combinations
 * of subsystem options. This module evaluates and ranks them. */

typedef struct { char* name; double* scores; int n_criteria; double total_score; } SynthesisOption;
typedef struct { SynthesisOption* opts; int n; int capacity; double* weights; int n_criteria; } SynthesisMatrix;

SynthesisMatrix* sm_create(int n_criteria, const double* weights) {
    SynthesisMatrix* sm = calloc(1, sizeof(SynthesisMatrix));
    if (!sm) return NULL;
    sm->n_criteria = n_criteria;
    sm->capacity = 64;
    sm->weights = calloc(n_criteria, sizeof(double));
    if (weights) memcpy(sm->weights, weights, n_criteria * sizeof(double));
    sm->opts = calloc(sm->capacity, sizeof(SynthesisOption));
    return sm;
}

void sm_free(SynthesisMatrix* sm) {
    if (!sm) return;
    for (int i = 0; i < sm->n; i++) { free(sm->opts[i].name); free(sm->opts[i].scores); }
    free(sm->opts); free(sm->weights); free(sm);
}

int sm_add_option(SynthesisMatrix* sm, const char* name, const double* scores) {
    if (!sm || !name || !scores) return -1;
    if (sm->n >= sm->capacity) { sm->capacity *= 2; sm->opts = realloc(sm->opts, sm->capacity*sizeof(SynthesisOption)); }
    sm->opts[sm->n].name = strdup(name);
    sm->opts[sm->n].scores = calloc(sm->n_criteria, sizeof(double));
    memcpy(sm->opts[sm->n].scores, scores, sm->n_criteria * sizeof(double));
    sm->opts[sm->n].n_criteria = sm->n_criteria;
    return sm->n++;
}

int sm_evaluate(SynthesisMatrix* sm) {
    if (!sm) return -1;
    for (int i = 0; i < sm->n; i++) {
        sm->opts[i].total_score = 0.0;
        for (int j = 0; j < sm->n_criteria; j++)
            sm->opts[i].total_score += sm->opts[i].scores[j] * sm->weights[j];
    }
    return 0;
}

int sm_find_best(SynthesisMatrix* sm, int* best_idx) {
    if (!sm || !best_idx || sm->n < 1) return -1;
    sm_evaluate(sm);
    *best_idx = 0;
    for (int i = 1; i < sm->n; i++)
        if (sm->opts[i].total_score > sm->opts[*best_idx].total_score) *best_idx = i;
    return 0;
}
