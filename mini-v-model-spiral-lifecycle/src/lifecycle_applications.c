/* lifecycle_applications.c -- L7: F-35 Lightning II lifecycle + medical device IEC 62304.
 * Reference: GAO F-35 Reports, DoD 5000.02, IEC 62304
 */
#include "lifecycle_core.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    double dev_cost_b, sustainment_cost_b;
    double dev_years, operational_years;
    int n_blocks, n_variants, n_test_flights;
    double reliability, availability;
} AircraftProgram;

AircraftProgram* acprog_new(double dev_cost, double sus_cost) {
    AircraftProgram *a = calloc(1, sizeof(AircraftProgram));
    a->dev_cost_b = dev_cost; a->sustainment_cost_b = sus_cost;
    a->reliability = 0.9; a->availability = 0.85;
    return a;
}
void acprog_free(AircraftProgram *a) { free(a); }

int acprog_total_lifecycle_cost(AircraftProgram *a, double *tlc) {
    if (!a || !tlc) return -1;
    *tlc = a->dev_cost_b + a->sustainment_cost_b;
    return 0;
}

int acprog_cost_ratio(AircraftProgram *a, double *ratio) {
    if (!a || !ratio) return -1;
    *ratio = a->sustainment_cost_b / fmax(a->dev_cost_b, 0.01);
    return 0;
}

int acprog_operational_readiness(AircraftProgram *a, double *readiness) {
    if (!a || !readiness) return -1;
    *readiness = a->reliability * a->availability;
    return 0;
}

int acprog_test_coverage(AircraftProgram *a, double *coverage) {
    if (!a || !coverage) return -1;
    double ideal_tests = (double)a->n_variants * 1000.0;
    *coverage = (double)a->n_test_flights / fmax(ideal_tests, 1.0);
    if (*coverage > 1.0) *coverage = 1.0;
    return 0;
}

void acprog_print(AircraftProgram *a) {
    double tlc, ratio, readiness, coverage;
    acprog_total_lifecycle_cost(a, &tlc);
    acprog_cost_ratio(a, &ratio);
    acprog_operational_readiness(a, &readiness);
    acprog_test_coverage(a, &coverage);
    printf("=== F-35 Lifecycle Analysis ===\n");
    printf("Dev=$%.1fB Sustain=$%.1fB TLC=$%.1fB O/S=%.1f\n",
           a->dev_cost_b, a->sustainment_cost_b, tlc, ratio);
    printf("Blocks=%d Variants=%d TestFlights=%d Readiness=%.3f\n",
           a->n_blocks, a->n_variants, a->n_test_flights, readiness);
}
