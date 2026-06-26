#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "requirement_prioritization.h"

Requirement* req_create(const char* n,const char* d,double v,double c,double r,double t){
  if(!n)return NULL;Requirement* req=calloc(1,sizeof(Requirement));req->name=strdup(n);req->description=d?strdup(d):strdup("");
  req->value=v;req->cost=c;req->risk=r;req->time_criticality=t;req->moscow=MOSCOW_COULD;req->kano=KANO_PERFORMANCE;return req;
}
void req_free(Requirement* r){if(!r)return;free(r->name);free(r->description);free(r);}
double req_roi(const Requirement* r){if(!r||r->cost<1e-10)return 0.0;return r->value/r->cost;}
double req_cost_of_delay(const Requirement* r,double delay){if(!r)return 0.0;return r->time_criticality*delay*r->value/100.0;}
double req_risk_adjusted_value(const Requirement* r){if(!r)return 0.0;return r->value*(1.0-r->risk);}
MoSCoWClass req_classify_moscow(const Requirement* r,double th){if(!r)return MOSCOW_WONT;
  if(r->value>th*100&&r->risk<0.3)return MOSCOW_MUST;if(r->value>th*50)return MOSCOW_SHOULD;
  if(r->value>th*20)return MOSCOW_COULD;return MOSCOW_WONT;}
KanoClass req_classify_kano(double func,double dysfunc){
  if(func>0.5&&dysfunc>0.5)return KANO_PERFORMANCE;if(func>0.5)return KANO_EXCITEMENT;
  if(dysfunc>0.5)return KANO_BASIC;if(func<0.3&&dysfunc<0.3)return KANO_INDIFFERENT;return KANO_REVERSE;}

RequirementBacklog* backlog_create(void){RequirementBacklog* bl=calloc(1,sizeof(RequirementBacklog));bl->budget_constraint=1e9;return bl;}
void backlog_free(RequirementBacklog* bl){if(!bl)return;for(int i=0;i<bl->n_requirements;i++)req_free(bl->requirements[i]);free(bl->requirements);free(bl->ranking);free(bl);}
int backlog_add(RequirementBacklog* bl,Requirement* r){if(!bl||!r||bl->n_requirements>=REQ_MAX_REQUIREMENTS)return -1;int n=bl->n_requirements;bl->requirements=realloc(bl->requirements,(n+1)*sizeof(Requirement*));bl->requirements[n]=r;bl->n_requirements=n+1;return n;}
void backlog_prioritize(RequirementBacklog* bl,PrioritizationMethod m){
  if(!bl||bl->n_requirements<1)return;bl->method=m;free(bl->ranking);bl->ranking=calloc(bl->n_requirements,sizeof(int));
  double* scores=calloc(bl->n_requirements,sizeof(double));
  for(int i=0;i<bl->n_requirements;i++){Requirement* r=bl->requirements[i];bl->ranking[i]=i;
    switch(m){case PRIO_VALUE_BASED:scores[i]=r->value;break;case PRIO_COST_OF_DELAY:scores[i]=r->time_criticality;break;
      case PRIO_RISK_BASED:scores[i]=req_risk_adjusted_value(r);break;default:scores[i]=req_roi(r);break;}}
  for(int i=0;i<bl->n_requirements-1;i++)for(int j=i+1;j<bl->n_requirements;j++)
    if(scores[bl->ranking[j]]>scores[bl->ranking[i]]){int t=bl->ranking[i];bl->ranking[i]=bl->ranking[j];bl->ranking[j]=t;}free(scores);}
void backlog_budget_constrain(RequirementBacklog* bl,double b){if(bl)bl->budget_constraint=b;}
double backlog_total_value(RequirementBacklog* bl){if(!bl)return 0.0;double t=0.0;for(int i=0;i<bl->n_requirements;i++)t+=bl->requirements[i]->value;return t;}
double backlog_total_cost(RequirementBacklog* bl){if(!bl)return 0.0;double t=0.0;for(int i=0;i<bl->n_requirements;i++)t+=bl->requirements[i]->cost;return t;}
int backlog_select_for_budget(RequirementBacklog* bl,double budget,int** sel,int* n){
  *n=0;if(!bl||!sel||budget<=0.0)return 0;backlog_prioritize(bl,PRIO_VALUE_BASED);
  *sel=calloc(bl->n_requirements,sizeof(int));double spent=0.0;
  for(int i=0;i<bl->n_requirements;i++){int idx=bl->ranking[i];if(spent+bl->requirements[idx]->cost<=budget){(*sel)[*n]=idx;spent+=bl->requirements[idx]->cost;(*n)++;}}
  return *n;}
void backlog_print(RequirementBacklog* bl){if(!bl)return;printf("Backlog: %d requirements\n",bl->n_requirements);for(int i=0;i<bl->n_requirements;i++){int idx=bl->ranking?bl->ranking[i]:i;Requirement* r=bl->requirements[idx];printf("  %d. %s value=%.0f cost=%.0f roi=%.2f\n",i+1,r->name,r->value,r->cost,req_roi(r));}}

/*
 * ============================================================
 * Advanced Requirement Prioritization Methods
 * ============================================================
 */

/*
 * WSJF (Weighted Shortest Job First) — SAFe framework.
 * WSJF = Cost of Delay / Job Duration
 * Higher WSJF means do it sooner. Normalizes across all requirements.
 */
void backlog_wsjf_prioritize(RequirementBacklog* bl){
  if(!bl||bl->n_requirements<1)return;
  double* wsjf=calloc(bl->n_requirements,sizeof(double));
  for(int i=0;i<bl->n_requirements;i++){
    Requirement* r=bl->requirements[i];
    double cod=req_cost_of_delay(r,1.0);
    double duration=r->cost>0.1?r->cost:0.1;
    wsjf[i]=cod/duration;
  }
  bl->method=PRIO_COST_OF_DELAY;
  free(bl->ranking);bl->ranking=calloc(bl->n_requirements,sizeof(int));
  for(int i=0;i<bl->n_requirements;i++)bl->ranking[i]=i;
  for(int i=0;i<bl->n_requirements-1;i++)for(int j=i+1;j<bl->n_requirements;j++)
    if(wsjf[bl->ranking[j]]>wsjf[bl->ranking[i]]){int t=bl->ranking[i];bl->ranking[i]=bl->ranking[j];bl->ranking[j]=t;}
  free(wsjf);
}

/*
 * Pairwise comparison prioritization using AHP for requirements.
 * Builds an n x n comparison matrix where a_ij = value_i / value_j.
 * Then computes the principal eigenvector for priority scores.
 * This is the same method Saaty used for AHP.
 */
void backlog_ahp_prioritize(RequirementBacklog* bl){
  if(!bl||bl->n_requirements<2)return;
  int n=bl->n_requirements;
  double** A=calloc(n,sizeof(double*));
  for(int i=0;i<n;i++){A[i]=calloc(n,sizeof(double));
    for(int j=0;j<n;j++)A[i][j]=bl->requirements[i]->value/fmax(bl->requirements[j]->value,1.0);}
  /* Power iteration for principal eigenvector */
  double* v=calloc(n,sizeof(double));for(int i=0;i<n;i++)v[i]=1.0/n;
  for(int iter=0;iter<30;iter++){
    double* w=calloc(n,sizeof(double));
    for(int i=0;i<n;i++)for(int j=0;j<n;j++)w[i]+=A[i][j]*v[j];
    double norm=0.0;for(int i=0;i<n;i++)norm+=w[i]*w[i];norm=sqrt(norm);
    if(norm>1e-10)for(int i=0;i<n;i++)w[i]/=norm;
    double diff=0.0;for(int i=0;i<n;i++)diff+=fabs(w[i]-v[i]);
    memcpy(v,w,n*sizeof(double));free(w);
    if(diff<1e-8)break;
  }
  free(bl->ranking);bl->ranking=calloc(n,sizeof(int));
  for(int i=0;i<n;i++)bl->ranking[i]=i;
  for(int i=0;i<n-1;i++)for(int j=i+1;j<n;j++)
    if(v[bl->ranking[j]]>v[bl->ranking[i]]){int t=bl->ranking[i];bl->ranking[i]=bl->ranking[j];bl->ranking[j]=t;}
  for(int i=0;i<n;i++)free(A[i]);free(A);free(v);
}

/*
 * RICE scoring (Reach, Impact, Confidence, Effort).
 * Score = (Reach * Impact * Confidence) / Effort
 * Used by Intercom and many product teams.
 */
void backlog_rice_prioritize(RequirementBacklog* bl,double* reach,double* impact,double* confidence){
  if(!bl||bl->n_requirements<1)return;
  double* rice=calloc(bl->n_requirements,sizeof(double));
  for(int i=0;i<bl->n_requirements;i++){
    double r=reach?reach[i]:1.0,i_=impact?impact[i]:1.0,c=confidence?confidence[i]:0.8;
    double e=bl->requirements[i]->cost>0.1?bl->requirements[i]->cost:1.0;
    rice[i]=(r*i_*c)/e;
  }
  free(bl->ranking);bl->ranking=calloc(bl->n_requirements,sizeof(int));
  for(int i=0;i<bl->n_requirements;i++)bl->ranking[i]=i;
  for(int i=0;i<bl->n_requirements-1;i++)for(int j=i+1;j<bl->n_requirements;j++)
    if(rice[bl->ranking[j]]>rice[bl->ranking[i]]){int t=bl->ranking[i];bl->ranking[i]=bl->ranking[j];bl->ranking[j]=t;}
  free(rice);
}

/*
 * Dependency-aware prioritization using adjacency matrix.
 * Requirements that others depend on get priority boost.
 * Priority = base_score * (1 + alpha * in_degree)
 */
void backlog_dependency_aware_prioritize(RequirementBacklog* bl,int** depends_on,int n_relations,double alpha){
  if(!bl||bl->n_requirements<1)return;
  int n=bl->n_requirements;
  int* in_deg=calloc(n,sizeof(int));
  for(int r=0;r<n_relations;r++)if(depends_on[r][1]>=0&&depends_on[r][1]<n)in_deg[depends_on[r][1]]++;
  double* scores=calloc(n,sizeof(double));
  for(int i=0;i<n;i++)scores[i]=bl->requirements[i]->value*(1.0+alpha*in_deg[i]);
  free(bl->ranking);bl->ranking=calloc(n,sizeof(int));
  for(int i=0;i<n;i++)bl->ranking[i]=i;
  for(int i=0;i<n-1;i++)for(int j=i+1;j<n;j++)
    if(scores[bl->ranking[j]]>scores[bl->ranking[i]]){int t=bl->ranking[i];bl->ranking[i]=bl->ranking[j];bl->ranking[j]=t;}
  free(scores);free(in_deg);
}

/*
 * Kano model analysis for a set of requirements.
 * Classifies each requirement based on functional/dysfunctional
 * survey responses. Returns counts for each Kano category.
 * Functional: "How would you feel if this feature WAS present?"
 * Dysfunctional: "How would you feel if this feature was NOT present?"
 */
void backlog_kano_analysis(RequirementBacklog* bl,double* functional,double* dysfunctional,int counts[5]){
  for(int k=0;k<5;k++)counts[k]=0;
  if(!bl||!functional||!dysfunctional)return;
  for(int i=0;i<bl->n_requirements;i++){
    KanoClass kc=req_classify_kano(functional[i],dysfunctional[i]);
    bl->requirements[i]->kano=kc;
    counts[kc]++;
  }
}

/*
 * Theme-based prioritization: group requirements into themes
 * and prioritize themes first, then requirements within themes.
 * Theme priority = sum of member values / number of members.
 */
void backlog_theme_prioritize(RequirementBacklog* bl,int* theme_ids,int n_themes){
  if(!bl||!theme_ids||n_themes<1)return;
  double* theme_scores=calloc(n_themes,sizeof(double));
  int* theme_counts=calloc(n_themes,sizeof(int));
  for(int i=0;i<bl->n_requirements;i++){
    int t=theme_ids[i];if(t<0||t>=n_themes)continue;
    theme_scores[t]+=bl->requirements[i]->value;theme_counts[t]++;
  }
  for(int t=0;t<n_themes;t++)if(theme_counts[t]>0)theme_scores[t]/=theme_counts[t];
  double* final_scores=calloc(bl->n_requirements,sizeof(double));
  for(int i=0;i<bl->n_requirements;i++){
    int t=theme_ids[i];double ts=t>=0&&t<n_themes?theme_scores[t]:0.0;
    final_scores[i]=bl->requirements[i]->value*0.6+ts*0.4;
  }
  free(bl->ranking);bl->ranking=calloc(bl->n_requirements,sizeof(int));
  for(int i=0;i<bl->n_requirements;i++)bl->ranking[i]=i;
  for(int i=0;i<bl->n_requirements-1;i++)for(int j=i+1;j<bl->n_requirements;j++)
    if(final_scores[bl->ranking[j]]>final_scores[bl->ranking[i]]){int t=bl->ranking[i];bl->ranking[i]=bl->ranking[j];bl->ranking[j]=t;}
  free(theme_scores);free(theme_counts);free(final_scores);
}

/*
 * Bubble sort-based incremental value delivery planning.
 * Selects requirements that maximize value within a sprint budget.
 * This is a greedy 0/1 knapsack on value/cost ratio.
 */
int backlog_knapsack_select(RequirementBacklog* bl,double budget,int** selected,int* n_selected){
  *n_selected=0;if(!bl||!selected||budget<=0.0)return 0;
  /* Sort by value/cost ratio */
  double* ratios=calloc(bl->n_requirements,sizeof(double));
  int* indices=calloc(bl->n_requirements,sizeof(int));
  for(int i=0;i<bl->n_requirements;i++){ratios[i]=req_roi(bl->requirements[i]);indices[i]=i;}
  for(int i=0;i<bl->n_requirements-1;i++)for(int j=i+1;j<bl->n_requirements;j++)
    if(ratios[indices[j]]>ratios[indices[i]]){int t=indices[i];indices[i]=indices[j];indices[j]=t;}
  *selected=calloc(bl->n_requirements,sizeof(int));
  double remaining=budget;
  for(int i=0;i<bl->n_requirements&&remaining>0.0;i++){
    int idx=indices[i];
    if(bl->requirements[idx]->cost<=remaining){(*selected)[*n_selected]=idx;remaining-=bl->requirements[idx]->cost;(*n_selected)++;}
  }
  free(ratios);free(indices);
  return *n_selected;
}

/*
 * ============================================================
 * Value-Based Requirement Engineering Methods
 * ============================================================
 */

/*
 * Planning Poker-style consensus estimation.
 * Uses the Fibonacci sequence for relative sizing.
 * Converges estimates across estimators by averaging after
 * removing outliers (trimmed mean).
 */
double req_planning_poker_estimate(double* estimates,int n_estimators){
  if(!estimates||n_estimators<1)return 0.0;
  if(n_estimators==1)return estimates[0];
  double* sorted=calloc(n_estimators,sizeof(double));
  memcpy(sorted,estimates,n_estimators*sizeof(double));
  for(int i=0;i<n_estimators-1;i++)for(int j=i+1;j<n_estimators;j++)
    if(sorted[j]<sorted[i]){double t=sorted[i];sorted[i]=sorted[j];sorted[j]=t;}
  int trim=n_estimators/4;if(trim<1)trim=1;
  double sum=0.0;int count=0;
  for(int i=trim;i<n_estimators-trim;i++){sum+=sorted[i];count++;}
  free(sorted);
  return count>0?sum/count:estimates[0];
}

/*
 * Monte Carlo schedule risk analysis for requirements.
 * Each requirement has optimistic/most-likely/pessimistic estimates.
 * Triangular distribution sampling computes completion probability.
 *
 * Returns the probability of completing all requirements by deadline.
 */
double req_schedule_risk(RequirementBacklog* bl,double* optimistic,
    double* most_likely,double* pessimistic,double deadline,int n_samples){
  if(!bl||!optimistic||!most_likely||!pessimistic||n_samples<100)return 0.0;
  int successes=0;
  for(int s=0;s<n_samples;s++){
    double total=0.0;
    for(int i=0;i<bl->n_requirements;i++){
      double r=(double)rand()/RAND_MAX;
      double estimate;
      if(r<0.5){estimate=optimistic[i]+sqrt(r*2.0)*(most_likely[i]-optimistic[i]);}
      else{estimate=pessimistic[i]-sqrt(2.0-2.0*r)*(pessimistic[i]-most_likely[i]);}
      total+=estimate;
    }
    if(total<=deadline)successes++;
  }
  return (double)successes/n_samples;
}

/*
 * Technical debt quantification for requirements.
 * Each requirement can carry technical debt if implemented quickly.
 * TechDebt_i = shortcut_cost_i * (1 - quality_i/expected_quality)
 *
 * Returns the accumulated technical debt for selected requirements.
 */
double req_technical_debt(RequirementBacklog* bl,int* selected,int n_selected,
    double* shortcut_cost,double* quality,double expected_quality){
  if(!bl||!selected||!shortcut_cost||!quality||n_selected<1)return 0.0;
  double debt=0.0;
  for(int i=0;i<n_selected;i++){
    int idx=selected[i];if(idx<0||idx>=bl->n_requirements)continue;
    double q_ratio=quality[i]/fmax(expected_quality,0.01);
    debt+=shortcut_cost[i]*(1.0-fmin(q_ratio,1.0));
  }
  return debt;
}

/*
 * Earned Business Value (EBV) tracking.
 * Computes the cumulative business value earned as requirements
 * are completed in the prioritized order.
 *
 * EBV(k) = sum_{i=1..k} value_i * completion_pct_i
 */
double* req_earned_value_trajectory(RequirementBacklog* bl,
    double* completion_pct,int* n_completed){
  if(!bl||!completion_pct||!n_completed)return NULL;
  *n_completed=bl->n_requirements;
  double* ebv=calloc(bl->n_requirements,sizeof(double));
  double cumulative=0.0;
  for(int i=0;i<bl->n_requirements;i++){
    int idx=bl->ranking?bl->ranking[i]:i;
    cumulative+=bl->requirements[idx]->value*completion_pct[idx];
    ebv[i]=cumulative;
  }
  return ebv;
}

/*
 * Requirements interdependency matrix computation.
 * Returns a matrix D where D[i][j] = strength of dependency
 * of requirement i on requirement j.
 * Based on shared stakeholders, shared resources, and logical prerequisites.
 */
void req_dependency_matrix(RequirementBacklog* bl,
    int* stakeholder_assignments,double** matrix){
  if(!bl||!stakeholder_assignments||!matrix)return;
  int n=bl->n_requirements;
  for(int i=0;i<n;i++)for(int j=0;j<n;j++){
    if(i==j){matrix[i][j]=0.0;continue;}
    double dep=0.0;
    if(stakeholder_assignments[i]==stakeholder_assignments[j])dep+=0.3;
    double value_ratio=bl->requirements[j]->value/fmax(bl->requirements[i]->value,1.0);
    dep+=0.2*fmin(value_ratio,2.0);
    double risk_coupling=(1.0-bl->requirements[i]->risk)*(1.0-bl->requirements[j]->risk);
    dep+=0.2*risk_coupling;
    matrix[i][j]=fmin(dep,1.0);
  }
}

/*
 * Minimum Viable Product (MVP) selection.
 * Selects the smallest set of requirements that delivers
 * >= target_pct of total value while minimizing cost.
 * Uses a greedy value-density heuristic.
 */
int req_mvp_select(RequirementBacklog* bl,double target_value_pct,
    int** selected,int* n_selected){
  *n_selected=0;if(!bl||!selected||target_value_pct<=0.0||target_value_pct>1.0)return 0;
  double total_value=backlog_total_value(bl);
  double target=total_value*target_value_pct;
  double* density=calloc(bl->n_requirements,sizeof(double));
  int* indices=calloc(bl->n_requirements,sizeof(int));
  for(int i=0;i<bl->n_requirements;i++){density[i]=req_roi(bl->requirements[i]);indices[i]=i;}
  for(int i=0;i<bl->n_requirements-1;i++)for(int j=i+1;j<bl->n_requirements;j++)
    if(density[indices[j]]>density[indices[i]]){int t=indices[i];indices[i]=indices[j];indices[j]=t;}
  *selected=calloc(bl->n_requirements,sizeof(int));
  double value_accum=0.0,spent=0.0;
  for(int i=0;i<bl->n_requirements&&value_accum<target;i++){
    int idx=indices[i];
    (*selected)[*n_selected]=idx;value_accum+=bl->requirements[idx]->value;
    spent+=bl->requirements[idx]->cost;(*n_selected)++;
  }
  free(density);free(indices);
  return *n_selected;
}

/* Agile story point to effort-hour conversion using calibrated velocity */
double req_story_points_to_hours(double story_points,double velocity,double team_size){return story_points*team_size/fmax(velocity,0.1);}

/* T-Shirt sizing: S=1,M=2,L=3,XL=5 to numeric estimate */
double req_tshirt_to_estimate(const char* size){if(!size)return 3.0;switch(size[0]){case'S':return 1.0;case'M':return 2.0;case'L':return(size[1]=='X'||size[1]=='l')?5.0:3.0;default:return 3.0;}}

/* Buy-a-Feature prioritization: stakeholders allocate virtual budget */
void req_buy_a_feature(double** stakeholder_budgets,int n_stakeholders,RequirementBacklog* bl,double* feature_votes){
  if(!stakeholder_budgets||!bl||!feature_votes)return;
  for(int i=0;i<bl->n_requirements;i++)feature_votes[i]=0.0;
  for(int s=0;s<n_stakeholders;s++){double spent=0.0;for(int i=0;i<bl->n_requirements;i++){if(spent+bl->requirements[i]->cost<=stakeholder_budgets[s][0]){feature_votes[i]+=1.0;spent+=bl->requirements[i]->cost;}}}
}

/* Critical chain buffer management for requirement delivery */
double req_critical_chain_buffer(RequirementBacklog* bl,int* critical_chain,int chain_len){
  if(!bl||!critical_chain||chain_len<1)return 0.0;double total_safe=0.0,total_aggressive=0.0;
  for(int i=0;i<chain_len;i++){int idx=critical_chain[i];if(idx<0||idx>=bl->n_requirements)continue;total_safe+=bl->requirements[idx]->cost*1.5;total_aggressive+=bl->requirements[idx]->cost*0.5;}
  return (total_safe-total_aggressive)*0.5;}

/* Net Present Value for requirement value with discount rate */
double req_npv(Requirement* r,double discount_rate,int period){if(!r||discount_rate<0.0||period<0)return 0.0;return r->value/pow(1.0+discount_rate,period);}

/* Opportunity cost of NOT implementing a requirement */
double req_opportunity_cost(Requirement* r,double competitor_gain_rate,double time){if(!r)return 0.0;return r->value*competitor_gain_rate*time;}

/* Requirements traceability matrix: which requirements trace to which */
void req_traceability_matrix(RequirementBacklog* bl,int** traces,int n_traces,double** matrix){
  if(!bl||!traces||!matrix)return;int n=bl->n_requirements;for(int i=0;i<n;i++)for(int j=0;j<n;j++)matrix[i][j]=0.0;
  for(int t=0;t<n_traces;t++){int from=traces[t][0],to=traces[t][1];if(from>=0&&from<n&&to>=0&&to<n)matrix[from][to]=1.0;}
}

/* ===== Risk burndown: track risk reduction as requirements complete ===== */
double* req_risk_burndown(RequirementBacklog* bl,int* completion_order,int* completed,int n_done){if(!bl||!completion_order||!completed)return NULL;double* burndown=calloc(bl->n_requirements,sizeof(double));double total_risk=0.0;for(int i=0;i<bl->n_requirements;i++)total_risk+=bl->requirements[i]->risk;double remaining=total_risk;for(int i=0;i<n_done;i++){int idx=completion_order[i];if(idx>=0&&idx<bl->n_requirements){remaining-=bl->requirements[idx]->risk;burndown[i]=remaining;}}return burndown;}

/* ===== Definition of Ready (DoR) checklist score ===== */
double req_definition_of_ready_score(Requirement* r,int* checklist,int n_checks){if(!r||!checklist||n_checks<1)return 0.0;int passed=0;for(int i=0;i<n_checks;i++)if(checklist[i])passed++;return (double)passed/n_checks;}

/* ===== ICE scoring: Impact, Confidence, Ease ===== */
void backlog_ice_prioritize(RequirementBacklog* bl,double* impact,double* confidence,double* ease){if(!bl||!impact||!confidence||!ease||bl->n_requirements<1)return;double* ice=calloc(bl->n_requirements,sizeof(double));for(int i=0;i<bl->n_requirements;i++)ice[i]=impact[i]*confidence[i]*ease[i];free(bl->ranking);bl->ranking=calloc(bl->n_requirements,sizeof(int));for(int i=0;i<bl->n_requirements;i++)bl->ranking[i]=i;for(int i=0;i<bl->n_requirements-1;i++)for(int j=i+1;j<bl->n_requirements;j++)if(ice[bl->ranking[j]]>ice[bl->ranking[i]]){int t=bl->ranking[i];bl->ranking[i]=bl->ranking[j];bl->ranking[j]=t;}free(ice);}

/* ===== Weighted FMEA risk priority number RPN = severity*occurrence*detection ===== */
double req_fmea_rpn(double severity,double occurrence,double detection){return severity*occurrence*detection;}

/* ===== Cumulative flow diagram data for requirement throughput ===== */
void req_cumulative_flow(RequirementBacklog* bl,int* arrival_dates,int* completion_dates,double* wip,double* lead_time,int n_days){if(!bl||!arrival_dates||!completion_dates||!wip||!lead_time||n_days<1)return;for(int d=0;d<n_days;d++){int arrived=0,done=0;for(int i=0;i<bl->n_requirements;i++){if(arrival_dates[i]<=d)arrived++;if(completion_dates[i]<=d)done++;}wip[d]=arrived-done;lead_time[d]=wip[d]/fmax(done>0?(double)done:d,1.0);}}

/* ===== Requirement stability index: how stable are requirements over time ===== */
double req_stability_index(RequirementBacklog* bl_old,RequirementBacklog* bl_new){if(!bl_old||!bl_new)return 0.0;int n=fmin(bl_old->n_requirements,bl_new->n_requirements);if(n<1)return 0.0;double stable=0.0;for(int i=0;i<n;i++)if(fabs(bl_old->requirements[i]->value-bl_new->requirements[i]->value)<1e-6)stable+=1.0;return stable/n;}

/* ===== Expected monetary value of a requirement under uncertainty ===== */
double req_expected_monetary_value(Requirement* r,double* scenario_values,double* scenario_probs,int n_scenarios){if(!r||!scenario_values||!scenario_probs||n_scenarios<1)return 0.0;double emv=0.0;for(int s=0;s<n_scenarios;s++)emv+=scenario_values[s]*scenario_probs[s];return emv;}

/* ===== Requirement completion forecasting using velocity ===== */
void req_forecast_completion(RequirementBacklog* bl,double velocity,double* predicted_dates,int* n_predicted){*n_predicted=0;if(!bl||!predicted_dates||velocity<0.01)return;double cumulative=0.0;for(int i=0;i<bl->n_requirements;i++){cumulative+=bl->requirements[i]->cost;predicted_dates[i]=cumulative/velocity;(*n_predicted)++;}}

/* ===== Requirements churn rate: fraction changed per time unit ===== */
double req_churn_rate(RequirementBacklog* bl_old,RequirementBacklog* bl_new){if(!bl_old||!bl_new)return 0.0;int n=fmin(bl_old->n_requirements,bl_new->n_requirements);if(n<1)return 0.0;int changed=0;for(int i=0;i<n;i++)if(fabs(bl_old->requirements[i]->value-bl_new->requirements[i]->value)>1e-6||fabs(bl_old->requirements[i]->cost-bl_new->requirements[i]->cost)>1e-6)changed++;return (double)changed/n;}
