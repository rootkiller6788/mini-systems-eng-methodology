#include "ssm_core.h"
#include <stdio.h>
int main(void){
    printf("=== Comparison & SSM Learning Cycle ===\n\n");
    ProblemSituation* ps=ssm_situation_create("Urban Traffic Congestion");
    ssm_situation_add_stakeholder(ps,"Commuters");ssm_situation_add_stakeholder(ps,"Residents");
    ssm_situation_add_stakeholder(ps,"Business owners");ssm_situation_add_stakeholder(ps,"City council");
    ssm_situation_add_issue(ps,"Rush hour gridlock");ssm_situation_add_issue(ps,"Air pollution");
    ssm_situation_add_issue(ps,"Public transport underfunded");
    Weltanschauung* w1=ssm_weltanschauung_create("Mobility freedom","Cities exist to move people efficiently");
    ssm_w_add_assumption(w1,"Traffic is an engineering problem");ssm_w_add_assumption(w1,"Throughput is the goal");
    Weltanschauung* w2=ssm_weltanschauung_create("Livable city","Cities exist for quality of life");
    ssm_w_add_assumption(w2,"Traffic is a land-use problem");ssm_w_add_assumption(w2,"Walkability is the goal");
    ssm_situation_add_worldview(ps,w1);ssm_situation_add_worldview(ps,w2);
    ssm_situation_complexity(ps);ssm_situation_conflict_level(ps);
    ssm_situation_print(ps);

    Comparison* comp=ssm_comparison_create("Traffic Flow Model","Urban Traffic",SSM_COMPARE_MATRIX);
    ssm_comparison_add_gap(comp,0,"No systematic data collection",0.7,false,false);
    ssm_comparison_add_gap(comp,1,"Fragmented bus routes",0.5,true,false);
    ssm_comparison_add_gap(comp,2,"No congestion pricing",0.8,false,false);
    ssm_comparison_set_recommendation(comp,0,"Install traffic sensors citywide");
    ssm_comparison_set_recommendation(comp,1,"Integrate bus networks");
    ssm_comparison_set_recommendation(comp,2,"Pilot congestion charge zone");
    ssm_comparison_compute_alignment(comp);
    ssm_comparison_print(comp);

    SSMCycle* cycle=ssm_cycle_create();
    ssm_cycle_advance(cycle);ssm_cycle_advance(cycle);ssm_cycle_advance(cycle);
    ssm_cycle_add_learning(cycle,"Two fundamentally different worldviews identified");
    ssm_cycle_add_learning(cycle,"Engineering solutions don't address livability concerns");
    ssm_cycle_advance(cycle);ssm_cycle_advance(cycle);
    printf("\nSSM Cycle State: stage=%d iterations=%d accommodation=%.2f complete=%s\n",
        cycle->current_stage,cycle->iteration_count,
        ssm_accommodation_index(cycle),
        ssm_cycle_is_complete(cycle)?"YES":"NO (needs more cycles)");

    printf("\nCheckland: SSM never really ends. Each cycle deepens understanding.\n");
    ssm_cycle_free(cycle);ssm_comparison_free(comp);ssm_situation_free(ps);
    ssm_weltanschauung_free(w1);ssm_weltanschauung_free(w2);
    return 0;
}

