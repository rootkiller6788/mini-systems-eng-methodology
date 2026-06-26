#ifndef SSM_INTERVENTION_H
#define SSM_INTERVENTION_H
#include "ssm_types.h"

typedef enum { SSM_CHANGE_STRUCTURAL=0, SSM_CHANGE_PROCEDURAL=1, SSM_CHANGE_ATTITUDINAL=2, SSM_CHANGE_SYSTEMIC=3 } ChangeType;
typedef struct { int id; char* description; ChangeType type; double desirability; double feasibility; double sustainability; char** stakeholders_affected; int n_affected; double implementation_time; double risk_level; } SystemicChange;
typedef struct { char* name; SystemicChange** changes; int n_changes; double overall_desirability; double overall_feasibility; double implementation_priority; } InterventionPlan;

InterventionPlan* ssm_intervention_create(const char* name);
void ssm_intervention_free(InterventionPlan* ip);
int ssm_intervention_add_change(InterventionPlan* ip, const char* desc, ChangeType type, double desirability, double feasibility);
void ssm_intervention_add_affected_stakeholder(InterventionPlan* ip, int change_id, const char* stakeholder);
double ssm_intervention_compute_priority(InterventionPlan* ip);
void ssm_intervention_sort_by_priority(InterventionPlan* ip);
double ssm_intervention_implementation_risk(InterventionPlan* ip);
void ssm_intervention_print(const InterventionPlan* ip);
bool ssm_intervention_is_actionable(const InterventionPlan* ip);
double ssm_intervention_expected_impact(const InterventionPlan* ip);
void ssm_intervention_generate_roadmap(const InterventionPlan* ip, int* ordering, int* n_steps);

#endif
