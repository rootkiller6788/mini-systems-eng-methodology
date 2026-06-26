#include "cst_boundary.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

CSTBoundaryCritique* cst_boundary_critique_create(CSTSystem* sys){CSTBoundaryCritique* bc=calloc(1,sizeof(CSTBoundaryCritique));if(!bc)return NULL;bc->sys=sys;return bc;}
void cst_boundary_critique_free(CSTBoundaryCritique* bc){free(bc);}
void cst_boundary_critique_run(CSTBoundaryCritique* bc){cst_analyze_boundaries(bc->sys);cst_identify_blind_spots(bc->sys);bc->boundary_openness=bc->sys->awareness.boundary_openness;bc->n_blind_spots=bc->sys->awareness.n_blind_spots;bc->n_contested_boundaries=0;for(int i=0;i<bc->sys->awareness.n_boundaries;i++)if(bc->sys->awareness.boundaries[i].is_contested)bc->n_contested_boundaries++;bc->marginalized_inclusion=0.0;int nm=0;for(int i=0;i<bc->sys->awareness.n_boundaries;i++)if(bc->sys->awareness.boundaries[i].type==CST_MARGINALIZED){bc->marginalized_inclusion+=bc->sys->awareness.boundaries[i].ethical_weight;nm++;}if(nm>0)bc->marginalized_inclusion/=nm;bc->stakeholder_coverage=cst_stakeholder_inclusiveness(bc->sys);bc->ethical_adequacy=bc->marginalized_inclusion*0.5+(1.0-bc->boundary_openness)*0.3+bc->stakeholder_coverage*0.2;}
void cst_boundary_sweep_in(CSTBoundaryCritique* bc,int idx){if(idx<0||idx>=bc->sys->awareness.n_boundaries)return;bc->sys->awareness.boundaries[idx].type=CST_EMPIRICAL;bc->sys->awareness.boundaries[idx].is_contested=false;}
void cst_boundary_sweep_out(CSTBoundaryCritique* bc,int idx){if(idx<0||idx>=bc->sys->awareness.n_boundaries)return;bc->sys->awareness.boundaries[idx].type=CST_MARGINALIZED;bc->sys->awareness.boundaries[idx].is_contested=true;}
double cst_boundary_marginalization_index(const CSTBoundaryCritique* bc){return bc->marginalized_inclusion;}
double cst_boundary_contestation_index(const CSTBoundaryCritique* bc){return bc->sys->awareness.n_boundaries>0?(double)bc->n_contested_boundaries/bc->sys->awareness.n_boundaries:0.0;}
int cst_boundary_count_marginalized(const CSTBoundaryCritique* bc){int cnt=0;for(int i=0;i<bc->sys->awareness.n_boundaries;i++)if(bc->sys->awareness.boundaries[i].type==CST_MARGINALIZED)cnt++;return cnt;}
double cst_boundary_power_analysis(const CSTBoundaryCritique* bc){if(bc->sys->n_stakeholders<2)return 0.0;double mp=0.0,tp=0.0;for(int i=0;i<bc->sys->n_stakeholders;i++){double p=bc->sys->stakeholders[i].power;tp+=p;if(p>mp)mp=p;}return(tp>1e-10)?mp/tp:0.0;}
double cst_boundary_ethical_gap(const CSTBoundaryCritique* bc){double st=0.0,ac=0.0;for(int i=0;i<bc->sys->awareness.n_boundaries;i++){st+=bc->sys->awareness.boundaries[i].ethical_weight;ac+=bc->sys->awareness.boundaries[i].type==CST_MARGINALIZED?bc->sys->awareness.boundaries[i].ethical_weight*0.3:bc->sys->awareness.boundaries[i].ethical_weight;}if(bc->sys->awareness.n_boundaries==0)return 0.0;return st/bc->sys->awareness.n_boundaries-ac/bc->sys->awareness.n_boundaries;}
bool cst_boundary_needs_restructuring(const CSTBoundaryCritique* bc){return bc->n_contested_boundaries>bc->sys->awareness.n_boundaries/2||bc->marginalized_inclusion<0.3||cst_boundary_ethical_gap(bc)>0.3;}
int cst_boundary_recommend_sweep_in(const CSTBoundaryCritique* bc){int best=-1;double bw=-1.0;for(int i=0;i<bc->sys->awareness.n_boundaries;i++)if(bc->sys->awareness.boundaries[i].type==CST_MARGINALIZED&&bc->sys->awareness.boundaries[i].ethical_weight>bw){bw=bc->sys->awareness.boundaries[i].ethical_weight;best=i;}return best;}
const char* cst_boundary_type_name(CSTBoundaryType t){switch(t){case CST_EMPIRICAL:return"Empirical";case CST_NORMATIVE:return"Normative";case CST_SACRED:return"Sacred";case CST_MARGINALIZED:return"Marginalized";default:return"Unknown";}}
bool cst_boundary_is_ethical(const CSTBoundaryCritique* bc){return bc->ethical_adequacy>0.6&&bc->marginalized_inclusion>0.3&&bc->boundary_openness<0.7;}

typedef struct{char* q;CSTBoundaryType cat;int n;}UQ;
static UQ uq[12]={
    {"Who is the client?",CST_EMPIRICAL,1},{"What is the purpose?",CST_EMPIRICAL,2},{"Measure of improvement?",CST_EMPIRICAL,3},
    {"Who is the decision-maker?",CST_NORMATIVE,4},{"Resources controlled?",CST_NORMATIVE,5},{"Conditions NOT controlled?",CST_NORMATIVE,6},
    {"Who is the designer?",CST_SACRED,7},{"Expertise assumed?",CST_SACRED,8},{"Guarantor of success?",CST_SACRED,9},
    {"Who represents affected?",CST_MARGINALIZED,10},{"Are affected emancipated?",CST_MARGINALIZED,11},{"Worldview underlying?",CST_MARGINALIZED,12}
};
void cst_boundary_ulrich_checklist(const CSTBoundaryCritique* bc){
    printf("=== Ulrich 12 Boundary Questions ===\n");
    int covered[12]={0},n=0;
    for(int i=0;i<bc->sys->awareness.n_boundaries;i++)
        for(int q=0;q<12;q++)if(bc->sys->awareness.boundaries[i].type==uq[q].cat)covered[q]=1;
    for(int q=0;q<12;q++){printf("  Q%2d [%c] %s\n",q+1,covered[q]?'X':' ',uq[q].q);if(covered[q])n++;}
    printf("Coverage: %d/12 (%.0f%%)\n",n,n*100.0/12.0);
}
double cst_boundary_ulrich_coverage(const CSTBoundaryCritique* bc){
    int covered[12]={0};
    for(int i=0;i<bc->sys->awareness.n_boundaries;i++)
        for(int q=0;q<12;q++)if(bc->sys->awareness.boundaries[i].type==uq[q].cat)covered[q]=1;
    int n=0;for(int q=0;q<12;q++)if(covered[q])n++;return n/12.0;
}
double cst_boundary_stakeholder_alignment(const CSTBoundaryCritique* bc,int si){
    if(si<0||si>=bc->sys->n_stakeholders)return 0.0;
    double a=0.0;int n=0;
    for(int i=0;i<bc->sys->awareness.n_boundaries;i++){a+=bc->sys->awareness.boundaries[i].ethical_weight*bc->sys->stakeholders[si].salience;n++;}
    return(n>0)?a/n:0.0;
}
int cst_boundary_most_aligned_stakeholder(const CSTBoundaryCritique* bc){int best=-1;double ba=-1.0;for(int i=0;i<bc->sys->n_stakeholders;i++){double a=cst_boundary_stakeholder_alignment(bc,i);if(a>ba){ba=a;best=i;}}return best;}
int cst_boundary_least_aligned_stakeholder(const CSTBoundaryCritique* bc){int worst=-1;double wa=1e9;for(int i=0;i<bc->sys->n_stakeholders;i++){double a=cst_boundary_stakeholder_alignment(bc,i);if(a<wa){wa=a;worst=i;}}return worst;}
bool cst_boundary_has_shifted(const CSTBoundaryCritique* bc,int idx,double th){(void)th;return idx>=0&&idx<bc->sys->awareness.n_boundaries&&bc->sys->awareness.boundaries[idx].is_contested;}
int cst_boundary_shift_count(const CSTBoundaryCritique* bc){int c=0;for(int i=0;i<bc->sys->awareness.n_boundaries;i++)if(bc->sys->awareness.boundaries[i].is_contested)c++;return c;}
double cst_boundary_evolution_rate(const CSTBoundaryCritique* bc){return bc->sys->awareness.n_boundaries>0?(double)bc->n_contested_boundaries/bc->sys->awareness.n_boundaries:0.0;}
bool cst_boundary_is_stable(const CSTBoundaryCritique* bc){return cst_boundary_evolution_rate(bc)<0.3;}
bool cst_boundary_is_evolving(const CSTBoundaryCritique* bc){double r=cst_boundary_evolution_rate(bc);return r>=0.3&&r<0.7;}
bool cst_boundary_is_turbulent(const CSTBoundaryCritique* bc){return cst_boundary_evolution_rate(bc)>=0.7;}
int cst_boundary_sweep_all_marginalized(CSTBoundaryCritique* bc){int c=0;for(int i=0;i<bc->sys->awareness.n_boundaries;i++)if(bc->sys->awareness.boundaries[i].type==CST_MARGINALIZED){cst_boundary_sweep_in(bc,i);c++;}return c;}
int cst_boundary_sweep_all_sacred(CSTBoundaryCritique* bc){int c=0;for(int i=0;i<bc->sys->awareness.n_boundaries;i++)if(bc->sys->awareness.boundaries[i].type==CST_SACRED){cst_boundary_sweep_out(bc,i);c++;}return c;}
void cst_boundary_negotiate(CSTBoundaryCritique* bc,int a,int b){if(a<0||a>=bc->sys->awareness.n_boundaries||b<0||b>=bc->sys->awareness.n_boundaries)return;double avg=(bc->sys->awareness.boundaries[a].ethical_weight+bc->sys->awareness.boundaries[b].ethical_weight)/2.0;bc->sys->awareness.boundaries[a].ethical_weight=avg;bc->sys->awareness.boundaries[b].ethical_weight=avg;}
double cst_boundary_consensus_level(const CSTBoundaryCritique* bc){if(bc->sys->awareness.n_boundaries<2)return 1.0;double sum=0.0,sumsq=0.0;for(int i=0;i<bc->sys->awareness.n_boundaries;i++){double w=bc->sys->awareness.boundaries[i].ethical_weight;sum+=w;sumsq+=w*w;}double mean=sum/bc->sys->awareness.n_boundaries;return 1.0/(1.0+(sumsq/bc->sys->awareness.n_boundaries-mean*mean));}
int cst_boundary_conflict_hotspots(const CSTBoundaryCritique* bc,int* idxs,int max_n){int n=0;for(int i=0;i<bc->sys->awareness.n_boundaries&&n<max_n;i++)if(bc->sys->awareness.boundaries[i].is_contested)idxs[n++]=i;return n;}
void cst_boundary_mediate_conflicts(CSTBoundaryCritique* bc){int hs[32];int n=cst_boundary_conflict_hotspots(bc,hs,32);for(int i=0;i<n-1;i+=2)cst_boundary_negotiate(bc,hs[i],hs[i+1]);}
void cst_boundary_record_state(CSTBoundaryCritique* bc){printf("Boundary state: %d shifts, openness=%.3f\n",cst_boundary_shift_count(bc),bc->boundary_openness);}
void cst_boundary_print(const CSTBoundaryCritique* bc){printf("=== Boundary Critique ===\nOpenness=%.3f Ethical=%.3f MargIncl=%.3f Contested=%d Blind=%d\n",bc->boundary_openness,bc->ethical_adequacy,bc->marginalized_inclusion,bc->n_contested_boundaries,bc->n_blind_spots);}
bool cst_boundary_resolution_possible(const CSTBoundaryCritique* bc){return cst_boundary_consensus_level(bc)>0.3&&cst_boundary_shift_count(bc)<=bc->sys->awareness.n_boundaries/2;}

/* Boundary evolution classification */
CSTBoundaryState cst_boundary_classify_state(const CSTBoundaryCritique* bc){double r=cst_boundary_evolution_rate(bc);if(r<0.3)return CST_BOUNDARY_STABLE;if(r<0.7)return CST_BOUNDARY_EVOLVING;return CST_BOUNDARY_TURBULENT;}
const char* cst_boundary_state_label(CSTBoundaryState s){switch(s){case CST_BOUNDARY_STABLE:return"Stable";case CST_BOUNDARY_EVOLVING:return"Evolving";case CST_BOUNDARY_TURBULENT:return"Turbulent";default:return"Unknown";}}

/* Midgley systemic intervention boundary methods */
double cst_boundary_midgley_coefficient(const CSTBoundaryCritique* bc){
    double b=1.0-bc->boundary_openness;double e=bc->ethical_adequacy;double m=bc->marginalized_inclusion;
    return b*0.4+e*0.3+m*0.3;
}
int cst_boundary_identify_systemic_boundaries(const CSTBoundaryCritique* bc, int* indices, int max_n){
    int n=0;
    for(int i=0;i<bc->sys->awareness.n_boundaries&&n<max_n;i++)if(bc->sys->awareness.boundaries[i].ethical_weight>0.5)indices[n++]=i;
    return n;
}

/* Multiple negotiation strategies */
void cst_boundary_negotiate_strategy(CSTBoundaryCritique* bc, int a, int b, CSTNegotiationStrategy strategy){
    if(a<0||a>=bc->sys->awareness.n_boundaries||b<0||b>=bc->sys->awareness.n_boundaries)return;
    double wa=bc->sys->awareness.boundaries[a].ethical_weight,wb=bc->sys->awareness.boundaries[b].ethical_weight;
    double result;
    switch(strategy){
        case CST_NEGOTIATE_AVERAGE: result=(wa+wb)/2.0;break;
        case CST_NEGOTIATE_MAX: result=(wa>wb)?wa:wb;break;
        case CST_NEGOTIATE_MIN: result=(wa<wb)?wa:wb;break;
        case CST_NEGOTIATE_WEIGHTED: result=wa*0.7+wb*0.3;break;
        default:result=wa;break;
    }
    bc->sys->awareness.boundaries[a].ethical_weight=result;bc->sys->awareness.boundaries[b].ethical_weight=result;
}

/* Boundary spanning: connecting across system boundaries */
typedef struct{int internal;int external;double permeability;double trust_level;}BoundarySpan;
void cst_boundary_span_analyze(const CSTBoundaryCritique* bc, BoundarySpan* spans, int max_n){
    int n=bc->sys->awareness.n_boundaries;if(n>max_n)n=max_n;
    for(int i=0;i<n;i++){spans[i].internal=i;spans[i].external=(i+1)%bc->sys->awareness.n_boundaries;spans[i].permeability=bc->sys->awareness.boundaries[i].ethical_weight;spans[i].trust_level=1.0-bc->sys->awareness.boundaries[i].is_contested?0.7:0.3;}
}
double cst_boundary_span_strength(const CSTBoundaryCritique* bc){
    double sum=0.0;for(int i=0;i<bc->sys->awareness.n_boundaries;i++)sum+=bc->sys->awareness.boundaries[i].ethical_weight;
    return(bc->sys->awareness.n_boundaries>0)?sum/bc->sys->awareness.n_boundaries:0.0;
}

/* Boundary object analysis (Star & Griesemer, 1989) */
bool cst_boundary_is_boundary_object(const CSTBoundaryCritique* bc, int idx){
    if(idx<0||idx>=bc->sys->awareness.n_boundaries)return false;
    CSTBoundary* b=&bc->sys->awareness.boundaries[idx];
    return b->ethical_weight>0.4&&b->ethical_weight<0.8&&!b->is_contested;
}
int cst_boundary_count_boundary_objects(const CSTBoundaryCritique* bc){
    int c=0;for(int i=0;i<bc->sys->awareness.n_boundaries;i++)if(cst_boundary_is_boundary_object(bc,i))c++;return c;
}

/* Boundary critique self-assessment */
double cst_boundary_self_assessment(const CSTBoundaryCritique* bc){
    double u=cst_boundary_ulrich_coverage(bc),m=cst_boundary_midgley_coefficient(bc);
    double e=bc->ethical_adequacy,s=cst_boundary_consensus_level(bc);
    return(u*0.25+m*0.25+e*0.25+s*0.25);
}
