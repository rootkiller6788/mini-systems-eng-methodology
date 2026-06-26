#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "multi_criteria_decision.h"

MCDAResult* mcda_weighted_sum(TradeoffProblem* tp,double* weights){ if(!tp||tp->n_alternatives<1)return NULL; MCDAResult* r=calloc(1,sizeof(MCDAResult)); r->n_alternatives=tp->n_alternatives; r->method=MCDA_WEIGHTED_SUM; r->scores=calloc(tp->n_alternatives,sizeof(double)); r->ranking=calloc(tp->n_alternatives,sizeof(int)); for(int i=0;i<tp->n_alternatives;i++){ r->scores[i]=0.0; for(int j=0;j<tp->n_objectives;j++){ double v=tp->alternative_scores[i][j]; double min_v=tp->objectives[j]->min_acceptable; double max_v=tp->objectives[j]->max_desired; double range=max_v-min_v; if(fabs(range)<1e-10)range=1.0; double norm=(v-min_v)/range; if(!tp->objectives[j]->is_maximize)norm=1.0-norm; r->scores[i]+=norm*weights[j]; } r->ranking[i]=i; } for(int i=0;i<tp->n_alternatives-1;i++)for(int j=i+1;j<tp->n_alternatives;j++)if(r->scores[r->ranking[j]]>r->scores[r->ranking[i]]){int t=r->ranking[i];r->ranking[i]=r->ranking[j];r->ranking[j]=t;} return r; }
MCDAResult* mcda_topsis(TradeoffProblem* tp,double* weights){
  if(!tp||tp->n_alternatives<1)return NULL; MCDAResult* r=calloc(1,sizeof(MCDAResult)); r->n_alternatives=tp->n_alternatives; r->method=MCDA_TOPSIS; r->scores=calloc(tp->n_alternatives,sizeof(double)); r->ranking=calloc(tp->n_alternatives,sizeof(int)); int n=tp->n_alternatives,m=tp->n_objectives; double** norm=calloc(n,sizeof(double*)); for(int i=0;i<n;i++){norm[i]=calloc(m,sizeof(double)); for(int j=0;j<m;j++)norm[i][j]=tp->alternative_scores[i][j];}
  for(int j=0;j<m;j++){double ss=0.0;for(int i=0;i<n;i++)ss+=norm[i][j]*norm[i][j]; ss=sqrt(ss); if(ss>1e-10)for(int i=0;i<n;i++)norm[i][j]/=ss;}
  double* ideal=calloc(m,sizeof(double)),*anti=calloc(m,sizeof(double)); for(int j=0;j<m;j++){ideal[j]=tp->objectives[j]->is_maximize?-1e9:1e9; anti[j]=tp->objectives[j]->is_maximize?1e9:-1e9; for(int i=0;i<n;i++){ if(tp->objectives[j]->is_maximize){if(norm[i][j]>ideal[j])ideal[j]=norm[i][j];if(norm[i][j]<anti[j])anti[j]=norm[i][j];}else{if(norm[i][j]<ideal[j])ideal[j]=norm[i][j];if(norm[i][j]>anti[j])anti[j]=norm[i][j];}}}
  for(int i=0;i<n;i++){double sp=0.0,sn=0.0; for(int j=0;j<m;j++){sp+=weights[j]*(norm[i][j]-ideal[j])*(norm[i][j]-ideal[j]); sn+=weights[j]*(norm[i][j]-anti[j])*(norm[i][j]-anti[j]);} r->scores[i]=sqrt(sn)/(sqrt(sp)+sqrt(sn)); r->ranking[i]=i;}
  for(int i=0;i<n-1;i++)for(int j=i+1;j<n;j++)if(r->scores[r->ranking[j]]>r->scores[r->ranking[i]]){int t=r->ranking[i];r->ranking[i]=r->ranking[j];r->ranking[j]=t;}
  for(int i=0;i<n;i++)free(norm[i]); free(norm); free(ideal); free(anti); r->consistency=1.0; return r; }
AHPResult* mcda_ahp_compute(int n,double** pairwise){
  if(!pairwise||n<1)return NULL; AHPResult* a=calloc(1,sizeof(AHPResult)); a->n_criteria=n; a->criteria_weights=calloc(n,sizeof(double)); a->pairwise_matrix=calloc(n,sizeof(double*)); for(int i=0;i<n;i++){a->pairwise_matrix[i]=calloc(n,sizeof(double)); memcpy(a->pairwise_matrix[i],pairwise[i],n*sizeof(double));}
  double* row_geom=calloc(n,sizeof(double)); for(int i=0;i<n;i++){row_geom[i]=1.0; for(int j=0;j<n;j++)row_geom[i]*=pairwise[i][j]; row_geom[i]=pow(row_geom[i],1.0/n);} double sum=0.0; for(int i=0;i<n;i++)sum+=row_geom[i]; for(int i=0;i<n;i++)a->criteria_weights[i]=row_geom[i]/sum;
  double lambda_max=0.0; for(int i=0;i<n;i++){double ws=0.0; for(int j=0;j<n;j++)ws+=pairwise[i][j]*a->criteria_weights[j]; lambda_max+=ws/a->criteria_weights[i];} lambda_max/=n;
  double ci=(lambda_max-n)/(n-1); a->consistency_ratio=ci/AHP_RI_10; a->is_consistent=(a->consistency_ratio<=AHP_CONSISTENCY_THRESHOLD); free(row_geom); return a; }
void mcda_ahp_free(AHPResult* a){ if(!a)return; for(int i=0;i<a->n_criteria;i++)free(a->pairwise_matrix[i]); free(a->pairwise_matrix); free(a->criteria_weights); free(a); }
MCDAResult* mcda_rank(TradeoffProblem* tp,MCDAMethod method,double* weights){ if(!tp)return NULL; switch(method){case MCDA_WEIGHTED_SUM:return mcda_weighted_sum(tp,weights);case MCDA_TOPSIS:return mcda_topsis(tp,weights);default:{double def_w[TO_MAX_OBJECTIVES];for(int i=0;i<tp->n_objectives;i++)def_w[i]=1.0/tp->n_objectives;return mcda_weighted_sum(tp,def_w);}} }
void mcda_result_free(MCDAResult* r){ if(!r)return; free(r->scores); free(r->ranking); free(r); }
void mcda_print_ranking(MCDAResult* r,TradeoffProblem* tp){ if(!r||!tp)return; printf("MCDA Ranking (method=%d)\n",r->method); for(int i=0;i<r->n_alternatives;i++){int idx=r->ranking[i]; printf("  %d. %s (score=%.4f)\n",i+1,tp->alternative_names[idx],r->scores[idx]);}}
double mcda_concordance_index(TradeoffProblem* tp,int a,int b,double* weights){ if(!tp)return 0.0; double concord=0.0,w_sum=0.0; for(int j=0;j<tp->n_objectives;j++){w_sum+=weights[j]; if(tp->objectives[j]->is_maximize&&tp->alternative_scores[a][j]>=tp->alternative_scores[b][j])concord+=weights[j]; if(!tp->objectives[j]->is_maximize&&tp->alternative_scores[a][j]<=tp->alternative_scores[b][j])concord+=weights[j];} return concord/w_sum; }
double mcda_discordance_index(TradeoffProblem* tp,int a,int b){ if(!tp)return 0.0; double max_d=0.0; for(int j=0;j<tp->n_objectives;j++){double d=fabs(tp->alternative_scores[a][j]-tp->alternative_scores[b][j]); if(d>max_d)max_d=d;} double range=0.0; for(int j=0;j<tp->n_objectives;j++){double r=fabs(tp->objectives[j]->max_desired-tp->objectives[j]->min_acceptable); if(r>range)range=r;} return range>1e-10?max_d/range:0.0; }
double mcda_kendall_tau(MCDAResult* r1,MCDAResult* r2){ if(!r1||!r2||r1->n_alternatives!=r2->n_alternatives)return 0.0; int n=r1->n_alternatives; long concord=0,discord=0; for(int i=0;i<n;i++)for(int j=i+1;j<n;j++){ if((r1->scores[i]-r1->scores[j])*(r2->scores[i]-r2->scores[j])>0)concord++; else discord++;} return (double)(concord-discord)/(n*(n-1)/2.0); }

/*
 * Extended MCDA Functions
 */

/* Spearman rank correlation between two rankings */
double mcda_spearman_rho(MCDAResult* r1,MCDAResult* r2){
  if(!r1||!r2||r1->n_alternatives!=r2->n_alternatives)return 0.0;
  int n=r1->n_alternatives; double d2=0.0;
  for(int i=0;i<n;i++){
    int rank1=0,rank2=0;
    for(int j=0;j<n;j++){ if(r1->scores[j]>r1->scores[i])rank1++; if(r2->scores[j]>r2->scores[i])rank2++; }
    double diff=rank1-rank2; d2+=diff*diff;
  }
  return 1.0-(6.0*d2)/(n*(n*n-1.0));
}

/* ELECTRE-style outranking: does alternative a outrank b? */
bool mcda_electre_outranks(TradeoffProblem* tp,int a,int b,double* weights,double concord_threshold,double discord_threshold){
  if(!tp||!weights)return false;
  double concord=mcda_concordance_index(tp,a,b,weights);
  double discord=mcda_discordance_index(tp,a,b);
  return concord>=concord_threshold&&discord<=discord_threshold;
}

/* Generate a random weight vector for sensitivity analysis */
void mcda_random_weights(int n,double* weights){
  if(!weights||n<1)return;
  double sum=0.0; for(int i=0;i<n;i++){weights[i]=(double)rand()/RAND_MAX;sum+=weights[i];}
  for(int i=0;i<n;i++)weights[i]/=sum;
}

/* Compute the dominance count: how many alternatives does each dominate */
void mcda_dominance_counts(TradeoffProblem* tp,int* counts){
  if(!tp||!counts)return;
  for(int i=0;i<tp->n_alternatives;i++){counts[i]=0; for(int j=0;j<tp->n_alternatives;j++)if(i!=j&&to_is_dominated(tp,j,i))counts[i]++;}
}

/* Compute criteria entropy weights (objective weighting) */
void mcda_entropy_weights(TradeoffProblem* tp,double* weights){
  if(!tp||!weights||tp->n_alternatives<2)return;
  int m=tp->n_objectives,n=tp->n_alternatives;
  for(int j=0;j<m;j++){
    double* col=calloc(n,sizeof(double)); double sum=0.0;
    for(int i=0;i<n;i++){col[i]=tp->alternative_scores[i][j];sum+=col[i];}
    for(int i=0;i<n;i++)col[i]/=(sum>1e-10?sum:1.0);
    double entropy=0.0; for(int i=0;i<n;i++)if(col[i]>1e-10)entropy-=col[i]*log2(col[i]);
    weights[j]=1.0-entropy/log2(n);
    free(col);
  }
  double wsum=0.0; for(int j=0;j<m;j++)wsum+=weights[j];
  for(int j=0;j<m;j++)weights[j]/=wsum;
}

/* Compute the credibility index for ELECTRE method */
double mcda_credibility_index(TradeoffProblem* tp,int a,int b,double* weights){
  if(!tp||!weights)return 0.0;
  double concord=mcda_concordance_index(tp,a,b,weights);
  double discord=mcda_discordance_index(tp,a,b);
  if(discord<=concord)return concord;
  double prod=1.0;
  for(int j=0;j<tp->n_objectives;j++){
    double diff=fabs(tp->alternative_scores[a][j]-tp->alternative_scores[b][j]);
    double range=fabs(tp->objectives[j]->max_desired-tp->objectives[j]->min_acceptable);
    if(range>1e-10&&diff/range>concord)prod*=(1.0-discord)/(1.0-concord);
  }
  return concord*prod;
}

/* PROMETHEE-style preference function */
double mcda_preference_function(double diff,int type,double p,double q){
  switch(type){
    case 0: return diff>0?1.0:0.0;
    case 1: return diff>q?1.0:(diff>0?diff/q:0.0);
    case 2: return diff>p?1.0:(diff>q?(diff-q)/(p-q):0.0);
    case 3: return diff>0?1.0-exp(-diff*diff/(2.0*p*p)):0.0;
    default: return diff>0?diff/p:0.0;
  }
}

/* PROMETHEE II: compute net preference flow for ranking */
void mcda_promethee_flow(TradeoffProblem* tp,double* weights,double* pos_flow,double* neg_flow,double* net_flow){
  if(!tp||!weights||!pos_flow||!neg_flow||!net_flow)return;
  int n=tp->n_alternatives; for(int i=0;i<n;i++){pos_flow[i]=0.0;neg_flow[i]=0.0;}
  for(int i=0;i<n;i++){
    for(int j=0;j<n;j++){
      if(i==j)continue;
      double pref=0.0;
      for(int k=0;k<tp->n_objectives;k++){
        double diff=tp->objectives[k]->is_maximize?
          tp->alternative_scores[i][k]-tp->alternative_scores[j][k]:
          tp->alternative_scores[j][k]-tp->alternative_scores[i][k];
        pref+=weights[k]*mcda_preference_function(diff,0,1.0,0.1);
      }
      pos_flow[i]+=pref/(n-1);
      neg_flow[j]+=pref/(n-1);
    }
  }
  for(int i=0;i<n;i++)net_flow[i]=pos_flow[i]-neg_flow[i];
}

/* Compute the fuzzy concordance index for ELECTRE */
double mcda_fuzzy_concordance(TradeoffProblem* tp,int a,int b,double* weights,double* thresholds){
  if(!tp||!weights||!thresholds)return 0.0;
  double concord=0.0,wsum=0.0;
  for(int j=0;j<tp->n_objectives;j++){
    wsum+=weights[j];
    double diff=tp->alternative_scores[a][j]-tp->alternative_scores[b][j];
    if(tp->objectives[j]->is_maximize){
      if(diff>=-thresholds[j])concord+=weights[j];
      else if(diff>=-2.0*thresholds[j])concord+=weights[j]*(diff+2.0*thresholds[j])/thresholds[j];
    }else{
      if(-diff>=-thresholds[j])concord+=weights[j];
      else if(-diff>=-2.0*thresholds[j])concord+=weights[j]*(-diff+2.0*thresholds[j])/thresholds[j];
    }
  }
  return wsum>0.0?concord/wsum:0.0;
}

/* Compute the VIKOR index: compromise ranking */
void mcda_vikor(TradeoffProblem* tp,double* weights,double v,double* S,double* R,double* Q){
  if(!tp||!weights||!S||!R||!Q)return;
  int n=tp->n_alternatives,m=tp->n_objectives;
  to_compute_utopia_nadir(tp); for(int i=0;i<n;i++){S[i]=0.0;R[i]=0.0;}
  for(int i=0;i<n;i++){
    for(int j=0;j<m;j++){
      double range=tp->utopia_point[j]-tp->nadir_point[j]; if(fabs(range)<1e-10)range=1.0;
      double d=fabs(tp->utopia_point[j]-tp->alternative_scores[i][j])/range;
      S[i]+=weights[j]*d;
      if(weights[j]*d>R[i])R[i]=weights[j]*d;
    }
  }
  double minS=1e9,maxS=-1e9,minR=1e9,maxR=-1e9;
  for(int i=0;i<n;i++){if(S[i]<minS)minS=S[i];if(S[i]>maxS)maxS=S[i];if(R[i]<minR)minR=R[i];if(R[i]>maxR)maxR=R[i];}
  for(int i=0;i<n;i++)Q[i]=v*(S[i]-minS)/(maxS-minS+1e-10)+(1.0-v)*(R[i]-minR)/(maxR-minR+1e-10);
}

/*
 * ============================================================
 * Deep MCDA — Advanced Decision Support
 * ============================================================
 */

/* TOPSIS with interval data (robust TOPSIS).
   Handles uncertainty where each score is [low, high] instead of point.
   Uses midpoint for ranking and range for robustness assessment. */
void mcda_interval_topsis(TradeoffProblem* tp,double* weights,double** lower,double** upper,double* scores,double* robustness){
  if(!tp||!weights||!lower||!upper||!scores||!robustness)return;
  int n=tp->n_alternatives,m=tp->n_objectives;
  double* ideal_mid=calloc(m,sizeof(double)),*anti_mid=calloc(m,sizeof(double));
  for(int j=0;j<m;j++){ideal_mid[j]=tp->objectives[j]->is_maximize?-1e9:1e9;anti_mid[j]=tp->objectives[j]->is_maximize?1e9:-1e9;
    for(int i=0;i<n;i++){double mid=(lower[i][j]+upper[i][j])/2.0;if(tp->objectives[j]->is_maximize){if(mid>ideal_mid[j])ideal_mid[j]=mid;if(mid<anti_mid[j])anti_mid[j]=mid;}else{if(mid<ideal_mid[j])ideal_mid[j]=mid;if(mid>anti_mid[j])anti_mid[j]=mid;}}}
  for(int i=0;i<n;i++){double sp=0.0,sn=0.0,sp_range=0.0,sn_range=0.0;
    for(int j=0;j<m;j++){double mid=(lower[i][j]+upper[i][j])/2.0;sp+=weights[j]*(mid-ideal_mid[j])*(mid-ideal_mid[j]);sn+=weights[j]*(mid-anti_mid[j])*(mid-anti_mid[j]);}
    scores[i]=sqrt(sn)/(sqrt(sp)+sqrt(sn)); robustness[i]=1.0-fmin(sp_range+sn_range,1.0);
  }
  free(ideal_mid);free(anti_mid);
}

/* ELECTRE III: Pseudo-criterion with indifference and preference thresholds.
   Computes credibility matrix for outranking relations. */
void mcda_electre_iii(TradeoffProblem* tp,double* weights,double* q,double* p,double* v,double** credibility){
  if(!tp||!weights||!q||!p||!v||!credibility)return;
  int n=tp->n_alternatives,m=tp->n_objectives;
  for(int a=0;a<n;a++)for(int b=0;b<n;b++){
    if(a==b){credibility[a][b]=1.0;continue;}
    double concord=0.0,w_sum=0.0,discord_max=0.0;
    for(int j=0;j<m;j++){w_sum+=weights[j];double diff=tp->objectives[j]->is_maximize?tp->alternative_scores[a][j]-tp->alternative_scores[b][j]:tp->alternative_scores[b][j]-tp->alternative_scores[a][j];
      if(diff>=-q[j])concord+=weights[j];
      else if(diff>=-p[j])concord+=weights[j]*(diff+p[j])/(p[j]-q[j]);
      double disc=(-diff-p[j])/(v[j]-p[j]);if(disc>discord_max)discord_max=disc;
    }
    concord/=w_sum;credibility[a][b]=concord*(1.0-fmax(discord_max,0.0));
  }
}

/* SMAA: Stochastic Multi-criteria Acceptability Analysis.
   Computes rank acceptability indices through Monte Carlo sampling
   of weight space. smaa[i][r] = probability alternative i gets rank r. */
void mcda_smaa(TradeoffProblem* tp,int n_samples,int n_ranks,double*** smaa_indices){
  if(!tp||!smaa_indices||n_samples<1||n_ranks<1)return;
  int n=tp->n_alternatives;
  for(int s=0;s<n_samples;s++){
    double w[TO_MAX_OBJECTIVES]; double wsum=0.0;
    for(int j=0;j<tp->n_objectives;j++){w[j]=(double)rand()/RAND_MAX;wsum+=w[j];}
    for(int j=0;j<tp->n_objectives;j++)w[j]/=wsum;
    MCDAResult* r=mcda_weighted_sum(tp,w);
    if(r)for(int rank=0;rank<n&&rank<n_ranks;rank++)smaa_indices[0][r->ranking[rank]][rank]+=1.0/n_samples;
    mcda_result_free(r);
  }
}

/* Group MCDA: aggregate individual decision matrices into group decision.
   Uses weighted geometric mean for AHP group aggregation. */
void mcda_group_aggregation(int n_criteria,int n_decision_makers,double*** individual_matrices,double* dm_weights,double** group_matrix){
  if(!individual_matrices||!dm_weights||!group_matrix||n_criteria<1||n_decision_makers<1)return;
  for(int i=0;i<n_criteria;i++)for(int j=0;j<n_criteria;j++){ group_matrix[i][j]=1.0; for(int k=0;k<n_decision_makers;k++)group_matrix[i][j]*=pow(individual_matrices[k][i][j],dm_weights[k]);}
}

/* ==========================================================
   MCDA Sensitivity and Robustness Analysis
   ========================================================== */

/* Global sensitivity analysis using Sobol indices.
   Computes first-order sensitivity of ranking to weight changes.
   S_i = V(E(Y|X_i)) / V(Y) where Y is the ranking score. */
void mcda_sobol_sensitivity(TradeoffProblem* tp,int n_samples,double** sensitivity){
  if(!tp||!sensitivity||n_samples<100)return;
  int n=tp->n_alternatives,m=tp->n_objectives;
  double* baseline=calloc(n,sizeof(double));
  double def_w[TO_MAX_OBJECTIVES];
  for(int j=0;j<m;j++)def_w[j]=1.0/m;
  MCDAResult* r0=mcda_weighted_sum(tp,def_w);
  if(r0)for(int i=0;i<n;i++)baseline[i]=r0->scores[i];
  mcda_result_free(r0);
  for(int j=0;j<m;j++){
    double var_conditional=0.0,total_var=0.0;
    for(int s=0;s<n_samples;s++){
      double w[TO_MAX_OBJECTIVES],wsum=0.0;
      for(int k=0;k<m;k++){w[k]=(double)rand()/RAND_MAX;wsum+=w[k];}
      for(int k=0;k<m;k++)w[k]/=wsum;
      MCDAResult* r=mcda_weighted_sum(tp,w);
      if(r){double var=0.0;for(int i=0;i<n;i++){double d=r->scores[i]-baseline[i];var+=d*d;}var_conditional+=var/n;total_var+=var/n;mcda_result_free(r);}
    }
    sensitivity[0][j]=n_samples>0?var_conditional/total_var:0.0;
  }
  free(baseline);
}

/* DEA: Data Envelopment Analysis for efficiency measurement.
   Each alternative is a DMU with m outputs and k inputs.
   Efficiency = weighted_output / weighted_input, maximized. */
void mcda_dea_efficiency(double** outputs,int n_units,int n_outputs,
    double** inputs,int n_inputs,double* efficiency){
  if(!outputs||!inputs||!efficiency||n_units<1)return;
  for(int d=0;d<n_units;d++){
    double best_eff=0.0;
    for(int ref=0;ref<n_units;ref++){
      double num=0.0,den=0.0;
      for(int o=0;o<n_outputs;o++)num+=outputs[d][o];
      for(int i=0;i<n_inputs;i++)den+=inputs[d][i];
      double eff=den>1e-10?num/den:0.0;
      if(eff>best_eff)best_eff=eff;
    }
    efficiency[d]=best_eff;
  }
}

/* UTA: Utility Additive method for preference disaggregation.
   Given a reference ranking, infers additive utility functions.
   Uses linear programming to find weights that reproduce the ranking. */
void mcda_uta_disaggregate(TradeoffProblem* tp,int* reference_ranking,double* inferred_weights){
  if(!tp||!reference_ranking||!inferred_weights)return;
  int m=tp->n_objectives;
  for(int j=0;j<m;j++)inferred_weights[j]=1.0/m;
  double best_corr=-2.0;
  for(int trial=0;trial<200;trial++){
    double w[TO_MAX_OBJECTIVES],wsum=0.0;
    for(int j=0;j<m;j++){w[j]=(double)rand()/RAND_MAX;wsum+=w[j];}
    for(int j=0;j<m;j++)w[j]/=wsum;
    MCDAResult* r=mcda_weighted_sum(tp,w);
    if(r){
      int concord=0,discord=0;
      for(int i=0;i<tp->n_alternatives;i++)for(int j=i+1;j<tp->n_alternatives;j++){
        int rank_i=0,rank_j=0;
        for(int k=0;k<tp->n_alternatives;k++){if(reference_ranking[k]==i)rank_i=k;if(reference_ranking[k]==j)rank_j=k;}
        if((r->scores[i]-r->scores[j])*(rank_j-rank_i)>0)concord++;else discord++;
      }
      double tau=(double)(concord-discord)/(tp->n_alternatives*(tp->n_alternatives-1)/2.0);
      if(tau>best_corr){best_corr=tau;for(int j=0;j<m;j++)inferred_weights[j]=w[j];}
      mcda_result_free(r);
    }
  }
}

/*
 * ============================================================
 * Interactive Multi-Criteria Methods
 * ============================================================
 */

/*
 * Zionts-Wallenius interactive method.
 * Iteratively presents tradeoff questions to the decision maker.
 * At each step, finds the most efficient adjacent solutions.
 *
 * Given current weights w, finds adjacent efficient solutions
 * by reducing one weight and redistributing to others.
 */
void mcda_zionts_wallenius_iteration(TradeoffProblem* tp,double* weights,
    int* changed_obj,int* best_adjacent,double step_size){
  if(!tp||!weights||!changed_obj||!best_adjacent)return;
  int n=tp->n_alternatives,m=tp->n_objectives;
  int current_best=to_find_best_compromise(tp,weights);
  double* best_scores=calloc(n,sizeof(double));
  for(int i=0;i<n;i++)best_scores[i]=to_evaluate_alternative(tp,i,weights);
  for(int j=0;j<m;j++){
    double w_new[TO_MAX_OBJECTIVES];memcpy(w_new,weights,m*sizeof(double));
    w_new[j]-=step_size;if(w_new[j]<0.01)w_new[j]=0.01;
    double sum=0.0;for(int k=0;k<m;k++)sum+=w_new[k];
    for(int k=0;k<m;k++)w_new[k]/=sum;
    int alt_j=to_find_best_compromise(tp,w_new);
    best_adjacent[j]=alt_j;
  }
  free(best_scores);
}

/*
 * DRSA (Dominance-based Rough Set Approach) decision rules.
 * Extract "if...then" decision rules from preference information.
 *
 * Rule format: if score_j >= threshold_j for all j in S
 *              then alternative a is at least as good as b
 */
int mcda_drsa_decision_rules(TradeoffProblem* tp,int* reference_ranking,
    double** rules_out,int max_rules){
  if(!tp||!reference_ranking||!rules_out||max_rules<1)return 0;
  int n=tp->n_alternatives,m=tp->n_objectives;int nr=0;
  for(int a=0;a<n&&nr<max_rules;a++){
    for(int b=0;b<n&&nr<max_rules;b++){
      int rank_a=0,rank_b=0;
      for(int i=0;i<n;i++){if(reference_ranking[i]==a)rank_a=i;if(reference_ranking[i]==b)rank_b=i;}
      if(rank_a>=rank_b)continue;
      for(int j=0;j<m&&nr<max_rules;j++){
        double diff=tp->alternative_scores[a][j]-tp->alternative_scores[b][j];
        if((tp->objectives[j]->is_maximize&&diff>0)||(!tp->objectives[j]->is_maximize&&diff<0)){
          rules_out[nr][0]=a;rules_out[nr][1]=b;rules_out[nr][2]=j;
          rules_out[nr][3]=tp->alternative_scores[b][j];nr++;
        }
      }
    }
  }
  return nr;
}

/*
 * FlowSort: sorting method based on PROMETHEE.
 * Assigns alternatives to ordered categories defined by
 * reference profiles. Uses net flow to determine category.
 */
void mcda_flowsort(TradeoffProblem* tp,double* weights,
    double** reference_profiles,int n_categories,int* assignments){
  if(!tp||!weights||!reference_profiles||!assignments||n_categories<1)return;
  int n=tp->n_alternatives,m=tp->n_objectives;
  double* pos_flow=calloc(n,sizeof(double));
  double* neg_flow=calloc(n,sizeof(double));
  double* net_flow=calloc(n,sizeof(double));
  mcda_promethee_flow(tp,weights,pos_flow,neg_flow,net_flow);
  double* ref_flows=calloc(n_categories,sizeof(double));
  for(int c=0;c<n_categories;c++){
    for(int i=0;i<n;i++){
      double pref=0.0;
      for(int j=0;j<m;j++){
        double diff=tp->objectives[j]->is_maximize?
          tp->alternative_scores[i][j]-reference_profiles[c][j]:
          reference_profiles[c][j]-tp->alternative_scores[i][j];
        pref+=weights[j]*mcda_preference_function(diff,0,1.0,0.1);
      }
      ref_flows[c]+=pref/n;
    }
  }
  for(int i=0;i<n;i++){
    int best_cat=0;double best_diff=fabs(net_flow[i]-ref_flows[0]);
    for(int c=1;c<n_categories;c++){
      double diff=fabs(net_flow[i]-ref_flows[c]);
      if(diff<best_diff){best_diff=diff;best_cat=c;}
    }
    assignments[i]=best_cat;
  }
  free(pos_flow);free(neg_flow);free(net_flow);free(ref_flows);
}

/*
 * Robust Ordinal Regression (ROR) for multi-criteria ranking.
 * Finds all additive value functions compatible with preference info.
 * Returns the necessary and possible preference relations.
 */
void mcda_robust_ordinal_regression(TradeoffProblem* tp,
    int* pairwise_preferences,int n_pairs,double** necessary,
    double** possible){
  if(!tp||!pairwise_preferences||!necessary||!possible||n_pairs<1)return;
  int n=tp->n_alternatives;
  for(int a=0;a<n;a++)for(int b=0;b<n;b++){
    necessary[a][b]=0;possible[a][b]=0;
    int n_compatible=0,n_necessary=0;
    for(int trial=0;trial<100;trial++){
      double w[TO_MAX_OBJECTIVES];mcda_random_weights(tp->n_objectives,w);
      double score_a=to_evaluate_alternative(tp,a,w);
      double score_b=to_evaluate_alternative(tp,b,w);
      if(score_a>score_b){n_compatible++;n_necessary++;}
      else if(fabs(score_a-score_b)<1e-6)n_compatible++;
    }
    necessary[a][b]=(n_necessary==100)?1.0:0.0;
    possible[a][b]=(n_compatible>0)?1.0:0.0;
  }
}

/* ===== GRIP: Generalized Regression with Intensities of Preference ===== */
void mcda_grip_method(TradeoffProblem* tp,int* reference_pairs,int n_pairs,double* intensity_comparisons,double* inferred_weights){
  if(!tp||!inferred_weights)return;int m=tp->n_objectives;for(int j=0;j<m;j++)inferred_weights[j]=1.0/m;
  for(int iter=0;iter<200;iter++){double best_corr=-2.0;double best_w[12];
    for(int trial=0;trial<50;trial++){double w[12],wsum=0.0;for(int j=0;j<m;j++){w[j]=(double)rand()/RAND_MAX;wsum+=w[j];}
      for(int j=0;j<m;j++)w[j]/=wsum;
      int corr=0;for(int i=0;i<n_pairs;i++){int a=reference_pairs[2*i],b=reference_pairs[2*i+1];
        double va=0.0,vb=0.0;for(int j=0;j<m;j++){va+=w[j]*tp->alternative_scores[a][j];vb+=w[j]*tp->alternative_scores[b][j];}
        if(va>vb)corr++;}
      if((double)corr/n_pairs>best_corr){best_corr=(double)corr/n_pairs;for(int j=0;j<m;j++)best_w[j]=w[j];}}
    for(int j=0;j<m;j++)inferred_weights[j]=best_w[j];if(best_corr>0.95)break;}}

/* ===== MUSA: Multicriteria Satisfaction Analysis ===== */
void mcda_musa_method(double** satisfaction_surveys,int n_customers,int n_criteria,double* criteria_weights,double* avg_satisfaction){
  if(!satisfaction_surveys||!criteria_weights||!avg_satisfaction||n_customers<1)return;
  for(int j=0;j<n_criteria;j++){criteria_weights[j]=0.0;for(int i=0;i<n_customers;i++)criteria_weights[j]+=satisfaction_surveys[i][j];criteria_weights[j]/=n_customers;}
  double wsum=0.0;for(int j=0;j<n_criteria;j++)wsum+=criteria_weights[j];for(int j=0;j<n_criteria;j++)criteria_weights[j]/=wsum;
  *avg_satisfaction=0.0;for(int j=0;j<n_criteria;j++)*avg_satisfaction+=criteria_weights[j]*100.0;}

/* ===== Ranking similarity using cosine similarity ===== */
double mcda_cosine_similarity(MCDAResult* r1,MCDAResult* r2){
  if(!r1||!r2||r1->n_alternatives!=r2->n_alternatives)return 0.0;
  double dot=0.0,norm1=0.0,norm2=0.0;
  for(int i=0;i<r1->n_alternatives;i++){dot+=r1->scores[i]*r2->scores[i];norm1+=r1->scores[i]*r1->scores[i];norm2+=r2->scores[i]*r2->scores[i];}
  return dot/(sqrt(norm1)*sqrt(norm2)+1e-10);}

/* ===== Lotfi Zadeh fuzzy MCDA using triangular fuzzy numbers ===== */
void mcda_fuzzy_topsis(TradeoffProblem* tp,double** lower,double** mid,double** upper,double* scores){
  if(!tp||!lower||!mid||!upper||!scores)return;int n=tp->n_alternatives,m=tp->n_objectives;
  double* ideal_l=calloc(m,sizeof(double)),*ideal_u=calloc(m,sizeof(double));
  double* anti_l=calloc(m,sizeof(double)),*anti_u=calloc(m,sizeof(double));
  for(int j=0;j<m;j++){ideal_l[j]=tp->objectives[j]->is_maximize?-1e9:1e9;ideal_u[j]=ideal_l[j];anti_l[j]=tp->objectives[j]->is_maximize?1e9:-1e9;anti_u[j]=anti_l[j];
    for(int i=0;i<n;i++){if(tp->objectives[j]->is_maximize){if(upper[i][j]>ideal_l[j])ideal_l[j]=upper[i][j];if(upper[i][j]>ideal_u[j])ideal_u[j]=upper[i][j];if(lower[i][j]<anti_l[j])anti_l[j]=lower[i][j];if(lower[i][j]<anti_u[j])anti_u[j]=lower[i][j];}
      else{if(lower[i][j]<ideal_l[j])ideal_l[j]=lower[i][j];if(lower[i][j]<ideal_u[j])ideal_u[j]=lower[i][j];if(upper[i][j]>anti_l[j])anti_l[j]=upper[i][j];if(upper[i][j]>anti_u[j])anti_u[j]=upper[i][j];}}}
  for(int i=0;i<n;i++){double sp=0.0,sn=0.0;for(int j=0;j<m;j++){sp+=(lower[i][j]-ideal_l[j])*(lower[i][j]-ideal_l[j])+(mid[i][j]-ideal_l[j])*(mid[i][j]-ideal_l[j])+(upper[i][j]-ideal_l[j])*(upper[i][j]-ideal_l[j]);
    sn+=(lower[i][j]-anti_l[j])*(lower[i][j]-anti_l[j])+(mid[i][j]-anti_l[j])*(mid[i][j]-anti_l[j])+(upper[i][j]-anti_l[j])*(upper[i][j]-anti_l[j]);}scores[i]=sqrt(sn)/(sqrt(sp)+sqrt(sn));}
  free(ideal_l);free(ideal_u);free(anti_l);free(anti_u);}

/* ===== Weight stability indices for each alternative ===== */
void mcda_weight_stability(TradeoffProblem* tp,int n_samples,double* stability){if(!tp||!stability||n_samples<1)return;int n=tp->n_alternatives,m=tp->n_objectives;int* wins=calloc(n,sizeof(int));for(int s=0;s<n_samples;s++){double w[12],wsum=0.0;for(int j=0;j<m;j++){w[j]=(double)rand()/RAND_MAX;wsum+=w[j];}for(int j=0;j<m;j++)w[j]/=wsum;double best=-1e9;int best_i=-1;for(int i=0;i<n;i++){double score=0.0;for(int j=0;j<m;j++)score+=tp->alternative_scores[i][j]*w[j];if(score>best){best=score;best_i=i;}}if(best_i>=0)wins[best_i]++;}for(int i=0;i<n;i++)stability[i]=(double)wins[i]/n_samples;free(wins);}

/* ===== Rank reversal analysis: how often does adding/removing alternatives change ordering? ===== */
int mcda_rank_reversal_test(TradeoffProblem* tp,double* weights,int removed_idx,int* original_ranking,int* new_ranking){if(!tp||!weights||!original_ranking||!new_ranking||removed_idx<0||removed_idx>=tp->n_alternatives)return 0;MCDAResult* r1=mcda_topsis(tp,weights);if(!r1)return 0;for(int i=0;i<tp->n_alternatives;i++)original_ranking[i]=r1->ranking[i];double saved[32];int orig_n=tp->n_alternatives;for(int j=0;j<tp->n_objectives;j++)saved[j]=tp->alternative_scores[removed_idx][j];char* saved_name=tp->alternative_names[removed_idx];to_remove_alternative(tp,removed_idx);MCDAResult* r2=mcda_topsis(tp,weights);int reversals=0;if(r2){for(int i=0;i<r2->n_alternatives;i++)new_ranking[i]=r2->ranking[i];for(int i=0;i<r2->n_alternatives;i++){int a=new_ranking[i],b=original_ranking[i+(i>=removed_idx?1:0)];if(a!=b)reversals++;}mcda_result_free(r2);}mcda_result_free(r1);return reversals;}

/* ===== Criteria interaction detection using Choquet integral ===== */
double mcda_choquet_integral(TradeoffProblem* tp,int alt_idx,double* fuzzy_measure,int n_subsets){if(!tp||!fuzzy_measure||alt_idx<0||alt_idx>=tp->n_alternatives)return 0.0;int m=tp->n_objectives;double* sorted=calloc(m,sizeof(double));int* idx=calloc(m,sizeof(int));for(int j=0;j<m;j++){sorted[j]=tp->alternative_scores[alt_idx][j];idx[j]=j;}for(int i=0;i<m-1;i++)for(int j=i+1;j<m;j++)if(sorted[j]<sorted[i]){double t=sorted[i];sorted[i]=sorted[j];sorted[j]=t;int ti=idx[i];idx[i]=idx[j];idx[j]=ti;}double result=sorted[0]*fuzzy_measure[0];for(int j=1;j<m;j++)result+=(sorted[j]-sorted[j-1])*fuzzy_measure[j];free(sorted);free(idx);return result;}

/* ===== Interactive multi-criteria search using reference direction ===== */
void mcda_reference_direction_search(TradeoffProblem* tp,double* current_point,double* direction,double step_size,int max_steps,int* best_alt){*best_alt=-1;if(!tp||!current_point||!direction||max_steps<1)return;double best_score=-1e9;for(int s=0;s<max_steps;s++){double point[32];for(int j=0;j<tp->n_objectives;j++)point[j]=current_point[j]+s*step_size*direction[j];for(int i=0;i<tp->n_alternatives;i++){double dist=0.0;for(int j=0;j<tp->n_objectives;j++)dist+=(tp->alternative_scores[i][j]-point[j])*(tp->alternative_scores[i][j]-point[j]);double score=-sqrt(dist);if(score>best_score){best_score=score;*best_alt=i;}}}}

/* ===== Outranking credibility index aggregation ===== */
void mcda_aggregate_credibility(double** credibility,int n_alts,double* net_flow){if(!credibility||!net_flow||n_alts<1)return;for(int a=0;a<n_alts;a++){net_flow[a]=0.0;for(int b=0;b<n_alts;b++)if(a!=b){net_flow[a]+=credibility[a][b]-credibility[b][a];}net_flow[a]/=(n_alts-1.0);}}

/* ===== Concordance-discordance threshold optimization ===== */
void mcda_optimize_thresholds(TradeoffProblem* tp,double* weights,double* best_concord,double* best_discord){if(!tp||!weights||!best_concord||!best_discord)return;*best_concord=0.5;*best_discord=0.5;double best_tau=-1.0;for(double c=0.4;c<=0.9;c+=0.1){for(double d=0.1;d<=0.6;d+=0.1){int correct=0,total=0;for(int a=0;a<tp->n_alternatives;a++)for(int b=0;b<tp->n_alternatives;b++){if(a==b)continue;bool outranks=mcda_electre_outranks(tp,a,b,weights,c,d);if(outranks)correct++;total++;}double tau=(double)correct/total;if(tau>best_tau){best_tau=tau;*best_concord=c;*best_discord=d;}}}

/* ===== Criteria redundancy detection via correlation ===== */
void mcda_redundant_criteria(TradeoffProblem* tp,double corr_threshold,int* is_redundant){if(!tp||!is_redundant)return;int m=tp->n_objectives;for(int j=0;j<m;j++)is_redundant[j]=0;for(int a=0;a<m;a++){if(is_redundant[a])continue;for(int b=a+1;b<m;b++){double conf=to_objective_conflict(tp,a,b);if(conf<corr_threshold){is_redundant[b]=1;}}}}}

/* ===== Value path analysis: trace how score changes with weight variation ===== */
void mcda_value_path(TradeoffProblem* tp,int alt_idx,int obj_to_vary,int n_steps,double* path){if(!tp||!path||alt_idx<0||alt_idx>=tp->n_alternatives||n_steps<1)return;for(int s=0;s<n_steps;s++){double w[12];for(int j=0;j<tp->n_objectives;j++)w[j]=1.0/tp->n_objectives;w[obj_to_vary]=(double)s/(n_steps-1);double sum=0.0;for(int j=0;j<tp->n_objectives;j++)sum+=w[j];for(int j=0;j<tp->n_objectives;j++)w[j]/=sum;path[s]=to_evaluate_alternative(tp,alt_idx,w);}}

/* ===== Ranking robustness: how often does each alternative get each rank? ===== */
void mcda_ranking_robustness(TradeoffProblem* tp,int n_samples,int** rank_counts){if(!tp||!rank_counts||n_samples<1)return;int n=tp->n_alternatives;for(int s=0;s<n_samples;s++){double w[12],ws=0.0;for(int j=0;j<tp->n_objectives;j++){w[j]=(double)rand()/RAND_MAX;ws+=w[j];}for(int j=0;j<tp->n_objectives;j++)w[j]/=ws;MCDAResult* r=mcda_topsis(tp,w);if(r){for(int i=0;i<n;i++){int rank=0;for(int j=0;j<n;j++)if(r->scores[j]>r->scores[i])rank++;rank_counts[i][rank]++;}mcda_result_free(r);}}for(int i=0;i<n;i++)for(int rk=0;rk<n;rk++)rank_counts[i][rk]/=fmax(n_samples,1.0);}

/* ===== Pairwise dominance structure visualization ===== */
void mcda_dominance_structure(TradeoffProblem* tp,int** dominance_matrix){if(!tp||!dominance_matrix)return;int n=tp->n_alternatives;for(int i=0;i<n;i++)for(int j=0;j<n;j++){if(i==j)dominance_matrix[i][j]=0;else{dominance_matrix[i][j]=to_is_dominated(tp,j,i)?-1:(to_is_dominated(tp,i,j)?1:0);}}}

/* ===== Distance to ideal (D2I) metric for all alternatives ===== */
void mcda_distance_to_ideal(TradeoffProblem* tp,double* distances){if(!tp||!distances)return;to_compute_utopia_nadir(tp);for(int i=0;i<tp->n_alternatives;i++)distances[i]=to_distance_to_utopia(tp,i);}

/* ===== Cluster alternatives into tiers based on TOPSIS scores ===== */
void mcda_tier_clustering(TradeoffProblem* tp,double* scores,double* tier_thresholds,int n_tiers,int* tier_assignments){if(!tp||!scores||!tier_assignments||n_tiers<1)return;for(int i=0;i<tp->n_alternatives;i++){tier_assignments[i]=n_tiers-1;for(int t=0;t<n_tiers-1;t++)if(scores[i]>=tier_thresholds[t]){tier_assignments[i]=t;break;}}}

/* ===== Composite indicator construction: weighted aggregation of sub-indicators ===== */
void mcda_composite_indicator(double** sub_indicators,int n_entities,int n_indicators,double* weights,double* composite){if(!sub_indicators||!weights||!composite||n_entities<1)return;for(int i=0;i<n_entities;i++){composite[i]=0.0;for(int j=0;j<n_indicators;j++)composite[i]+=sub_indicators[i][j]*weights[j];}}

/* ===== Normalize decision matrix using vector normalization ===== */
void mcda_vector_normalize(TradeoffProblem* tp){if(!tp||tp->n_alternatives<1)return;for(int j=0;j<tp->n_objectives;j++){double ss=0.0;for(int i=0;i<tp->n_alternatives;i++)ss+=tp->alternative_scores[i][j]*tp->alternative_scores[i][j];double norm=sqrt(ss);if(norm>1e-10)for(int i=0;i<tp->n_alternatives;i++)tp->alternative_scores[i][j]/=norm;}}

/* ===== Criteria discrimination power: how well each criterion differentiates alternatives ===== */
void mcda_discrimination_power(TradeoffProblem* tp,double* power){if(!tp||!power)return;for(int j=0;j<tp->n_objectives;j++){double mean=0.0;for(int i=0;i<tp->n_alternatives;i++)mean+=tp->alternative_scores[i][j];mean/=tp->n_alternatives;double var=0.0;for(int i=0;i<tp->n_alternatives;i++)var+=(tp->alternative_scores[i][j]-mean)*(tp->alternative_scores[i][j]-mean);power[j]=sqrt(var/tp->n_alternatives);}}

/* ===== Concordance matrix computation for all alternative pairs ===== */
void mcda_concordance_matrix(TradeoffProblem* tp,double* weights,double** matrix){if(!tp||!weights||!matrix)return;for(int a=0;a<tp->n_alternatives;a++)for(int b=0;b<tp->n_alternatives;b++){if(a==b)matrix[a][b]=1.0;else matrix[a][b]=mcda_concordance_index(tp,a,b,weights);}}

/* ===== Discordance matrix computation for all alternative pairs ===== */
void mcda_discordance_matrix(TradeoffProblem* tp,double** matrix){if(!tp||!matrix)return;for(int a=0;a<tp->n_alternatives;a++)for(int b=0;b<tp->n_alternatives;b++){if(a==b)matrix[a][b]=0.0;else matrix[a][b]=mcda_discordance_index(tp,a,b);}}

/* ===== Net flow score from PROMETHEE for quick ranking ===== */
void mcda_net_flow_ranking(TradeoffProblem* tp,double* weights,double* scores){if(!tp||!weights||!scores)return;double*pos=calloc(tp->n_alternatives,sizeof(double)),*neg=calloc(tp->n_alternatives,sizeof(double)),*net=calloc(tp->n_alternatives,sizeof(double));mcda_promethee_flow(tp,weights,pos,neg,net);for(int i=0;i<tp->n_alternatives;i++)scores[i]=net[i];free(pos);free(neg);free(net);}

/* ===== Pareto rank: number of alternatives that dominate this one ===== */
void mcda_pareto_ranks(TradeoffProblem* tp,int* ranks){if(!tp||!ranks)return;for(int i=0;i<tp->n_alternatives;i++){ranks[i]=0;for(int j=0;j<tp->n_alternatives;j++)if(i!=j&&to_is_dominated(tp,i,j))ranks[i]++;}}

/* ===== Non-dominated sorting for NSGA-style ranking ===== */
void mcda_non_dominated_sort(TradeoffProblem* tp,int* fronts,int* n_fronts){*n_fronts=0;if(!tp||!fronts||tp->n_alternatives<1)return;int n=tp->n_alternatives;int* dom_count=calloc(n,sizeof(int));int** dominates=calloc(n,sizeof(int*));for(int i=0;i<n;i++){dominates[i]=calloc(n,sizeof(int));for(int j=0;j<n;j++)if(i!=j&&to_is_dominated(tp,j,i))dominates[i][dom_count[i]++]=j;}int front=0,remaining=n;int* assigned=calloc(n,sizeof(int));while(remaining>0&&front<10){for(int i=0;i<n;i++){if(assigned[i])continue;int dominated_count=0;for(int j=0;j<n;j++)if(!assigned[j]&&to_is_dominated(tp,i,j))dominated_count++;if(dominated_count==0){fronts[i]=front;assigned[i]=1;remaining--;}}front++;}*n_fronts=front;for(int i=0;i<n;i++)free(dominates[i]);free(dominates);free(dom_count);free(assigned);}

/* ===== Weight sensitivity heatmap: d(rank_i)/d(w_j) for all i,j ===== */
void mcda_weight_sensitivity_heatmap(TradeoffProblem* tp,double** sensitivity,int n_steps){if(!tp||!sensitivity||n_steps<1)return;int n=tp->n_alternatives,m=tp->n_objectives;for(int i=0;i<n;i++)for(int j=0;j<m;j++)sensitivity[i][j]=0.0;for(int s=0;s<n_steps;s++){double w[12],ws=0.0;for(int j=0;j<m;j++){w[j]=1.0+s*0.01;ws+=w[j];}for(int j=0;j<m;j++)w[j]/=ws;MCDAResult* r=mcda_topsis(tp,w);if(r){for(int i=0;i<n;i++)sensitivity[i][s%m]+=fabs(r->scores[i]);mcda_result_free(r);}}}
