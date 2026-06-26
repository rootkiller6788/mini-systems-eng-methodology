#ifndef SSM_CATWOE_H
#define SSM_CATWOE_H

#include "ssm_types.h"

/* ============================================================================
 * CATWOE Analysis — Customers, Actors, Transformation, Worldview, Owners, Environment
 *
 * CATWOE is the formal mnemonic for constructing root definitions
 * in Soft Systems Methodology. Each element must be explicitly
 * addressed to ensure a well-formulated definition.
 *
 * Dependencies: ssm_types.h
 * Companion:    src/ssm_catwoe.c
 * ============================================================================ */

/* ── CATWOE Lifecycle ──────────────────────────── */
CATWOE* ssm_catwoe_create(void);
void    ssm_catwoe_free(CATWOE* c);
CATWOE* ssm_catwoe_clone(const CATWOE* c);

/* ── Component Setters ──────────────────────────── */
void ssm_catwoe_set_customer(CATWOE* c, const char* cust);
void ssm_catwoe_add_customer(CATWOE* c, const char* cust);
void ssm_catwoe_set_actor(CATWOE* c, const char* actor);
void ssm_catwoe_add_actor(CATWOE* c, const char* actor);
void ssm_catwoe_set_transformation(CATWOE* c, const char* input,
                                     const char* output, TransformationType t);
void ssm_catwoe_set_worldview(CATWOE* c, const char* wv);
void ssm_catwoe_set_owner(CATWOE* c, const char* owner);
void ssm_catwoe_add_owner(CATWOE* c, const char* owner);
void ssm_catwoe_add_constraint(CATWOE* c, const char* constraint);
void ssm_catwoe_set_efficacy(CATWOE* c, double val);
void ssm_catwoe_set_efficiency(CATWOE* c, double val);
void ssm_catwoe_set_effectiveness(CATWOE* c, double val);

/* ── Analysis ───────────────────────────────────── */
double ssm_catwoe_completeness(const CATWOE* c);
double ssm_catwoe_consistency(const CATWOE* c);
double ssm_catwoe_measurability_score(const CATWOE* c);
bool   ssm_catwoe_is_well_formulated(const CATWOE* c);

/* ── Worldview Operations ───────────────────────── */
Weltanschauung* ssm_weltanschauung_create(const char* name,
                                             const char* description);
void            ssm_weltanschauung_free(Weltanschauung* w);
void            ssm_w_add_assumption(Weltanschauung* w, const char* a);
void            ssm_w_add_implication(Weltanschauung* w, const char* imp);
double          ssm_w_conflict_score(const Weltanschauung* w1,
                                        const Weltanschauung* w2);
double          ssm_w_accommodation_potential(const Weltanschauung* w1,
                                                 const Weltanschauung* w2);
void            ssm_w_print(const Weltanschauung* w);

/* ── Print ──────────────────────────────────────── */
void ssm_catwoe_print(const CATWOE* c);

#endif /* SSM_CATWOE_H */
