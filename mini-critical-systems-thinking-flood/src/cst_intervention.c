#include "cst_intervention.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

CSTInterventionPortfolio* cst_intervention_create(CSTSystem* sys) {
    CSTInterventionPortfolio* ip = calloc(1, sizeof(CSTInterventionPortfolio));
    ip->sys = sys; return ip;
}
void cst_intervention_free(CSTInterventionPortfolio* ip) {
    if (!ip) return;
    for (int i = 0; i < ip->n_interventions; i++) {
        free(ip->interventions[i].name); free(ip->interventions[i].description);
        for (int j = 0; j < ip->interventions[i].n_participants; j++) free(ip->interventions[i].participants[j]);
        free(ip->interventions[i].participants);
        for (int j = 0; j < ip->interventions[i].n_methods; j++) free(ip->interventions[i].methods_used[j]);
        free(ip->interventions[i].methods_used);
    }
    free(ip->interventions); free(ip);
}
int cst_intervention_add(CSTInterventionPortfolio* ip, const char* name, const char* desc) {
    ip->interventions = realloc(ip->interventions, (size_t)(ip->n_interventions + 1) * sizeof(CSTIntervention));
    int i = ip->n_interventions++; memset(&ip->interventions[i], 0, sizeof(CSTIntervention));
    ip->interventions[i].name = strdup(name); ip->interventions[i].description = strdup(desc);
    ip->interventions[i].phase = CST_INTERV_DIAGNOSIS; ip->interventions[i].success_prob = 0.5;
    ip->interventions[i].ethical_score = 0.5; return i;
}
void cst_intervention_add_participant(CSTInterventionPortfolio* ip, int idx, const char* name) {
    if (idx < 0 || idx >= ip->n_interventions) return;
    CSTIntervention* in = &ip->interventions[idx];
    in->participants = realloc(in->participants, (size_t)(in->n_participants + 1) * sizeof(char*));
    in->participants[in->n_participants++] = strdup(name);
}
void cst_intervention_add_method(CSTInterventionPortfolio* ip, int idx, const char* method) {
    if (idx < 0 || idx >= ip->n_interventions) return;
    CSTIntervention* in = &ip->interventions[idx];
    in->methods_used = realloc(in->methods_used, (size_t)(in->n_methods + 1) * sizeof(char*));
    in->methods_used[in->n_methods++] = strdup(method);
}
void cst_intervention_advance(CSTInterventionPortfolio* ip, int idx) {
    if (idx < 0 || idx >= ip->n_interventions) return;
    CSTIntervention* in = &ip->interventions[idx];
    if (in->phase < CST_INTERV_ITERATE) in->phase = (CSTInterventionPhase)((int)in->phase + 1);
    else { in->phase = CST_INTERV_DIAGNOSIS; in->iteration_count++; }
    in->progress = (double)in->phase / 4.0;
}
void cst_intervention_evaluate(CSTInterventionPortfolio* ip, int idx) {
    if (idx < 0 || idx >= ip->n_interventions) return;
    CSTIntervention* in = &ip->interventions[idx];
    in->success_prob = 0.0; int ns = ip->sys->n_stakeholders;
    for (int i = 0; i < ns; i++) if (!ip->sys->stakeholders[i].is_marginalized) in->success_prob += ip->sys->stakeholders[i].salience;
    if (ns > 0) in->success_prob /= ns;
    if (in->n_participants >= 3 && in->n_methods >= 2) in->success_prob *= 1.2;
    if (in->success_prob > 1.0) in->success_prob = 1.0;
    in->ethical_score = ip->sys->emancipatory_index * 0.4 + (1.0 - ip->sys->awareness.boundary_openness) * 0.3 +
                        (double)in->n_participants / fmax(ip->sys->n_stakeholders, 1) * 0.3;
}
double cst_intervention_ethical_score(const CSTInterventionPortfolio* ip) {
    if (ip->n_interventions == 0) return 0.0; double sum = 0.0;
    for (int i = 0; i < ip->n_interventions; i++) { cst_intervention_evaluate((CSTInterventionPortfolio*)ip, i); sum += ip->interventions[i].ethical_score; }
    return sum / ip->n_interventions;
}
double cst_intervention_midgley_index(const CSTInterventionPortfolio* ip) {
    return ip->sys->awareness.critical_awareness_index * 0.4 + cst_intervention_ethical_score(ip) * 0.3 + ip->sys->pluralism_index * 0.3;
}
bool cst_intervention_is_systemic(const CSTInterventionPortfolio* ip, int idx) {
    if (idx < 0 || idx >= ip->n_interventions) return false;
    return ip->interventions[idx].n_methods >= 2 && ip->interventions[idx].n_participants >= 3;
}
int cst_intervention_best_candidate(const CSTInterventionPortfolio* ip) {
    int best = -1; double best_score = -1.0;
    for (int i = 0; i < ip->n_interventions; i++) {
        cst_intervention_evaluate((CSTInterventionPortfolio*)ip, i);
        double s = ip->interventions[i].success_prob * 0.6 + ip->interventions[i].ethical_score * 0.4;
        if (s > best_score) { best_score = s; best = i; }
    }
    return best;
}
double cst_intervention_success_rate(const CSTInterventionPortfolio* ip) { if(ip->n_interventions==0)return 0.0; double s=0.0; for(int i=0;i<ip->n_interventions;i++)s+=ip->interventions[i].success_prob; return s/ip->n_interventions; }
int cst_intervention_count_completed(const CSTInterventionPortfolio* ip) { int c=0; for(int i=0;i<ip->n_interventions;i++)if(ip->interventions[i].phase>=CST_INTERV_EVALUATE)c++; return c; }
bool cst_intervention_all_completed(const CSTInterventionPortfolio* ip) { return cst_intervention_count_completed(ip)==ip->n_interventions; }
void cst_intervention_start(CSTInterventionPortfolio* ip, int idx) { if(idx<0||idx>=ip->n_interventions)return; ip->interventions[idx].phase=CST_INTERV_DIAGNOSIS; ip->interventions[idx].progress=0.0; ip->interventions[idx].iteration_count=0; }
void cst_intervention_terminate(CSTInterventionPortfolio* ip, int idx) { if(idx<0||idx>=ip->n_interventions)return; ip->interventions[idx].phase=CST_INTERV_EVALUATE; ip->interventions[idx].progress=1.0; ip->n_completed++; }
bool cst_intervention_is_active(const CSTInterventionPortfolio* ip, int idx) { if(idx<0||idx>=ip->n_interventions)return false; return ip->interventions[idx].phase<CST_INTERV_EVALUATE; }
void cst_intervention_risk_assessment(const CSTInterventionPortfolio* ip, int idx) { if(idx<0||idx>=ip->n_interventions)return; CSTIntervention* in=&ip->interventions[idx]; printf("Risk '%s': fail=%.2f part=%d/%d meth=%d\n",in->name,1.0-in->success_prob,in->n_participants,ip->sys->n_stakeholders,in->n_methods); }
void cst_intervention_learn_from_failure(CSTInterventionPortfolio* ip, int idx) { if(idx<0||idx>=ip->n_interventions)return; ip->sys->critical_depth=fmin(1.0,ip->sys->critical_depth+0.1); ip->sys->iteration++; }
void cst_intervention_compare_two(const CSTInterventionPortfolio* ip, int a, int b) { if(a<0||a>=ip->n_interventions||b<0||b>=ip->n_interventions)return; printf("Compare %s vs %s: succ=%.2f/%.2f eth=%.2f/%.2f\n",ip->interventions[a].name,ip->interventions[b].name,ip->interventions[a].success_prob,ip->interventions[b].success_prob,ip->interventions[a].ethical_score,ip->interventions[b].ethical_score); }
void cst_intervention_print(const CSTInterventionPortfolio* ip) { printf("Portfolio(%d): Midgley=%.3f Best=%d\n",ip->n_interventions,cst_intervention_midgley_index(ip),cst_intervention_best_candidate(ip)); for(int i=0;i<ip->n_interventions;i++) printf("  %s succ=%.2f eth=%.2f part=%d meth=%d\n",ip->interventions[i].name,ip->interventions[i].success_prob,ip->interventions[i].ethical_score,ip->interventions[i].n_participants,ip->interventions[i].n_methods); }
void cst_intervention_print_one(const CSTInterventionPortfolio* ip, int idx) { if(idx<0||idx>=ip->n_interventions)return; printf("Intervention: %s (iter=%d prog=%.0f%% succ=%.2f)\n",ip->interventions[idx].name,ip->interventions[idx].iteration_count,ip->interventions[idx].progress*100,ip->interventions[idx].success_prob); }

/* Intervention budget and resource planning */
double cst_intervention_budget_estimate(const CSTInterventionPortfolio* ip, int idx){
    if(idx<0||idx>=ip->n_interventions)return 0.0;
    return ip->interventions[idx].n_participants*15000.0+ip->interventions[idx].n_methods*25000.0+ip->interventions[idx].iteration_count*10000.0;
}
int cst_intervention_timeline_months(const CSTInterventionPortfolio* ip, int idx){
    if(idx<0||idx>=ip->n_interventions)return 0;
    return 3+(ip->interventions[idx].n_methods*2)+(ip->interventions[idx].iteration_count*6);
}

/* Intervention risk matrix */
typedef struct{double prob;double impact;const char* name;}RiskItem;
double cst_intervention_risk_score(const CSTInterventionPortfolio* ip, int idx){
    double p=1.0-ip->interventions[idx].success_prob;
    double i=1.0-ip->interventions[idx].ethical_score;
    return p*i;
}
bool cst_intervention_is_high_risk(const CSTInterventionPortfolio* ip, int idx){
    return cst_intervention_risk_score(ip,idx)>0.25;
}
bool cst_intervention_is_safe_bet(const CSTInterventionPortfolio* ip, int idx){
    return cst_intervention_risk_score(ip,idx)<0.1;
}

/* Intervention sequencing */
void cst_intervention_sequence_optimal(CSTInterventionPortfolio* ip, int* order){
    double scores[64];int indices[64];int n=ip->n_interventions;
    for(int i=0;i<n;i++){scores[i]=ip->interventions[i].success_prob*ip->interventions[i].ethical_score;indices[i]=i;}
    for(int i=0;i<n-1;i++)for(int j=i+1;j<n;j++)if(scores[j]>scores[i]){double ts=scores[i];scores[i]=scores[j];scores[j]=ts;int ti=indices[i];indices[i]=indices[j];indices[j]=ti;}
    for(int i=0;i<n;i++)order[i]=indices[i];
}

/* Parallel vs sequential execution analysis */
bool cst_intervention_can_parallelize(const CSTInterventionPortfolio* ip, int a, int b){
    return ip->interventions[a].n_participants+ip->interventions[b].n_participants<=ip->sys->n_stakeholders;
}
int cst_intervention_max_parallel(const CSTInterventionPortfolio* ip){
    int total_part=0;for(int i=0;i<ip->n_interventions;i++)total_part+=ip->interventions[i].n_participants;
    return(total_part>0&&ip->sys->n_stakeholders>0)?ip->sys->n_stakeholders/total_part+1:1;
}

/* Learning transfer between interventions */
double cst_intervention_learning_transfer(const CSTInterventionPortfolio* ip, int from, int to){
    if(from<0||from>=ip->n_interventions||to<0||to>=ip->n_interventions)return 0.0;
    return ip->interventions[from].success_prob*0.3;
}

/* Intervention stakeholder engagement matrix */
double cst_intervention_engagement_index(const CSTInterventionPortfolio* ip, int idx){
    if(idx<0||idx>=ip->n_interventions)return 0.0;
    return(double)ip->interventions[idx].n_participants/fmax(ip->sys->n_stakeholders,1);
}
const char* cst_intervention_engagement_level(const CSTInterventionPortfolio* ip, int idx){
    double e=cst_intervention_engagement_index(ip,idx);
    if(e<0.2)return"Inform";if(e<0.4)return"Consult";if(e<0.6)return"Involve";if(e<0.8)return"Collaborate";return"Empower";
}

/* Critical path analysis for interventions */
int cst_intervention_critical_path(CSTInterventionPortfolio* ip, int* path, int max_n){
    cst_intervention_sequence_optimal(ip,path);
    int n=ip->n_interventions;if(n>max_n)n=max_n;return n;
}
int cst_intervention_total_duration(const CSTInterventionPortfolio* ip){
    int total=0;for(int i=0;i<ip->n_interventions;i++)total+=cst_intervention_timeline_months(ip,i);return total;
}

/* Stakeholder readiness assessment */
double cst_intervention_readiness(const CSTInterventionPortfolio* ip, int idx){
    if(idx<0||idx>=ip->n_interventions)return 0.0;
    return ip->sys->critical_depth*0.3+ip->sys->pluralism_index*0.3+ip->interventions[idx].success_prob*0.4;
}
bool cst_intervention_is_ready(const CSTInterventionPortfolio* ip, int idx){return cst_intervention_readiness(ip,idx)>0.5;}

/* Legacy and sustainability of interventions */
double cst_intervention_legacy_index(const CSTInterventionPortfolio* ip, int idx){
    if(idx<0||idx>=ip->n_interventions)return 0.0;
    return ip->interventions[idx].success_prob*ip->interventions[idx].ethical_score;
}
int cst_intervention_count_sustainable(const CSTInterventionPortfolio* ip){
    int c=0;for(int i=0;i<ip->n_interventions;i++)if(cst_intervention_legacy_index(ip,i)>0.3)c++;return c;
}

/* Intervention portfolio balancing (BCG matrix inspired) */
void cst_intervention_portfolio_classify(const CSTInterventionPortfolio* ip, int* stars, int* cash_cows, int* question_marks, int* dogs){
    *stars=*cash_cows=*question_marks=*dogs=0;
    for(int i=0;i<ip->n_interventions;i++){double s=ip->interventions[i].success_prob;double e=ip->interventions[i].ethical_score;if(s>0.6&&e>0.6)(*stars)++;else if(s>0.6)(*cash_cows)++;else if(e>0.6)(*question_marks)++;else(*dogs)++;}
}

/* Synergy analysis between interventions */
double cst_intervention_synergy(const CSTInterventionPortfolio* ip, int a, int b){
    if(a<0||a>=ip->n_interventions||b<0||b>=ip->n_interventions)return 0.0;
    double method_overlap=0.0;for(int i=0;i<ip->interventions[a].n_methods;i++)for(int j=0;j<ip->interventions[b].n_methods;j++)method_overlap+=1.0;
    double part_overlap=0.0;for(int i=0;i<ip->interventions[a].n_participants;i++)for(int j=0;j<ip->interventions[b].n_participants;j++)part_overlap+=1.0;
    return(method_overlap/(fmax(ip->interventions[a].n_methods*ip->interventions[b].n_methods,1))*0.5+part_overlap/(fmax(ip->interventions[a].n_participants*ip->interventions[b].n_participants,1))*0.5);
}

/* Intervention scaling readiness (ExpandNet/WHO) */
double cst_intervention_scaling_readiness(const CSTInterventionPortfolio* ip, int idx){
    if(idx<0||idx>=ip->n_interventions)return 0.0;
    return ip->interventions[idx].success_prob*0.4+ip->interventions[idx].ethical_score*0.3+cst_intervention_legacy_index(ip,idx)*0.3;
}
bool cst_intervention_ready_to_scale(const CSTInterventionPortfolio* ip, int idx){return cst_intervention_scaling_readiness(ip,idx)>0.6;}

/* Adaptive management integration (Holling, 1978) */
double cst_intervention_adaptive_capacity(const CSTInterventionPortfolio* ip){return cst_adaptation_capacity(ip->sys);}
bool cst_intervention_is_adaptive(const CSTInterventionPortfolio* ip){return cst_intervention_adaptive_capacity(ip)>0.4&&ip->n_interventions>1;}

/* Theory of Change validation */
typedef struct{char* long_term_outcome;char** preconditions;int n_preconditions;char** assumptions;int n_assumptions;}TOCModel;
double cst_intervention_toc_plausibility(const CSTInterventionPortfolio* ip, int idx){
    if(idx<0||idx>=ip->n_interventions)return 0.0;
    return ip->interventions[idx].success_prob*ip->interventions[idx].ethical_score;
}
bool cst_intervention_toc_is_plausible(const CSTInterventionPortfolio* ip, int idx){return cst_intervention_toc_plausibility(ip,idx)>0.3;}

/* TSI Intervention (in-memory representation for integrity checks).
 * Corresponds to the Lean 4 TSIIntervention structure. */
typedef struct {
    char* creativity;        /* creativity phase description */
    char* choice;            /* choice phase description */
    char* implementation;    /* implementation phase description */
} TSIIntervention;

/* Intervention integrity check: verify that a TSI intervention
 * spans all three phases (creativity, choice, implementation). */
int cst_check_intervention_complete(TSIIntervention* tsi) {
    if (!tsi) return 0;
    int has_creativity = (tsi->creativity && strlen(tsi->creativity) > 0);
    int has_choice     = (tsi->choice && strlen(tsi->choice) > 0);
    int has_impl       = (tsi->implementation && strlen(tsi->implementation) > 0);
    return (has_creativity && has_choice && has_impl) ? 1 : 0;
}
