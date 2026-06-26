/* sos_app4.c -- L7-3: Autonomous vehicle platooning as SoS (SAE J3016 Level 4).
 * Reference: SAE J3016, VDMA AV Platooning Standards
 */
#include "sos_types.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    double headway_s, speed_mps, fuel_saving_pct;
    int n_vehicles, platoon_size;
    double comm_latency_ms, safety_margin;
} PlatoonSoS;

PlatoonSoS* platoon_new(int n_vehicles, double speed, double headway) {
    PlatoonSoS *p = calloc(1, sizeof(PlatoonSoS));
    p->n_vehicles = n_vehicles;
    p->speed_mps = speed;
    p->headway_s = headway;
    p->safety_margin = 0.9;
    return p;
}
void platoon_free(PlatoonSoS *p) { free(p); }

int platoon_fuel_saving(PlatoonSoS *p, double *saving_pct) {
    if (!p || !saving_pct) return -1;
    double ideal_saving = 0.15;
    *saving_pct = ideal_saving * (1.0 - exp(-0.3 * (double)p->n_vehicles));
    return 0;
}

int platoon_safety_index(PlatoonSoS *p, double *safety) {
    if (!p || !safety) return -1;
    double min_headway = 0.5;
    *safety = p->headway_s / fmax(min_headway, 0.1);
    if (*safety > 1.0) *safety = 1.0;
    return 0;
}

int platoon_communication_reliability(PlatoonSoS *p, double *reliability) {
    if (!p || !reliability) return -1;
    *reliability = exp(-p->comm_latency_ms / 100.0);
    return 0;
}

int platoon_throughput(PlatoonSoS *p, double *throughput_veh_per_h) {
    if (!p || !throughput_veh_per_h) return -1;
    double gap = p->headway_s * p->speed_mps;
    *throughput_veh_per_h = 3600.0 * p->speed_mps / fmax(gap, 1.0);
    return 0;
}

void platoon_print(PlatoonSoS *p) {
    double saving, safety, reliability, throughput;
    platoon_fuel_saving(p, &saving);
    platoon_safety_index(p, &safety);
    platoon_communication_reliability(p, &reliability);
    platoon_throughput(p, &throughput);
    printf("=== AV Platooning SoS (SAE L4) ===\n");
    printf("Vehicles=%d Speed=%.1fm/s Headway=%.2fs\n",
           p->n_vehicles, p->speed_mps, p->headway_s);
    printf("FuelSaving=%.1f%% Safety=%.2f CommRel=%.3f Throughput=%.0f veh/h\n",
           saving*100, safety, reliability, throughput);
}
