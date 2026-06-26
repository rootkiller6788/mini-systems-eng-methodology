/* stakeholder_app3.c -- L7-3: AHP applied to US DoD supplier selection (Tam & Tummala 2001).
 * Reference: Saaty (1980) The Analytic Hierarchy Process
 *            Tam & Tummala (2001) Omega 29(2):171-182
 */
#include "stakeholder_model.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    double **matrix, *priority_vector;
    double lambda_max, ci, cr;
    int n;
} AHPSupplier;

AHPSupplier* ahp_supplier_new(int n) {
    if (n < 1 || n > 15) return NULL;
    AHPSupplier *a = calloc(1, sizeof(AHPSupplier));
    a->n = n;
    a->matrix = calloc((size_t)n, sizeof(double*));
    a->priority_vector = calloc((size_t)n, sizeof(double));
    for (int i = 0; i < n; i++) {
        a->matrix[i] = calloc((size_t)n, sizeof(double));
        a->matrix[i][i] = 1.0;
    }
    return a;
}
void ahp_supplier_free(AHPSupplier *a) {
    if (!a) return;
    for (int i = 0; i < a->n; i++) free(a->matrix[i]);
    free(a->matrix); free(a->priority_vector); free(a);
}
int ahp_set_pairwise(AHPSupplier *a, int i, int j, double val) {
    if (!a || i < 0 || j < 0 || i >= a->n || j >= a->n || val <= 0) return -1;
    a->matrix[i][j] = val; a->matrix[j][i] = 1.0 / val;
    return 0;
}
int ahp_compute_priorities(AHPSupplier *a) {
    if (!a) return -1;
    double *v = calloc((size_t)a->n, sizeof(double));
    for (int i = 0; i < a->n; i++) v[i] = 1.0 / (double)a->n;
    for (int iter = 0; iter < 50; iter++) {
        double *next = calloc((size_t)a->n, sizeof(double));
        double sum = 0.0;
        for (int i = 0; i < a->n; i++) {
            for (int j = 0; j < a->n; j++) next[i] += a->matrix[i][j] * v[j];
            sum += next[i];
        }
        double diff = 0.0;
        for (int i = 0; i < a->n; i++) {
            next[i] /= sum;
            diff += fabs(next[i] - v[i]);
            v[i] = next[i];
        }
        free(next);
        if (diff < 1e-8) break;
    }
    for (int i = 0; i < a->n; i++) a->priority_vector[i] = v[i];
    free(v);
    return 0;
}
int ahp_consistency_ratio(AHPSupplier *a) {
    if (!a || a->n < 2) return -1;
    double *aw = calloc((size_t)a->n, sizeof(double));
    for (int i = 0; i < a->n; i++)
        for (int j = 0; j < a->n; j++)
            aw[i] += a->matrix[i][j] * a->priority_vector[j];
    a->lambda_max = 0.0;
    for (int i = 0; i < a->n; i++)
        a->lambda_max += aw[i] / (a->priority_vector[i] + 1e-10);
    a->lambda_max /= (double)a->n;
    a->ci = (a->lambda_max - (double)a->n) / (double)(a->n - 1);
    double ri[] = {0,0,0.58,0.90,1.12,1.24,1.32,1.41,1.45,1.49,1.51,1.48,1.56,1.57,1.59};
    a->cr = (a->n <= 15) ? a->ci / ri[a->n - 1] : a->ci / 1.6;
    free(aw);
    return 0;
}
int ahp_select_best_supplier(AHPSupplier *a, int *best_idx, double *score) {
    if (!a || !best_idx || !score) return -1;
    *best_idx = 0; *score = a->priority_vector[0];
    for (int i = 1; i < a->n; i++)
        if (a->priority_vector[i] > *score) {
            *score = a->priority_vector[i];
            *best_idx = i;
        }
    return 0;
}
void ahp_print_supplier_analysis(AHPSupplier *a, const char **names) {
    if (!a) return;
    printf("=== AHP Supplier Selection (DoD) ===\n");
    printf("CR=%.4f (consistent: %s)\n", a->cr, a->cr < 0.10 ? "YES" : "NO");
    for (int i = 0; i < a->n; i++)
        printf("  %s: %.4f\n", names ? names[i] : "Alt", a->priority_vector[i]);
}
