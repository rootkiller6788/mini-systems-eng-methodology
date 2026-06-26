/* mbse_app2.c -- L7-2: Boeing 787 MBSE electrical system + AUTOSAR modeling.
 *
 * Applies MBSE principles to aircraft electrical system architecture.
 * Demonstrates SysML-style requirements allocation, block definition,
 * and parametric constraints for aerospace certification.
 *
 * Reference: Boeing 787 Systems (SAE ARP4754A), AUTOSAR Classic Platform
 *            INCOSE Systems Engineering Handbook (2015) Ch.9
 */
#include "mbse_core.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    double total_power_kw, generator_capacity_kw;
    double wiring_weight_kg, apu_power_kw;
    int n_generators, n_buses, n_loads;
    double redundancy_ratio;
} ElectricalSystem;

ElectricalSystem* elec_new(int n_gen, double gen_cap, int n_loads) {
    ElectricalSystem *e = calloc(1, sizeof(ElectricalSystem));
    e->n_generators = n_gen;
    e->generator_capacity_kw = gen_cap;
    e->n_loads = n_loads;
    e->redundancy_ratio = 1.0;
    return e;
}
void elec_free(ElectricalSystem *e) { free(e); }

int elec_power_margin(ElectricalSystem *e, double *margin) {
    if (!e || !margin) return -1;
    double total_cap = e->generator_capacity_kw * (double)e->n_generators;
    *margin = (total_cap - e->total_power_kw) / total_cap;
    return 0;
}

int elec_redundancy_check(ElectricalSystem *e, double *n_plus_k) {
    if (!e || !n_plus_k) return -1;
    *n_plus_k = ((double)e->n_generators * e->generator_capacity_kw - e->total_power_kw) /
                 fmax(e->generator_capacity_kw, 1.0);
    return 0;
}

int elec_wiring_efficiency(ElectricalSystem *e, double *efficiency) {
    if (!e || !efficiency) return -1;
    *efficiency = 1.0 - 0.002 * e->wiring_weight_kg;
    if (*efficiency < 0.7) *efficiency = 0.7;
    return 0;
}

int elec_load_analysis(ElectricalSystem *e, double *avg_load, double *peak_load) {
    if (!e || !avg_load || !peak_load) return -1;
    *avg_load = e->total_power_kw / fmax((double)e->n_loads, 1.0);
    *peak_load = *avg_load * 1.35;
    return 0;
}

void elec_print_mbse_report(ElectricalSystem *e) {
    double margin, n_plus_k, efficiency, avg, peak;
    elec_power_margin(e, &margin);
    elec_redundancy_check(e, &n_plus_k);
    elec_wiring_efficiency(e, &efficiency);
    elec_load_analysis(e, &avg, &peak);
    printf("=== Boeing 787 Electrical MBSE Analysis ===\n");
    printf("Generators=%d Capacity=%.0fkW/bus Loads=%d\n",
           e->n_generators, e->generator_capacity_kw, e->n_loads);
    printf("Power Margin=%.1f%% N+K=%.1f Efficiency=%.3f\n",
           margin*100, n_plus_k, efficiency);
    printf("Avg Load=%.1fkW Peak=%.1fkW\n", avg, peak);
}
