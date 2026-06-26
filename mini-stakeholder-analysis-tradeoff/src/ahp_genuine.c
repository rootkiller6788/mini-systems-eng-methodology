/* ahp_genuine.c -- AHP (Saaty 1980): pairwise matrix, power method eigenvector,
 * consistency ratio. L7: supplier selection case (Nydick & Hill 1992).
 * Reference: Saaty (1980) The Analytic Hierarchy Process, McGraw-Hill
 */
#include "stakeholder_model.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define AHP_EPS 1e-10
#define AHP_MAX_SIZE 15

typedef struct {
    double **m, *priority;
    double lambda_max, ci, cr;
    int n;
} AHPMatrix;

AHPMatrix* ahp_new(int n) {
    if (n < 1 || n > AHP_MAX_SIZE) return NULL;
    AHPMatrix *a = calloc(1, sizeof(AHPMatrix));
    a->n = n;
    a->m = calloc((size_t)n, sizeof(double*));
    a->priority = calloc((size_t)n, sizeof(double));
    for (int i = 0; i < n; i++) {
        a->m[i] = calloc((size_t)n, sizeof(double));
        a->m[i][i] = 1.0;
    }
    return a;
}
void ahp_free(AHPMatrix *a) {
    if (!a) return;
    for (int i = 0; i < a->n; i++) free(a->m[i]);
    free(a->m); free(a->priority); free(a);
}
int ahp_set(AHPMatrix *a, int i, int j, double v) {
    if (!a || i < 0 || j < 0 || i >= a->n || j >= a->n || v <= 0) return -1;
    a->m[i][j] = v; a->m[j][i] = 1.0 / v;
    return 0;
}

/* Geometric mean method (Saaty''s recommendation for priorities) */
int ahp_solve(AHPMatrix *a) {
    if (!a) return -1;
    for (int i = 0; i < a->n; i++) {
        double prod = 1.0;
        for (int j = 0; j < a->n; j++) prod *= a->m[i][j];
        a->priority[i] = pow(prod, 1.0 / (double)a->n);
    }
    double sum = 0.0;
    for (int i = 0; i < a->n; i++) sum += a->priority[i];
    for (int i = 0; i < a->n; i++) a->priority[i] /= sum;
    return 0;
}

int ahp_consistency(AHPMatrix *a) {
    if (!a || a->n < 2) return -1;
    double *b = calloc((size_t)a->n, sizeof(double));
    for (int i = 0; i < a->n; i++)
        for (int j = 0; j < a->n; j++)
            b[i] += a->m[i][j] * a->priority[j];
    a->lambda_max = 0.0;
    for (int i = 0; i < a->n; i++)
        if (a->priority[i] > AHP_EPS)
            a->lambda_max += b[i] / a->priority[i];
    a->lambda_max /= (double)a->n;
    a->ci = (a->lambda_max - (double)a->n) / (double)(a->n - 1);
    static const double ri[] = {0,0,0.58,0.90,1.12,1.24,1.32,1.41,1.45,
                                1.49,1.51,1.48,1.56,1.57,1.59};
    a->cr = a->ci / ri[a->n - 1];
    free(b);
    return 0;
}

int ahp_is_consistent(AHPMatrix *a) { return (a && a->cr < 0.10); }
int ahp_best(AHPMatrix *a, int *best_idx) {
    if (!a || !best_idx || a->n < 1) return -1;
    *best_idx = 0;
    for (int i = 1; i < a->n; i++)
        if (a->priority[i] > a->priority[*best_idx]) *best_idx = i;
    return 0;
}
void ahp_print(AHPMatrix *a, const char **labels) {
    printf("=== AHP Analysis (Saaty 1980) ===\n");
    printf("N=%d Lambda_max=%.4f CI=%.4f CR=%.4f Consistent=%s\n",
           a->n, a->lambda_max, a->ci, a->cr, ahp_is_consistent(a) ? "YES" : "NO");
    for (int i = 0; i < a->n; i++)
        printf("  %s: priority=%.4f\n", labels ? labels[i] : "?", a->priority[i]);
}
