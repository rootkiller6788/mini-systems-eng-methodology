#ifndef SSM_COMPARISON_H
#define SSM_COMPARISON_H

#include "ssm_types.h"

/* ============================================================================
 * Comparison & Accommodation — SSM Stages 5-7
 *
 * Stage 5: Compare conceptual models with the real-world situation.
 * Stage 6: Define desirable and culturally feasible changes.
 * Stage 7: Take action to improve the situation.
 *
 * The comparison generates debate, not a "correct" answer.
 * Accommodation is the goal — a framework where conflicting
 * worldviews can coexist productively.
 *
 * Dependencies: ssm_types.h
 * Companion:    src/ssm_comparison.c
 * ============================================================================ */

/* ── Problem Situation Lifecycle ────────────────── */
ProblemSituation* ssm_situation_create(const char* name);
void              ssm_situation_free(ProblemSituation* ps);
void              ssm_situation_add_stakeholder(ProblemSituation* ps,
                                                   const char* s);
void              ssm_situation_add_issue(ProblemSituation* ps,
                                             const char* issue);
void              ssm_situation_add_structure(ProblemSituation* ps,
                                                 const char* structure);
void              ssm_situation_add_worldview(ProblemSituation* ps,
                                                 Weltanschauung* w);
double            ssm_situation_complexity(ProblemSituation* ps);
double            ssm_situation_conflict_level(ProblemSituation* ps);

/* ── Comparison Lifecycle ───────────────────────── */
Comparison* ssm_comparison_create(const char* model_name,
                                     const char* situation_name,
                                     ComparisonMethod method);
void        ssm_comparison_free(Comparison* comp);

/* ── Gap Analysis ───────────────────────────────── */
int  ssm_comparison_add_gap(Comparison* comp, int activity_id,
                               const char* description, double severity,
                               bool exists, bool works_well);
void ssm_comparison_set_recommendation(Comparison* comp, int gap_idx,
                                          const char* recommendation);
double ssm_comparison_compute_alignment(Comparison* comp);

/* ── Agenda Generation ──────────────────────────── */
void   ssm_comparison_generate_agenda(Comparison* comp, int max_items);
void   ssm_comparison_add_agenda_item(Comparison* comp, const char* item,
                                         int priority);
void   ssm_comparison_sort_agenda(Comparison* comp);
char** ssm_comparison_get_agenda(const Comparison* comp, int* n);

/* ── Accommodation ──────────────────────────────── */
double ssm_accommodation_index(SSMCycle* cycle);
double ssm_accommodation_between(Comparison* comp,
                                    const Weltanschauung* w1,
                                    const Weltanschauung* w2);
bool   ssm_accommodation_achieved(const SSMCycle* cycle);

/* ── SSM Cycle Management ───────────────────────── */
SSMCycle* ssm_cycle_create(void);
void      ssm_cycle_free(SSMCycle* cycle);
void      ssm_cycle_advance(SSMCycle* cycle);
void      ssm_cycle_add_learning(SSMCycle* cycle, const char* insight);
void      ssm_cycle_add_rootdef(SSMCycle* cycle, RootDefinition* rd);
void      ssm_cycle_add_model(SSMCycle* cycle, ConceptualModel* cm);
void      ssm_cycle_add_comparison(SSMCycle* cycle, Comparison* comp);
bool      ssm_cycle_is_complete(const SSMCycle* cycle);

/* ── Change Evaluation ──────────────────────────── */
double ssm_change_desirability(const Comparison* comp, int agenda_idx);
double ssm_change_feasibility(const Comparison* comp, int agenda_idx);
double ssm_change_priority(const Comparison* comp, int agenda_idx);

/* ── Print ──────────────────────────────────────── */
void ssm_situation_print(const ProblemSituation* ps);
void ssm_comparison_print(const Comparison* comp);
void ssm_cycle_print(const SSMCycle* cycle);

#endif /* SSM_COMPARISON_H */
