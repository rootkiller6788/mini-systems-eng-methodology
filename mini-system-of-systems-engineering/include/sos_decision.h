#ifndef SOS_DECISION_H
#define SOS_DECISION_H
#include "sos_types.h"
#include <stdbool.h>
/* Multi-criteria decision analysis for SoS architecture selection.
   Supports weighted scoring, sensitivity, feasibility, and robustness. */
typedef struct { char* option_name; double cost,benefit,risk,time_to_implement; int* affected_systems; int n_affected; double score; } SoSDecisionOption;
typedef struct { SoSDecisionOption* options; int n_options,cap; double* weights; int n_weights; } SoSDecisionMatrix;
SoSDecisionOption* sos_decision_option_create(const char* n,double c,double b,double r);
void sos_decision_option_free(SoSDecisionOption* o);
SoSDecisionOption* sos_decision_clone_option(const SoSDecisionOption* src);
void sos_decision_option_add_affected(SoSDecisionOption* o,int id);
bool sos_decision_option_affects(const SoSDecisionOption* o,int id);
void sos_decision_option_set_time(SoSDecisionOption* o,double t);
SoSDecisionMatrix* sos_decision_matrix_create(void);
void sos_decision_matrix_free(SoSDecisionMatrix* dm);
int sos_decision_add_option(SoSDecisionMatrix* dm,SoSDecisionOption* o);
void sos_decision_evaluate(SoSDecisionMatrix* dm);
int sos_decision_best_option(const SoSDecisionMatrix* dm);
int sos_decision_count_feasible(const SoSDecisionMatrix* dm,double mc,double mr);
void sos_decision_rank_options(SoSDecisionMatrix* dm,int* ranking);
void sos_decision_set_weight(SoSDecisionMatrix* dm,int i,double w);
double sos_decision_sensitivity(SoSDecisionMatrix* dm,int o,int w,double d);
double sos_decision_robustness(const SoSDecisionMatrix* dm,int o);
double sos_decision_expected_value(const SoSDecisionOption* o,double p);
void sos_decision_print(const SoSDecisionMatrix* dm);
#endif