/* cst_applications.c -- L7: Flood disaster management CST case.
 *
 * Applies Critical Systems Thinking to flood disaster management.
 * Demonstrates boundary setting, stakeholder inclusion, and
 * methodological pluralism in environmental crisis response.
 *
 * Reference: Checkland (1981), Flood & Jackson (1991)
 */
#include "cst_core.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    double flood_risk, response_time_hours;
    int n_agencies, n_affected;
    double coord_score, prep_index;
} FloodCase;

FloodCase* flood_case_new(double risk, double response, int agencies) {
    FloodCase *f = calloc(1, sizeof(FloodCase));
    f->flood_risk = risk;
    f->response_time_hours = response;
    f->n_agencies = agencies;
    f->coord_score = 0.5;
    return f;
}
void flood_case_free(FloodCase *f) { free(f); }

int flood_boundary_risk(FloodCase *f, double *exposure_risk) {
    if (!f || !exposure_risk) return -1;
    *exposure_risk = f->flood_risk * (1.0 + (double)f->n_affected * 0.01);
    return 0;
}

int flood_coordination_gap(FloodCase *f, double *gap) {
    if (!f || !gap) return -1;
    double ideal_coord = 1.0;
    if (f->n_agencies > 10) ideal_coord = 0.8;
    *gap = ideal_coord - f->coord_score;
    if (*gap < 0) *gap = 0;
    return 0;
}

int flood_response_adequacy(FloodCase *f, double *adequacy) {
    if (!f || !adequacy) return -1;
    double threshold = 24.0;
    *adequacy = (f->response_time_hours < threshold) ? 1.0 :
                (threshold / f->response_time_hours);
    return 0;
}

void flood_print_cst_analysis(FloodCase *f) {
    double risk, gap, adequacy;
    flood_boundary_risk(f, &risk);
    flood_coordination_gap(f, &gap);
    flood_response_adequacy(f, &adequacy);
    printf("=== CST Flood Disaster Analysis ===\n");
    printf("Risk=%.2f Agencies=%d Coordination=%.2f Gap=%.2f\n",
           f->flood_risk, f->n_agencies, f->coord_score, gap);
    printf("Response Time=%.1fh Adequacy=%.2f Exposure Risk=%.2f\n",
           f->response_time_hours, adequacy, risk);
    printf("CST Recommendation: %s\n",
           gap > 0.3 ? "Improve inter-agency coordination" :
           adequacy < 0.7 ? "Accelerate response capability" :
           "Boundary management adequate");
}

/* Boundary critique reflection: ensures marginalized stakeholders are heard */
int flood_stakeholder_inclusion_index(const FloodCase* f, int* n_marginalized) {
    if (!f || !n_marginalized) return -1;
    *n_marginalized = f->n_affected / 10;
    return (*n_marginalized > 0) ? 1 : 0;
}
