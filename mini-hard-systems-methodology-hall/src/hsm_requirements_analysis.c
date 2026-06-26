#include "../include/hsm_core.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

/* Requirements Analysis — Hall's Systems Engineering Methodology (1962, 1969)
 *
 * Hall's methodology: a seven-phase iterative process.
 * Phase 1: Problem Definition — define system scope and stakeholders
 * Phase 2: Value System Design — establish criteria and tradeoffs
 * Phase 3: System Synthesis — generate alternative solutions
 * Phase 4: Systems Analysis — evaluate alternatives against criteria
 * Phase 5: Optimization — select best alternative
 * Phase 6: Decision Making — stakeholder review and selection
 * Phase 7: Planning for Action — implementation roadmap */

typedef struct {
    char*    requirement_id;
    char*    description;
    int      priority;           /* 1=critical, 2=high, 3=medium, 4=low */
    int      verification_method; /* inspection, analysis, demo, test, cert */
    double   cost_impact;
    double   schedule_impact;
    int      is_allocated;
    int      parent_spec;        /* traceability to parent requirement */
} Requirement;

typedef struct {
    Requirement*  reqs;
    int           n_reqs;
    int           capacity;
    double        total_cost;
    double        critical_path_days;
} RequirementsDatabase;

RequirementsDatabase* reqdb_create(int initial_capacity) {
    RequirementsDatabase* db = calloc(1, sizeof(RequirementsDatabase));
    if (!db) return NULL;
    db->capacity = initial_capacity > 0 ? initial_capacity : 64;
    db->reqs = calloc(db->capacity, sizeof(Requirement));
    db->n_reqs = 0;
    return db;
}

void reqdb_free(RequirementsDatabase* db) {
    if (!db) return;
    for (int i = 0; i < db->n_reqs; i++) {
        free(db->reqs[i].requirement_id);
        free(db->reqs[i].description);
    }
    free(db->reqs);
    free(db);
}

int reqdb_add(RequirementsDatabase* db, const char* id, const char* desc,
               int priority, double cost, double schedule) {
    if (!db || !id || !desc) return -1;
    if (db->n_reqs >= db->capacity) {
        db->capacity *= 2;
        db->reqs = realloc(db->reqs, db->capacity * sizeof(Requirement));
    }
    Requirement* r = &db->reqs[db->n_reqs];
    r->requirement_id = strdup(id);
    r->description = strdup(desc);
    r->priority = priority;
    r->cost_impact = cost;
    r->schedule_impact = schedule;
    r->is_allocated = 0;
    r->parent_spec = -1;
    db->n_reqs++;
    return db->n_reqs - 1;
}

int reqdb_allocate(RequirementsDatabase* db, int req_idx, int parent_idx) {
    if (!db || req_idx < 0 || req_idx >= db->n_reqs) return -1;
    db->reqs[req_idx].is_allocated = 1;
    db->reqs[req_idx].parent_spec = parent_idx;
    return 0;
}

int reqdb_count_by_priority(RequirementsDatabase* db, int priority) {
    if (!db) return 0;
    int count = 0;
    for (int i = 0; i < db->n_reqs; i++)
        if (db->reqs[i].priority == priority) count++;
    return count;
}

double reqdb_total_cost(RequirementsDatabase* db) {
    if (!db) return 0.0;
    double total = 0.0;
    for (int i = 0; i < db->n_reqs; i++)
        total += db->reqs[i].cost_impact;
    db->total_cost = total;
    return total;
}

int reqdb_verify_traceability(RequirementsDatabase* db, int* n_orphans, int* n_fully_linked) {
    if (!db || !n_orphans || !n_fully_linked) return -1;
    *n_orphans = 0;
    *n_fully_linked = 0;
    for (int i = 0; i < db->n_reqs; i++) {
        if (db->reqs[i].parent_spec < 0) (*n_orphans)++;
        else if (db->reqs[i].is_allocated) (*n_fully_linked)++;
    }
    return 0;
}

void reqdb_print_summary(RequirementsDatabase* db) {
    if (!db) return;
    printf("=== Requirements Database ===\n");
    printf("Total requirements: %d\n", db->n_reqs);
    printf("Critical (P1): %d, High (P2): %d, Medium (P3): %d, Low (P4): %d\n",
           reqdb_count_by_priority(db, 1), reqdb_count_by_priority(db, 2),
           reqdb_count_by_priority(db, 3), reqdb_count_by_priority(db, 4));
    printf("Total cost impact: %.2f\n", reqdb_total_cost(db));
}
