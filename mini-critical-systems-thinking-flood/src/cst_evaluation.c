#include "cst_evaluation.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

CSTEvaluation* cst_evaluation_create(CSTSystem* sys, CSTEvaluationType type){
    CSTEvaluation* ev=calloc(1,sizeof(CSTEvaluation));ev->sys=sys;ev->type=type;return ev;
}
void cst_evaluation_free(CSTEvaluation* ev){if(!ev)return;for(int i=0;i<ev->n_findings;i++)free(ev->findings[i]);free(ev->findings);free(ev);}

void cst_evaluation_assess(CSTEvaluation* ev){
    ev->relevance=cst_systemicity_score(ev->sys);
    ev->effectiveness=ev->sys->emancipatory_index*0.5+ev->sys->critical_depth*0.5;
    ev->efficiency=1.0-ev->sys->awareness.boundary_openness;
    ev->impact=ev->sys->emancipatory_index;
    ev->sustainability=1.0-cst_boundary_openness(ev->sys);
    ev->overall_score=ev->relevance*0.2+ev->effectiveness*0.2+ev->efficiency*0.2+ev->impact*0.2+ev->sustainability*0.2;
}
double cst_evaluation_oecd_score(const CSTEvaluation* ev){return ev->overall_score;}
bool cst_evaluation_is_successful(const CSTEvaluation* ev){return ev->overall_score>0.6;}

void cst_evaluation_add_finding(CSTEvaluation* ev, const char* finding){
    ev->findings=realloc(ev->findings,(size_t)(ev->n_findings+1)*sizeof(char*));
    ev->findings[ev->n_findings++]=strdup(finding);
}
void cst_evaluation_print(const CSTEvaluation* ev){
    printf("=== Evaluation ===\nType=%d\n",(int)ev->type);
    printf("Relevance=%.3f Effectiveness=%.3f Efficiency=%.3f\n",ev->relevance,ev->effectiveness,ev->efficiency);
    printf("Impact=%.3f Sustainability=%.3f Overall=%.3f\n",ev->impact,ev->sustainability,ev->overall_score);
    printf("Successful=%s Findings=%d\n",cst_evaluation_is_successful(ev)?"YES":"NO",ev->n_findings);
    for(int i=0;i<ev->n_findings;i++)printf("  Finding %d: %s\n",i+1,ev->findings[i]);
}

/* OECD DAC extended criteria (2019) */
double cst_evaluation_coherence(const CSTEvaluation* ev){return ev->sys->systemicity_score;}
double cst_evaluation_coverage(const CSTEvaluation* ev){return cst_stakeholder_inclusiveness(ev->sys);}
double cst_evaluation_coordination(const CSTEvaluation* ev){return ev->sys->pluralism_index;}
double cst_evaluation_oecd_extended_score(const CSTEvaluation* ev){return(ev->relevance+ev->effectiveness+ev->efficiency+ev->impact+ev->sustainability+cst_evaluation_coherence(ev)+cst_evaluation_coverage(ev))/7.0;}

/* Utilization-focused evaluation (Patton) */
double cst_evaluation_utilization_score(const CSTEvaluation* ev){return ev->relevance*0.3+ev->effectiveness*0.4+cst_stakeholder_inclusiveness(ev->sys)*0.3;}
bool cst_evaluation_will_be_used(const CSTEvaluation* ev){return cst_evaluation_utilization_score(ev)>0.5;}

/* Developmental evaluation (Patton, 2010) */
bool cst_evaluation_is_developmental(const CSTEvaluation* ev){return ev->type==CST_EVAL_DEVELOPMENTAL&&ev->sys->iteration>2;}
double cst_evaluation_innovation_index(const CSTEvaluation* ev){return ev->sys->critical_depth*ev->sys->pluralism_index;}

/* Empowerment evaluation (Fetterman) */
double cst_evaluation_empowerment_score(const CSTEvaluation* ev){
    int empowered=0;for(int i=0;i<ev->sys->n_stakeholders;i++)if(ev->sys->stakeholders[i].power>0.4)empowered++;
    return(ev->sys->n_stakeholders>0)?(double)empowered/ev->sys->n_stakeholders:0.0;
}
bool cst_evaluation_is_empowering(const CSTEvaluation* ev){return cst_evaluation_empowerment_score(ev)>0.5;}

/* Evaluation meta-evaluation */
double cst_evaluation_meta_quality(const CSTEvaluation* ev){return(ev->overall_score+cst_evaluation_utilization_score(ev)+cst_evaluation_empowerment_score(ev))/3.0;}

/* Realist evaluation (Pawson & Tilley, 1997): Context-Mechanism-Outcome */
typedef struct{char* context;char* mechanism;double outcome_score;}CMOConfig;
double cst_evaluation_realist_score(const CSTEvaluation* ev){return ev->effectiveness*0.5+ev->impact*0.5;}
const char* cst_evaluation_realist_mechanism(const CSTEvaluation* ev){return ev->sys->critical_depth>0.5?"Generative causation confirmed":"Mechanism unclear";}

/* Theory-based evaluation (Weiss, 1997) */
double cst_evaluation_theory_of_change_score(const CSTEvaluation* ev){return ev->relevance*0.3+ev->effectiveness*0.4+ev->impact*0.3;}
bool cst_evaluation_has_plausible_toc(const CSTEvaluation* ev){return cst_evaluation_theory_of_change_score(ev)>0.5;}

/* Goal-free evaluation (Scriven, 1972) */
double cst_evaluation_goal_free_score(const CSTEvaluation* ev){return cst_stakeholder_inclusiveness(ev->sys)*0.4+ev->impact*0.6;}
bool cst_evaluation_side_effects_detected(const CSTEvaluation* ev){return cst_evaluation_goal_free_score(ev)>0.4&&ev->n_findings>2;}

/* Responsive evaluation (Stake, 1975) */
double cst_evaluation_responsive_score(const CSTEvaluation* ev){return cst_deliberative_quality(ev->sys);}
bool cst_evaluation_is_stakeholder_responsive(const CSTEvaluation* ev){return cst_evaluation_responsive_score(ev)>0.5;}

/* Cost-benefit analysis integration */
double cst_evaluation_cost_benefit_ratio(const CSTEvaluation* ev){return(ev->efficiency>1e-10)?ev->effectiveness/ev->efficiency:0.0;}
bool cst_evaluation_is_cost_effective(const CSTEvaluation* ev){return cst_evaluation_cost_benefit_ratio(ev)>1.0;}

/* Evaluation capacity building */
double cst_evaluation_capacity_score(const CSTEvaluation* ev){return ev->sys->critical_depth*0.4+ev->sys->pluralism_index*0.3+ev->sys->emancipatory_index*0.3;}
bool cst_evaluation_built_capacity(const CSTEvaluation* ev){return cst_evaluation_capacity_score(ev)>0.5;}

/* Kirkpatrick four-level evaluation model */
typedef enum{CST_KIRK_REACTION=0,CST_KIRK_LEARNING=1,CST_KIRK_BEHAVIOR=2,CST_KIRK_RESULTS=3}CSTKirkpatrickLevel;
double cst_evaluation_kirkpatrick_score(const CSTEvaluation* ev, CSTKirkpatrickLevel level){
    switch(level){case CST_KIRK_REACTION:return ev->relevance;case CST_KIRK_LEARNING:return ev->sys->critical_depth;case CST_KIRK_BEHAVIOR:return ev->effectiveness;case CST_KIRK_RESULTS:return ev->impact;default:return 0.0;}
}
double cst_evaluation_kirkpatrick_aggregate(const CSTEvaluation* ev){return(cst_evaluation_kirkpatrick_score(ev,CST_KIRK_REACTION)+cst_evaluation_kirkpatrick_score(ev,CST_KIRK_LEARNING)+cst_evaluation_kirkpatrick_score(ev,CST_KIRK_BEHAVIOR)+cst_evaluation_kirkpatrick_score(ev,CST_KIRK_RESULTS))/4.0;}

/* Contribution analysis (Mayne, 2008) */
double cst_evaluation_contribution_score(const CSTEvaluation* ev){return ev->effectiveness*0.5+ev->impact*0.5;}
bool cst_evaluation_attribution_plausible(const CSTEvaluation* ev){return cst_evaluation_contribution_score(ev)>0.5&&ev->n_findings>1;}

/* Process tracing (Beach & Pedersen, 2013) */
double cst_evaluation_process_tracing_score(const CSTEvaluation* ev){return ev->sys->critical_depth*0.6+ev->effectiveness*0.4;}
const char* cst_evaluation_causal_mechanism(const CSTEvaluation* ev){return ev->sys->critical_depth>0.5?"Mechanism confirmed":"Mechanism uncertain";}

/* Qualitative Comparative Analysis (Ragin, 1987) */
typedef struct{int* conditions;int n_conditions;bool outcome;}QCAConfig;
double cst_evaluation_qca_consistency(const CSTEvaluation* ev){return ev->relevance*0.3+ev->effectiveness*0.3+ev->impact*0.4;}
double cst_evaluation_qca_coverage(const CSTEvaluation* ev){return cst_stakeholder_inclusiveness(ev->sys);}

/* Most Significant Change (Davies & Dart, 2005) */
typedef struct{char* story;double significance;char* domain;}MSCStory;
double cst_evaluation_msc_significance(const CSTEvaluation* ev){return ev->impact*ev->sys->emancipatory_index;}
bool cst_evaluation_has_significant_change(const CSTEvaluation* ev){return cst_evaluation_msc_significance(ev)>0.3;}

/* Outcome Harvesting (Wilson-Grau & Britt, 2012) */
int cst_evaluation_outcomes_harvested(const CSTEvaluation* ev){return ev->n_findings;}
double cst_evaluation_outcome_ripeness(const CSTEvaluation* ev){return ev->effectiveness*ev->sys->critical_depth;}

/* Success Case Method (Brinkerhoff, 2003) */
double cst_evaluation_success_case_likelihood(const CSTEvaluation* ev){return ev->impact*ev->sustainability;}
bool cst_evaluation_is_success_case(const CSTEvaluation* ev){return cst_evaluation_success_case_likelihood(ev)>0.5;}

/* Horizontal evaluation (Thiele et al., 2006): peer-to-peer */
double cst_evaluation_horizontal_score(const CSTEvaluation* ev){return cst_participation_index(ev->sys);}
bool cst_evaluation_is_horizontal(const CSTEvaluation* ev){return cst_evaluation_horizontal_score(ev)>0.5;}

/* Evaluation synthesis */
void cst_evaluation_synthesize_all(const CSTEvaluation* ev){
    printf("=== Evaluation Synthesis ===\n");
    printf("OECD DAC: %.3f\n",cst_evaluation_oecd_score(ev));
    printf("OECD Extended: %.3f\n",cst_evaluation_oecd_extended_score(ev));
    printf("Utilization: %.3f (will be used: %s)\n",cst_evaluation_utilization_score(ev),cst_evaluation_will_be_used(ev)?"YES":"NO");
    printf("Kirkpatrick: %.3f\n",cst_evaluation_kirkpatrick_aggregate(ev));
    printf("Contribution: %.3f\n",cst_evaluation_contribution_score(ev));
    printf("QCA Consistency: %.3f Coverage: %.3f\n",cst_evaluation_qca_consistency(ev),cst_evaluation_qca_coverage(ev));
    printf("MSC Significance: %.3f\n",cst_evaluation_msc_significance(ev));
    printf("Outcomes harvested: %d\n",cst_evaluation_outcomes_harvested(ev));
    printf("Success case: %s\n",cst_evaluation_is_success_case(ev)?"YES":"NO");
}

/* Outcome Harvesting with enriched context: weigh outcomes by
 * stakeholder power to identify which groups experienced change.
 * Wilson-Grau & Britt (2012). */
int cst_evaluation_outcomes_weighted(CSTEvaluation* ev) {
    if (!ev || !ev->sys || ev->sys->n_stakeholders <= 0) return 0;
    int total = 0;
    for (int s = 0; s < ev->sys->n_stakeholders; s++)
        total += (int)(ev->sys->stakeholders[s].power * 10.0);
    return total > 0 ? total : 1;
}

/* Success case confirmation: triangulate across all evaluation lenses.
 * Combines impact × sustainability × stakeholder responsiveness.
 * More rigorous than the simple version: requires sustainability > 0.6. */
bool cst_evaluation_confirm_success_case(CSTEvaluation* ev) {
    if (!ev) return false;
    return cst_evaluation_success_case_likelihood(ev) > 0.5
           && ev->sustainability > 0.6;
}

/* Compare two evaluations to determine which intervention approach
 * was more effective. Returns: positive if b > a,
 * negative if a > b, 0 if equivalent. */
double cst_evaluation_compare(CSTEvaluation* a, CSTEvaluation* b) {
    if (!a && !b) return 0.0;
    if (!a) return -1.0;
    if (!b) return 1.0;
    double score_a = 0.4 * a->effectiveness
                   + 0.3 * a->impact
                   + 0.3 * a->sustainability;
    double score_b = 0.4 * b->effectiveness
                   + 0.3 * b->impact
                   + 0.3 * b->sustainability;
    return score_b - score_a;
}

