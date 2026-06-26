/* sos_app2.c -- L7-2: European air traffic management SoS (SESAR) + autonomous vehicle platooning.
 * Reference: SESAR Joint Undertaking, SAE J3016 Taxonomy
 */
#include "sos_types.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    double flights_per_day, avg_delay_min, capacity_util;
    int n_centers, n_airports, n_aircraft;
    double coordination_index;
} AirTrafficSoS;

AirTrafficSoS* atm_new(double flights, int centers, int airports) {
    AirTrafficSoS *a = calloc(1, sizeof(AirTrafficSoS));
    a->flights_per_day = flights;
    a->n_centers = centers;
    a->n_airports = airports;
    a->coordination_index = 0.5;
    return a;
}
void atm_free(AirTrafficSoS *a) { free(a); }

int atm_capacity_margin(AirTrafficSoS *a, double max_capacity, double *margin) {
    if (!a || !margin) return -1;
    *margin = 1.0 - a->flights_per_day / fmax(max_capacity, 1.0);
    return 0;
}

int atm_delay_index(AirTrafficSoS *a, double *index) {
    if (!a || !index) return -1;
    *index = a->avg_delay_min / 15.0;
    if (*index > 1.0) *index = 1.0;
    return 0;
}

int atm_coordination_quality(AirTrafficSoS *a, double *quality) {
    if (!a || !quality) return -1;
    double interop = a->coordination_index * (double)a->n_centers / 10.0;
    *quality = interop > 1.0 ? 1.0 : interop;
    return 0;
}

int atm_seamless_handover(AirTrafficSoS *a, double *handover_success) {
    if (!a || !handover_success) return -1;
    *handover_success = 0.85 + 0.15 * a->coordination_index;
    return 0;
}

void atm_print_sesar(AirTrafficSoS *a) {
    double margin, delay, coord, handover;
    atm_capacity_margin(a, 30000.0, &margin);
    atm_delay_index(a, &delay);
    atm_coordination_quality(a, &coord);
    atm_seamless_handover(a, &handover);
    printf("=== SESAR Air Traffic Management SoS ===\n");
    printf("Flights/day=%.0f Centers=%d Airports=%d\n",
           a->flights_per_day, a->n_centers, a->n_airports);
    printf("CapacityMargin=%.1f%% DelayIndex=%.2f CoordQuality=%.2f Handover=%.3f\n",
           margin*100, delay, coord, handover);
}
