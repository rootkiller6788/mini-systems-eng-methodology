#include "ssm_core.h"
#include <stdio.h>
int main(void){
    printf("=== CATWOE Analysis: Hospital Waiting Times ===\n\n");
    CATWOE* c=ssm_catwoe_create();
    ssm_catwoe_set_customer(c,"Patients waiting for treatment");
    ssm_catwoe_add_customer(c,"Referring GPs");
    ssm_catwoe_set_actor(c,"Hospital staff (nurses, doctors, admin)");
    ssm_catwoe_add_actor(c,"IT support team");
    ssm_catwoe_set_transformation(c,"Patient on waiting list","Patient treated and discharged",SSM_TRANSFORM_CONCRETE);
    ssm_catwoe_set_worldview(c,"Timely healthcare is a right; waiting lists are a system failure");
    ssm_catwoe_set_owner(c,"Hospital Trust Board");
    ssm_catwoe_add_owner(c,"Department of Health");
    ssm_catwoe_add_constraint(c,"Annual budget cycle limits resource flexibility");
    ssm_catwoe_add_constraint(c,"Clinical staff shortage (national)");
    ssm_catwoe_add_constraint(c,"Legal maximum 18-week waiting time");
    ssm_catwoe_set_efficacy(c,0.75); ssm_catwoe_set_efficiency(c,0.6);
    ssm_catwoe_set_effectiveness(c,0.8);

    ssm_catwoe_print(c);
    printf("\nCompleteness: %.2f  Consistency: %.2f  Well-formulated: %s\n",
        ssm_catwoe_completeness(c),ssm_catwoe_consistency(c),
        ssm_catwoe_is_well_formulated(c)?"YES":"NO");

    Weltanschauung* w1=ssm_weltanschauung_create("Clinical efficiency","Healthcare is about clinical outcomes");
    ssm_w_add_assumption(w1,"Waiting lists are a capacity problem");
    ssm_w_add_implication(w1,"More resources = shorter waits");

    Weltanschauung* w2=ssm_weltanschauung_create("Patient experience","Healthcare is about quality of life");
    ssm_w_add_assumption(w2,"Waiting lists cause anxiety and deterioration");
    ssm_w_add_implication(w2,"Communication matters as much as speed");

    printf("\nWorldview conflict: %.2f  Accommodation potential: %.2f\n",
        ssm_w_conflict_score(w1,w2),ssm_w_accommodation_potential(w1,w2));

    ssm_weltanschauung_free(w1);ssm_weltanschauung_free(w2);ssm_catwoe_free(c);
    return 0;
}

