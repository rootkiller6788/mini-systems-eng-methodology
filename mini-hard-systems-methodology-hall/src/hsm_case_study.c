/* hsm_app2.c -- L7-2: International Space Station requirements traceability + medical device HSM.
 *
 * Applies Hall's Systems Engineering Methodology to ISS module
 * requirements management, demonstrating traceability across
 * multiple international partners and verification levels.
 *
 * Reference: NASA SE Handbook (SP-2007-6105), Hall (1962)
 *            IEC 62304 Medical Device Software Lifecycle
 */
#include "hsm_core.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    double mass_kg, power_watts, thermal_watts;
    int n_modules, n_requirements, n_verified;
    double *req_met;
    char **module_names;
} ISSRequirements;

ISSRequirements* iss_new(int n_modules) {
    ISSRequirements *i = calloc(1, sizeof(ISSRequirements));
    i->n_modules = n_modules;
    i->req_met = calloc((size_t)n_modules, sizeof(double));
    i->module_names = calloc((size_t)n_modules, sizeof(char*));
    return i;
}
void iss_free(ISSRequirements *i) {
    if (!i) return;
    free(i->req_met);
    for (int j = 0; j < i->n_modules; j++) free(i->module_names[j]);
    free(i->module_names);
    free(i);
}

int iss_set_module(ISSRequirements *i, int idx, const char *name, double mass, double power) {
    if (!i || idx < 0 || idx >= i->n_modules || !name) return -1;
    i->module_names[idx] = strdup(name);
    i->mass_kg += mass; i->power_watts += power;
    return 0;
}

int iss_trace_coverage(ISSRequirements *i, double *coverage) {
    if (!i || !coverage) return -1;
    *coverage = (i->n_requirements > 0) ?
                (double)i->n_verified / (double)i->n_requirements : 0.0;
    return 0;
}

int iss_mass_budget(ISSRequirements *i, double budget_kg, double *margin) {
    if (!i || !margin) return -1;
    *margin = (budget_kg - i->mass_kg) / budget_kg;
    return 0;
}

int iss_power_budget(ISSRequirements *i, double budget_w, double *margin) {
    if (!i || !margin) return -1;
    *margin = (budget_w - i->power_watts) / budget_w;
    return 0;
}

void iss_print_traceability(ISSRequirements *i) {
    double coverage, mass_margin, power_margin;
    iss_trace_coverage(i, &coverage);
    iss_mass_budget(i, 450000.0, &mass_margin);
    iss_power_budget(i, 120000.0, &power_margin);
    printf("=== ISS Requirements Traceability (HSM) ===\n");
    printf("Modules=%d Reqs=%d Verified=%d Coverage=%.1f%%\n",
           i->n_modules, i->n_requirements, i->n_verified, coverage * 100);
    printf("Mass=%.0f/450000kg (%.1f%%) Power=%.0f/120000W (%.1f%%)\n",
           i->mass_kg, mass_margin * 100, i->power_watts, power_margin * 100);
}
