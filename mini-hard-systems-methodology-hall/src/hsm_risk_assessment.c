#include "../include/hsm_core.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Risk Assessment — Hall Phase 4: Systems Analysis
 *
 * Technical risk, schedule risk, and cost risk assessment for system alternatives.
 * Risk = Probability × Impact (standard SE risk model). */

typedef struct { char* id; char* description; double probability; double impact; int is_mitigated; } RiskItem;
typedef struct { RiskItem* items; int n; int capacity; double total_exposure; } RiskRegister;

RiskRegister* rr_create(int cap) {
    RiskRegister* rr = calloc(1, sizeof(RiskRegister));
    if (!rr) return NULL;
    rr->capacity = cap > 0 ? cap : 64;
    rr->items = calloc(rr->capacity, sizeof(RiskItem));
    return rr;
}

void rr_free(RiskRegister* rr) {
    if (!rr) return;
    for (int i = 0; i < rr->n; i++) { free(rr->items[i].id); free(rr->items[i].description); }
    free(rr->items); free(rr);
}

int rr_add(RiskRegister* rr, const char* id, const char* desc, double prob, double impact) {
    if (!rr || !id || !desc) return -1;
    if (rr->n >= rr->capacity) { rr->capacity *= 2; rr->items = realloc(rr->items, rr->capacity*sizeof(RiskItem)); }
    rr->items[rr->n].id = strdup(id);
    rr->items[rr->n].description = strdup(desc);
    rr->items[rr->n].probability = prob;
    rr->items[rr->n].impact = impact;
    rr->items[rr->n].is_mitigated = 0;
    rr->total_exposure += prob * impact;
    return rr->n++;
}

double rr_mitigate(RiskRegister* rr, int idx, double reduction) {
    if (!rr || idx < 0 || idx >= rr->n) return -1.0;
    RiskItem* ri = &rr->items[idx];
    double old_exp = ri->probability * ri->impact;
    ri->probability *= (1.0 - reduction);
    if (ri->probability < 0.0) ri->probability = 0.0;
    ri->is_mitigated = 1;
    double new_exp = ri->probability * ri->impact;
    rr->total_exposure += (new_exp - old_exp);
    return rr->total_exposure;
}
