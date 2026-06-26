/* ssm_app2.c -- L7-2: UK Prison Service SSM intervention + community policing CATWOE.
 *
 * Applies Checkland''s Soft Systems Methodology to UK Prison Service
 * and community policing. Demonstrates CATWOE analysis, root definitions,
 * and conceptual modeling for social systems intervention.
 *
 * Reference: Checkland (1981) Systems Thinking, Systems Practice
 *            Checkland & Scholes (1990) SSM in Action
 */
#include "ssm_core.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    char *customers, *actors, *transformation;
    char *weltanschauung, *owner, *environment;
    double clarity_score, feasibility;
} CatwoeAnalysis;

CatwoeAnalysis* catwoe_new(const char *c, const char *a, const char *t,
                            const char *w, const char *o, const char *e) {
    CatwoeAnalysis *ca = calloc(1, sizeof(CatwoeAnalysis));
    ca->customers = strdup(c); ca->actors = strdup(a);
    ca->transformation = strdup(t); ca->weltanschauung = strdup(w);
    ca->owner = strdup(o); ca->environment = strdup(e);
    return ca;
}
void catwoe_free(CatwoeAnalysis *ca) {
    if (!ca) return;
    free(ca->customers); free(ca->actors); free(ca->transformation);
    free(ca->weltanschauung); free(ca->owner); free(ca->environment);
    free(ca);
}

int catwoe_completeness(CatwoeAnalysis *ca, double *score) {
    if (!ca || !score) return -1;
    int n = 0;
    if (strlen(ca->customers) > 0) n++;
    if (strlen(ca->actors) > 0) n++;
    if (strlen(ca->transformation) > 0) n++;
    if (strlen(ca->weltanschauung) > 0) n++;
    if (strlen(ca->owner) > 0) n++;
    if (strlen(ca->environment) > 0) n++;
    *score = (double)n / 6.0;
    return 0;
}

int catwoe_clarity_index(CatwoeAnalysis *ca, double *clarity) {
    if (!ca || !clarity) return -1;
    int total = (int)strlen(ca->customers) + (int)strlen(ca->actors) +
                (int)strlen(ca->transformation) + (int)strlen(ca->weltanschauung);
    *clarity = total > 80 ? 1.0 : (double)total / 80.0;
    return 0;
}

int catwoe_weltanschauung_check(CatwoeAnalysis *ca, double *conflict_potential) {
    if (!ca || !conflict_potential) return -1;
    int len = (int)strlen(ca->weltanschauung);
    *conflict_potential = (len > 30) ? 0.7 : (len > 15) ? 0.4 : 0.2;
    return 0;
}

void catwoe_print(CatwoeAnalysis *ca) {
    double completeness, clarity, conflict;
    catwoe_completeness(ca, &completeness);
    catwoe_clarity_index(ca, &clarity);
    catwoe_weltanschauung_check(ca, &conflict);
    printf("=== SSM CATWOE: UK Prison Service ===\n");
    printf("C=%s\nA=%s\nT=%s\n", ca->customers, ca->actors, ca->transformation);
    printf("W=%s\nO=%s\nE=%s\n", ca->weltanschauung, ca->owner, ca->environment);
    printf("Completeness=%.2f Clarity=%.2f ConflictPotential=%.2f\n",
           completeness, clarity, conflict);
}
