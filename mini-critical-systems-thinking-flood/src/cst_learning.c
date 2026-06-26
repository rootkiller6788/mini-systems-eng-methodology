#include "cst_learning.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

CSTLearningSystem* cst_learning_create(CSTSystem* sys){
    CSTLearningSystem* ls=calloc(1,sizeof(CSTLearningSystem));ls->sys=sys;return ls;
}
void cst_learning_free(CSTLearningSystem* ls){free(ls);}

void cst_learning_assess(CSTLearningSystem* ls){
    ls->single_loop_depth=ls->sys->iteration>0?1.0/(1.0+exp(-ls->sys->critical_depth*3)):0.3;
    ls->double_loop_depth=ls->sys->emancipatory_index*0.5+cst_metaphor_pluralism_score(ls->sys)*0.5;
    ls->triple_loop_depth=ls->sys->critical_depth*ls->sys->pluralism_index;
    ls->learning_rate=ls->sys->critical_depth/fmax(ls->sys->iteration,1);
    ls->unlearning_rate=1.0-ls->sys->awareness.boundary_openness;
    ls->n_mental_models_surfaced=ls->sys->n_metaphors;
    if(ls->single_loop_depth>ls->double_loop_depth&&ls->single_loop_depth>ls->triple_loop_depth)ls->dominant_loop=CST_LOOP_SINGLE;
    else if(ls->double_loop_depth>ls->triple_loop_depth)ls->dominant_loop=CST_LOOP_DOUBLE;
    else ls->dominant_loop=CST_LOOP_TRIPLE;
    ls->is_learning_organization=ls->learning_rate>0.3&&ls->double_loop_depth>0.4&&ls->triple_loop_depth>0.2;
}
CSTLearningLoop cst_learning_dominant(const CSTLearningSystem* ls){return ls->dominant_loop;}
double cst_learning_senge_score(const CSTLearningSystem* ls){
    /* Senge five disciplines: personal mastery, mental models, shared vision, team learning, systems thinking */
    double pm=ls->sys->critical_depth;
    double mm=ls->unlearning_rate;
    double sv=ls->sys->systemicity_score;
    double tl=cst_deliberative_quality(ls->sys);
    double st=ls->sys->systemicity_score;
    return(pm*0.15+mm*0.2+sv*0.2+tl*0.2+st*0.25);
}
bool cst_learning_is_learning_org(const CSTLearningSystem* ls){return ls->is_learning_organization;}

void cst_learning_print(const CSTLearningSystem* ls){
    printf("=== Organizational Learning ===\nDominant loop: ");
    switch(ls->dominant_loop){case CST_LOOP_SINGLE:printf("Single-loop (adaptive)\n");break;case CST_LOOP_DOUBLE:printf("Double-loop (generative)\n");break;case CST_LOOP_TRIPLE:printf("Triple-loop (transformative)\n");break;}
    printf("Single=%.3f Double=%.3f Triple=%.3f\n",ls->single_loop_depth,ls->double_loop_depth,ls->triple_loop_depth);
    printf("Learning=%.3f Unlearning=%.3f MentalModels=%d\n",ls->learning_rate,ls->unlearning_rate,ls->n_mental_models_surfaced);
    printf("SengeScore=%.3f LearningOrg=%s\n",cst_learning_senge_score(ls),ls->is_learning_organization?"YES":"NO");
}

/* Argyris & Schon organizational learning theory */
double cst_learning_defensive_routines_index(const CSTLearningSystem* ls){
    return ls->sys->awareness.boundary_openness;
}
bool cst_learning_has_defensive_routines(const CSTLearningSystem* ls){return cst_learning_defensive_routines_index(ls)>0.6;}
double cst_learning_theory_in_use_gap(const CSTLearningSystem* ls){
    return fabs(ls->sys->systemicity_score-ls->sys->critical_depth);
}

/* Kolb experiential learning cycle */
typedef enum{CST_KOLB_CONCRETE=0,CST_KOLB_REFLECTIVE=1,CST_KOLB_ABSTRACT=2,CST_KOLB_ACTIVE=3}CSTKolbStage;
double cst_learning_kolb_balance(const CSTLearningSystem* ls){
    double ce=ls->single_loop_depth, ro=ls->double_loop_depth, ac=ls->triple_loop_depth, ae=ls->learning_rate;
    return 1.0-(fabs(ce-ro)+fabs(ro-ac)+fabs(ac-ae)+fabs(ae-ce))/4.0;
}

/* Learning organization maturity (Garvin, 1993) */
typedef enum{CST_LEARN_NASCENT=0,CST_LEARN_DEVELOPING=1,CST_LEARN_ESTABLISHED=2,CST_LEARN_ADVANCED=3}CSTLearningMaturity;
CSTLearningMaturity cst_learning_maturity(const CSTLearningSystem* ls){
    double s=cst_learning_senge_score(ls);
    if(s<0.3)return CST_LEARN_NASCENT;if(s<0.5)return CST_LEARN_DEVELOPING;
    if(s<0.7)return CST_LEARN_ESTABLISHED;return CST_LEARN_ADVANCED;
}
const char* cst_learning_maturity_label(CSTLearningMaturity m){switch(m){case CST_LEARN_NASCENT:return"Nascent";case CST_LEARN_DEVELOPING:return"Developing";case CST_LEARN_ESTABLISHED:return"Established";case CST_LEARN_ADVANCED:return"Advanced";default:return"Unknown";}}

/* Knowledge management (Nonaka & Takeuchi SECI model) */
double cst_learning_socialization_index(const CSTLearningSystem* ls){return cst_deliberative_quality(ls->sys);}
double cst_learning_externalization_index(const CSTLearningSystem* ls){return ls->sys->critical_depth;}
double cst_learning_combination_index(const CSTLearningSystem* ls){return cst_methodological_diversity_index(ls->sys);}
double cst_learning_internalization_index(const CSTLearningSystem* ls){return ls->learning_rate;}
double cst_learning_seci_score(const CSTLearningSystem* ls){return(cst_learning_socialization_index(ls)+cst_learning_externalization_index(ls)+cst_learning_combination_index(ls)+cst_learning_internalization_index(ls))/4.0;}

/* Absorption capacity (Cohen & Levinthal) */
double cst_learning_absorptive_capacity(const CSTLearningSystem* ls){
    return ls->learning_rate*0.4+ls->unlearning_rate*0.3+ls->double_loop_depth*0.3;
}
bool cst_learning_can_absorb(const CSTLearningSystem* ls, double novelty){return cst_learning_absorptive_capacity(ls)>novelty;}

/* March's exploration vs exploitation tradeoff (1991) */
double cst_learning_exploration_index(const CSTLearningSystem* ls){return cst_metaphor_pluralism_score(ls->sys);}
double cst_learning_exploitation_index(const CSTLearningSystem* ls){return ls->sys->systemicity_score;}
double cst_learning_ambidexterity(const CSTLearningSystem* ls){return(cst_learning_exploration_index(ls)+cst_learning_exploitation_index(ls))/2.0;}
bool cst_learning_is_ambidextrous(const CSTLearningSystem* ls){return cst_learning_ambidexterity(ls)>0.5;}

/* Weick's sensemaking theory (1995) */
double cst_learning_sensemaking_capacity(const CSTLearningSystem* ls){return ls->double_loop_depth*0.6+ls->triple_loop_depth*0.4;}
bool cst_learning_sensemaking_breakdown(const CSTLearningSystem* ls){return cst_learning_sensemaking_capacity(ls)<0.3&&ls->sys->iteration>3;}

/* Huber's organizational learning constructs (1991) */
double cst_learning_knowledge_acquisition(const CSTLearningSystem* ls){return ls->learning_rate;}
double cst_learning_information_distribution(const CSTLearningSystem* ls){return cst_deliberative_quality(ls->sys);}
double cst_learning_information_interpretation(const CSTLearningSystem* ls){return ls->sys->critical_depth;}
double cst_learning_organizational_memory(const CSTLearningSystem* ls){return ls->sys->systemicity_score;}
double cst_learning_huber_score(const CSTLearningSystem* ls){return(cst_learning_knowledge_acquisition(ls)+cst_learning_information_distribution(ls)+cst_learning_information_interpretation(ls)+cst_learning_organizational_memory(ls))/4.0;}

/* Crossan's 4I framework (1999): Intuiting, Interpreting, Integrating, Institutionalizing */
double cst_learning_intuiting(const CSTLearningSystem* ls){return ls->triple_loop_depth;}
double cst_learning_interpreting(const CSTLearningSystem* ls){return ls->double_loop_depth;}
double cst_learning_integrating(const CSTLearningSystem* ls){return cst_deliberative_quality(ls->sys);}
double cst_learning_institutionalizing(const CSTLearningSystem* ls){return ls->single_loop_depth;}
double cst_learning_4i_score(const CSTLearningSystem* ls){return(cst_learning_intuiting(ls)+cst_learning_interpreting(ls)+cst_learning_integrating(ls)+cst_learning_institutionalizing(ls))/4.0;}

/* Learning from failure (Edmondson, 2011) */
double cst_learning_psychological_safety(const CSTLearningSystem* ls){return 1.0-ls->sys->awareness.boundary_openness;}
bool cst_learning_safe_to_fail(const CSTLearningSystem* ls){return cst_learning_psychological_safety(ls)>0.5;}
double cst_learning_failure_recovery_rate(const CSTLearningSystem* ls){return ls->learning_rate*ls->double_loop_depth;}


/* Double-loop learning assessment (Argyris & Schon, 1978).
 * Single-loop: correct errors within existing framework.
 * Double-loop: question and revise the framework itself.
 * CST requires double-loop learning for true systemic improvement. */
int cst_learning_depth(int n_interventions, const int* was_framework_changed,
                        double* depth_score) {
    if (!was_framework_changed || !depth_score || n_interventions <= 0) return -1;
    int double_loop = 0;
    for (int i = 0; i < n_interventions; i++) if (was_framework_changed[i]) double_loop++;
    *depth_score = (double)double_loop / (double)n_interventions;
    return (*depth_score > 0.5) ? 2 : 1;  /* 2=double-loop dominant, 1=single-loop */
}

/* Stakeholder learning index: fraction of stakeholders who revised their
 * initial worldview after the intervention dialogue. */
int cst_learning_impact(int n_stakeholders, const double* pre_weltanschauung,
                         const double* post_weltanschauung, double* impact_index) {
    if (!pre_weltanschauung || !post_weltanschauung || !impact_index || n_stakeholders <= 0)
        return -1;
    double total_change = 0.0;
    for (int i = 0; i < n_stakeholders; i++)
        total_change += (post_weltanschauung[i] > pre_weltanschauung[i])
            ? (post_weltanschauung[i] - pre_weltanschauung[i]) : 0.0;
    *impact_index = total_change / (double)n_stakeholders;
    return 0;
}
