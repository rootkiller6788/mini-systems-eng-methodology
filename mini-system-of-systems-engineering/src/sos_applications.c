/* sos_applications.c -- L7: Smart grid SoS (IEEE 2030) + NATO air defense SoS.
 * Reference: IEEE 2030-2011, NATO STO Technical Reports
 */
#include "sos_types.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    double generation_mw, load_mw, storage_mwh;
    int n_generators, n_consumers, n_storage_units;
    double renewable_pct, reliability;
} SmartGridSoS;

SmartGridSoS* grid_new(double gen, double load, int n_gen) {
    SmartGridSoS *g = calloc(1, sizeof(SmartGridSoS));
    g->generation_mw = gen; g->load_mw = load;
    g->n_generators = n_gen;
    return g;
}
void grid_free(SmartGridSoS *g) { free(g); }

int grid_power_balance(SmartGridSoS *g, double *balance) {
    if (!g || !balance) return -1;
    *balance = g->generation_mw / fmax(g->load_mw, 0.1);
    return 0;
}

int grid_renewable_penetration(SmartGridSoS *g, double *score) {
    if (!g || !score) return -1;
    *score = g->renewable_pct / 100.0;
    return 0;
}

int grid_reliability_index(SmartGridSoS *g, double *index) {
    if (!g || !index) return -1;
    *index = g->reliability * (1.0 - 0.1 * (1.0 - g->renewable_pct / 100.0));
    return 0;
}

int grid_storage_adequacy(SmartGridSoS *g, double *adequacy) {
    if (!g || !adequacy) return -1;
    double hour_load = g->load_mw;
    *adequacy = g->storage_mwh / fmax(hour_load * 4.0, 0.1);
    if (*adequacy > 1.0) *adequacy = 1.0;
    return 0;
}

void grid_print(SmartGridSoS *g) {
    double balance, ren, rel, storage;
    grid_power_balance(g, &balance);
    grid_renewable_penetration(g, &ren);
    grid_reliability_index(g, &rel);
    grid_storage_adequacy(g, &storage);
    printf("=== Smart Grid SoS (IEEE 2030) ===\n");
    printf("Generation=%.0fMW Load=%.0fMW GenUnits=%d\n",
           g->generation_mw, g->load_mw, g->n_generators);
    printf("Balance=%.2f Renewables=%.0f%% Reliability=%.3f Storage=%.2f\n",
           balance, g->renewable_pct, rel, storage);
}
