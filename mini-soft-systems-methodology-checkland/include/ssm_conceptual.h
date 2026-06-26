#ifndef SSM_CONCEPTUAL_H
#define SSM_CONCEPTUAL_H

#include "ssm_types.h"

/* ============================================================================
 * Conceptual Models — Activity Systems and Logical Dependencies
 *
 * Stage 4 of SSM: build conceptual models of the human activity
 * systems named in the root definitions. These are NOT models of
 * the real world — they are models of what the system MUST DO
 * to BE the system named in the root definition.
 *
 * Models are built from the minimum necessary activities (verbs),
 * structured by logical dependency.
 *
 * Dependencies: ssm_types.h
 * Companion:    src/ssm_conceptual.c
 * ============================================================================ */

/* ── Conceptual Model Lifecycle ─────────────────── */
ConceptualModel* ssm_conceptual_create(const char* rd_text);
void             ssm_conceptual_free(ConceptualModel* cm);

/* ── Activity Management ────────────────────────── */
int  ssm_conceptual_add_activity(ConceptualModel* cm, const char* name,
                                    const char* description);
ConceptualActivity* ssm_conceptual_get_activity(ConceptualModel* cm,
                                                   int activity_id);
void ssm_conceptual_add_dependency(ConceptualModel* cm, int from_id,
                                      int to_id);
int  ssm_conceptual_remove_activity(ConceptualModel* cm, int id);

/* ── Activity Configuration ────────────────────── */
void ssm_activity_set_level(ConceptualActivity* act, int level);
void ssm_activity_add_indicator(ConceptualActivity* act,
                                   const char* indicator, double target);
void ssm_activity_set_monitoring(ConceptualActivity* act, double freq);

/* ── Model Analysis ─────────────────────────────── */
int    ssm_conceptual_count_levels(ConceptualModel* cm);
double ssm_conceptual_coverage(ConceptualModel* cm);
double ssm_conceptual_complexity(ConceptualModel* cm);
double ssm_conceptual_connectivity(const ConceptualModel* cm);
bool   ssm_conceptual_is_connected(const ConceptualModel* cm);
int*   ssm_conceptual_topological_sort(ConceptualModel* cm, int* n);

/* Cycle detection in dependency graph.
 * Returns number of cycles found; *cycle_members gets IDs.
 * Returns 0 if acyclic (well-formed model). */
int    ssm_conceptual_find_cycles(ConceptualModel* cm,
                                     int** cycle_members, int* n_cycles);

/* ── Model Validation ───────────────────────────── */
bool   ssm_conceptual_validate_3e(const ConceptualModel* cm,
                                     double eff_min, double effi_min,
                                     double effe_min);
bool   ssm_conceptual_is_complete(const ConceptualModel* cm);
double ssm_conceptual_monitorability(const ConceptualModel* cm);

/* ── Print ──────────────────────────────────────── */
void ssm_conceptual_print(const ConceptualModel* cm);
void ssm_conceptual_print_activity(const ConceptualActivity* act);

#endif /* SSM_CONCEPTUAL_H */
