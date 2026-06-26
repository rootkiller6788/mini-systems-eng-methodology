/* cst_app2.c -- L7-2: COVID-19 pandemic response CST analysis + organizational boundary critique.
 *
 * Applies Critical Systems Thinking to pandemic governance across
 * organizational boundaries. Demonstrates TSI methodology with
 * boundary critique for public health decision-making.
 *
 * Reference: Flood & Jackson (1991) Creative Problem Solving
 *            Churchman (1979) The Systems Approach and Its Enemies
 */
#include "cst_core.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    double cases_per_million, deaths_per_million;
    double vacc_rate, response_time_days;
    int lockdown_days, n_countries;
    double *response_times;
} COVIDCase;

COVIDCase* covid_new(int n) {
    COVIDCase *c = calloc(1, sizeof(COVIDCase));
    c->n_countries = n;
    c->response_times = calloc((size_t)n, sizeof(double));
    return c;
}
void covid_free(COVIDCase *c) {
    if (c) { free(c->response_times); free(c); }
}

int covid_response_analysis(COVIDCase *c, double *avg_response, double *variance) {
    if (!c || !avg_response || !variance || c->n_countries < 1) return -1;
    double sum = 0.0;
    for (int i = 0; i < c->n_countries; i++) sum += c->response_times[i];
    *avg_response = sum / (double)c->n_countries;
    *variance = 0.0;
    for (int i = 0; i < c->n_countries; i++) {
        double d = c->response_times[i] - *avg_response;
        *variance += d * d;
    }
    *variance /= (double)c->n_countries;
    return 0;
}

typedef struct {
    char *sector, *boundary_name;
    double openness, coordination;
    int n_stakeholders;
} PandemicBoundary;

PandemicBoundary* pb_create(const char *name, const char *sector) {
    PandemicBoundary *pb = calloc(1, sizeof(PandemicBoundary));
    pb->boundary_name = strdup(name ? name : "");
    pb->sector = strdup(sector ? sector : "");
    pb->openness = 0.5; pb->coordination = 0.5;
    return pb;
}
void pb_free(PandemicBoundary *pb) {
    if (pb) { free(pb->boundary_name); free(pb->sector); free(pb); }
}

/* Boundary critique score: lower = more problematic boundaries */
double pb_critique_score(PandemicBoundary *b, int n) {
    if (!b || n < 2) return NAN;
    double sum_open = 0.0, sum_coord = 0.0;
    for (int i = 0; i < n; i++) {
        sum_open += b[i].openness;
        sum_coord += b[i].coordination;
    }
    double avg_coord = sum_coord / (double)n;
    double frag = 0.0;
    double avg_open = sum_open / (double)n;
    for (int i = 0; i < n; i++) {
        double d = b[i].openness - avg_open;
        frag += d * d;
    }
    return avg_coord * (1.0 - sqrt(frag / (double)n));
}

void pb_print_analysis(PandemicBoundary *b, int n) {
    printf("=== CST Pandemic Governance Boundary Critique ===\n");
    for (int i = 0; i < n; i++)
        printf("  [%s] open=%.2f coord=%.2f\n", b[i].sector, b[i].openness, b[i].coordination);
    printf("Critique Score: %.4f (<0.3=fragmented >0.6=integrated)\n", pb_critique_score(b, n));
}
