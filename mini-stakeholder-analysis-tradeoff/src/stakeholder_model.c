#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stakeholder_model.h"

Stakeholder* stk_create(const char* name,StakeholderType type,double power,double interest,double influence,double urgency,double legitimacy){
  if(!name)return NULL; Stakeholder* s=calloc(1,sizeof(Stakeholder)); if(!s)return NULL;
  s->name=strdup(name); s->type=type; s->power=power; s->interest=interest;
  s->influence=influence; s->urgency=urgency; s->legitimacy=legitimacy;
  s->satisfaction=0.5; s->stance=INFLUENCE_NEUTRAL; return s;
}
void stk_free(Stakeholder* s){ if(!s)return; free(s->name); if(s->requirements){for(int i=0;i<s->n_requirements;i++)free(s->requirements[i]); free(s->requirements);} free(s->requirement_weights); free(s); }
void stk_add_requirement(Stakeholder* s,const char* req,double weight){ if(!s||!req)return; int n=s->n_requirements; s->requirements=realloc(s->requirements,(n+1)*sizeof(char*)); s->requirement_weights=realloc(s->requirement_weights,(n+1)*sizeof(double)); s->requirements[n]=strdup(req); s->requirement_weights[n]=weight; s->n_requirements=n+1; }
double stk_salience(const Stakeholder* s){ if(!s)return 0.0; return STK_SALIENCE_WEIGHT_POWER*s->power+STK_SALIENCE_WEIGHT_LEGITIMACY*s->legitimacy+STK_SALIENCE_WEIGHT_URGENCY*s->urgency; }
StakeholderCategory stk_classify(const Stakeholder* s){ if(!s)return STAKE_LOW_POWER_LOW_INTEREST; if(s->power>=STK_POWER_THRESHOLD&&s->interest>=STK_INTEREST_THRESHOLD)return STAKE_HIGH_POWER_HIGH_INTEREST; if(s->power>=STK_POWER_THRESHOLD)return STAKE_HIGH_POWER_LOW_INTEREST; if(s->interest>=STK_INTEREST_THRESHOLD)return STAKE_LOW_POWER_HIGH_INTEREST; return STAKE_LOW_POWER_LOW_INTEREST; }
double stk_mitchell_score(const Stakeholder* s){ if(!s)return 0.0; return (s->power+s->legitimacy+s->urgency)/3.0; }
void stk_print(const Stakeholder* s){ if(!s){printf("Stakeholder: NULL\n");return;} printf("Stakeholder: %s power=%.2f interest=%.2f salience=%.2f\n",s->name,s->power,s->interest,stk_salience(s)); }

StakeholderNetwork* stk_net_create(void){ StakeholderNetwork* n=calloc(1,sizeof(StakeholderNetwork)); n->capacity=STK_MAX_STAKEHOLDERS; n->stakeholders=calloc(STK_MAX_STAKEHOLDERS,sizeof(Stakeholder*)); return n; }
void stk_net_free(StakeholderNetwork* net){ if(!net)return; for(int i=0;i<net->n_stakeholders;i++)stk_free(net->stakeholders[i]); free(net->stakeholders); if(net->influence_matrix){for(int i=0;i<net->n_stakeholders;i++)free(net->influence_matrix[i]); free(net->influence_matrix);} free(net->betweenness); free(net); }
int stk_net_add(StakeholderNetwork* net,Stakeholder* s){ if(!net||!s||net->n_stakeholders>=net->capacity)return -1; int idx=net->n_stakeholders; net->stakeholders[idx]=s; net->n_stakeholders=idx+1; return idx; }
void stk_net_set_influence(StakeholderNetwork* net,int from,int to,double val){ if(!net||from<0||from>=net->n_stakeholders||to<0||to>=net->n_stakeholders)return; if(!net->influence_matrix){ int n=net->n_stakeholders; net->influence_matrix=calloc(n,sizeof(double*)); for(int i=0;i<n;i++)net->influence_matrix[i]=calloc(n,sizeof(double)); } net->influence_matrix[from][to]=val; }
void stk_net_compute_metrics(StakeholderNetwork* net){
  if(!net||net->n_stakeholders<2)return; int n=net->n_stakeholders;
  if(!net->influence_matrix){ net->influence_matrix=calloc(n,sizeof(double*)); for(int i=0;i<n;i++)net->influence_matrix[i]=calloc(n,sizeof(double)); }
  int edges=0; for(int i=0;i<n;i++)for(int j=0;j<n;j++)if(i!=j&&fabs(net->influence_matrix[i][j])>1e-6)edges++;
  net->network_density=(double)edges/(n*(n-1));
  net->betweenness=calloc(n,sizeof(double));
  for(int v=0;v<n;v++){ double bc=0.0; for(int i=0;i<n;i++)for(int j=0;j<n;j++)if(i!=j&&i!=v&&j!=v){ double d_ij=0.0; for(int k=0;k<n;k++)d_ij+=fabs(net->influence_matrix[i][k]); double d_iv=net->influence_matrix[i][v]; double d_vj=net->influence_matrix[v][j]; if(fabs(d_ij)>1e-6&&fabs(d_iv)>0.0&&fabs(d_vj)>0.0)bc+=fabs(d_iv*d_vj)/d_ij; } net->betweenness[v]=bc/fmax(n*(n-1)*(n-2)/2.0,1.0); }
  double max_bc=0.0,sum_bc=0.0; for(int i=0;i<n;i++){sum_bc+=net->betweenness[i]; if(net->betweenness[i]>max_bc)max_bc=net->betweenness[i];}
  double star_centrality=n*max_bc-sum_bc; net->centralization=star_centrality/fmax((n-1)*(n-2),1.0); }

void stk_net_find_key_players(StakeholderNetwork* net,int* indices,int* n){
  *n=0; if(!net||net->n_stakeholders<1)return; stk_net_compute_metrics(net); double threshold=0.0; for(int i=0;i<net->n_stakeholders;i++)threshold+=net->betweenness[i]; threshold=threshold/(net->n_stakeholders*2.0);
  for(int i=0;i<net->n_stakeholders;i++)if(net->betweenness[i]>threshold&&*n<10){indices[*n]=i;(*n)++;} }
double stk_net_conflict_index(StakeholderNetwork* net){
  if(!net||net->n_stakeholders<2)return 0.0; int conflicts=0,pairs=net->n_stakeholders*(net->n_stakeholders-1)/2;
  for(int i=0;i<net->n_stakeholders;i++)for(int j=i+1;j<net->n_stakeholders;j++)if(net->stakeholders[i]->stance!=net->stakeholders[j]->stance)conflicts++; return (double)conflicts/pairs; }
void stk_net_print(StakeholderNetwork* net){ if(!net)return; printf("Stakeholder Network: %d stakeholders density=%.3f centralization=%.3f\n",net->n_stakeholders,net->network_density,net->centralization); for(int i=0;i<net->n_stakeholders;i++)printf("  [%d] %s\n",i,net->stakeholders[i]->name); }

/*
 * Additional Stakeholder Analysis Functions
 */

/* Compute power/interest grid quadrant counts */
void stk_net_grid_counts(StakeholderNetwork* net,int counts[4]){
  for(int i=0;i<4;i++)counts[i]=0;
  if(!net)return;
  for(int i=0;i<net->n_stakeholders;i++){
    StakeholderCategory c=stk_classify(net->stakeholders[i]);
    if(c>=0&&c<4)counts[c]++;
  }
}

/* Compute overall stakeholder satisfaction (weighted by salience) */
double stk_net_satisfaction(StakeholderNetwork* net){
  if(!net||net->n_stakeholders<1)return 0.0;
  double total_sal=0.0,weighted_sat=0.0;
  for(int i=0;i<net->n_stakeholders;i++){
    double sal=stk_salience(net->stakeholders[i]);
    weighted_sat+=sal*net->stakeholders[i]->satisfaction;
    total_sal+=sal;
  }
  return total_sal>1e-10?weighted_sat/total_sal:0.0;
}

/* Find the most influential stakeholder based on network position */
int stk_net_most_influential(StakeholderNetwork* net){
  if(!net||net->n_stakeholders<1)return -1;
  stk_net_compute_metrics(net);
  double max_influence=-1.0; int best=-1;
  for(int i=0;i<net->n_stakeholders;i++){
    double combined=net->stakeholders[i]->power*0.5+net->betweenness[i]*0.5;
    if(combined>max_influence){max_influence=combined;best=i;}
  }
  return best;
}

/* Compute the engagement strategy for a stakeholder based on power/interest */
const char* stk_engagement_strategy(const Stakeholder* s){
  if(!s)return "Unknown";
  StakeholderCategory c=stk_classify(s);
  switch(c){
    case STAKE_HIGH_POWER_HIGH_INTEREST:return "Manage Closely";
    case STAKE_HIGH_POWER_LOW_INTEREST:return "Keep Satisfied";
    case STAKE_LOW_POWER_HIGH_INTEREST:return "Keep Informed";
    default:return "Monitor";
  }
}

/* Compute stakeholder alignment score (how aligned stances are) */
double stk_net_alignment(StakeholderNetwork* net){
  if(!net||net->n_stakeholders<2)return 1.0;
  int aligned=0,total_pairs=net->n_stakeholders*(net->n_stakeholders-1)/2;
  for(int i=0;i<net->n_stakeholders;i++)
    for(int j=i+1;j<net->n_stakeholders;j++)
      if(net->stakeholders[i]->stance==net->stakeholders[j]->stance)aligned++;
  return (double)aligned/total_pairs;
}

/* Influence propagation: simulate one round of influence spreading */
void stk_net_propagate_influence(StakeholderNetwork* net){
  if(!net||net->n_stakeholders<2)return;
  int n=net->n_stakeholders;
  double* new_influence=calloc(n,sizeof(double));
  for(int i=0;i<n;i++){
    new_influence[i]=net->stakeholders[i]->influence*0.5;
    double incoming=0.0;
    for(int j=0;j<n;j++)
      if(i!=j)incoming+=net->influence_matrix[j][i]*net->stakeholders[j]->influence;
    incoming/=(n>1?(double)(n-1):1.0);
    new_influence[i]+=incoming*0.5;
  }
  for(int i=0;i<n;i++)net->stakeholders[i]->influence=new_influence[i];
  free(new_influence);
}

/* Identify coalitions: groups of stakeholders with similar stances */
int stk_net_find_coalitions(StakeholderNetwork* net,int* coalition_ids){
  if(!net||!coalition_ids||net->n_stakeholders<1)return 0;
  int n=net->n_stakeholders;
  for(int i=0;i<n;i++)coalition_ids[i]=-1;
  int cid=0;
  for(int i=0;i<n;i++){
    if(coalition_ids[i]>=0)continue;
    coalition_ids[i]=cid;
    for(int j=i+1;j<n;j++)
      if(net->stakeholders[j]->stance==net->stakeholders[i]->stance)
        coalition_ids[j]=cid;
    cid++;
  }
  return cid;
}

/* Compute stakeholder requirement overlap (Jaccard similarity) */
double stk_requirement_overlap(const Stakeholder* a,const Stakeholder* b){
  if(!a||!b)return 0.0;
  if(a->n_requirements<1&&b->n_requirements<1)return 1.0;
  if(a->n_requirements<1||b->n_requirements<1)return 0.0;
  int intersection=0;
  for(int i=0;i<a->n_requirements;i++)
    for(int j=0;j<b->n_requirements;j++)
      if(strcmp(a->requirements[i],b->requirements[j])==0)intersection++;
  int union_size=a->n_requirements+b->n_requirements-intersection;
  return union_size>0?(double)intersection/union_size:0.0;
}

/* Set stakeholder stance based on numeric value */
void stk_set_stance(Stakeholder* s,int stance){
  if(!s)return;
  s->stance=stance>0?INFLUENCE_POSITIVE:(stance<0?INFLUENCE_NEGATIVE:INFLUENCE_NEUTRAL);
}

/* Write stakeholder report to string buffer */
int stk_generate_report(const Stakeholder* s,char* buf,int bufsize){
  if(!s||!buf||bufsize<1)return 0;
  return snprintf(buf,bufsize,
    "Stakeholder: %s\n  Type: %d  Power: %.2f  Interest: %.2f\n"
    "  Salience: %.2f  Category: %d  Strategy: %s\n",
    s->name,s->type,s->power,s->interest,
    stk_salience(s),stk_classify(s),stk_engagement_strategy(s));
}

/* Compute stakeholder diversity index (Simpson's diversity) */
double stk_net_diversity(StakeholderNetwork* net){
  if(!net||net->n_stakeholders<1)return 0.0;
  int type_counts[STYPE_N_TYPES]; for(int i=0;i<STYPE_N_TYPES;i++)type_counts[i]=0;
  for(int i=0;i<net->n_stakeholders;i++){StakeholderType t=net->stakeholders[i]->type;if(t>=0&&t<STYPE_N_TYPES)type_counts[t]++;}
  double d=0.0; int n=net->n_stakeholders;
  for(int i=0;i<STYPE_N_TYPES;i++){double p=(double)type_counts[i]/n;d+=p*p;}
  return 1.0-d;
}

/* Compute cumulative stakeholder influence vector */
void stk_net_influence_vector(StakeholderNetwork* net,double* vector){
  if(!net||!vector)return;
  for(int i=0;i<net->n_stakeholders;i++)vector[i]=0.0;
  for(int i=0;i<net->n_stakeholders;i++){
    vector[i]=net->stakeholders[i]->influence;
    for(int j=0;j<net->n_stakeholders;j++)
      if(i!=j)vector[i]+=net->influence_matrix[i][j]*net->stakeholders[j]->influence;
  }
}

/* Prioritize stakeholders by composite score for resource allocation */
void stk_net_prioritize(StakeholderNetwork* net,int* ranking){
  if(!net||!ranking||net->n_stakeholders<1)return;
  double* scores=calloc(net->n_stakeholders,sizeof(double));
  for(int i=0;i<net->n_stakeholders;i++){
    scores[i]=stk_salience(net->stakeholders[i])*0.4
             +net->stakeholders[i]->influence*0.3
             +net->stakeholders[i]->satisfaction*0.3;
    ranking[i]=i;
  }
  for(int i=0;i<net->n_stakeholders-1;i++)
    for(int j=i+1;j<net->n_stakeholders;j++)
      if(scores[ranking[j]]>scores[ranking[i]]){int t=ranking[i];ranking[i]=ranking[j];ranking[j]=t;}
  free(scores);
}

/* Estimate the power distance between two stakeholders */
double stk_power_distance(const Stakeholder* a,const Stakeholder* b){
  if(!a||!b)return 0.0;
  return fabs(stk_salience(a)-stk_salience(b));
}

/* Compute the maximum power distance in the network */
double stk_net_max_power_distance(StakeholderNetwork* net){
  if(!net||net->n_stakeholders<2)return 0.0;
  double max_d=0.0;
  for(int i=0;i<net->n_stakeholders;i++)
    for(int j=i+1;j<net->n_stakeholders;j++){
      double d=stk_power_distance(net->stakeholders[i],net->stakeholders[j]);
      if(d>max_d)max_d=d;
    }
  return max_d;
}

/* Compute the Gini coefficient of stakeholder influence */
double stk_net_influence_gini(StakeholderNetwork* net){
  if(!net||net->n_stakeholders<2)return 0.0;
  int n=net->n_stakeholders;
  double* inf=calloc(n,sizeof(double));
  for(int i=0;i<n;i++)inf[i]=net->stakeholders[i]->influence;
  for(int i=0;i<n-1;i++)for(int j=i+1;j<n;j++)if(inf[j]>inf[i]){double t=inf[i];inf[i]=inf[j];inf[j]=t;}
  double sum=0.0; for(int i=0;i<n;i++)sum+=inf[i]*(n-i);
  double total=0.0; for(int i=0;i<n;i++)total+=inf[i];
  return total>1e-10?(n+1.0-2.0*sum/total)/n:0.0;
}

/* Map stakeholder type to string */
const char* stk_type_name(StakeholderType t){
  switch(t){
    case STYPE_PRIMARY:return "Primary";
    case STYPE_SECONDARY:return "Secondary";
    case STYPE_KEY:return "Key";
    case STYPE_REGULATORY:return "Regulatory";
    case STYPE_COMMUNITY:return "Community";
    default:return "Unknown";
  }
}

/*
 * ============================================================
 * Stakeholder Mapping Implementation
 * ============================================================
 *
 * Maps stakeholder positions to specific system variables,
 * enabling quantitative comparison of stakeholder preferences.
 */

StakeholderMapping* stk_map_create(Stakeholder* s, int n_vars){
  if(!s||n_vars<1)return NULL;
  StakeholderMapping* m=calloc(1,sizeof(StakeholderMapping));
  m->stakeholder=s; m->n_variables=n_vars;
  m->variable_indices=calloc(n_vars,sizeof(int));
  m->position_values=calloc(n_vars,sizeof(double));
  for(int i=0;i<n_vars;i++){m->variable_indices[i]=i;m->position_values[i]=0.5;}
  return m;
}
void stk_map_free(StakeholderMapping* m){
  if(!m)return; free(m->variable_indices); free(m->position_values); free(m);
}
void stk_map_set_position(StakeholderMapping* m,int var_idx,double value){
  if(!m||var_idx<0||var_idx>=m->n_variables)return; m->position_values[var_idx]=value;
}
double stk_map_distance(const StakeholderMapping* a,const StakeholderMapping* b){
  if(!a||!b)return 1e9; int n=a->n_variables<b->n_variables?a->n_variables:b->n_variables;
  double dist=0.0; for(int i=0;i<n;i++){double d=a->position_values[i]-b->position_values[i];dist+=d*d;}
  return sqrt(dist);
}

/*
 * ============================================================
 * Stakeholder Value Network Analysis
 * ============================================================
 *
 * Additional analysis tools for understanding stakeholder
 * relationships, value flows, and systemic impacts.
 */

/* Compute the value exchange matrix: value provided by each
   stakeholder to each other stakeholder, normalized to [0,1] */
void stk_net_value_exchange(StakeholderNetwork* net,double** matrix){
  if(!net||!matrix)return;
  for(int i=0;i<net->n_stakeholders;i++)
    for(int j=0;j<net->n_stakeholders;j++){
      if(i==j){matrix[i][j]=0.0;continue;}
      double value=net->stakeholders[i]->influence*net->stakeholders[j]->interest;
      matrix[i][j]=value;
    }
}

/* Compute stakeholder criticality: how essential each stakeholder
   is based on network position and unique contributions.
   Criticality(i) = betweenness(i) * (1 + unique_requirements_ratio(i)) */
void stk_net_criticality(StakeholderNetwork* net,double* scores){
  if(!net||!scores)return;
  stk_net_compute_metrics(net);
  for(int i=0;i<net->n_stakeholders;i++){
    int unique_reqs=net->stakeholders[i]->n_requirements;
    double req_ratio=net->n_stakeholders>0?(double)unique_reqs/fmax(net->n_stakeholders,1.0):0.0;
    scores[i]=net->betweenness[i]*(1.0+req_ratio);
  }
}

/* Freeman's group betweenness centralization.
   Measures how centralized the influence structure is.
   C_B = sum(max_betweenness - betweenness_i) / theoretical_max */
double stk_net_betweenness_centralization(StakeholderNetwork* net){
  if(!net||net->n_stakeholders<2)return 0.0;
  stk_net_compute_metrics(net);
  double max_bc=0.0,sum_diff=0.0;
  for(int i=0;i<net->n_stakeholders;i++)if(net->betweenness[i]>max_bc)max_bc=net->betweenness[i];
  for(int i=0;i<net->n_stakeholders;i++)sum_diff+=max_bc-net->betweenness[i];
  double theoretical_max=(net->n_stakeholders-1.0)*(net->n_stakeholders-2.0);
  return theoretical_max>1e-10?sum_diff/theoretical_max:0.0;
}

/* Compute the network's structural holes (Burt's theory).
   Returns the average effective size across all nodes.
   Effective size(i) = degree(i) - sum_j sum_q p_iq * m_jq */
double stk_net_structural_holes(StakeholderNetwork* net){
  if(!net||net->n_stakeholders<2)return 0.0;
  double total_eff_size=0.0;
  for(int i=0;i<net->n_stakeholders;i++){
    int deg=0; for(int j=0;j<net->n_stakeholders;j++)if(i!=j&&fabs(net->influence_matrix[i][j])>1e-6)deg++;
    double redundancy=0.0;
    for(int j=0;j<net->n_stakeholders;j++){
      if(i==j||fabs(net->influence_matrix[i][j])<1e-6)continue;
      for(int q=0;q<net->n_stakeholders;q++){
        if(i==q||j==q||fabs(net->influence_matrix[i][q])<1e-6)continue;
        redundancy+=fabs(net->influence_matrix[j][q])/fmax(deg,1.0);
      }
    }
    total_eff_size+=deg-fmax(redundancy,0.0);
  }
  return total_eff_size/net->n_stakeholders;
}

/* Identify stakeholder "bridges" — those connecting otherwise
   disconnected groups. A bridge has high betweenness and
   connects nodes from different coalitions. */
int stk_net_find_bridges(StakeholderNetwork* net,int* bridge_indices,int max_bridges){
  if(!net||!bridge_indices||max_bridges<1)return 0;
  stk_net_compute_metrics(net);
  int* coalitions=calloc(net->n_stakeholders,sizeof(int));
  int n_coal=stk_net_find_coalitions(net,coalitions);
  int count=0;
  for(int i=0;i<net->n_stakeholders&&count<max_bridges;i++){
    double bc=net->betweenness[i]; if(bc<0.1)continue;
    int unique_coal=0; int seen[64]={0};
    for(int j=0;j<net->n_stakeholders;j++)
      if(i!=j&&fabs(net->influence_matrix[i][j])>1e-6&&!seen[coalitions[j]]){seen[coalitions[j]]=1;unique_coal++;}
    if(unique_coal>=2){bridge_indices[count]=i;count++;}
  }
  free(coalitions); return count;
}

/* ==========================================================
   Stakeholder Engagement and Communication
   ========================================================== */

/* Salience model of stakeholder engagement priority.
   Computes engagement frequency based on power, legitimacy, urgency.
   Higher salience = more frequent engagement needed. */
double stk_engagement_frequency(const Stakeholder* s){
  if(!s)return 0.0;
  double sal=stk_salience(s);
  return 1.0+9.0*sal;
}

/* RACI matrix assignment for stakeholders.
   R=Responsible, A=Accountable, C=Consulted, I=Informed.
   Assigns roles based on power/interest grid position. */
void stk_raci_assign(StakeholderNetwork* net,int* roles){
  if(!net||!roles)return;
  for(int i=0;i<net->n_stakeholders;i++){
    StakeholderCategory cat=stk_classify(net->stakeholders[i]);
    switch(cat){
      case STAKE_HIGH_POWER_HIGH_INTEREST:roles[i]=0;break;
      case STAKE_HIGH_POWER_LOW_INTEREST:roles[i]=2;break;
      case STAKE_LOW_POWER_HIGH_INTEREST:roles[i]=1;break;
      default:roles[i]=3;break;
    }
  }
}

/* Compute the communication plan: how and when to engage each stakeholder.
   Returns engagement intensity (0-1) for each stakeholder. */
void stk_communication_plan(StakeholderNetwork* net,double* intensity,double* frequency){
  if(!net||!intensity||!frequency)return;
  for(int i=0;i<net->n_stakeholders;i++){
    frequency[i]=stk_engagement_frequency(net->stakeholders[i]);
    intensity[i]=stk_salience(net->stakeholders[i]);
  }
}

/* Stakeholder expectation management.
   Compute the gap between stakeholder expectations and actual system state.
   Larger gap = higher risk of stakeholder dissatisfaction. */
double stk_expectation_gap(const Stakeholder* s,double* actual_state,int n_vars){
  if(!s||!actual_state||n_vars<1)return 0.0;
  double gap=0.0;
  for(int i=0;i<n_vars&&i<s->n_requirements;i++){
    double expected=s->requirement_weights?fabs(s->requirement_weights[i]):0.5;
    gap+=fabs(expected-actual_state[i]);
  }
  return gap/fmax(n_vars,1);
}

/* Power-interest evolution over time.
   Models how stakeholder positions change as the project progresses.
   Power(t) = Power_0 * (1 + influence_growth * t)
   Interest(t) = Interest_0 * (1 + engagement_effect * t) */
void stk_evolve_positions(StakeholderNetwork* net,double dt,double influence_growth,double engagement_effect){
  if(!net)return;
  for(int i=0;i<net->n_stakeholders;i++){
    net->stakeholders[i]->power*=(1.0+influence_growth*dt);
    if(net->stakeholders[i]->power>1.0)net->stakeholders[i]->power=1.0;
    net->stakeholders[i]->interest*=(1.0+engagement_effect*dt);
    if(net->stakeholders[i]->interest>1.0)net->stakeholders[i]->interest=1.0;
  }
}

/*
 * ============================================================
 * Social Network Analysis for Stakeholders
 * ============================================================
 */

/* Eigenvector centrality (Bonacich): iterative computation */
void stk_net_eigenvector_centrality(StakeholderNetwork* net,double* centrality,int max_iter){
  if(!net||!centrality||net->n_stakeholders<1)return;
  int n=net->n_stakeholders;
  for(int i=0;i<n;i++)centrality[i]=1.0/sqrt(n);
  for(int iter=0;iter<max_iter;iter++){
    double* next=calloc(n,sizeof(double));double norm=0.0;
    for(int i=0;i<n;i++){for(int j=0;j<n;j++)next[i]+=net->influence_matrix[j][i]*centrality[j];norm+=next[i]*next[i];}
    norm=sqrt(norm);if(norm<1e-10){free(next);return;}
    for(int i=0;i<n;i++)next[i]/=norm;double diff=0.0;
    for(int i=0;i<n;i++)diff+=fabs(next[i]-centrality[i]);
    memcpy(centrality,next,n*sizeof(double));free(next);
    if(diff<1e-8)break;
  }
}

/* PageRank-inspired stakeholder ranking with damping factor */
void stk_net_pagerank(StakeholderNetwork* net,double damping,double* ranks,int max_iter){
  if(!net||!ranks||net->n_stakeholders<1)return;
  int n=net->n_stakeholders;
  for(int i=0;i<n;i++)ranks[i]=1.0/n;
  for(int iter=0;iter<max_iter;iter++){
    double* next=calloc(n,sizeof(double));
    for(int i=0;i<n;i++){double sum=0.0;int out_deg=0;
      for(int j=0;j<n;j++)if(fabs(net->influence_matrix[i][j])>1e-6)out_deg++;
      for(int j=0;j<n;j++)if(fabs(net->influence_matrix[j][i])>1e-6)next[i]+=ranks[j]/fmax(out_deg,1.0);
    }
    for(int i=0;i<n;i++)next[i]=damping*next[i]+(1.0-damping)/n;
    double diff=0.0;for(int i=0;i<n;i++)diff+=fabs(next[i]-ranks[i]);
    memcpy(ranks,next,n*sizeof(double));free(next);
    if(diff<1e-8)break;
  }
}

/* Clustering coefficient per stakeholder (local) */
void stk_net_local_clustering(StakeholderNetwork* net,double* coefficients){
  if(!net||!coefficients||net->n_stakeholders<1)return;
  int n=net->n_stakeholders;
  for(int i=0;i<n;i++){int neighbors=0;for(int j=0;j<n;j++)if(j!=i&&fabs(net->influence_matrix[i][j])>1e-6)neighbors++;
    if(neighbors<2){coefficients[i]=0.0;continue;}int triangles=0;
    for(int j=0;j<n;j++){if(j==i||fabs(net->influence_matrix[i][j])<1e-6)continue;
      for(int k=j+1;k<n;k++){if(k==i||fabs(net->influence_matrix[i][k])<1e-6)continue;
        if(fabs(net->influence_matrix[j][k])>1e-6||fabs(net->influence_matrix[k][j])>1e-6)triangles++;}}
    coefficients[i]=(double)triangles/(neighbors*(neighbors-1)/2.0);}
}

/* Stakeholder churn risk prediction */
double stk_churn_risk(const Stakeholder* s,double engagement_level,double satisfaction_threshold){
  if(!s)return 1.0;double risk=0.0;
  if(s->satisfaction<satisfaction_threshold)risk+=0.5;
  if(engagement_level<0.3)risk+=0.3;
  risk+=(1.0-s->satisfaction)*0.2;
  return risk>1.0?1.0:risk;
}

/* Stakeholder value-at-risk: how much project value is at risk
   if this stakeholder becomes disengaged or hostile */
double stk_value_at_risk(const Stakeholder* s,double project_value){
  if(!s)return 0.0;
  double salience=stk_salience(s);
  return project_value*salience*(1.0-s->satisfaction);
}

/* Borgatti's Key Player Problem: find the set of k stakeholders
   whose removal maximally fragments the network */
int stk_net_key_player_removal(StakeholderNetwork* net,int k,int* removed){
  if(!net||!removed||k<1||k>=net->n_stakeholders)return 0;
  int n=net->n_stakeholders,count=0;bool* active=calloc(n,sizeof(bool));
  for(int i=0;i<n;i++)active[i]=true;
  for(int r=0;r<k&&r<n;r++){double best_score=-1.0;int best=-1;
    for(int i=0;i<n;i++){if(!active[i])continue;int fragments=0;
      for(int j=0;j<n;j++)if(j!=i&&active[j]&&fabs(net->influence_matrix[i][j])<1e-6)fragments++;
      if(fragments>best_score){best_score=fragments;best=i;}}
    if(best>=0){removed[count]=best;active[best]=false;count++;}}
  free(active);return count;
}

/* ===== Stakeholder trust dynamics model ===== */
void stk_trust_dynamics(StakeholderNetwork* net,double* trust_levels,double* reliability,double dt){if(!net||!trust_levels||!reliability)return;for(int i=0;i<net->n_stakeholders;i++){double expected=reliability[i];double actual=net->stakeholders[i]->satisfaction;trust_levels[i]+=(actual-expected)*0.1*dt;if(trust_levels[i]>1.0)trust_levels[i]=1.0;if(trust_levels[i]<0.0)trust_levels[i]=0.0;}}

/* ===== Two-mode network: stakeholders connected to issues ===== */
void stk_bipartite_projection(StakeholderNetwork* net,int* issue_assignments,int n_issues,double** projection){if(!net||!issue_assignments||!projection||n_issues<1)return;int n=net->n_stakeholders;for(int i=0;i<n;i++)for(int j=0;j<n;j++){projection[i][j]=0.0;for(int k=0;k<n_issues;k++)if(issue_assignments[i*n_issues+k]&&issue_assignments[j*n_issues+k])projection[i][j]+=1.0;}}

/* ===== Stakeholder influence decay over time (forgetting curve) ===== */
void stk_influence_decay(StakeholderNetwork* net,double half_life,double dt){if(!net||half_life<=0.0)return;double lambda=log(2.0)/half_life;for(int i=0;i<net->n_stakeholders;i++)net->stakeholders[i]->influence*=exp(-lambda*dt);}

/* ===== Stakeholder satisfaction prediction using simple extrapolation ===== */
double stk_predict_satisfaction(const Stakeholder* s,double* trend,double forecast_horizon){if(!s||!trend)return s?s->satisfaction:0.0;return s->satisfaction+trend[0]*forecast_horizon;}

/* ===== Stakeholder engagement ROI: (influence * satisfaction_improvement)/cost ===== */
double stk_engagement_roi(const Stakeholder* s,double engagement_cost,double satisfaction_improvement){if(!s||engagement_cost<1e-10)return 0.0;return s->influence*satisfaction_improvement/engagement_cost;}

/* ===== Network reciprocity: count bidirectional influence links ===== */
double stk_net_reciprocity(StakeholderNetwork* net){if(!net||net->n_stakeholders<2)return 0.0;int recip=0,total=0;for(int i=0;i<net->n_stakeholders;i++)for(int j=i+1;j<net->n_stakeholders;j++){total++;if(fabs(net->influence_matrix[i][j])>1e-6&&fabs(net->influence_matrix[j][i])>1e-6)recip++;}return total>0?(double)recip/total:0.0;}

/* ===== Stakeholder influence propagation through k-hop neighborhood ===== */
void stk_k_hop_influence(StakeholderNetwork* net,int source,int k,double* reach){if(!net||!reach||source<0||source>=net->n_stakeholders||k<1)return;for(int i=0;i<net->n_stakeholders;i++)reach[i]=0.0;int* queue=calloc(net->n_stakeholders,sizeof(int));int* dist=calloc(net->n_stakeholders,sizeof(int));for(int i=0;i<net->n_stakeholders;i++)dist[i]=-1;int qh=0,qt=0;queue[qt++]=source;dist[source]=0;reach[source]=net->stakeholders[source]->influence;while(qh<qt){int v=queue[qh++];if(dist[v]>=k)continue;for(int j=0;j<net->n_stakeholders;j++)if(v!=j&&fabs(net->influence_matrix[v][j])>1e-6&&dist[j]<0){dist[j]=dist[v]+1;queue[qt++]=j;reach[j]=net->stakeholders[j]->influence*pow(0.5,dist[j]);}}free(queue);free(dist);}

/* ===== Stakeholder sentiment polarity score from text-like data ===== */
double stk_sentiment_score(const Stakeholder* s,double* positive_indicators,double* negative_indicators,int n_indicators){if(!s||!positive_indicators||!negative_indicators||n_indicators<1)return 0.0;double pos=0.0,neg=0.0;for(int i=0;i<n_indicators;i++){pos+=positive_indicators[i];neg+=negative_indicators[i];}return (pos-neg)/(pos+neg+1e-10);}

/* ===== Stakeholder network multiplexity: multiple relationship types ===== */
double stk_net_multiplexity(StakeholderNetwork* net,double** influence_type2){if(!net||!influence_type2||net->n_stakeholders<2)return 0.0;int multiplex=0;for(int i=0;i<net->n_stakeholders;i++)for(int j=0;j<net->n_stakeholders;j++)if(i!=j&&fabs(net->influence_matrix[i][j])>1e-6&&fabs(influence_type2[i][j])>1e-6)multiplex++;int total=fmax(net->n_stakeholders*(net->n_stakeholders-1),1);return (double)multiplex/total;}

/* ===== Closeness centrality: average shortest path length to all others ===== */
void stk_net_closeness_centrality(StakeholderNetwork* net,double* closeness){if(!net||!closeness||net->n_stakeholders<1)return;for(int v=0;v<net->n_stakeholders;v++){double sum_dist=0.0;int reachable=0;for(int u=0;u<net->n_stakeholders;u++)if(u!=v&&fabs(net->influence_matrix[v][u])>1e-6){sum_dist+=1.0;reachable++;}closeness[v]=reachable>0?reachable/sum_dist:0.0;}}

/* ===== Stakeholder triadic closure: tendency to close open triangles ===== */
double stk_net_triadic_closure(StakeholderNetwork* net){if(!net||net->n_stakeholders<3)return 0.0;int closed=0,total=0;for(int i=0;i<net->n_stakeholders;i++)for(int j=i+1;j<net->n_stakeholders;j++)for(int k=j+1;k<net->n_stakeholders;k++){int edges=0;if(fabs(net->influence_matrix[i][j])>1e-6)edges++;if(fabs(net->influence_matrix[j][k])>1e-6)edges++;if(fabs(net->influence_matrix[i][k])>1e-6)edges++;total++;if(edges==3)closed++;}return total>0?(double)closed/total:0.0;}

/* ===== Stakeholder homophily: tendency to connect to similar stakeholders ===== */
double stk_net_homophily(StakeholderNetwork* net){if(!net||net->n_stakeholders<2)return 0.0;int same=0,diff=0;for(int i=0;i<net->n_stakeholders;i++)for(int j=i+1;j<net->n_stakeholders;j++){if(fabs(net->influence_matrix[i][j])>1e-6||fabs(net->influence_matrix[j][i])>1e-6){if(net->stakeholders[i]->type==net->stakeholders[j]->type)same++;else diff++;}}return (same+diff)>0?(double)same/(same+diff):0.0;}

/* ===== Core-periphery structure detection in stakeholder network ===== */
void stk_net_core_periphery(StakeholderNetwork* net,int* is_core,int* n_core){*n_core=0;if(!net||!is_core||net->n_stakeholders<2)return;double* degree=calloc(net->n_stakeholders,sizeof(double));for(int i=0;i<net->n_stakeholders;i++){for(int j=0;j<net->n_stakeholders;j++)if(i!=j&&fabs(net->influence_matrix[i][j])>1e-6)degree[i]++;}double avg_deg=0.0;for(int i=0;i<net->n_stakeholders;i++)avg_deg+=degree[i];avg_deg/=net->n_stakeholders;for(int i=0;i<net->n_stakeholders;i++){if(degree[i]>=avg_deg){is_core[i]=1;(*n_core)++;}else is_core[i]=0;}free(degree);}

/* ===== Stakeholder stress: disagreement between desired and actual outcomes ===== */
double stk_stress_level(const Stakeholder* s,double* actual_outcomes,int n_outcomes){if(!s||!actual_outcomes||n_outcomes<1)return 0.0;double stress=0.0;for(int i=0;i<n_outcomes&&i<s->n_requirements;i++){double desired=s->requirement_weights?s->requirement_weights[i]:0.5;stress+=fabs(desired-actual_outcomes[i]);}return stress/n_outcomes;}

/* ===== Latent stakeholder identification: find unengaged but affected parties ===== */
double stk_latent_influence_score(StakeholderNetwork* net,int* latent_candidates,int* n_latent,int max_latent){*n_latent=0;if(!net||!latent_candidates||max_latent<1)return 0.0;double total_impact=0.0;for(int i=0;i<net->n_stakeholders;i++){double impact=net->stakeholders[i]->influence*net->stakeholders[i]->interest;if(impact<0.3&&impact>0.05&&*n_latent<max_latent){latent_candidates[*n_latent]=i;(*n_latent)++;total_impact+=impact;}}return total_impact;}

/* ===== Stakeholder diversity index using Blau index ===== */
double stk_net_blau_index(StakeholderNetwork* net){if(!net||net->n_stakeholders<1)return 0.0;int type_counts[6]={0};double n=net->n_stakeholders;for(int i=0;i<net->n_stakeholders;i++){StakeholderType t=net->stakeholders[i]->type;if(t>=0&&t<6)type_counts[t]++;}double blau=1.0;for(int t=0;t<6;t++){double p=type_counts[t]/n;blau-=p*p;}return blau;}

/* ===== Power law exponent of stakeholder influence distribution ===== */
double stk_net_power_law_exponent(StakeholderNetwork* net){if(!net||net->n_stakeholders<2)return 0.0;double sum_log=0.0,sum_log_x=0.0;for(int i=0;i<net->n_stakeholders;i++){double x=fmax(net->stakeholders[i]->influence,1e-10);sum_log+=log(x);}double n=net->n_stakeholders;double avg_log=sum_log/n;double var_log=0.0;for(int i=0;i<net->n_stakeholders;i++){double d=log(fmax(net->stakeholders[i]->influence,1e-10))-avg_log;var_log+=d*d;}return 1.0+n/var_log;}

/* ===== Stakeholder engagement effectiveness model ===== */
double stk_engagement_effectiveness(const Stakeholder* s,double engagement_intensity,double duration){if(!s)return 0.0;return s->satisfaction+(1.0-s->satisfaction)*(1.0-exp(-engagement_intensity*duration));}

/* ===== Stakeholder risk exposure index based on salience and stance ===== */
double stk_risk_exposure(const Stakeholder* s,double project_value){if(!s)return 0.0;if(s->stance==INFLUENCE_POSITIVE)return 0.0;if(s->stance==INFLUENCE_NEUTRAL)return stk_salience(s)*project_value*0.3;return stk_salience(s)*project_value;}

/* ===== Betweenness centrality for stakeholder influence paths ===== */
void stk_betweenness_path_count(StakeholderNetwork* net,int source,int target,int* path_count,double* influence_sum){*path_count=0;*influence_sum=0.0;if(!net||source<0||target<0)return;int n=net->n_stakeholders;int* visited=calloc(n,sizeof(int));double* dist=calloc(n,sizeof(double));int* paths=calloc(n,sizeof(int));for(int i=0;i<n;i++)dist[i]=1e9;dist[source]=0.0;paths[source]=1;for(int iter=0;iter<n;iter++){int u=-1;double min_d=1e9;for(int i=0;i<n;i++)if(!visited[i]&&dist[i]<min_d){min_d=dist[i];u=i;}if(u<0||u==target)break;visited[u]=1;for(int v=0;v<n;v++)if(!visited[v]&&fabs(net->influence_matrix[u][v])>1e-6){double new_d=dist[u]+1.0;if(new_d<dist[v]-1e-6){dist[v]=new_d;paths[v]=paths[u];}else if(fabs(new_d-dist[v])<1e-6)paths[v]+=paths[u];}}*path_count=paths[target];*influence_sum=dist[target];free(visited);free(dist);free(paths);}

/* ===== Structural equivalence: stakeholders with similar network positions ===== */
double stk_structural_equivalence(StakeholderNetwork* net,int a,int b){if(!net||a<0||b<0||a>=net->n_stakeholders||b>=net->n_stakeholders)return 0.0;double dist=0.0;for(int k=0;k<net->n_stakeholders;k++){if(k==a||k==b)continue;double da=fabs(net->influence_matrix[a][k])+fabs(net->influence_matrix[k][a]);double db=fabs(net->influence_matrix[b][k])+fabs(net->influence_matrix[k][b]);dist+=(da-db)*(da-db);}return 1.0/(1.0+sqrt(dist));}

/* ===== Average path length ignoring disconnected pairs ===== */
double stk_net_average_path(StakeholderNetwork* net){if(!net||net->n_stakeholders<2)return 0.0;int n=net->n_stakeholders;double total=0.0;int paths=0;for(int i=0;i<n;i++)for(int j=i+1;j<n;j++){int cnt;double sum;stk_betweenness_path_count(net,i,j,&cnt,&sum);if(cnt>0){total+=sum;paths++;}}return paths>0?total/paths:0.0;}

/* ===== Flow betweenness: fraction of max-flow that passes through a node ===== */
void stk_flow_betweenness(StakeholderNetwork* net,double* flow_scores){if(!net||!flow_scores)return;int n=net->n_stakeholders;for(int v=0;v<n;v++){flow_scores[v]=0.0;for(int s=0;s<n;s++)for(int t=0;t<n;t++)if(s!=t&&s!=v&&t!=v){double st_flow=net->influence_matrix[s][t];flow_scores[v]+=fabs(st_flow);}}double total=0.0;for(int v=0;v<n;v++)total+=flow_scores[v];if(total>1e-10)for(int v=0;v<n;v++)flow_scores[v]/=total;}

/* ===== Radial graph drawing coordinates using spring embedding ===== */
void stk_spring_layout(StakeholderNetwork* net,double* x,double* y,int n_iter){if(!net||!x||!y||n_iter<1)return;int n=net->n_stakeholders;for(int i=0;i<n;i++){x[i]=(double)rand()/RAND_MAX;y[i]=(double)rand()/RAND_MAX;}for(int iter=0;iter<n_iter;iter++){for(int i=0;i<n;i++){double fx=0.0,fy=0.0;for(int j=0;j<n;j++){if(i==j)continue;double dx=x[i]-x[j],dy=y[i]-y[j];double d=sqrt(dx*dx+dy*dy);if(d<1e-6)d=1e-6;double force=1.0/(d*d);fx+=force*dx/d;fy+=force*dy/d;}x[i]+=fx*0.01;y[i]+=fy*0.01;}}}

/* ===== Stakeholder map: 2D projection using power vs interest axes ===== */
void stk_power_interest_map(StakeholderNetwork* net,double* x,double* y){if(!net||!x||!y)return;for(int i=0;i<net->n_stakeholders;i++){x[i]=net->stakeholders[i]->power;y[i]=net->stakeholders[i]->interest;}}

/* ===== Stakeholder network transitivity ratio ===== */
double stk_net_transitivity(StakeholderNetwork* net){if(!net||net->n_stakeholders<3)return 0.0;int triads=0,transitive=0;for(int i=0;i<net->n_stakeholders;i++)for(int j=0;j<net->n_stakeholders;j++)for(int k=0;k<net->n_stakeholders;k++){if(i==j||j==k||i==k)continue;int e_ij=fabs(net->influence_matrix[i][j])>1e-6,e_jk=fabs(net->influence_matrix[j][k])>1e-6,e_ik=fabs(net->influence_matrix[i][k])>1e-6;if(e_ij&&e_jk){triads++;if(e_ik)transitive++;}}return triads>0?(double)transitive/triads:0.0;}

/* ===== Stakeholder power asymmetry (Gini of influence distribution) ===== */
double stk_power_asymmetry(StakeholderNetwork* net){if(!net||net->n_stakeholders<2)return 0.0;int n=net->n_stakeholders;double* inf=calloc(n,sizeof(double));for(int i=0;i<n;i++)inf[i]=net->stakeholders[i]->influence;for(int i=0;i<n-1;i++)for(int j=i+1;j<n;j++)if(inf[j]>inf[i]){double t=inf[i];inf[i]=inf[j];inf[j]=t;}double sum=0.0;for(int i=0;i<n;i++)sum+=inf[i]*(n-i);double total=0.0;for(int i=0;i<n;i++)total+=inf[i];double gini=total>1e-10?(n+1.0-2.0*sum/total)/n:0.0;free(inf);return gini;}

/* End of stakeholder_model.c */

/* ── Stakeholder Salience Model (Mitchell, Agle & Wood, 1997) ────
 * Salience = weighted combination of power, legitimacy, urgency.
 * High salience stakeholders demand priority attention. */
double stk_salience_mitchell(const Stakeholder* s) {
    if (!s) return 0.0;
    return 0.4 * s->power + 0.3 * s->legitimacy + 0.3 * s->urgency;
}

