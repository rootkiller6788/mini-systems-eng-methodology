#include "ssm_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
int main(void){
printf("=== SSM Test Suite ===\n");
CATWOE* c=ssm_catwoe_create(); assert(c!=NULL);
ssm_catwoe_set_customer(c,"Patients"); assert(c->customers);
ssm_catwoe_set_actor(c,"Nurses"); assert(c->actors);
Weltanschauung* w1=ssm_weltanschauung_create("Clinical","Medical"); assert(w1!=NULL);
ssm_w_add_assumption(w1,"Test"); assert(w1->n_assumptions==1);
RootDefinition* rd=ssm_rootdef_create("Healthcare",SSM_RD_PRIMARY_TASK); assert(rd!=NULL);
ssm_rootdef_set_pqr(rd,"what","how","why"); assert(rd->what_to_do);
ConceptualModel* cm=ssm_conceptual_create("CM"); assert(cm!=NULL);
ssm_conceptual_add_activity(cm,"act","desc"); assert(cm->n_activities==1);
ProblemSituation* ps=ssm_situation_create("Sit"); assert(ps!=NULL);
ssm_situation_add_stakeholder(ps,"stk1"); assert(ps->n_stakeholders==1);
Comparison* comp=ssm_comparison_create("cmp","sit",SSM_COMPARE_MATRIX); assert(comp!=NULL);
SSMCycle* cycle=ssm_cycle_create(); assert(cycle!=NULL);
ssm_cycle_advance(cycle); assert(cycle->current_stage==SSM_STAGE_2_EXPRESSION);
ssm_catwoe_free(c); ssm_weltanschauung_free(w1); ssm_rootdef_free(rd);
ssm_conceptual_free(cm); ssm_situation_free(ps); ssm_comparison_free(comp); ssm_cycle_free(cycle);
printf("\n=== All asserts passed ===\n");
return 0;
}
