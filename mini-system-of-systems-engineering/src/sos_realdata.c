/* sos_app3.c -- L7-2: US Air Force C2 constellation SoS (Gorod 2008).
 * Reference: Gorod et al. (2008) System of Systems Engineering
 */
#include "sos_types.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    int n_satellites, n_ground_stations, n_command_nodes;
    double bandwidth_mbps, latency_ms, coverage_pct;
    double resilience_index, redundancy_ratio;
} C2Constellation;

C2Constellation* c2_new(int sats, int stations, int nodes) {
    C2Constellation *c = calloc(1, sizeof(C2Constellation));
    c->n_satellites = sats;
    c->n_ground_stations = stations;
    c->n_command_nodes = nodes;
    c->resilience_index = 0.8;
    return c;
}
void c2_free(C2Constellation *c) { free(c); }

int c2_coverage_score(C2Constellation *c, double *score) {
    if (!c || !score) return -1;
    *score = c->coverage_pct * (1.0 - exp(-0.1 * (double)c->n_satellites));
    return 0;
}

int c2_resilience(C2Constellation *c, double *resilience) {
    if (!c || !resilience) return -1;
    double redundancy = (double)c->n_satellites / fmax((double)c->n_ground_stations, 1.0);
    *resilience = 1.0 - exp(-0.3 * redundancy);
    return 0;
}

int c2_command_latency(C2Constellation *c, double *latency_score) {
    if (!c || !latency_score) return -1;
    double target = 100.0;
    *latency_score = target / fmax(c->latency_ms, 1.0);
    if (*latency_score > 1.0) *latency_score = 1.0;
    return 0;
}

int c2_interoperability(C2Constellation *c, double *iop) {
    if (!c || !iop) return -1;
    *iop = 1.0 - 1.0 / fmax((double)c->n_command_nodes, 2.0);
    return 0;
}

void c2_print(C2Constellation *c) {
    double cov, res, lat, iop;
    c2_coverage_score(c, &cov);
    c2_resilience(c, &res);
    c2_command_latency(c, &lat);
    c2_interoperability(c, &iop);
    printf("=== USAF C2 Constellation SoS (Gorod 2008) ===\n");
    printf("Sats=%d Stations=%d CmdNodes=%d\n",
           c->n_satellites, c->n_ground_stations, c->n_command_nodes);
    printf("Coverage=%.3f Resilience=%.3f Latency=%.3f Interop=%.3f\n",
           cov, res, lat, iop);
}
