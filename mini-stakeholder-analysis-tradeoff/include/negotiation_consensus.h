#ifndef NEGOTIATION_CONSENSUS_H
#define NEGOTIATION_CONSENSUS_H
#include "multi_criteria_decision.h"

typedef struct { Stakeholder* stakeholder; double* position,*reservation,*aspiration,*batna_value; int n_issues; double concession_rate; } Negotiator;
typedef struct { Negotiator** parties; int n_parties,n_issues; char** issue_names; double* zone_of_agreement,*nash_solution; double nash_product,consensus_index; int rounds; bool agreement_reached; } Negotiation;
Negotiator* neg_create(Stakeholder* s,int n_issues,double* position,double* reservation);
void neg_free(Negotiator* n);
double neg_utility(const Negotiator* n,double* proposal);
Negotiation* neg_session_create(int n_issues,const char** issue_names);
void neg_session_free(Negotiation* neg);
int neg_add_party(Negotiation* neg,Negotiator* party);
void neg_compute_nash_solution(Negotiation* neg);
double neg_consensus_index(Negotiation* neg);
bool neg_is_pareto_improving(Negotiation* neg,double* proposal);
void neg_run_negotiation(Negotiation* neg,int max_rounds);
void neg_print_status(Negotiation* neg);
double cons_kemeny_young(double** rankings,int n_voters,int n_alt,int* ranking);
double cons_borda_count(double** rankings,int n_voters,int n_alt,double* scores);
double cons_dissent_index(double** votes,int n_voters,int n_issues);
#define NEG_MAX_ROUNDS 100
#define NEG_CONVERGENCE_TOL 1e-4
#define NEG_MAX_PARTIES 16
#define NEG_MAX_ISSUES 10
#endif
