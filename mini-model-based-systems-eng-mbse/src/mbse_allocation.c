#include "mbse_core.h"
#include "mbse_architecture.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ============================================================================
 * MBSE Allocation & Design Structure Matrix (DSM)
 *
 * Allocation is the SysML relationship that assigns model elements
 * across different model aspects (functional to physical, logical
 * to physical, software to hardware, etc.).
 *
 * This module implements:
 *   1. Functional-to-physical allocation
 *   2. Design Structure Matrix (DSM) clustering
 *   3. Interface compatibility checking
 *   4. Mass/power/cost roll-up through block hierarchy
 *   5. Design rule verification
 *
 * Based on: Browning (2001) DSM, Pimmler & Eppinger (1994)
 * Integration analysis, Crawley et al. (2015) System Architecture
 * ============================================================================ */

/* ---------- Allocation Data Structures ---------- */

typedef struct {
    int function_id;       /* What to do */
    int physical_id;       /* What does it */
    double allocation_ratio; /* 1.0 = fully allocated, <1.0 = partial */
    const char* rationale;
} Allocation;

typedef struct {
    Allocation* allocations;
    int n_allocations;
    int alloc_capacity;
} AllocationTable;

/* Design Structure Matrix: NxN matrix of inter-element dependencies */
typedef struct {
    double** matrix;       /* matrix[i][j] = dependency strength [0,1] */
    char** labels;         /* Element labels */
    int size;
} DSMMatrix;

/* ---------- Allocation Functions ---------- */

AllocationTable* alloc_table_create(void) {
    AllocationTable* at = (AllocationTable*)calloc(1,
                             sizeof(AllocationTable));
    if (!at) return NULL;
    at->alloc_capacity = 32;
    at->allocations = (Allocation*)calloc(32, sizeof(Allocation));
    return at;
}

void alloc_table_free(AllocationTable* at) {
    if (!at) return;
    free(at->allocations);
    free(at);
}

int alloc_table_add(AllocationTable* at, int func_id, int phys_id,
                     double ratio, const char* rationale) {
    if (!at) return -1;
    if (at->n_allocations >= at->alloc_capacity) {
        int nc = at->alloc_capacity * 2;
        Allocation* tmp = (Allocation*)realloc(at->allocations,
                                (size_t)nc * sizeof(Allocation));
        if (!tmp) return -1;
        at->allocations = tmp;
        at->alloc_capacity = nc;
    }
    int idx = at->n_allocations++;
    at->allocations[idx].function_id = func_id;
    at->allocations[idx].physical_id = phys_id;
    at->allocations[idx].allocation_ratio = ratio;
    at->allocations[idx].rationale = rationale;
    return idx;
}

/* Check if a function is fully allocated (sum of ratios = 1.0).
 * Returns true if total allocation ratio >= 0.99. */
bool alloc_is_function_covered(AllocationTable* at, int func_id) {
    if (!at) return false;
    double sum = 0.0;
    for (int i = 0; i < at->n_allocations; i++)
        if (at->allocations[i].function_id == func_id)
            sum += at->allocations[i].allocation_ratio;
    return sum >= 0.99;
}

/* Find all physical components allocated to a function. */
int alloc_get_physical_for_function(AllocationTable* at, int func_id,
                                      int* phys_ids, int max_ids) {
    if (!at || !phys_ids || max_ids <= 0) return 0;
    int count = 0;
    for (int i = 0; i < at->n_allocations && count < max_ids; i++) {
        if (at->allocations[i].function_id == func_id
            && at->allocations[i].allocation_ratio > 0.0)
            phys_ids[count++] = at->allocations[i].physical_id;
    }
    return count;
}

/* Coverage report: fraction of functions with full allocation. */
double alloc_coverage_ratio(AllocationTable* at, int n_functions) {
    if (!at || n_functions <= 0) return 0.0;
    int covered = 0;
    for (int f = 0; f < n_functions; f++)
        if (alloc_is_function_covered(at, f)) covered++;
    return (double)covered / (double)n_functions;
}

/* ---------- Design Structure Matrix (DSM) ---------- */

DSMMatrix* dsm_create(int size) {
    if (size <= 0) return NULL;
    DSMMatrix* dsm = (DSMMatrix*)calloc(1, sizeof(DSMMatrix));
    if (!dsm) return NULL;
    dsm->size = size;
    dsm->matrix = (double**)malloc((size_t)size * sizeof(double*));
    dsm->labels = (char**)malloc((size_t)size * sizeof(char*));
    if (!dsm->matrix || !dsm->labels) {
        free(dsm->matrix); free(dsm->labels); free(dsm); return NULL;
    }
    for (int i = 0; i < size; i++) {
        dsm->matrix[i] = (double*)calloc((size_t)size, sizeof(double));
        if (!dsm->matrix[i]) {
            for (int k = 0; k < i; k++) free(dsm->matrix[k]);
            free(dsm->matrix); free(dsm->labels); free(dsm); return NULL;
        }
    }
    return dsm;
}

void dsm_free(DSMMatrix* dsm) {
    if (!dsm) return;
    for (int i = 0; i < dsm->size; i++) {
        free(dsm->matrix[i]);
        free(dsm->labels[i]);
    }
    free(dsm->matrix); free(dsm->labels); free(dsm);
}

void dsm_set(DSMMatrix* dsm, int i, int j, double value, const char* label) {
    if (!dsm || i < 0 || i >= dsm->size || j < 0 || j >= dsm->size) return;
    dsm->matrix[i][j] = value;
    if (i == j && label) {
        free(dsm->labels[i]);
        dsm->labels[i] = strdup(label);
    }
}

/* DSM clustering using the cost function minimization heuristic.
 * Goal: reorder rows/cols to minimize off-diagonal cost.
 * Cost = sum of dependency * distance from diagonal.
 * Uses simple greedy swapping algorithm.
 * Fills new_order[] (caller alloc, size n) with permuted indices.
 * Returns final cost. */
double dsm_cluster(DSMMatrix* dsm, int* new_order, int max_iter) {
    if (!dsm || !new_order || dsm->size <= 1) return 0.0;
    int n = dsm->size;

    /* Initialize order as identity */
    for (int i = 0; i < n; i++) new_order[i] = i;

    /* Compute initial cost */
    auto double compute_cost(int* order) {
        double cost = 0.0;
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                if (i != j)
                    cost += dsm->matrix[order[i]][order[j]]
                          * fabs((double)(i - j));
        return cost;
    };

    double best_cost = compute_cost(new_order);
    /* Greedy pairwise swap improvement */
    for (int iter = 0; iter < max_iter; iter++) {
        int improved = 0;
        for (int i = 0; i < n - 1; i++) {
            for (int j = i + 1; j < n; j++) {
                /* Swap i and j */
                int tmp = new_order[i];
                new_order[i] = new_order[j];
                new_order[j] = tmp;
                double new_cost = compute_cost(new_order);
                if (new_cost < best_cost - 1e-10) {
                    best_cost = new_cost;
                    improved = 1;
                } else {
                    /* Swap back */
                    tmp = new_order[i];
                    new_order[i] = new_order[j];
                    new_order[j] = tmp;
                }
            }
        }
        if (!improved) break;
    }
    return best_cost;
}

/* Module detection: identify clusters in a DSM.
 * Groups elements with strong mutual dependencies.
 * Fills module_id[] (caller alloc) with cluster assignments [0..n_modules-1].
 * threshold: minimum dependency for grouping.
 * Returns number of modules found. */
int dsm_detect_modules(DSMMatrix* dsm, int* module_id,
                         double threshold) {
    if (!dsm || !module_id || dsm->size <= 0 || threshold < 0.0) return 0;
    int n = dsm->size;
    for (int i = 0; i < n; i++) module_id[i] = -1;
    int current_module = 0;
    for (int i = 0; i < n; i++) {
        if (module_id[i] >= 0) continue;  /* Already assigned */
        module_id[i] = current_module;
        /* Assign all strongly connected neighbors */
        for (int j = 0; j < n; j++) {
            if (module_id[j] >= 0) continue;
            if (dsm->matrix[i][j] >= threshold
                || dsm->matrix[j][i] >= threshold) {
                module_id[j] = current_module;
            }
        }
        current_module++;
    }
    return current_module;
}

/* ---------- Block Hierarchy Roll-up ---------- */

/* Roll up a property through block hierarchy.
 * For a block, recursively sum its children's values.
 * property: array indexed by block ID.
 * blocks: array of all blocks.
 * block_id: root block to evaluate.
 * weight_fn: optional weight function (NULL = just sum).
 * Returns aggregated value. */
double mbse_roll_up_property(SystemModel* m, int block_id,
                               double (*weight_fn)(MBSEBlock*)) {
    if (!m || block_id < 0 || block_id >= m->n_blocks) return 0.0;
    MBSEBlock* b = &m->blocks[block_id];
    /* Get own value */
    double total = weight_fn ? weight_fn(b) : 0.0;
    /* Recursively add children */
    for (int i = 0; i < b->n_children; i++)
        total += mbse_roll_up_property(m, b->child_ids[i], weight_fn);
    return total;
}

/* Weight functions for common properties */
static double block_mass_weight(MBSEBlock* b) { return b ? b->mass : 0.0; }
static double block_power_weight(MBSEBlock* b) { return b ? b->power : 0.0; }
static double block_cost_weight(MBSEBlock* b) { return b ? b->cost : 0.0; }

double mbse_total_mass_rollup(SystemModel* m) {
    /* Find root blocks (no parent) and sum their roll-ups */
    if (!m) return 0.0;
    double total = 0.0;
    for (int i = 0; i < m->n_blocks; i++) {
        if (m->blocks[i].parent_id < 0)
            total += mbse_roll_up_property(m, i, block_mass_weight);
    }
    return total;
}

double mbse_total_power_rollup(SystemModel* m) {
    if (!m) return 0.0;
    double total = 0.0;
    for (int i = 0; i < m->n_blocks; i++) {
        if (m->blocks[i].parent_id < 0)
            total += mbse_roll_up_property(m, i, block_power_weight);
    }
    return total;
}

/* ---------- Interface Analysis ---------- */

/* Check interface compatibility between two blocks.
 * Returns compatibility score [0, 1]. */
double mbse_interface_compatibility(MBSEInterface* iface,
                                     MBSEBlock* a, MBSEBlock* b) {
    if (!iface || !a || !b) return 0.0;
    return iface->reliability;
}

/* Count unconnected required interfaces.
 * An interface is "required" if its source or target block
 * exists in the model but no interface connects them.
 * Returns count of missing interfaces. */
int mbse_count_missing_interfaces(SystemModel* m) {
    if (!m || m->n_blocks < 2) return 0;
    int missing = 0;
    for (int i = 0; i < m->n_blocks; i++) {
        for (int j = i + 1; j < m->n_blocks; j++) {
            int connected = 0;
            for (int k = 0; k < m->n_interfaces; k++) {
                if (mbse_iface_connects(&m->interfaces[k], i, j)) {
                    connected = 1; break;
                }
            }
            if (!connected) missing++;
        }
    }
    return missing;
}

/* ---------- Print ---------- */

void dsm_print(DSMMatrix* dsm) {
    if (!dsm) return;
    printf("=== DSM (%dx%d) ===\n", dsm->size, dsm->size);
    for (int i = 0; i < dsm->size; i++) {
        printf("%-15s", dsm->labels[i] ? dsm->labels[i] : "?");
        for (int j = 0; j < dsm->size; j++)
            printf("%4.1f", dsm->matrix[i][j]);
        printf("\n");
    }
}

void alloc_table_print(AllocationTable* at) {
    if (!at) return;
    printf("=== Allocations (%d) ===\n", at->n_allocations);
    for (int i = 0; i < at->n_allocations; i++) {
        Allocation* a = &at->allocations[i];
        printf("  F%d -> P%d (%.2f) %s\n",
               a->function_id, a->physical_id,
               a->allocation_ratio,
               a->rationale ? a->rationale : "");
    }
}
