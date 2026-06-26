/* mbse_applications.c -- L7: NASA JPL CubeSat MBSE case study.
 *
 * Applies MBSE to CubeSat mission design following JPL practices.
 * Covers requirements modeling, block diagrams, interface definitions,
 * and parametric analysis for small satellite missions.
 *
 * Reference: NASA CubeSat Launch Initiative, JPL MBSE Handbook
 *            ISO 15288 Systems Engineering Process
 */
#include "mbse_core.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    double mass_kg, power_w, volume_u;
    double data_rate_kbps, pointing_accuracy_deg;
    int n_subsystems, n_interfaces, n_requirements;
    char *mission_name;
} CubeSat;

CubeSat* cubesat_new(const char *name, int n_subsys) {
    CubeSat *cs = calloc(1, sizeof(CubeSat));
    cs->mission_name = strdup(name);
    cs->n_subsystems = n_subsys;
    cs->volume_u = (double)n_subsys;
    return cs;
}
void cubesat_free(CubeSat *cs) {
    if (cs) { free(cs->mission_name); free(cs); }
}

int cubesat_power_margin(CubeSat *cs, double solar_panel_w, double *margin) {
    if (!cs || !margin) return -1;
    *margin = (solar_panel_w - cs->power_w) / fmax(solar_panel_w, 0.1);
    return 0;
}

int cubesat_mass_budget(CubeSat *cs, double launch_capacity_kg, double *utilization) {
    if (!cs || !utilization) return -1;
    *utilization = cs->mass_kg / fmax(launch_capacity_kg, 0.1);
    return 0;
}

int cubesat_data_link(CubeSat *cs, double *contact_time_s, double *data_per_pass) {
    if (!cs || !contact_time_s || !data_per_pass) return -1;
    double contact = 600.0;
    *contact_time_s = contact;
    *data_per_pass = cs->data_rate_kbps * contact / 8.0;
    return 0;
}

int cubesat_pointing_budget(CubeSat *cs, double *budget_ok) {
    if (!cs || !budget_ok) return -1;
    *budget_ok = (cs->pointing_accuracy_deg < 1.0) ? 1.0 : 0.5;
    return 0;
}

void cubesat_print_mbse(CubeSat *cs) {
    double margin, util, contact, data, budget;
    cubesat_power_margin(cs, 15.0, &margin);
    cubesat_mass_budget(cs, 5.0, &util);
    cubesat_data_link(cs, &contact, &data);
    cubesat_pointing_budget(cs, &budget);
    printf("=== CubeSat MBSE: %s ===\n", cs->mission_name);
    printf("Mass=%.1f/5kg (%.0f%%) Power=%.1f/15W PowerMargin=%.1f%%\n",
           cs->mass_kg, util*100, cs->power_w, margin*100);
    printf("DataRate=%.0fKbps DataPerPass=%.0fKB Pointing=%.2fdeg OK=%.0f%%\n",
           cs->data_rate_kbps, data, cs->pointing_accuracy_deg, budget*100);
    printf("Subsystems=%d Interfaces=%d Reqs=%d\n",
           cs->n_subsystems, cs->n_interfaces, cs->n_requirements);
}
