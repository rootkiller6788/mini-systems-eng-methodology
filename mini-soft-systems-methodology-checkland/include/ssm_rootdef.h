#ifndef SSM_ROOTDEF_H
#define SSM_ROOTDEF_H

#include "ssm_types.h"

/* ============================================================================
 * Root Definitions — PQR Formulation and 5E Analysis
 *
 * A root definition is the core of SSM Stage 3. It states what the
 * system IS in a concise, tightly constructed sentence:
 *   "A system to do P by Q in order to achieve R"
 *
 * Dependencies: ssm_types.h
 * Companion:    src/ssm_rootdef.c
 * ============================================================================ */

/* ── Lifecycle ──────────────────────────────────── */
RootDefinition* ssm_rootdef_create(const char* name, RootDefType type);
void            ssm_rootdef_free(RootDefinition* rd);
RootDefinition* ssm_rootdef_clone(const RootDefinition* rd);

/* ── PQR Formulation ────────────────────────────── */
void ssm_rootdef_set_pqr(RootDefinition* rd, const char* what,
                          const char* how, const char* why);

/* Build root definition text from PQR components.
 * Formats: "A system to [P] by [Q] in order to achieve [R]"
 * The full text is stored in rd->root_definition. */
void ssm_rootdef_synthesize(RootDefinition* rd);

/* Parse an existing root definition text into PQR components.
 * Uses NLP-like heuristic: looks for "to [P] by [Q]" pattern. */
void ssm_rootdef_parse(RootDefinition* rd, const char* text);

/* ── CATWOE Association ─────────────────────────── */
void ssm_rootdef_set_catwoe(RootDefinition* rd, CATWOE* catwoe);
double ssm_rootdef_catwoe_fit(const RootDefinition* rd);

/* ── 5E Analysis ────────────────────────────────── */
void   ssm_rootdef_set_efficacy(RootDefinition* rd, double val);
void   ssm_rootdef_set_efficiency(RootDefinition* rd, double val);
void   ssm_rootdef_set_effectiveness(RootDefinition* rd, double val);
void   ssm_rootdef_set_ethicality(RootDefinition* rd, double val);
void   ssm_rootdef_set_elegance(RootDefinition* rd, double val);
double ssm_rootdef_5e_composite(const RootDefinition* rd);
bool   ssm_rootdef_is_viable(const RootDefinition* rd);

/* ── Coherence & Consistency ────────────────────── */
double ssm_rootdef_internal_coherence(const RootDefinition* rd);
bool   ssm_rootdef_monitorable(const RootDefinition* rd);

/* ── Multi-RD Operations ────────────────────────── */
double ssm_rootdef_pair_conflict(const RootDefinition* a,
                                    const RootDefinition* b);
void   ssm_rootdef_rank_by_relevance(RootDefinition** rds, int n);
int    ssm_rootdef_select_primary(RootDefinition** rds, int n);

/* ── Print ──────────────────────────────────────── */
void ssm_rootdef_print(const RootDefinition* rd);

#endif /* SSM_ROOTDEF_H */
