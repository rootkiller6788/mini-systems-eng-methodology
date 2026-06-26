/* sos_advanced.c -- L8: Advanced SoS - dynamic reconfiguration, SoS resilience theory.
 * Reference: Jamshidi (2009) SoS Engineering, Haimes (2009) Risk Modeling
 */
#include "sos_types.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* SoS Resilience: absorb, adapt, recover capabilities */
typedef struct {
    double absorption, adaptation, recovery;
    double resilience_score;
} SoSResilience;

SoSResilience* res_new(double absorb, double adapt, double recover) {
    SoSResilience *r = calloc(1, sizeof(SoSResilience));
    r->absorption = absorb; r->adaptation = adapt; r->recovery = recover;
    return r;
}
void res_free(SoSResilience *r) { free(r); }

/* Resilience = (absorption * adaptation * recovery)^(1/3) */
double res_compute(SoSResilience *r) {
    if (!r) return 0.0;
    r->resilience_score = pow(r->absorption * r->adaptation * r->recovery,
                               1.0 / 3.0);
    return r->resilience_score;
}

/* Reconfiguration cost for changing SoS topology */
typedef struct {
    int n_systems, n_connections;
    double *connectivity_matrix;
    double reconfig_cost;
} SoSTopology;

SoSTopology* topo_new(int n) {
    SoSTopology *t = calloc(1, sizeof(SoSTopology));
    t->n_systems = n;
    t->connectivity_matrix = calloc((size_t)(n * n), sizeof(double));
    t->reconfig_cost = 1.0;
    return t;
}
void topo_free(SoSTopology *t) {
    if (t) { free(t->connectivity_matrix); free(t); }
}

int topo_set_link(SoSTopology *t, int i, int j, double strength) {
    if (!t || i < 0 || j < 0 || i >= t->n_systems || j >= t->n_systems) return -1;
    t->connectivity_matrix[i * t->n_systems + j] = strength;
    t->n_connections++;
    return 0;
}

double topo_density(SoSTopology *t) {
    if (!t || t->n_systems < 2) return 0.0;
    int max_edges = t->n_systems * (t->n_systems - 1);
    return (double)t->n_connections / fmax((double)max_edges, 1.0);
}

int topo_reconfigure(SoSTopology *t, const double *new_matrix, double *cost) {
    if (!t || !new_matrix || !cost) return -1;
    *cost = 0.0;
    for (int i = 0; i < t->n_systems * t->n_systems; i++) {
        double diff = new_matrix[i] - t->connectivity_matrix[i];
        *cost += diff * diff;
    }
    *cost *= t->reconfig_cost;
    return 0;
}
