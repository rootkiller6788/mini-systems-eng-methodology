#ifndef CST_TSI_H
#define CST_TSI_H
#include "cst_core.h"

/* Total Systems Intervention (Flood & Jackson, 1991)
 * Meta-methodology: Creativity -> Choice -> Implementation -> Reflection */
typedef struct {
    CSTSystem* sys;
    int creativity_rounds_done, implementation_rounds_done;
    double creativity_score, choice_score, implementation_score, reflection_score;
    int n_methods_applied, n_cycles_completed;
} CSTTSI;

CSTTSI* cst_tsi_create(CSTSystem* sys);
void cst_tsi_free(CSTTSI* tsi);
void cst_tsi_run_creativity(CSTTSI* tsi);
void cst_tsi_run_choice(CSTTSI* tsi);
void cst_tsi_run_implementation(CSTTSI* tsi);
void cst_tsi_run_reflection(CSTTSI* tsi);
void cst_tsi_run_full_cycle(CSTTSI* tsi);
void cst_tsi_run_n_cycles(CSTTSI* tsi, int n);
double cst_tsi_overall_quality(const CSTTSI* tsi);
bool cst_tsi_is_converged(const CSTTSI* tsi);
void cst_tsi_print(const CSTTSI* tsi);
double cst_tsi_creativity_depth(const CSTTSI* tsi);
double cst_tsi_choice_appropriateness(const CSTTSI* tsi);
double cst_tsi_implementation_fidelity(const CSTTSI* tsi);
double cst_tsi_reflection_thoroughness(const CSTTSI* tsi);
bool cst_tsi_is_balanced(const CSTTSI* tsi);
void cst_tsi_phase_report(const CSTTSI* tsi);
int cst_tsi_weakest_phase(const CSTTSI* tsi);

#endif /* CST_TSI_H */