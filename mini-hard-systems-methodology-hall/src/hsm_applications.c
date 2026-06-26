/* hsm_applications.c -- L7: NASA Apollo LM requirements + Boeing 787 metrics.
 *
 * Applies HSM to two historic systems engineering cases:
 * - Apollo Lunar Module: 1960s SE practices with Hall''s morphology
 * - Boeing 787: modern global supply chain SE challenges
 *
 * Reference: Hall (1962), NASA Apollo Experience Reports
 *            Boeing 787 Lessons Learned (GAO-11-380)
 */
#include "hsm_core.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    double weight_kg, power_w, volume_m3;
    double cost_m, reliability, dev_time_months;
} SpaceReq;

SpaceReq* sr_new(double w, double p, double v, double c) {
    SpaceReq *s = calloc(1, sizeof(SpaceReq));
    s->weight_kg = w; s->power_w = p;
    s->volume_m3 = v; s->cost_m = c;
    s->reliability = 0.99;
    return s;
}
void sr_free(SpaceReq *s) { free(s); }

int apollo_trade_score(SpaceReq *r, double *score) {
    if (!r || !score) return -1;
    double w_norm = 1.0 - r->weight_kg / 5000.0;
    double p_norm = 1.0 - r->power_w / 2000.0;
    double c_norm = 1.0 - r->cost_m / 500.0;
    *score = 0.4 * w_norm + 0.2 * p_norm + 0.1 * r->reliability + 0.3 * c_norm;
    return 0;
}

typedef struct {
    double dev_cost_b, dev_time_months;
    int n_suppliers, change_orders;
    double delay_months, overrun_percent;
} AircraftProg;

AircraftProg* ac_new(double cost_b, double time_m, int suppliers) {
    AircraftProg *p = calloc(1, sizeof(AircraftProg));
    p->dev_cost_b = cost_b; p->dev_time_months = time_m;
    p->n_suppliers = suppliers;
    return p;
}
void ac_free(AircraftProg *p) { free(p); }

int b787_cost_overrun(AircraftProg *p, double *overrun_pct) {
    if (!p || !overrun_pct) return -1;
    *overrun_pct = (p->dev_cost_b - 5.0) / 5.0 * 100.0;
    return 0;
}

int b787_supplier_risk(AircraftProg *p, double *risk_score) {
    if (!p || !risk_score) return -1;
    *risk_score = 1.0 - exp(-0.02 * (double)p->n_suppliers);
    return 0;
}

int b787_schedule_performance(AircraftProg *p, double *spi) {
    if (!p || !spi) return -1;
    *spi = p->dev_time_months / (p->dev_time_months + p->delay_months);
    return 0;
}

void print_apollo_analysis(SpaceReq *r) {
    double score;
    apollo_trade_score(r, &score);
    printf("=== Apollo LM Trade Study (HSM) ===\n");
    printf("Weight=%.0fkg Power=%.0fW Cost=$%.0fM Rel=%.3f\n",
           r->weight_kg, r->power_w, r->cost_m, r->reliability);
    printf("Trade Score=%.4f\n", score);
}
