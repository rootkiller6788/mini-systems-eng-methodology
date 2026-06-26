#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tradeoff_analysis.h"
#include "multi_criteria_decision.h"

TradeoffProblem* to_create(const char* name,int n_obj,const char** names,double* weights,bool* maximize){
  if(!name||n_obj<1)return NULL; TradeoffProblem* tp=calloc(1,sizeof(TradeoffProblem)); if(!tp)return NULL;
  tp->name=strdup(name); tp->n_objectives=n_obj; tp->objectives=calloc(n_obj,sizeof(Objective*));
  for(int i=0;i<n_obj;i++){ tp->objectives[i]=calloc(1,sizeof(Objective)); tp->objectives[i]->name=names?strdup(names[i]):strdup("obj"); tp->objectives[i]->weight=weights?weights[i]:1.0/n_obj; tp->objectives[i]->is_maximize=maximize?maximize[i]:true; tp->objectives[i]->min_acceptable=0.0; tp->objectives[i]->max_desired=1.0; }
  tp->alternative_scores=NULL; tp->alternative_names=NULL; tp->n_alternatives=0; tp->utopia_point=calloc(n_obj,sizeof(double)); tp->nadir_point=calloc(n_obj,sizeof(double)); return tp; }
void to_free(TradeoffProblem* tp){ if(!tp)return; free(tp->name); for(int i=0;i<tp->n_objectives;i++){free(tp->objectives[i]->name); free(tp->objectives[i]->units); free(tp->objectives[i]);} free(tp->objectives); if(tp->alternative_scores){for(int i=0;i<tp->n_alternatives;i++)free(tp->alternative_scores[i]); free(tp->alternative_scores);} for(int i=0;i<tp->n_alternatives;i++)free(tp->alternative_names[i]); free(tp->alternative_names); free(tp->pareto_front); free(tp->utopia_point); free(tp->nadir_point); free(tp); }
void to_set_objective_limits(TradeoffProblem* tp,int idx,double min,double max,const char* units){ if(!tp||idx<0||idx>=tp->n_objectives)return; tp->objectives[idx]->min_acceptable=min; tp->objectives[idx]->max_desired=max; free(tp->objectives[idx]->units); tp->objectives[idx]->units=units?strdup(units):strdup(""); }
int to_add_alternative(TradeoffProblem* tp,const char* name,double* scores){ if(!tp||!name||!scores||tp->n_alternatives>=TO_MAX_ALTERNATIVES)return -1; int n=tp->n_alternatives; tp->alternative_scores=realloc(tp->alternative_scores,(n+1)*sizeof(double*)); tp->alternative_names=realloc(tp->alternative_names,(n+1)*sizeof(char*)); tp->alternative_scores[n]=calloc(tp->n_objectives,sizeof(double)); memcpy(tp->alternative_scores[n],scores,tp->n_objectives*sizeof(double)); tp->alternative_names[n]=strdup(name); tp->n_alternatives=n+1; return n; }
double to_evaluate_alternative(TradeoffProblem* tp,int idx,double* weights){ if(!tp||idx<0||idx>=tp->n_alternatives)return 0.0; double score=0.0; for(int i=0;i<tp->n_objectives;i++)score+=tp->alternative_scores[idx][i]*weights[i]; return score; }
bool to_is_dominated(TradeoffProblem* tp,int a,int b){ if(!tp)return false; bool worse_in_all=true,better_in_some=false; for(int i=0;i<tp->n_objectives;i++){ if(tp->objectives[i]->is_maximize){ if(tp->alternative_scores[a][i]>tp->alternative_scores[b][i]+TO_EPSILON)worse_in_all=false; if(tp->alternative_scores[a][i]<tp->alternative_scores[b][i]-TO_EPSILON)better_in_some=true; }else{ if(tp->alternative_scores[a][i]<tp->alternative_scores[b][i]-TO_EPSILON)worse_in_all=false; if(tp->alternative_scores[a][i]>tp->alternative_scores[b][i]+TO_EPSILON)better_in_some=true; }} return worse_in_all&&better_in_some; }
void to_compute_pareto_front(TradeoffProblem* tp){
  if(!tp||tp->n_alternatives<1)return; free(tp->pareto_front); tp->pareto_front=calloc(tp->n_alternatives,sizeof(int)); tp->n_pareto=0; bool* dominated=calloc(tp->n_alternatives,sizeof(bool));
  for(int i=0;i<tp->n_alternatives;i++){ for(int j=0;j<tp->n_alternatives;j++)if(i!=j&&to_is_dominated(tp,i,j)){dominated[i]=true;break;} if(!dominated[i])tp->pareto_front[tp->n_pareto++]=i; } free(dominated); }
double to_tradeoff_ratio(TradeoffProblem* tp,int oa,int ob,int alt){ if(!tp||oa<0||ob<0||alt<0||alt>=tp->n_alternatives||oa>=tp->n_objectives||ob>=tp->n_objectives)return 0.0; double denom=fabs(tp->alternative_scores[alt][ob]); return denom>1e-10?fabs(tp->alternative_scores[alt][oa])/denom:0.0; }
int to_find_best_compromise(TradeoffProblem* tp,double* weights){ if(!tp||tp->n_alternatives<1)return -1; double* w=weights; if(!w){ w=calloc(tp->n_objectives,sizeof(double)); for(int i=0;i<tp->n_objectives;i++)w[i]=1.0/tp->n_objectives; } to_compute_utopia_nadir(tp); int best=-1; double best_dist=1e9; for(int i=0;i<tp->n_alternatives;i++){ double d=to_distance_to_utopia(tp,i); if(d<best_dist){best_dist=d;best=i;} } if(!weights)free(w); return best; }
void to_compute_utopia_nadir(TradeoffProblem* tp){ if(!tp||tp->n_alternatives<1)return; for(int i=0;i<tp->n_objectives;i++){ tp->utopia_point[i]=tp->objectives[i]->is_maximize?-1e9:1e9; tp->nadir_point[i]=tp->objectives[i]->is_maximize?1e9:-1e9; for(int j=0;j<tp->n_alternatives;j++){ double v=tp->alternative_scores[j][i]; if(tp->objectives[i]->is_maximize){ if(v>tp->utopia_point[i])tp->utopia_point[i]=v; if(v<tp->nadir_point[i])tp->nadir_point[i]=v; }else{ if(v<tp->utopia_point[i])tp->utopia_point[i]=v; if(v>tp->nadir_point[i])tp->nadir_point[i]=v; }}} }
double to_distance_to_utopia(TradeoffProblem* tp,int alt){ if(!tp||alt<0||alt>=tp->n_alternatives)return 1e9; double d=0.0; for(int i=0;i<tp->n_objectives;i++){ double range=tp->utopia_point[i]-tp->nadir_point[i]; if(fabs(range)<TO_EPSILON)range=1.0; d+=(tp->alternative_scores[alt][i]-tp->utopia_point[i])*(tp->alternative_scores[alt][i]-tp->utopia_point[i])/(range*range); } return sqrt(d); }
void to_print_pareto_front(TradeoffProblem* tp){ if(!tp)return; to_compute_pareto_front(tp); printf("Pareto Front for %s: %d alternatives\n",tp->name,tp->n_pareto); for(int i=0;i<tp->n_pareto;i++){ int idx=tp->pareto_front[i]; printf("  %s: ",tp->alternative_names[idx]); for(int j=0;j<tp->n_objectives;j++)printf("%.3f ",tp->alternative_scores[idx][j]); printf("\n"); }}

/*
 * Extended Tradeoff Analysis Functions
 */

/* Compute the knee point of the Pareto front (maximum curvature) */
int to_find_knee_point(TradeoffProblem* tp){
  if(!tp||tp->n_pareto<3)return tp&&tp->n_pareto>0?tp->pareto_front[0]:-1;
  to_compute_pareto_front(tp);
  double max_curvature=-1.0; int knee=-1;
  for(int i=1;i<tp->n_pareto-1;i++){
    int prev=tp->pareto_front[i-1],curr=tp->pareto_front[i],next=tp->pareto_front[i+1];
    double d1=0.0,d2=0.0; for(int j=0;j<tp->n_objectives;j++){ double v1=tp->alternative_scores[curr][j]-tp->alternative_scores[prev][j]; double v2=tp->alternative_scores[next][j]-tp->alternative_scores[curr][j]; d1+=v1*v1; d2+=v2*v2; }
    double curvature=fabs(sqrt(d1)-sqrt(d2));
    if(curvature>max_curvature){max_curvature=curvature;knee=curr;}
  }
  return knee>=0?knee:tp->pareto_front[0];
}

/* Compute pairwise tradeoff matrix between all alternatives */
void to_tradeoff_matrix(TradeoffProblem* tp,double** matrix){
  if(!tp||!matrix)return;
  for(int i=0;i<tp->n_alternatives;i++)
    for(int j=0;j<tp->n_alternatives;j++){
      if(i==j){matrix[i][j]=0.0;continue;}
      double gain=0.0,loss=0.0;
      for(int k=0;k<tp->n_objectives;k++){
        double diff=tp->alternative_scores[j][k]-tp->alternative_scores[i][k];
        if(tp->objectives[k]->is_maximize){ if(diff>0)gain+=diff; else loss-=diff; }
        else{ if(diff<0)gain-=diff; else loss+=diff; }
      }
      matrix[i][j]=(loss>1e-10)?gain/loss:1e9;
    }
}

/* Normalize all scores to [0,1] range using min-max normalization */
void to_normalize_scores(TradeoffProblem* tp){
  if(!tp||tp->n_alternatives<1)return;
  for(int j=0;j<tp->n_objectives;j++){
    double min_v=1e9,max_v=-1e9;
    for(int i=0;i<tp->n_alternatives;i++){ double v=tp->alternative_scores[i][j]; if(v<min_v)min_v=v; if(v>max_v)max_v=v; }
    double range=max_v-min_v; if(fabs(range)<TO_EPSILON)range=1.0;
    for(int i=0;i<tp->n_alternatives;i++)tp->alternative_scores[i][j]=(tp->alternative_scores[i][j]-min_v)/range;
  }
}

/* Compute the hypervolume of the Pareto front (quality indicator) */
double to_pareto_hypervolume(TradeoffProblem* tp,double* ref_point){
  if(!tp||tp->n_pareto<1)return 0.0;
  to_compute_pareto_front(tp);
  double hv=0.0;
  for(int i=0;i<tp->n_pareto;i++){
    int idx=tp->pareto_front[i]; double vol=1.0;
    for(int j=0;j<tp->n_objectives;j++){
      double margin=ref_point?fabs(ref_point[j]-tp->alternative_scores[idx][j]):1.0;
      vol*=fmax(margin,0.0);
    }
    hv+=vol;
  }
  return hv;
}

/* Sensitivity analysis: how much can weights change before ranking flips */
int to_weight_sensitivity(TradeoffProblem* tp,int alt_idx,double* orig_weights,double* ranges,int n_steps){
  if(!tp||alt_idx<0||alt_idx>=tp->n_alternatives||!orig_weights||!ranges)return 0;
  int flips=0; double step=1.0/n_steps;
  for(int j=0;j<tp->n_objectives&&j<TO_MAX_OBJECTIVES;j++){
    double orig_w=orig_weights[j];
    for(int s=-n_steps;s<=n_steps;s++){
      double new_w=orig_w+s*step*0.1; if(new_w<0.0)new_w=0.0; if(new_w>1.0)new_w=1.0;
      double test_w[TO_MAX_OBJECTIVES]; for(int k=0;k<tp->n_objectives;k++)test_w[k]=orig_weights[k];
      test_w[j]=new_w; double sum=0.0; for(int k=0;k<tp->n_objectives;k++)sum+=test_w[k];
      for(int k=0;k<tp->n_objectives;k++)test_w[k]/=sum;
      double new_score=to_evaluate_alternative(tp,alt_idx,test_w);
      double old_score=to_evaluate_alternative(tp,alt_idx,orig_weights);
      if(fabs(new_score-old_score)>0.01)flips++;
    }
  }
  return flips;
}

/* Find alternatives that are robust (perform well across weight variations) */
int to_find_robust_alternatives(TradeoffProblem* tp,int* robust_idxs,int max_robust){
  if(!tp||!robust_idxs||max_robust<1)return 0;
  int count=0;
  for(int i=0;i<tp->n_alternatives&&count<max_robust;i++){
    double avg_rank=0.0;
    for(int j=0;j<tp->n_objectives;j++){ int better=0; for(int k=0;k<tp->n_alternatives;k++)if(k!=i){ if(tp->objectives[j]->is_maximize&&tp->alternative_scores[i][j]>=tp->alternative_scores[k][j])better++; else if(!tp->objectives[j]->is_maximize&&tp->alternative_scores[i][j]<=tp->alternative_scores[k][j])better++; } avg_rank+=(double)better/(tp->n_alternatives-1); }
    avg_rank/=tp->n_objectives;
    if(avg_rank>0.5){robust_idxs[count]=i;count++;}
  }
  return count;
}

/* Compute the opportunity cost of choosing one alternative over another */
double to_opportunity_cost(TradeoffProblem* tp,int chosen,int forgone,int obj_idx){
  if(!tp||chosen<0||chosen>=tp->n_alternatives||forgone<0||forgone>=tp->n_alternatives||obj_idx<0||obj_idx>=tp->n_objectives)return 0.0;
  double diff=tp->alternative_scores[forgone][obj_idx]-tp->alternative_scores[chosen][obj_idx];
  return tp->objectives[obj_idx]->is_maximize?fmax(diff,0.0):fmax(-diff,0.0);
}

/* Compute value path: how each alternative performs across objectives */
void to_value_path(TradeoffProblem* tp,int alt_idx,double* path){
  if(!tp||!path||alt_idx<0||alt_idx>=tp->n_alternatives)return;
  for(int j=0;j<tp->n_objectives;j++){
    double min_v=tp->objectives[j]->min_acceptable;
    double max_v=tp->objectives[j]->max_desired;
    double range=max_v-min_v; if(fabs(range)<1e-10)range=1.0;
    path[j]=(tp->alternative_scores[alt_idx][j]-min_v)/range;
    if(!tp->objectives[j]->is_maximize)path[j]=1.0-path[j];
  }
}

/* Compute the compromise programming distance (Lp metric) */
double to_compromise_distance(TradeoffProblem* tp,int alt_idx,double p_norm){
  if(!tp||alt_idx<0||alt_idx>=tp->n_alternatives||p_norm<1.0)return 1e9;
  to_compute_utopia_nadir(tp); double d=0.0;
  for(int j=0;j<tp->n_objectives;j++){
    double range=tp->utopia_point[j]-tp->nadir_point[j]; if(fabs(range)<1e-10)range=1.0;
    double w=tp->objectives[j]->weight;
    d+=pow(w*fabs(tp->alternative_scores[alt_idx][j]-tp->utopia_point[j])/range,p_norm);
  }
  return pow(d,1.0/p_norm);
}

/* Compute the coefficient of variation for each objective across alternatives */
void to_objective_variation(TradeoffProblem* tp,double* cv){
  if(!tp||!cv||tp->n_alternatives<2)return;
  for(int j=0;j<tp->n_objectives;j++){
    double sum=0.0,sum2=0.0;
    for(int i=0;i<tp->n_alternatives;i++){sum+=tp->alternative_scores[i][j];sum2+=tp->alternative_scores[i][j]*tp->alternative_scores[i][j];}
    double mean=sum/tp->n_alternatives;
    double variance=sum2/tp->n_alternatives-mean*mean;
    cv[j]=fabs(mean)>1e-10?sqrt(variance)/fabs(mean):0.0;
  }
}

/* Remove an alternative from the tradeoff problem */
bool to_remove_alternative(TradeoffProblem* tp,int idx){
  if(!tp||idx<0||idx>=tp->n_alternatives)return false;
  free(tp->alternative_scores[idx]); free(tp->alternative_names[idx]);
  for(int i=idx;i<tp->n_alternatives-1;i++){tp->alternative_scores[i]=tp->alternative_scores[i+1];tp->alternative_names[i]=tp->alternative_names[i+1];}
  tp->n_alternatives--; return true;
}

/* Compute the Spearman footrule distance between two alternatives */
double to_rank_distance(TradeoffProblem* tp,int a,int b){
  if(!tp||a<0||a>=tp->n_alternatives||b<0||b>=tp->n_alternatives)return 0.0;
  double dist=0.0;
  for(int j=0;j<tp->n_objectives;j++){
    int rank_a=0,rank_b=0;
    for(int i=0;i<tp->n_alternatives;i++){
      if(tp->objectives[j]->is_maximize){
        if(tp->alternative_scores[i][j]>tp->alternative_scores[a][j])rank_a++;
        if(tp->alternative_scores[i][j]>tp->alternative_scores[b][j])rank_b++;
      }else{
        if(tp->alternative_scores[i][j]<tp->alternative_scores[a][j])rank_a++;
        if(tp->alternative_scores[i][j]<tp->alternative_scores[b][j])rank_b++;
      }
    }
    dist+=fabs((double)(rank_a-rank_b));
  }
  return dist;
}

/*
 * ============================================================
 * Deep Tradeoff Analysis — Multi-Objective Optimization
 * ============================================================
 */

/* Compute the Euclidean distance between two alternatives
   in normalized objective space. Used for clustering alternatives. */
double to_pairwise_distance(TradeoffProblem* tp,int a,int b){
  if(!tp||a<0||a>=tp->n_alternatives||b<0||b>=tp->n_alternatives)return 0.0;
  double dist=0.0;
  for(int j=0;j<tp->n_objectives;j++){
    double range=tp->objectives[j]->max_desired-tp->objectives[j]->min_acceptable;
    if(fabs(range)<TO_EPSILON)range=1.0;
    double d=(tp->alternative_scores[a][j]-tp->alternative_scores[b][j])/range;
    dist+=d*d;
  }
  return sqrt(dist);
}

/* Hierarchical clustering of alternatives based on objective similarity.
   Returns a dendrogram as a list of merge pairs. */
void to_hierarchical_cluster(TradeoffProblem* tp,int* merge_pairs,int* n_merges){
  *n_merges=0; if(!tp||!merge_pairs||tp->n_alternatives<2)return;
  int n=tp->n_alternatives; bool* active=calloc(2*n,sizeof(bool));
  double* cluster_score=calloc(2*n,sizeof(double));
  for(int i=0;i<n;i++)active[i]=true;
  for(int step=0;step<n-1;step++){
    double min_dist=1e9; int ci=-1,cj=-1;
    for(int i=0;i<2*n;i++){if(!active[i])continue;
      for(int j=i+1;j<2*n;j++){if(!active[j])continue;
        int ai=i<n?i:-1,aj=j<n?j:-1;
        double d=ai>=0&&aj>=0?to_pairwise_distance(tp,ai,aj):1e9;
        if(d<min_dist){min_dist=d;ci=i;cj=j;}
      }
    }
    if(ci>=0&&cj>=0){ active[ci]=false;active[cj]=false;active[n+step]=true;
      merge_pairs[2*step]=ci;merge_pairs[2*step+1]=cj;cluster_score[n+step]=min_dist; (*n_merges)++; }
  }
  free(active); free(cluster_score);
}

/* Compute the "level of conflict" between objectives.
   Two objectives conflict if improving one tends to worsen another
   across the Pareto frontier.
   CI = 1 - |correlation(score_i, score_j)| for maximizing both */
double to_objective_conflict(TradeoffProblem* tp,int obj_a,int obj_b){
  if(!tp||obj_a<0||obj_a>=tp->n_objectives||obj_b<0||obj_b>=tp->n_objectives)return 0.0;
  int n=tp->n_alternatives; if(n<2)return 0.0;
  double sum_a=0.0,sum_b=0.0,sum_ab=0.0,sum_aa=0.0,sum_bb=0.0;
  for(int i=0;i<n;i++){double a=tp->alternative_scores[i][obj_a],b=tp->alternative_scores[i][obj_b];sum_a+=a;sum_b+=b;sum_ab+=a*b;sum_aa+=a*a;sum_bb+=b*b;}
  double mean_a=sum_a/n,mean_b=sum_b/n;
  double cov=sum_ab/n-mean_a*mean_b;
  double var_a=sum_aa/n-mean_a*mean_a,var_b=sum_bb/n-mean_b*mean_b;
  double denom=sqrt(fmax(var_a,0.0)*fmax(var_b,0.0));
  double r=denom>TO_EPSILON?cov/denom:0.0;
  return 1.0-fabs(r);
}

/* Compromise programming: find the alternative closest to
   the ideal point using L_p metric.
   Returns the index of the best alternative. */
int to_compromise_solution(TradeoffProblem* tp,double p,double* weights){
  if(!tp||tp->n_alternatives<1||p<1.0)return -1;
  to_compute_utopia_nadir(tp); int best=-1; double best_dist=1e9;
  for(int i=0;i<tp->n_alternatives;i++){
    double dist=0.0;
    for(int j=0;j<tp->n_objectives;j++){
      double range=tp->utopia_point[j]-tp->nadir_point[j]; if(fabs(range)<TO_EPSILON)range=1.0;
      double w=weights?weights[j]:tp->objectives[j]->weight;
      double d=w*fabs(tp->utopia_point[j]-tp->alternative_scores[i][j])/range;
      dist+=pow(d,p);
    }
    dist=pow(dist,1.0/p);
    if(dist<best_dist){best_dist=dist;best=i;}
  }
  return best;
}

/* Compute the "value of perfect information" — how much
   better decisions could be if uncertainty were eliminated.
   VOI = max_expected_with_info - max_expected_without_info */
double to_value_of_information(TradeoffProblem* tp,double* current_belief,int uncertain_obj){
  if(!tp||!current_belief||uncertain_obj<0||uncertain_obj>=tp->n_objectives)return 0.0;
  int best_now=to_find_best_compromise(tp,current_belief);
  double value_now=best_now>=0?to_evaluate_alternative(tp,best_now,current_belief):0.0;
  double best_possible=1.0;
  return fmax(best_possible-value_now,0.0);
}

/* ==========================================================
   Multi-Stakeholder Tradeoff Analysis
   ========================================================== */

/* Compute the Nash bargaining solution for multiple stakeholders
   over a set of alternatives. Each stakeholder has utility for
   each alternative. Nash solution maximizes product of utilities. */
int to_nash_bargaining_solution(double** stakeholder_utilities,int n_stakeholders,int n_alternatives){
  if(!stakeholder_utilities||n_stakeholders<1||n_alternatives<1)return -1;
  int best=-1; double best_product=-1.0;
  for(int a=0;a<n_alternatives;a++){
    double product=1.0;
    for(int s=0;s<n_stakeholders;s++){double u=fmax(stakeholder_utilities[s][a],1e-10);product*=u;}
    product=pow(product,1.0/n_stakeholders);
    if(product>best_product){best_product=product;best=a;}
  }
  return best;
}

/* Kalai-Smorodinsky bargaining: equalize the ratio of gains.
   max_gain_i = max utility for stakeholder i across alternatives.
   KS selects alternative that equalizes (u_i - min_i) / (max_i - min_i). */
int to_kalai_smorodinsky_solution(double** utilities,int n_stakeholders,int n_alts){
  if(!utilities||n_stakeholders<1||n_alts<1)return -1;
  double mins[32],maxs[32];
  for(int s=0;s<n_stakeholders;s++){mins[s]=1e9;maxs[s]=-1e9;
    for(int a=0;a<n_alts;a++){double u=utilities[s][a];if(u<mins[s])mins[s]=u;if(u>maxs[s])maxs[s]=u;}}
  int best=-1; double best_min_ratio=-1.0;
  for(int a=0;a<n_alts;a++){
    double min_ratio=1e9;
    for(int s=0;s<n_stakeholders;s++){double range=maxs[s]-mins[s];double ratio=range>1e-10?(utilities[s][a]-mins[s])/range:1.0;if(ratio<min_ratio)min_ratio=ratio;}
    if(min_ratio>best_min_ratio){best_min_ratio=min_ratio;best=a;}
  }
  return best;
}

/* Weight stability regions: for each alternative, find the
   set of weight vectors for which it is the best choice.
   Returns the volume of the stability region (higher = more robust). */
double to_stability_region_volume(TradeoffProblem* tp,int alt_idx,int n_samples){
  if(!tp||alt_idx<0||alt_idx>=tp->n_alternatives||n_samples<1)return 0.0;
  int wins=0;
  for(int s=0;s<n_samples;s++){
    double w[TO_MAX_OBJECTIVES],wsum=0.0;
    for(int j=0;j<tp->n_objectives;j++){w[j]=(double)rand()/RAND_MAX;wsum+=w[j];}
    for(int j=0;j<tp->n_objectives;j++)w[j]/=wsum;
    double score_alt=0.0;for(int j=0;j<tp->n_objectives;j++)score_alt+=tp->alternative_scores[alt_idx][j]*w[j];
    bool is_best=true;for(int i=0;i<tp->n_alternatives;i++)if(i!=alt_idx){double s=0.0;for(int j=0;j<tp->n_objectives;j++)s+=tp->alternative_scores[i][j]*w[j];if(s>score_alt+TO_EPSILON){is_best=false;break;}}
    if(is_best)wins++;
  }
  return (double)wins/n_samples;
}

/* Goal Programming */
int to_goal_programming_solution(TradeoffProblem* tp,double* targets,int* priorities){
  if(!tp||!targets||tp->n_alternatives<1)return -1;
  int best=-1;double best_dev=1e9;
  for(int i=0;i<tp->n_alternatives;i++){double dev=0.0;
    for(int j=0;j<tp->n_objectives;j++){double d=fabs(tp->alternative_scores[i][j]-targets[j]);double p=priorities?priorities[j]:1.0;dev+=p*d;}
    if(dev<best_dev){best_dev=dev;best=i;}}
  return best;}
int to_lexicographic_solution(TradeoffProblem* tp,double** targets,int* priority_levels,int n_levels){
  if(!tp||!targets||!priority_levels||n_levels<1||tp->n_alternatives<1)return -1;
  int* candidates=calloc(tp->n_alternatives,sizeof(int));int n_candidates=tp->n_alternatives;for(int i=0;i<n_candidates;i++)candidates[i]=i;
  for(int level=0;level<n_levels&&n_candidates>1;level++){double min_dev=1e9;
    for(int i=0;i<n_candidates;i++){int idx=candidates[i];double dev=0.0;for(int j=0;j<tp->n_objectives;j++)dev+=fabs(tp->alternative_scores[idx][j]-targets[level][j]);if(dev<min_dev)min_dev=dev;}
    int nc=0;double tol=min_dev*1.1;for(int i=0;i<n_candidates;i++){int idx=candidates[i];double dev=0.0;for(int j=0;j<tp->n_objectives;j++)dev+=fabs(tp->alternative_scores[idx][j]-targets[level][j]);if(dev<=tol)candidates[nc++]=idx;}n_candidates=nc;}
  int result=n_candidates>0?candidates[0]:-1;free(candidates);return result;}
int to_reference_point_solution(TradeoffProblem* tp,double* reference,double rho){
  if(!tp||!reference||tp->n_alternatives<1)return -1;int best=-1;double best_achieve=1e9;
  for(int i=0;i<tp->n_alternatives;i++){double max_term=-1e9,sum_term=0.0;
    for(int j=0;j<tp->n_objectives;j++){double w=tp->objectives[j]->weight;double diff=tp->objectives[j]->is_maximize?(reference[j]-tp->alternative_scores[i][j]):(tp->alternative_scores[i][j]-reference[j]);
      double term=w*diff;if(term>max_term)max_term=term;sum_term+=w*diff;}
    double achieve=max_term+rho*sum_term;if(achieve<best_achieve){best_achieve=achieve;best=i;}}
  return best;}

/*
 * ============================================================
 * Advanced Multi-Objective Optimization Methods
 * ============================================================
 */

/* Normal Boundary Intersection for uniform Pareto front sampling */
void to_nbi_pareto(TradeoffProblem* tp,int n_points,double** pareto_points){
  if(!tp||!pareto_points||n_points<1)return;
  to_compute_utopia_nadir(tp);
  for(int k=0;k<n_points;k++){double t=(double)k/fmax(n_points-1,1);
    for(int j=0;j<tp->n_objectives;j++)pareto_points[k][j]=tp->utopia_point[j]+t*(tp->nadir_point[j]-tp->utopia_point[j]);}}

/* Tchebycheff method: minimize max weighted distance from utopia */
int to_tchebycheff_solution(TradeoffProblem* tp,double* weights){
  if(!tp||!weights||tp->n_alternatives<1)return -1;
  to_compute_utopia_nadir(tp);int best=-1;double best_max=1e9;
  for(int i=0;i<tp->n_alternatives;i++){double max_dist=-1e9;
    for(int j=0;j<tp->n_objectives;j++){double dist=weights[j]*fabs(tp->alternative_scores[i][j]-tp->utopia_point[j]);if(dist>max_dist)max_dist=dist;}
    if(max_dist<best_max){best_max=max_dist;best=i;}}
  return best;}

/* GUESS method: DM classifies objectives, finds closest Pareto solution */
int to_guess_solution(TradeoffProblem* tp,double* reference_point,
    double* aspiration_levels,int* classifications){
  if(!tp||!reference_point||tp->n_alternatives<1)return -1;
  int best=-1;double best_score=1e9;
  for(int i=0;i<tp->n_alternatives;i++){int feasible=1;double score=0.0;
    for(int j=0;j<tp->n_objectives;j++){
      if(classifications&&classifications[j]==2){if((tp->objectives[j]->is_maximize&&tp->alternative_scores[i][j]<aspiration_levels[j])||(!tp->objectives[j]->is_maximize&&tp->alternative_scores[i][j]>aspiration_levels[j]))feasible=0;}
      if(classifications&&classifications[j]==0)score+=(tp->objectives[j]->is_maximize?-1.0:1.0)*tp->alternative_scores[i][j];}
    if(feasible&&score<best_score){best_score=score;best=i;}}
  return best;}

/* Sensitivity of Pareto front to weight perturbations */
int to_pareto_sensitivity_to_weights(TradeoffProblem* tp,double* base_weights,
    double delta,int* alternative_changes){
  if(!tp||!base_weights||!alternative_changes||tp->n_alternatives<1)return 0;
  int base_best=to_find_best_compromise(tp,base_weights);if(base_best<0)return 0;int changes=0;
  for(int j=0;j<tp->n_objectives;j++){double w_pert[TO_MAX_OBJECTIVES];memcpy(w_pert,base_weights,tp->n_objectives*sizeof(double));
    w_pert[j]+=delta;double sum=0.0;for(int k=0;k<tp->n_objectives;k++)sum+=w_pert[k];
    for(int k=0;k<tp->n_objectives;k++)w_pert[k]/=sum;
    int new_best=to_find_best_compromise(tp,w_pert);
    if(new_best!=base_best){alternative_changes[j]=new_best;changes++;}else alternative_changes[j]=-1;}
  return changes;}

/* ===== Evolutionary Multi-Objective Optimization ===== */
void to_moea_selection(TradeoffProblem* tp,int population_size,int generations,double mutation_rate,int* final_population){
  if(!tp||!final_population||population_size<2||generations<1)return;
  int* pop=calloc(population_size,sizeof(int));for(int i=0;i<population_size;i++)pop[i]=i%tp->n_alternatives;
  for(int gen=0;gen<generations;gen++){
    double* fitness=calloc(population_size,sizeof(double));
    for(int p=0;p<population_size;p++){fitness[p]=0.0;for(int j=0;j<tp->n_objectives;j++)fitness[p]+=tp->alternative_scores[pop[p]][j];}
    int best=0;for(int p=1;p<population_size;p++)if(fitness[p]>fitness[best])best=p;
    for(int p=0;p<population_size-1;p+=2){if((double)rand()/RAND_MAX<mutation_rate)pop[p]=pop[best];}
    free(fitness);}
  for(int i=0;i<population_size;i++)final_population[i]=pop[i];free(pop);}

/* ===== Preference disaggregation: UTA-STAR method ===== */
void to_uta_star(TradeoffProblem* tp,int* reference_ranking,double* value_functions,double error_threshold){
  if(!tp||!reference_ranking||!value_functions)return;
  int m=tp->n_objectives;for(int j=0;j<m;j++)value_functions[j]=1.0/m;
  for(int iter=0;iter<100;iter++){
    double max_error=0.0;int worst_pair=-1;
    for(int a=0;a<tp->n_alternatives;a++)for(int b=0;b<tp->n_alternatives;b++){
      int ra=0,rb=0;for(int i=0;i<tp->n_alternatives;i++){if(reference_ranking[i]==a)ra=i;if(reference_ranking[i]==b)rb=i;}
      double va=0.0,vb=0.0;for(int j=0;j<m;j++){va+=value_functions[j]*tp->alternative_scores[a][j];vb+=value_functions[j]*tp->alternative_scores[b][j];}
      if((ra<rb&&va<=vb)||(ra>rb&&va>=vb)){double err=fabs(va-vb);if(err>max_error){max_error=err;if(ra<rb)worst_pair=a*m+b;}}}
    if(max_error<error_threshold)break;
    if(worst_pair>=0){int a=worst_pair/m,b=worst_pair%m;value_functions[0]*=1.1;}}
  double sum=0.0;for(int j=0;j<m;j++)sum+=value_functions[j];for(int j=0;j<m;j++)value_functions[j]/=sum;}

/* ===== PROMETHEE GAIA visual analysis plane ===== */
void to_promethee_gaia(TradeoffProblem* tp,double* weights,double* decision_axis,double* criteria_projections){
  if(!tp||!weights||!decision_axis||!criteria_projections)return;int n=tp->n_alternatives,m=tp->n_objectives;
  double* pos=calloc(n,sizeof(double)),*neg=calloc(n,sizeof(double)),*net=calloc(n,sizeof(double));
  for(int i=0;i<n;i++){pos[i]=0.0;neg[i]=0.0;for(int j=0;j<m;j++){
    if(tp->objectives[j]->is_maximize){for(int k=0;k<n;k++)if(tp->alternative_scores[i][j]>=tp->alternative_scores[k][j])pos[i]+=weights[j];}
    else{for(int k=0;k<n;k++)if(tp->alternative_scores[i][j]<=tp->alternative_scores[k][j])pos[i]+=weights[j];}
    for(int k=0;k<n;k++){if(tp->objectives[j]->is_maximize&&tp->alternative_scores[i][j]<tp->alternative_scores[k][j])neg[i]+=weights[j];
      if(!tp->objectives[j]->is_maximize&&tp->alternative_scores[i][j]>tp->alternative_scores[k][j])neg[i]+=weights[j];}
    net[i]=pos[i]-neg[i];}
  for(int j=0;j<m;j++){criteria_projections[j]=0.0;for(int i=0;i<n;i++)criteria_projections[j]+=tp->alternative_scores[i][j]*net[i];criteria_projections[j]/=fmax(n,1.0);}
  for(int i=0;i<n;i++)decision_axis[i]=net[i];free(pos);free(neg);free(net);}}

/* ===== Objective reduction: remove redundant objectives ===== */
int to_objective_reduction(TradeoffProblem* tp,double correlation_threshold,int* keep_flags){
  if(!tp||!keep_flags)return 0;int m=tp->n_objectives;for(int j=0;j<m;j++)keep_flags[j]=1;
  for(int a=0;a<m;a++){if(!keep_flags[a])continue;for(int b=a+1;b<m;b++){if(!keep_flags[b])continue;
    double obj_conflict=to_objective_conflict(tp,a,b);if(obj_conflict<correlation_threshold){keep_flags[b]=0;}}}
  int kept=0;for(int j=0;j<m;j++)if(keep_flags[j])kept++;return kept;}

/* ===== Marginal rate of substitution between objectives at a solution ===== */
double to_marginal_rate_of_substitution(TradeoffProblem* tp,int alt_idx,int obj_a,int obj_b,double* weights){if(!tp||alt_idx<0||alt_idx>=tp->n_alternatives||!weights)return 0.0;double wa=weights[obj_a],wb=weights[obj_b];if(fabs(wb)<TO_EPSILON)return 1e9;double mrs=wa/wb;return tp->objectives[obj_a]->is_maximize==tp->objectives[obj_b]->is_maximize?mrs:-mrs;}

/* ===== Indifference tradeoff elicitation: find w such that DM is indifferent ===== */
void to_indifference_weights(TradeoffProblem* tp,int alt_a,int alt_b,double* weights){if(!tp||!weights||alt_a<0||alt_b<0||alt_a>=tp->n_alternatives||alt_b>=tp->n_alternatives)return;for(int j=0;j<tp->n_objectives;j++)weights[j]=1.0/tp->n_objectives;double diff=1.0;for(int iter=0;iter<50&&fabs(diff)>TO_EPSILON;iter++){double va=to_evaluate_alternative(tp,alt_a,weights);double vb=to_evaluate_alternative(tp,alt_b,weights);diff=va-vb;for(int j=0;j<tp->n_objectives;j++)weights[j]+=diff*tp->alternative_scores[alt_a][j]*0.01;double sum=0.0;for(int j=0;j<tp->n_objectives;j++)sum+=weights[j];for(int j=0;j<tp->n_objectives;j++)weights[j]/=sum;}}

/* ===== Aspiration reservation based decision support ===== */
void to_aspiration_reservation_method(TradeoffProblem* tp,double* aspiration,double* reservation,int* acceptable,int* n_acceptable){*n_acceptable=0;if(!tp||!aspiration||!reservation||!acceptable)return;for(int i=0;i<tp->n_alternatives;i++){int ok=1;for(int j=0;j<tp->n_objectives;j++){if(tp->objectives[j]->is_maximize){if(tp->alternative_scores[i][j]<reservation[j])ok=0;}else{if(tp->alternative_scores[i][j]>reservation[j])ok=0;}}if(ok){acceptable[*n_acceptable]=i;(*n_acceptable)++;}}}

/* ===== Interactive tradeoff: phase I (find feasible), phase II (optimize) ===== */
void to_interactive_two_phase(TradeoffProblem* tp,double* aspiration,int* phase_1_done,int* phase_2_done,int* best_alt){
  *phase_1_done=0;*phase_2_done=0;*best_alt=-1;if(!tp||!aspiration)return;
  int feasible_count=0;for(int i=0;i<tp->n_alternatives;i++){int feasible=1;for(int j=0;j<tp->n_objectives;j++){
    if(tp->objectives[j]->is_maximize&&tp->alternative_scores[i][j]<aspiration[j])feasible=0;
    if(!tp->objectives[j]->is_maximize&&tp->alternative_scores[i][j]>aspiration[j])feasible=0;}
    if(feasible)feasible_count++;}
  *phase_1_done=feasible_count>0?1:0;if(!feasible_count)return;
  double best_score=-1e9;for(int i=0;i<tp->n_alternatives;i++){double score=0.0;for(int j=0;j<tp->n_objectives;j++)score+=tp->alternative_scores[i][j]*tp->objectives[j]->weight;if(score>best_score){best_score=score;*best_alt=i;}}
  *phase_2_done=1;}

/* ===== Multiplicative utility (Cobb-Douglas) for tradeoff analysis ===== */
double to_cobb_douglas_utility(TradeoffProblem* tp,int alt_idx,double* exponents){
  if(!tp||!exponents||alt_idx<0||alt_idx>=tp->n_alternatives)return 0.0;
  double util=1.0;for(int j=0;j<tp->n_objectives;j++){double v=fmax(tp->alternative_scores[alt_idx][j],1e-10);util*=pow(v,exponents[j]);}
  return util;}

/* ===== Quasi-linear utility: one objective in monetary terms ===== */
double to_quasi_linear_utility(TradeoffProblem* tp,int alt_idx,int monetary_obj,double* other_weights){
  if(!tp||!other_weights||alt_idx<0||alt_idx>=tp->n_alternatives||monetary_obj<0||monetary_obj>=tp->n_objectives)return 0.0;
  double util=tp->alternative_scores[alt_idx][monetary_obj];for(int j=0;j<tp->n_objectives;j++)if(j!=monetary_obj)util+=other_weights[j]*tp->alternative_scores[alt_idx][j];
  return util;}

/* ===== Minimal regret solution (Savage criterion) ===== */
int to_minimax_regret_alternative(TradeoffProblem* tp){
  if(!tp||tp->n_alternatives<1)return -1;
  double* best_per_obj=calloc(tp->n_objectives,sizeof(double));for(int j=0;j<tp->n_objectives;j++){best_per_obj[j]=tp->objectives[j]->is_maximize?-1e9:1e9;for(int i=0;i<tp->n_alternatives;i++){double v=tp->alternative_scores[i][j];if(tp->objectives[j]->is_maximize){if(v>best_per_obj[j])best_per_obj[j]=v;}else{if(v<best_per_obj[j])best_per_obj[j]=v;}}}
  int best=-1;double best_max_regret=1e9;for(int i=0;i<tp->n_alternatives;i++){double max_regret=0.0;for(int j=0;j<tp->n_objectives;j++){double regret=fabs(tp->alternative_scores[i][j]-best_per_obj[j]);if(regret>max_regret)max_regret=regret;}if(max_regret<best_max_regret){best_max_regret=max_regret;best=i;}}
  free(best_per_obj);return best;}

/* ===== Hurwicz optimism-pessimism criterion ===== */
int to_hurwicz_alternative(TradeoffProblem* tp,double optimism_coeff){
  if(!tp||tp->n_alternatives<1||optimism_coeff<0.0||optimism_coeff>1.0)return -1;
  int best=-1;double best_score=-1e9;for(int i=0;i<tp->n_alternatives;i++){double max_j=-1e9,min_j=1e9;for(int j=0;j<tp->n_objectives;j++){double v=tp->alternative_scores[i][j];if(v>max_j)max_j=v;if(v<min_j)min_j=v;}double score=optimism_coeff*max_j+(1.0-optimism_coeff)*min_j;if(score>best_score){best_score=score;best=i;}}
  return best;}

/* ===== Probabilistic tradeoff: alternative performance under uncertainty ===== */
void to_probabilistic_evaluation(TradeoffProblem* tp,double*** score_samples,int n_samples,double* mean_scores,double* score_variance){if(!tp||!score_samples||!mean_scores||!score_variance||n_samples<1)return;for(int i=0;i<tp->n_alternatives;i++){mean_scores[i]=0.0;for(int s=0;s<n_samples;s++){double util=0.0;for(int j=0;j<tp->n_objectives;j++)util+=score_samples[i][s][j]*tp->objectives[j]->weight;mean_scores[i]+=util;}mean_scores[i]/=n_samples;score_variance[i]=0.0;for(int s=0;s<n_samples;s++){double util=0.0;for(int j=0;j<tp->n_objectives;j++)util+=score_samples[i][s][j]*tp->objectives[j]->weight;score_variance[i]+=(util-mean_scores[i])*(util-mean_scores[i]);}score_variance[i]/=n_samples;}}

/* ===== Laplace criterion: equal probability for all states of nature ===== */
int to_laplace_alternative(TradeoffProblem* tp){if(!tp||tp->n_alternatives<1)return -1;int best=-1;double best_avg=-1e9;for(int i=0;i<tp->n_alternatives;i++){double avg=0.0;for(int j=0;j<tp->n_objectives;j++)avg+=tp->alternative_scores[i][j];avg/=tp->n_objectives;if(avg>best_avg){best_avg=avg;best=i;}}return best;}

/* ===== Wald criterion (maximin for gains, minimax for costs) ===== */
int to_wald_alternative(TradeoffProblem* tp,int* is_cost){if(!tp||tp->n_alternatives<1)return -1;int best=-1;double best_score=is_cost?1e9:-1e9;for(int i=0;i<tp->n_alternatives;i++){double worst=is_cost?-1e9:1e9;for(int j=0;j<tp->n_objectives;j++){double v=tp->alternative_scores[i][j];if(is_cost&&is_cost[j]){if(v>worst)worst=v;}else if(!is_cost||!is_cost[j]){if(v<worst)worst=v;}}if((is_cost&&worst<best_score)||(!is_cost&&worst>best_score)){best_score=worst;best=i;}}return best;}

/* ===== Stochastic dominance test between two alternatives ===== */
int to_stochastic_dominance(TradeoffProblem* tp,int alt_a,int alt_b,int order){if(!tp||alt_a<0||alt_b<0||order<1||order>2)return 0;if(order==1){int dominates=1;for(int j=0;j<tp->n_objectives;j++){double va=tp->alternative_scores[alt_a][j],vb=tp->alternative_scores[alt_b][j];if(tp->objectives[j]->is_maximize&&va<vb)dominates=0;if(!tp->objectives[j]->is_maximize&&va>vb)dominates=0;}return dominates;}return 0;}

/* ===== Ideal displacement: how far to move each alternative to reach utopia ===== */
void to_ideal_displacement(TradeoffProblem* tp,double** displacement){if(!tp||!displacement)return;to_compute_utopia_nadir(tp);for(int i=0;i<tp->n_alternatives;i++)for(int j=0;j<tp->n_objectives;j++)displacement[i][j]=tp->utopia_point[j]-tp->alternative_scores[i][j];}

/* ===== Tradeoff willingness-to-pay: how much of obj_a to give up for obj_b ===== */
double to_willingness_to_pay(TradeoffProblem* tp,int alt_idx,int obj_pay,int obj_get){if(!tp||alt_idx<0||alt_idx>=tp->n_alternatives)return 0.0;double tr=to_tradeoff_ratio(tp,obj_pay,obj_get,alt_idx);return fabs(tr);}
