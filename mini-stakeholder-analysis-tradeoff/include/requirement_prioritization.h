#ifndef REQUIREMENT_PRIORITIZATION_H
#define REQUIREMENT_PRIORITIZATION_H
#include "negotiation_consensus.h"

typedef enum { PRIO_MOSCOW=0,PRIO_KANO=1,PRIO_VALUE_BASED=2,PRIO_COST_OF_DELAY=3,PRIO_RISK_BASED=4,PRIO_N_METHODS=5 } PrioritizationMethod;
typedef enum { MOSCOW_MUST=0,MOSCOW_SHOULD=1,MOSCOW_COULD=2,MOSCOW_WONT=3 } MoSCoWClass;
typedef enum { KANO_BASIC=0,KANO_PERFORMANCE=1,KANO_EXCITEMENT=2,KANO_INDIFFERENT=3,KANO_REVERSE=4 } KanoClass;
typedef struct { char* name,*description; double value,cost,risk,time_criticality; int stakeholder_count; MoSCoWClass moscow; KanoClass kano; double priority_score; } Requirement;
typedef struct { Requirement** requirements; int n_requirements; int* ranking; PrioritizationMethod method; double budget_constraint; } RequirementBacklog;
Requirement* req_create(const char* name,const char* desc,double value,double cost,double risk,double time_crit);
void req_free(Requirement* r);
double req_roi(const Requirement* r);
double req_cost_of_delay(const Requirement* r,double delay);
double req_risk_adjusted_value(const Requirement* r);
MoSCoWClass req_classify_moscow(const Requirement* r,double threshold);
KanoClass req_classify_kano(double functional,double dysfunctional);
RequirementBacklog* backlog_create(void);
void backlog_free(RequirementBacklog* bl);
int backlog_add(RequirementBacklog* bl,Requirement* r);
void backlog_prioritize(RequirementBacklog* bl,PrioritizationMethod m);
void backlog_budget_constrain(RequirementBacklog* bl,double budget);
double backlog_total_value(RequirementBacklog* bl);
double backlog_total_cost(RequirementBacklog* bl);
int backlog_select_for_budget(RequirementBacklog* bl,double budget,int** selected,int* n);
void backlog_print(RequirementBacklog* bl);
#define REQ_MOSCOW_THRESHOLD 0.5
#define REQ_MAX_REQUIREMENTS 128
#endif
