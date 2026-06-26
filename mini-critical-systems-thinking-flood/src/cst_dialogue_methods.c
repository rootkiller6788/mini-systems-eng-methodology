#include "../include/cst_core.h"
#include <stdlib.h>
#include <string.h>

/* Dialogue Methods — Flood & Jackson (1991) CST
 *
 * Boundary critique requires structured dialogue across stakeholder groups.
 * This module implements facilitation protocols for surfacing hidden assumptions. */

typedef struct { char* stakeholder; char* concern; int priority; int is_addressed; } DialogueItem;
typedef struct { DialogueItem* items; int n; int capacity; char* facilitator; int round; } DialogueSession;

DialogueSession* dl_create(const char* facilitator, int cap) {
    DialogueSession* d = calloc(1, sizeof(DialogueSession));
    if (!d) return NULL;
    d->facilitator = facilitator ? strdup(facilitator) : strdup("facilitator");
    d->capacity = cap > 0 ? cap : 32;
    d->items = calloc(d->capacity, sizeof(DialogueItem));
    d->n = 0; d->round = 0;
    return d;
}

void dl_free(DialogueSession* d) {
    if (!d) return;
    for (int i = 0; i < d->n; i++) { free(d->items[i].stakeholder); free(d->items[i].concern); }
    free(d->items); free(d->facilitator); free(d);
}

int dl_raise(DialogueSession* d, const char* stakeholder, const char* concern, int priority) {
    if (!d || !stakeholder || !concern) return -1;
    if (d->n >= d->capacity) { d->capacity *= 2; d->items = realloc(d->items, d->capacity*sizeof(DialogueItem)); }
    d->items[d->n].stakeholder = strdup(stakeholder);
    d->items[d->n].concern = strdup(concern);
    d->items[d->n].priority = priority;
    d->items[d->n].is_addressed = 0;
    return d->n++;
}

int dl_unaddressed_count(DialogueSession* d) {
    if (!d) return 0; int c = 0;
    for (int i = 0; i < d->n; i++) if (!d->items[i].is_addressed) c++;
    return c;
}

int dl_advance_round(DialogueSession* d) {
    if (!d) return -1;
    for (int i = 0; i < d->n; i++) if (!d->items[i].is_addressed && d->items[i].priority <= d->round+1) d->items[i].is_addressed = 1;
    d->round++; return d->round;
}
