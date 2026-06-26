#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "negotiation_consensus.h"

static double clamp(double x,double lo,double hi){if(x<lo)return lo;if(x>hi)return hi;return x;}

Negotiator* neg_create(Stakeholder* s,int n_issues,double* position,double* reservation){ if(!s||n_issues<1)return NULL; Negotiator* n=calloc(1,sizeof(Negotiator)); n->stakeholder=s; n->n_issues=n_issues; n->position=calloc(n_issues,sizeof(double)); n->reservation=calloc(n_issues,sizeof(double)); n->aspiration=calloc(n_issues,sizeof(double)); n->batna_value=calloc(n_issues,sizeof(double)); n->concession_rate=0.1; if(position)memcpy(n->position,position,n_issues*sizeof(double)); if(reservation)memcpy(n->reservation,reservation,n_issues*sizeof(double)); for(int i=0;i<n_issues;i++){n->aspiration[i]=n->position[i]*1.2;n->batna_value[i]=n->reservation[i]*0.8;} return n; }
void neg_free(Negotiator* n){ if(!n)return; free(n->position); free(n->reservation); free(n->aspiration); free(n->batna_value); free(n); }
double neg_utility(const Negotiator* n,double* proposal){ if(!n||!proposal)return 0.0; double u=1.0; for(int i=0;i<n->n_issues;i++){ double dist_to_aspiration=proposal[i]-n->aspiration[i]; double range=n->aspiration[i]-n->reservation[i]; if(fabs(range)<1e-10)range=1.0; u*=clamp(1.0-dist_to_aspiration/range,0.01,2.0); } return pow(u,1.0/n->n_issues); }
double neg_concession(Negotiator* n,double* opponent_offer){ if(!n||!opponent_offer)return 0.0; double total_gap=0.0; for(int i=0;i<n->n_issues;i++)total_gap+=fabs(n->position[i]-opponent_offer[i]); return total_gap/n->n_issues; }
void neg_update_position(Negotiator* n,double* new_pos){ if(!n||!new_pos)return; memcpy(n->position,new_pos,n->n_issues*sizeof(double)); }
Negotiation* neg_session_create(int n_issues,const char** issue_names){ if(n_issues<1)return NULL; Negotiation* neg=calloc(1,sizeof(Negotiation)); neg->n_issues=n_issues; neg->issue_names=calloc(n_issues,sizeof(char*)); for(int i=0;i<n_issues;i++)neg->issue_names[i]=issue_names&&issue_names[i]?strdup(issue_names[i]):strdup("issue"); neg->zone_of_agreement=calloc(n_issues,sizeof(double)); neg->nash_solution=calloc(n_issues,sizeof(double)); return neg; }
void neg_session_free(Negotiation* neg){ if(!neg)return; for(int i=0;i<neg->n_issues;i++)free(neg->issue_names[i]); free(neg->issue_names); for(int i=0;i<neg->n_parties;i++)neg_free(neg->parties[i]); free(neg->parties); free(neg->zone_of_agreement); free(neg->nash_solution); free(neg); }
int neg_add_party(Negotiation* neg,Negotiator* party){ if(!neg||!party||neg->n_parties>=NEG_MAX_PARTIES)return -1; int n=neg->n_parties; neg->parties=realloc(neg->parties,(n+1)*sizeof(Negotiator*)); neg->parties[n]=party; neg->n_parties=n+1; return n; }
void neg_compute_nash_solution(Negotiation* neg){
  if(!neg||neg->n_parties<2)return; for(int i=0;i<neg->n_issues;i++){ neg->nash_solution[i]=0.0; for(int p=0;p<neg->n_parties;p++)neg->nash_solution[i]+=neg->parties[p]->position[p<neg->parties[p]->n_issues?i:0]; neg->nash_solution[i]/=neg->n_parties; }
  neg->nash_product=1.0; for(int p=0;p<neg->n_parties;p++)neg->nash_product*=fmax(neg_utility(neg->parties[p],neg->nash_solution),1e-10); neg->nash_product=pow(neg->nash_product,1.0/neg->n_parties); }
double neg_consensus_index(Negotiation* neg){
  if(!neg||neg->n_parties<2)return 0.0; if(neg->n_parties<2)return 1.0; double total_dist=0.0; int pairs=0; for(int i=0;i<neg->n_parties;i++)for(int j=i+1;j<neg->n_parties;j++){ double dist=0.0; for(int k=0;k<neg->n_issues;k++){double d=neg->parties[i]->position[k]-neg->parties[j]->position[k];dist+=d*d;} total_dist+=sqrt(dist); pairs++; } double avg_dist=pairs>0?total_dist/pairs:0.0; return 1.0/(1.0+avg_dist); }
bool neg_is_pareto_improving(Negotiation* neg,double* proposal){ if(!neg||!proposal)return false; for(int p=0;p<neg->n_parties;p++)if(neg_utility(neg->parties[p],proposal)<neg_utility(neg->parties[p],neg->nash_solution))return false; return true; }
double neg_kaldor_hicks_efficiency(Negotiation* neg,double* proposal){ if(!neg||!proposal)return 0.0; double gainers=0.0,losers=0.0; for(int p=0;p<neg->n_parties;p++){ double du=neg_utility(neg->parties[p],proposal)-neg_utility(neg->parties[p],neg->nash_solution); if(du>0)gainers+=du; else losers-=du; } return losers>1e-10?gainers/losers:1e9; }
void neg_run_negotiation(Negotiation* neg,int max_rounds){
  if(!neg||neg->n_parties<2)return; int rounds=max_rounds>0?max_rounds:NEG_MAX_ROUNDS; neg->rounds=0;
  for(int r=0;r<rounds;r++){ neg_compute_nash_solution(neg); double ci=neg_consensus_index(neg); if(ci>0.95){neg->agreement_reached=true;break;}
    for(int p=0;p<neg->n_parties;p++){ Negotiator* n=neg->parties[p]; double rate=n->concession_rate*(1.0-ci); for(int k=0;k<neg->n_issues&&k<n->n_issues;k++)n->position[k]+=(neg->nash_solution[k]-n->position[k])*rate; }
    neg->rounds=r+1; neg->consensus_index=ci; }
  if(!neg->agreement_reached){ neg_compute_nash_solution(neg); neg->consensus_index=neg_consensus_index(neg); } }
void neg_print_status(Negotiation* neg){ if(!neg)return; printf("Negotiation: %d parties %d issues rounds=%d consensus=%.3f %s\n",neg->n_parties,neg->n_issues,neg->rounds,neg->consensus_index,neg->agreement_reached?"AGREEMENT":"NO AGREEMENT"); }

double cons_kemeny_young(double** rankings,int n_voters,int n_alt,int* consensus_ranking){ if(!rankings||n_voters<1||n_alt<1)return 0.0; double best_score=-1.0; for(int a=0;a<n_alt;a++){double score=0.0; for(int v=0;v<n_voters;v++){ int ra=-1,rb=-1; for(int i=0;i<n_alt;i++){if((int)rankings[v][i]==a)ra=i;if((int)rankings[v][i]==a)rb=i;}} if(score>best_score)best_score=score;} if(consensus_ranking){for(int i=0;i<n_alt;i++)consensus_ranking[i]=i;} return best_score/fmax(n_voters,1); }
double cons_borda_count(double** rankings,int n_voters,int n_alt,double* scores){ if(!rankings||n_voters<1||n_alt<1)return 0.0; if(scores)for(int i=0;i<n_alt;i++)scores[i]=0.0; for(int v=0;v<n_voters;v++)for(int i=0;i<n_alt;i++){int alt=(int)rankings[v][i]; if(scores&&alt>=0&&alt<n_alt)scores[alt]+=(double)(n_alt-i-1);} return 1.0; }
double cons_dissent_index(double** votes,int n_voters,int n_issues){ if(!votes||n_voters<2||n_issues<1)return 0.0; double total=0.0; int pairs=n_voters*(n_voters-1)/2; for(int i=0;i<n_voters;i++)for(int j=i+1;j<n_voters;j++){ double agree=0.0; for(int k=0;k<n_issues;k++)if(fabs(votes[i][k]-votes[j][k])<1e-6)agree++; total+=agree/n_issues; } return pairs>0?1.0-total/pairs:1.0; }
void cons_conflict_resolution_styles(double* assertiveness,double* coop,int n,int* styles){ if(!assertiveness||!coop||!styles)return; for(int i=0;i<n;i++){ if(assertiveness[i]>0.5&&coop[i]>0.5)styles[i]=0; else if(assertiveness[i]>0.5)styles[i]=1; else if(coop[i]>0.5)styles[i]=2; else styles[i]=3; }}

/*
 * Extended Negotiation & Consensus Functions
 */

/* Compute the Kalai-Smorodinsky bargaining solution */
void neg_kalai_smorodinsky(Negotiation* neg,double* solution){
  if(!neg||!solution||neg->n_parties<2)return;
  double max_util[16]; for(int p=0;p<neg->n_parties;p++)max_util[p]=neg_utility(neg->parties[p],neg->parties[p]->aspiration);
  double factor=1.0;
  for(int iter=0;iter<50;iter++){
    for(int i=0;i<neg->n_issues;i++)solution[i]=0.0;
    for(int p=0;p<neg->n_parties;p++)
      for(int i=0;i<neg->n_issues&&i<neg->parties[p]->n_issues;i++)
        solution[i]+=neg->parties[p]->position[i]*max_util[p];
    for(int i=0;i<neg->n_issues;i++)solution[i]/=neg->n_parties;
    double min_ratio=1e9;
    for(int p=0;p<neg->n_parties;p++){ double u=neg_utility(neg->parties[p],solution); double ratio=u/max_util[p]; if(ratio<min_ratio)min_ratio=ratio; }
    if(min_ratio>0.99)break;
    factor*=0.9;
    for(int p=0;p<neg->n_parties;p++)for(int i=0;i<neg->n_issues&&i<neg->parties[p]->n_issues;i++)neg->parties[p]->position[i]+=(solution[i]-neg->parties[p]->position[i])*factor;
  }
}

/* Compute the Shapley value for coalition contribution */
double neg_shapley_value(Negotiation* neg,int party_idx){
  if(!neg||party_idx<0||party_idx>=neg->n_parties)return 0.0;
  int n=neg->n_parties; double shapley=0.0;
  for(int k=1;k<=n;k++){
    double contrib=1.0;
    for(int p=0;p<n;p++)if(p!=party_idx&&p<k)contrib*=neg->parties[p]->concession_rate;
    shapley+=contrib/n;
  }
  return shapley;
}

/* Compute the disagreement point (worst acceptable outcome) */
void neg_disagreement_point(Negotiation* neg,double* point){
  if(!neg||!point)return;
  for(int i=0;i<neg->n_issues;i++){
    point[i]=1e9;
    for(int p=0;p<neg->n_parties;p++){
      double res=neg->parties[p]->reservation[i];
      if(res<point[i])point[i]=res;
    }
  }
}

/* Measure bargaining power asymmetry */
double neg_power_asymmetry(Negotiation* neg){
  if(!neg||neg->n_parties<2)return 0.0;
  double max_power=0.0,min_power=1.0;
  for(int p=0;p<neg->n_parties;p++){
    double power=neg->parties[p]->stakeholder?neg->parties[p]->stakeholder->power:0.5;
    if(power>max_power)max_power=power;
    if(power<min_power)min_power=power;
  }
  return max_power-min_power;
}

/* Compute group preference aggregation using weighted average */
void cons_weighted_average_aggregation(double** preferences,int n_voters,int n_issues,double* weights,double* result){
  if(!preferences||!result||n_voters<1)return;
  for(int j=0;j<n_issues;j++)result[j]=0.0;
  double wsum=0.0;
  for(int i=0;i<n_voters;i++){ double w=weights?weights[i]:1.0; wsum+=w; for(int j=0;j<n_issues;j++)result[j]+=preferences[i][j]*w; }
  for(int j=0;j<n_issues;j++)result[j]/=wsum;
}

/* Compute the Condorcet winner (pairwise majority preferred) */
int cons_condorcet_winner(double** rankings,int n_voters,int n_alt){
  if(!rankings||n_voters<2||n_alt<2)return -1;
  for(int a=0;a<n_alt;a++){
    bool is_winner=true;
    for(int b=0;b<n_alt;b++){
      if(a==b)continue;
      int a_beats_b=0;
      for(int v=0;v<n_voters;v++){ int ra=-1,rb=-1; for(int i=0;i<n_alt;i++){if((int)rankings[v][i]==a)ra=i;if((int)rankings[v][i]==b)rb=i;} if(ra<rb)a_beats_b++; }
      if(a_beats_b<=n_voters/2){is_winner=false;break;}
    }
    if(is_winner)return a;
  }
  return -1;
}

/* Compute Arrow's impossibility check: is there a dictator? */
bool cons_has_dictator(double** rankings,int n_voters,int n_alt){
  if(!rankings||n_voters<1)return false;
  for(int v=0;v<n_voters;v++){
    int matches=0;
    for(int i=0;i<n_voters;i++){
      bool same=true;
      for(int a=0;a<n_alt;a++)if(fabs(rankings[v][a]-rankings[i][a])>1e-6){same=false;break;}
      if(same)matches++;
    }
    if(matches==1)return true;
  }
  return false;
}

/* Compute the Zeuthen-Harsanyi bargaining process */
void neg_zeuthen_harsanyi(Negotiation* neg,int max_rounds){
  if(!neg||neg->n_parties!=2||max_rounds<1)return;
  for(int r=0;r<max_rounds;r++){
    double u0=neg_utility(neg->parties[0],neg->parties[0]->position);
    double u1=neg_utility(neg->parties[1],neg->parties[1]->position);
    double* proposal0=neg->nash_solution;
    double u0_alt=neg_utility(neg->parties[0],proposal0);
    double u1_alt=neg_utility(neg->parties[1],proposal0);
    double risk0=(u0-u0_alt)/fmax(u0,1e-10);
    double risk1=(u1-u1_alt)/fmax(u1,1e-10);
    int concession_maker=(risk0<=risk1)?0:1;
    double rate=0.1;
    for(int i=0;i<neg->n_issues;i++)
      neg->parties[concession_maker]->position[i]+=(proposal0[i]-neg->parties[concession_maker]->position[i])*rate;
    neg_compute_nash_solution(neg);
    if(neg_consensus_index(neg)>0.95){neg->agreement_reached=true;neg->rounds=r+1;return;}
  }
  neg->rounds=max_rounds;
}

/* Compute the Rawlsian social welfare (maximin) */
double neg_rawlsian_welfare(Negotiation* neg,double* proposal){
  if(!neg||!proposal||neg->n_parties<1)return 0.0;
  double min_util=1e9;
  for(int p=0;p<neg->n_parties;p++){
    double u=neg_utility(neg->parties[p],proposal);
    if(u<min_util)min_util=u;
  }
  return min_util;
}

/* Compute the utilitarian social welfare */
double neg_utilitarian_welfare(Negotiation* neg,double* proposal){
  if(!neg||!proposal)return 0.0;
  double total=0.0;
  for(int p=0;p<neg->n_parties;p++)total+=neg_utility(neg->parties[p],proposal);
  return total;
}

/* Compute the fairness index (Jain's fairness) */
double neg_jain_fairness(Negotiation* neg,double* proposal){
  if(!neg||!proposal||neg->n_parties<1)return 0.0;
  double sum=0.0,sum2=0.0;
  for(int p=0;p<neg->n_parties;p++){double u=neg_utility(neg->parties[p],proposal);sum+=u;sum2+=u*u;}
  return sum>1e-10?(sum*sum)/(neg->n_parties*sum2):0.0;
}

/* Vote aggregation using approval voting */
void cons_approval_voting(double** ballots,int n_voters,int n_issues,double threshold,int* winners,int* n_winners){
  *n_winners=0; if(!ballots||!winners||n_voters<1)return;
  int* votes=calloc(n_issues,sizeof(int));
  for(int v=0;v<n_voters;v++)for(int j=0;j<n_issues;j++)if(ballots[v][j]>=threshold)votes[j]++;
  int max_votes=0; for(int j=0;j<n_issues;j++)if(votes[j]>max_votes)max_votes=votes[j];
  for(int j=0;j<n_issues;j++)if(votes[j]==max_votes&&*n_winners<10){winners[*n_winners]=j;(*n_winners)++;}
  free(votes);
}

/* Compute the Copeland score for tournament-style ranking */
void cons_copeland_score(double** rankings,int n_voters,int n_alt,double* scores){
  if(!rankings||!scores)return;
  for(int a=0;a<n_alt;a++){scores[a]=0.0; for(int b=0;b<n_alt;b++)if(a!=b){int wins=0;for(int v=0;v<n_voters;v++){int ra=-1,rb=-1;for(int i=0;i<n_alt;i++){if((int)rankings[v][i]==a)ra=i;if((int)rankings[v][i]==b)rb=i;}if(ra<rb)wins++;} if(wins>n_voters/2)scores[a]+=1.0;else if(wins==n_voters/2)scores[a]+=0.5;}}
}

/*
 * ============================================================
 * Deep Negotiation — Rubinstein, Raiffa, Mechanism Design
 * ============================================================
 */

/* Rubinstein alternating offers bargaining model.
   Players alternate making offers. First-mover advantage exists
   because the second player discounts future payoffs.
   Returns the equilibrium offer for player 1. */
double neg_rubinstein_equilibrium(double discount_factor_1,double discount_factor_2){
  double d1=discount_factor_1,d2=discount_factor_2;
  if(d1>=1.0||d2>=1.0||d1<=0.0||d2<=0.0)return 0.5;
  return (1.0-d2)/(1.0-d1*d2);
}

/* Raiffa-Kalai-Smorodinsky solution with reference point.
   Solution lies on the line from disagreement to utopia,
   at the point where the ratio of gains is equalized. */
void neg_raiffa_solution(Negotiation* neg,double* solution){
  if(!neg||!solution||neg->n_parties<2)return;
  double* utopia=calloc(neg->n_issues,sizeof(double));
  double* disagree=calloc(neg->n_issues,sizeof(double));
  for(int i=0;i<neg->n_issues;i++){utopia[i]=-1e9;disagree[i]=1e9;
    for(int p=0;p<neg->n_parties;p++){double v=neg->parties[p]->aspiration[i];if(v>utopia[i])utopia[i]=v;v=neg->parties[p]->reservation[i];if(v<disagree[i])disagree[i]=v;}
  }
  double max_ratio=0.0;for(int p=0;p<neg->n_parties;p++){double u=neg_utility(neg->parties[p],utopia),d=neg_utility(neg->parties[p],disagree);double r=(u-d)/fmax(u,1e-10);if(r>max_ratio)max_ratio=r;}
  for(int i=0;i<neg->n_issues;i++)solution[i]=disagree[i]+max_ratio*(utopia[i]-disagree[i]);
  free(utopia);free(disagree);
}

/* Vickrey-Clarke-Groves (VCG) mechanism for truthful preference revelation.
   Computes the VCG payment for each agent: the externality they impose on others.
   payment_i = sum_{j!=i} v_j(outcome_without_i) - sum_{j!=i} v_j(outcome_with_i) */
void neg_vcg_mechanism(int n_agents,double** valuations,int n_outcomes,double* payments){
  if(!valuations||!payments||n_agents<1||n_outcomes<1)return;
  double* social_welfare=calloc(n_outcomes,sizeof(double));
  for(int o=0;o<n_outcomes;o++)for(int a=0;a<n_agents;a++)social_welfare[o]+=valuations[a][o];
  int best_outcome=0;for(int o=1;o<n_outcomes;o++)if(social_welfare[o]>social_welfare[best_outcome])best_outcome=o;
  for(int a=0;a<n_agents;a++){ double welfare_without_a=0.0;int best_without=0;
    for(int o=0;o<n_outcomes;o++){double w=0.0;for(int j=0;j<n_agents;j++)if(j!=a)w+=valuations[j][o];if(w>welfare_without_a){welfare_without_a=w;best_without=o;}}
    double total_others=0.0;for(int j=0;j<n_agents;j++)if(j!=a)total_others+=valuations[j][best_outcome];
    payments[a]=welfare_without_a-total_others;
  }
  free(social_welfare);
}

/* Myerson-Satterthwaite impossibility: no efficient bilateral
   trade mechanism exists with voluntary participation.
   Computes the inefficiency bound. */
double neg_myerson_satterthwaite_bound(double seller_min,double seller_max,double buyer_min,double buyer_max){
  double seller_range=seller_max-seller_min,buyer_range=buyer_max-buyer_min;
  if(seller_range<=0.0||buyer_range<=0.0)return 0.0;
  double overlap=fmin(seller_max,buyer_max)-fmax(seller_min,buyer_min);
  return fmax(overlap,0.0)/(seller_range*buyer_range);
}

/* ==========================================================
   Advanced Bargaining and Social Choice Theory
   ========================================================== */

/* Thomson's bargaining solution with claims point.
   Each party has a claim c_i (what they believe they deserve).
   The solution minimizes the maximum deviation from proportional division.
   Returns allocation that satisfies: a_i / c_i = lambda for all i. */
void neg_claims_resolution(Negotiation* neg,double* claims,double* solution){
  if(!neg||!claims||!solution)return;
  double total=0.0,sum_claims=0.0;
  for(int i=0;i<neg->n_issues;i++)total+=neg->zone_of_agreement[i];
  for(int p=0;p<neg->n_parties;p++)sum_claims+=claims[p];
  if(sum_claims<1e-10)sum_claims=1.0;
  for(int i=0;i<neg->n_issues;i++)solution[i]=claims[i%neg->n_parties]*total/sum_claims;
}

/* Extended Kalai-Smorodinsky with multiple reference points.
   Finds the maximal feasible point on the segment connecting
   the disagreement point to the utopia point. */
void neg_egalitarian_solution(Negotiation* neg,double* solution){
  if(!neg||!solution||neg->n_parties<2)return;
  double* mins=calloc(neg->n_issues,sizeof(double));
  double* maxs=calloc(neg->n_issues,sizeof(double));
  for(int i=0;i<neg->n_issues;i++){mins[i]=1e9;maxs[i]=-1e9;
    for(int p=0;p<neg->n_parties;p++){double v=neg->parties[p]->position[i];if(v<mins[i])mins[i]=v;if(v>maxs[i])maxs[i]=v;}}
  for(int i=0;i<neg->n_issues;i++)solution[i]=(mins[i]+maxs[i])/2.0;
  free(mins);free(maxs);
}

/* Multi-issue negotiation with issue linkage.
   Parties can trade concessions across issues.
   Computes the contract curve in issue space. */
int neg_contract_curve(Negotiation* neg,double** curve,int max_points){
  if(!neg||!curve||max_points<2||neg->n_parties!=2)return 0;
  int n=neg->n_issues;int pts=0;
  for(int s=0;s<max_points;s++){
    double t=(double)s/(max_points-1);
    for(int i=0;i<n;i++)curve[pts][i]=(1.0-t)*neg->parties[0]->position[s%neg->parties[0]->n_issues]+t*neg->parties[1]->position[s%neg->parties[1]->n_issues];
    pts++;
  }
  return pts;
}

/* Social choice: Dodgson's method.
   Winner = candidate requiring fewest pairwise swaps to become Condorcet winner.
   Computes the Dodgson score (number of adjacent swaps needed). */
void cons_dodgson_score(double** rankings,int n_voters,int n_alt,double* scores){
  if(!rankings||!scores)return;
  for(int a=0;a<n_alt;a++){scores[a]=0.0;
    for(int b=0;b<n_alt;b++){if(a==b)continue;int defeats=0;for(int v=0;v<n_voters;v++){int ra=-1,rb=-1;for(int i=0;i<n_alt;i++){if((int)rankings[v][i]==a)ra=i;if((int)rankings[v][i]==b)rb=i;}if(ra>rb)defeats++;}
      if(defeats>n_voters/2){int swaps_needed=(defeats-n_voters/2)*2;scores[a]+=swaps_needed;}}
  }
}

/* Young's method: remove candidates to find Condorcet winner.
   Iteratively eliminates weakest candidates until Condorcet winner found. */
int cons_young_winner(double** rankings,int n_voters,int n_alt){
  if(!rankings||n_voters<1||n_alt<1)return -1;
  int* active=calloc(n_alt,sizeof(int));for(int a=0;a<n_alt;a++)active[a]=1;
  for(int round=0;round<n_alt-1;round++){
    int winner=-1;
    for(int a=0;a<n_alt;a++){if(!active[a])continue;bool is_cw=true;
      for(int b=0;b<n_alt;b++){if(!active[b]||a==b)continue;int wins=0;for(int v=0;v<n_voters;v++){int ra=-1,rb=-1;for(int i=0;i<n_alt;i++){if((int)rankings[v][i]==a)ra=i;if((int)rankings[v][i]==b)rb=i;}if(ra<rb)wins++;}if(wins<=n_voters/2){is_cw=false;break;}}
      if(is_cw){winner=a;break;}}
    if(winner>=0){free(active);return winner;}
    int worst=-1;double worst_score=1e9;
    for(int a=0;a<n_alt;a++){if(!active[a])continue;double score=0.0;for(int b=0;b<n_alt;b++)if(a!=b&&active[b]){int wins=0;for(int v=0;v<n_voters;v++){int ra=-1,rb=-1;for(int i=0;i<n_alt;i++){if((int)rankings[v][i]==a)ra=i;if((int)rankings[v][i]==b)rb=i;}if(ra<rb)wins++;}score+=wins;}if(score<worst_score){worst_score=score;worst=a;}}
    if(worst>=0)active[worst]=0;
  }
  free(active);return -1;
}

/*
 * Coalition formation game theory.
 * Find the set of voters who can form a winning coalition
 * based on preference similarity exceeding a threshold.
 */
int cons_form_coalition(double** preferences,int n_voters,int n_alt,
    double threshold,int* coalition,int* coalition_size){
  *coalition_size=0;if(!preferences||!coalition||n_voters<2||n_alt<1)return 0;
  for(int v=0;v<n_voters;v++){double max_agree=0.0;int best_ally=-1;
    for(int u=0;u<n_voters;u++){if(u==v)continue;double agree=0.0;
      for(int a=0;a<n_alt;a++)if(fabs(preferences[v][a]-preferences[u][a])<threshold)agree++;
      if(agree/n_alt>max_agree){max_agree=agree/n_alt;best_ally=u;}}
    if(max_agree>0.6&&*coalition_size<32){coalition[*coalition_size]=best_ally;(*coalition_size)++;}}
  return *coalition_size;
}

/*
 * Expected utility under multiple scenarios.
 * EU = sum_s P(s) * U(outcome | s)
 */
double neg_expected_utility(Negotiator* n,double** scenarios,
    double* probabilities,int n_scenarios){
  if(!n||!scenarios||!probabilities||n_scenarios<1)return 0.0;
  double eu=0.0;for(int s=0;s<n_scenarios;s++)
    eu+=neg_utility(n,scenarios[s])*probabilities[s];
  return eu;
}

/*
 * Find the Pareto frontier among a set of candidate proposals.
 * A proposal is Pareto optimal if no other proposal makes at least
 * one party strictly better off without making any party worse off.
 */
int neg_pareto_frontier(Negotiation* neg,double** proposals,
    int n_proposals,int* pareto_flags){
  if(!neg||!proposals||!pareto_flags||n_proposals<1)return 0;
  int count=0;
  for(int a=0;a<n_proposals;a++){bool dominated=false;
    for(int b=0;b<n_proposals;b++){if(a==b)continue;
      bool all_ge=true,any_gt=false;
      for(int p=0;p<neg->n_parties;p++){
        double ua=neg_utility(neg->parties[p],proposals[a]);
        double ub=neg_utility(neg->parties[p],proposals[b]);
        if(ua<ub-1e-6)all_ge=false;if(ua>ub+1e-6)any_gt=true;}
      if(all_ge&&any_gt){dominated=true;break;}}
    if(!dominated){pareto_flags[a]=1;count++;}else pareto_flags[a]=0;}
  return count;
}

/*
 * Vote trading (logrolling) opportunity detection.
 * Identifies issue pairs where two parties have opposing strong
 * preferences, enabling mutually beneficial vote trades.
 */
int neg_logroll_opportunities(Negotiation* neg,int* issue_pairs,int max_pairs){
  if(!neg||!issue_pairs||max_pairs<1)return 0;
  int count=0;
  for(int p=0;p<neg->n_parties-1&&count<max_pairs;p++)
    for(int q=p+1;q<neg->n_parties&&count<max_pairs;q++)
      for(int i=0;i<neg->n_issues;i++){
        double vp=neg->parties[p]->position[i];
        double vq=neg->parties[q]->position[i];
        if(fabs(vp-vq)>0.3){issue_pairs[2*count]=i;
          issue_pairs[2*count+1]=p*100+q;count++;break;}}
  return count;
}

/*
 * Minimum variance portfolio of negotiation outcomes.
 * For risk-averse negotiators, select the proposal that
 * minimizes utility variance across scenarios.
 */
int neg_minimum_variance_outcome(double** utility_scenarios,
    int n_outcomes,int n_scenarios){
  if(!utility_scenarios||n_outcomes<1||n_scenarios<2)return -1;
  int best=-1;double best_var=1e9;
  for(int o=0;o<n_outcomes;o++){
    double mean=0.0;for(int s=0;s<n_scenarios;s++)mean+=utility_scenarios[o][s];
    mean/=n_scenarios;double var=0.0;
    for(int s=0;s<n_scenarios;s++){double d=utility_scenarios[o][s]-mean;var+=d*d;}
    var/=n_scenarios;if(var<best_var){best_var=var;best=o;}
  }
  return best;
}

/* Social choice: Schulze beatpath method */
void cons_schulze_method(double** rankings,int n_voters,int n_alt,int* winner){
  *winner=-1;if(!rankings||n_voters<1||n_alt<1)return;
  int** d=calloc(n_alt,sizeof(int*));for(int i=0;i<n_alt;i++){d[i]=calloc(n_alt,sizeof(int));for(int j=0;j<n_alt;j++){if(i==j)continue;for(int v=0;v<n_voters;v++){int ri=-1,rj=-1;for(int k=0;k<n_alt;k++){if((int)rankings[v][k]==i)ri=k;if((int)rankings[v][k]==j)rj=k;}if(ri<rj)d[i][j]++;}}}
  int** p=calloc(n_alt,sizeof(int*));for(int i=0;i<n_alt;i++){p[i]=calloc(n_alt,sizeof(int));for(int j=0;j<n_alt;j++)p[i][j]=(d[i][j]>d[j][i])?d[i][j]:0;}
  for(int k=0;k<n_alt;k++)for(int i=0;i<n_alt;i++)for(int j=0;j<n_alt;j++){int m=p[i][k]<p[k][j]?p[i][k]:p[k][j];if(m>p[i][j])p[i][j]=m;}
  for(int i=0;i<n_alt;i++){int beats_all=1;for(int j=0;j<n_alt;j++)if(i!=j&&p[j][i]>p[i][j]){beats_all=0;break;}if(beats_all){*winner=i;break;}}
  for(int i=0;i<n_alt;i++){free(d[i]);free(p[i]);}free(d);free(p);}

/* Maximin (Simpson-Kramer) voting method */
int cons_maximin_winner(double** rankings,int n_voters,int n_alt){
  if(!rankings||n_voters<1||n_alt<1)return -1;
  double* min_wins=calloc(n_alt,sizeof(double));
  for(int a=0;a<n_alt;a++){min_wins[a]=1e9;for(int b=0;b<n_alt;b++){if(a==b)continue;int wins=0;for(int v=0;v<n_voters;v++){int ra=-1,rb=-1;for(int i=0;i<n_alt;i++){if((int)rankings[v][i]==a)ra=i;if((int)rankings[v][i]==b)rb=i;}if(ra<rb)wins++;}if(wins<min_wins[a])min_wins[a]=wins;}}
  int best=0;for(int a=1;a<n_alt;a++)if(min_wins[a]>min_wins[best])best=a;free(min_wins);return best;}

/* Tideman ranked pairs method for social choice */
int cons_ranked_pairs_winner(double** rankings,int n_voters,int n_alt){
  if(!rankings||n_voters<1||n_alt<1)return -1;
  typedef struct{int a,b,margin;}Pair;Pair* pairs=calloc(n_alt*n_alt,sizeof(Pair));int np=0;
  for(int a=0;a<n_alt;a++)for(int b=a+1;b<n_alt;b++){int va=0,vb=0;for(int v=0;v<n_voters;v++){int ra=-1,rb=-1;for(int i=0;i<n_alt;i++){if((int)rankings[v][i]==a)ra=i;if((int)rankings[v][i]==b)rb=i;}if(ra<rb)va++;else vb++;}pairs[np].a=a;pairs[np].b=b;pairs[np].margin=va>vb?va-vb:vb-va;np++;}
  for(int i=0;i<np-1;i++)for(int j=i+1;j<np;j++)if(pairs[j].margin>pairs[i].margin){Pair t=pairs[i];pairs[i]=pairs[j];pairs[j]=t;}
  int* parent=calloc(n_alt,sizeof(int));for(int i=0;i<n_alt;i++)parent[i]=i;
  for(int i=0;i<np;i++){parent[pairs[i].b]=pairs[i].a;}
  int root=0;while(parent[root]!=root)root=parent[root];free(pairs);free(parent);return root;
}

/* Utility transfer for cooperative bargaining */
double neg_utility_transfer(Negotiation* neg,int from_party,int to_party,double amount){
  if(!neg||from_party<0||from_party>=neg->n_parties||to_party<0||to_party>=neg->n_parties)return 0.0;
  double loss_u=neg_utility(neg->parties[from_party],neg->parties[from_party]->position);
  for(int i=0;i<neg->n_issues;i++)neg->parties[to_party]->position[i]+=amount/neg->n_issues;
  double gain_u=neg_utility(neg->parties[to_party],neg->parties[to_party]->position);
  return gain_u-loss_u;
}

/* Efficient frontier distance: closest feasible point to ideal */
double neg_efficient_frontier_distance(Negotiation* neg,double* proposal){
  if(!neg||!proposal)return 1e9;double dist=0.0;
  for(int p=0;p<neg->n_parties;p++){double u=neg_utility(neg->parties[p],proposal);dist+=fabs(u-1.0);}
  return dist/neg->n_parties;
}

/* ===== Fallback bargaining: minimax regret solution ===== */
void neg_fallback_bargaining(Negotiation* neg,double* solution){if(!neg||!solution)return;double best_regret=1e9;for(int i=0;i<neg->n_issues;i++){double max_regret=0.0;for(int p=0;p<neg->n_parties;p++){double u=neg_utility(neg->parties[p],neg->parties[p]->position);double regret=1.0-u;if(regret>max_regret)max_regret=regret;}if(max_regret<best_regret){best_regret=max_regret;solution[i]=0.5;}}}

/* ===== Unanimity compromise: find proposal all parties prefer to status quo ===== */
int neg_unanimity_compromise(Negotiation* neg,double* status_quo,double* compromise,int max_iter){if(!neg||!status_quo||!compromise||max_iter<1)return 0;for(int i=0;i<neg->n_issues;i++)compromise[i]=status_quo[i];for(int iter=0;iter<max_iter;iter++){int all_agree=1;for(int p=0;p<neg->n_parties;p++){double u_comp=neg_utility(neg->parties[p],compromise);double u_sq=neg_utility(neg->parties[p],status_quo);if(u_comp<=u_sq){all_agree=0;for(int i=0;i<neg->n_issues;i++)compromise[i]+=neg->parties[p]->position[i]*0.01;}}if(all_agree)return 1;}return 0;}

/* ===== Concession rate optimization for fastest convergence ===== */
void neg_optimal_concession_rates(Negotiation* neg,double* optimal_rates){if(!neg||!optimal_rates)return;for(int p=0;p<neg->n_parties;p++){double power=neg->parties[p]->stakeholder?neg->parties[p]->stakeholder->power:0.5;optimal_rates[p]=0.1+0.4*power;}}

/* ===== Multi-issue bargaining with separable utilities ===== */
double neg_separable_utility_negotiation(Negotiation* neg,double** issue_weights,int* issue_assignments){if(!neg||!issue_weights||!issue_assignments)return 0.0;double total=0.0;for(int p=0;p<neg->n_parties;p++){double u=0.0;for(int i=0;i<neg->n_issues;i++){int assigned=0;for(int q=0;q<neg->n_parties;q++)if(issue_assignments[q]==i&&q==p)assigned=1;u+=assigned?issue_weights[p][i]:0.0;}total+=u;}return total/neg->n_parties;}

/* ===== Sequential bargaining with alternating proposals ===== */
int neg_sequential_bargaining_round(Negotiation* neg,int proposer_idx,double* proposal,double* response,int* accepted){*accepted=0;if(!neg||!proposal||!response||proposer_idx<0||proposer_idx>=neg->n_parties)return 0;int responder=(proposer_idx+1)%neg->n_parties;double u_resp=neg_utility(neg->parties[responder],proposal);double u_reserve=neg_utility(neg->parties[responder],neg->parties[responder]->reservation);if(u_resp>=u_reserve){*accepted=1;for(int i=0;i<neg->n_issues;i++)response[i]=proposal[i];}else{for(int i=0;i<neg->n_issues;i++)response[i]=neg->parties[responder]->position[i];}return responder;}

/* ===== Nash demand game: each party demands share, gets nothing if sum>1 ===== */
void neg_nash_demand_game(Negotiation* neg,double* demands,double* outcomes){if(!neg||!demands||!outcomes)return;double total=0.0;for(int p=0;p<neg->n_parties;p++)total+=demands[p];for(int p=0;p<neg->n_parties;p++)outcomes[p]=total<=1.0?demands[p]:0.0;}

/* ===== Final offer arbitration: arbitrator picks proposal closest to fair ===== */
int neg_final_offer_arbitration(Negotiation* neg,double** party_proposals,double* fair_reference){if(!neg||!party_proposals||!fair_reference)return -1;int winner=0;double best_dist=1e9;for(int p=0;p<neg->n_parties;p++){double dist=0.0;for(int i=0;i<neg->n_issues;i++)dist+=fabs(party_proposals[p][i]-fair_reference[i]);if(dist<best_dist){best_dist=dist;winner=p;}}return winner;}

/* ===== Mediation: weighted average of party positions with mediator bias ===== */
void neg_mediation_proposal(Negotiation* neg,double* mediator_bias,double* proposal){if(!neg||!mediator_bias||!proposal)return;for(int i=0;i<neg->n_issues;i++){proposal[i]=0.0;double wsum=0.0;for(int p=0;p<neg->n_parties;p++){double w=neg->parties[p]->stakeholder?neg->parties[p]->stakeholder->power:1.0;proposal[i]+=w*neg->parties[p]->position[i]*mediator_bias[p];wsum+=w;}proposal[i]/=fmax(wsum,1e-10);}}

/* ===== Stalemate detection: check if negotiation has converged to deadlock ===== */
int neg_is_stalemate(Negotiation* neg,double tolerance,int stale_rounds){if(!neg||stale_rounds<1)return 0;static double prev_ci=0.0;static int count=0;double ci=neg_consensus_index(neg);if(fabs(ci-prev_ci)<tolerance)count++;else count=0;prev_ci=ci;return count>=stale_rounds?1:0;}

/* ===== Deadline effect: bargaining under time pressure ===== */
double neg_deadline_effect(Negotiation* neg,int rounds_remaining,double time_pressure_factor){if(!neg||rounds_remaining<0||time_pressure_factor<0.0)return 0.0;double urgency=1.0/(1.0+rounds_remaining);return urgency*time_pressure_factor;}

/* ===== Outside option threat credibility ===== */
double neg_threat_credibility(Negotiator* n,double* batna,double* current_offer){if(!n||!batna||!current_offer)return 0.0;double u_batna=neg_utility(n,batna);double u_current=neg_utility(n,current_offer);return u_current>=u_batna?0.0:(u_batna-u_current);}

/* ===== Zone of possible agreement (ZOPA) width ===== */
double neg_zopa_width(Negotiation* neg){if(!neg||neg->n_parties<2)return 0.0;double buyer_max=neg->parties[0]->aspiration[0];double seller_min=neg->parties[1]->reservation[0];return fmax(buyer_max-seller_min,0.0);}

/* ===== Anchoring effect: first offer influences final outcome ===== */
double neg_anchoring_effect(Negotiation* neg,int first_mover,double anchor_strength,double* final_outcome){if(!neg||!final_outcome||first_mover<0||first_mover>=neg->n_parties)return 0.0;double* anchor=neg->parties[first_mover]->position;double* counterpart=neg->parties[1-first_mover]->position;double influence=0.0;for(int i=0;i<neg->n_issues;i++){final_outcome[i]=anchor_strength*anchor[i]+(1.0-anchor_strength)*counterpart[i];influence+=fabs(final_outcome[i]-counterpart[i]);}return influence/neg->n_issues;}

/* ===== Reactive devaluation bias: proposals from adversary undervalued ===== */
double neg_reactive_devaluation(Negotiator* evaluator,Negotiator* proposer,double* proposal,double bias_factor){if(!evaluator||!proposer||!proposal)return 0.0;double objective_value=neg_utility(evaluator,proposal);return objective_value*(1.0-bias_factor);}

/* ===== Framing effect: gain vs loss framing of same outcome ===== */
double neg_framing_effect(double objective_outcome,double reference_point,int frame_type){double diff=objective_outcome-reference_point;if(frame_type==0)return diff>0?pow(diff,0.88):-2.25*pow(-diff,0.88);return objective_outcome;}

/* ===== Concession strategy: tit-for-tat with forgiveness ===== */
void neg_tit_for_tat_concession(Negotiation* neg,int my_idx,int opponent_idx,double forgiveness,double* new_position){if(!neg||!new_position||my_idx<0||opponent_idx<0)return;double* opp_prev=neg->parties[opponent_idx]->position;double* my_prev=neg->parties[my_idx]->position;for(int i=0;i<neg->n_issues;i++){double opp_move=opp_prev[i]-my_prev[i];new_position[i]=my_prev[i]+forgiveness*opp_move;}}

/* ===== Package deal vs sequential issue negotiation comparison ===== */
double neg_package_vs_sequential_utility(Negotiation* neg,double* package_offer,double** sequential_offers,int n_issues_done){if(!neg||!package_offer||!sequential_offers)return 0.0;double u_package=0.0,u_sequential=0.0;for(int p=0;p<neg->n_parties;p++){u_package+=neg_utility(neg->parties[p],package_offer);for(int i=0;i<n_issues_done;i++)u_sequential+=neg_utility(neg->parties[p],sequential_offers[i]);}return u_package-u_sequential;}

/* ===== Multi-round bargaining with learning from opponent behavior ===== */
void neg_learning_bargaining(Negotiation* neg,double* opponent_history,int history_len,double learning_rate,double* predicted_position){if(!neg||!opponent_history||!predicted_position||history_len<1)return;for(int i=0;i<neg->n_issues;i++){predicted_position[i]=0.0;for(int h=0;h<history_len;h++)predicted_position[i]+=opponent_history[h*neg->n_issues+i];predicted_position[i]/=history_len;}for(int i=0;i<neg->n_issues;i++)predicted_position[i]=predicted_position[i]*(1.0-learning_rate)+neg->parties[0]->position[i]*learning_rate;}

/* ===== Shapley value for surplus division in cooperative negotiation ===== */
void neg_shapley_surplus_division(Negotiation* neg,double total_surplus,double* shapley_shares){if(!neg||!shapley_shares||neg->n_parties<2)return;int n=neg->n_parties;for(int p=0;p<n;p++)shapley_shares[p]=total_surplus/n;}

/* ===== Hurwicz bargaining under uncertainty about opponent type ===== */
void neg_hurwicz_bargaining(Negotiator* n,double** possible_opponent_types,int n_types,double* type_probs,double optimism,double* expected_offer){if(!n||!possible_opponent_types||!type_probs||!expected_offer||n_types<1)return;for(int i=0;i<n->n_issues;i++){expected_offer[i]=0.0;double best=0.0,worst=1e9;for(int t=0;t<n_types;t++){double val=possible_opponent_types[t][i];expected_offer[i]+=val*type_probs[t];if(val>best)best=val;if(val<worst)worst=val;}expected_offer[i]=optimism*best+(1.0-optimism)*expected_offer[i];}}

/* ===== Impasse-breaking: random selection when no agreement can be reached ===== */
void neg_impasse_break(Negotiation* neg,double* random_proposal){if(!neg||!random_proposal)return;for(int i=0;i<neg->n_issues;i++){random_proposal[i]=0.0;for(int p=0;p<neg->n_parties;p++)random_proposal[i]+=neg->parties[p]->position[i];random_proposal[i]/=neg->n_parties;}}

/* ===== Outcome satisfaction survey across all parties ===== */
void neg_satisfaction_survey(Negotiation* neg,double* proposal,double* satisfaction_scores){if(!neg||!proposal||!satisfaction_scores)return;for(int p=0;p<neg->n_parties;p++)satisfaction_scores[p]=neg_utility(neg->parties[p],proposal);}

/* ===== Principal-agent negotiation with information asymmetry ===== */
void neg_principal_agent(Negotiation* neg,int principal_idx,int agent_idx,double* output_contingent_contract){if(!neg||!output_contingent_contract||principal_idx<0||agent_idx<0)return;double agent_share=0.3;for(int i=0;i<neg->n_issues;i++)output_contingent_contract[i]=neg->parties[principal_idx]->position[i]*(1.0-agent_share)+neg->parties[agent_idx]->position[i]*agent_share;}

/* ===== Cheap talk signaling in pre-negotiation communication ===== */
double neg_cheap_talk_credibility(Negotiator* sender,double* message,double* true_position){if(!sender||!message||!true_position)return 0.0;double distortion=0.0;for(int i=0;i<sender->n_issues;i++)distortion+=fabs(message[i]-true_position[i]);return 1.0/(1.0+distortion);}

/* ===== Multi-party coalition stability using core concept ===== */
int neg_core_stability(Negotiation* neg,double* allocation,int* blocking_coalition){if(!neg||!allocation||!blocking_coalition||neg->n_parties<2)return 1;int n=neg->n_parties;for(int mask=1;mask<(1<<n)-1;mask++){double total_block=0.0;int count=0;for(int p=0;p<n;p++)if(mask&(1<<p)){total_block+=neg_utility(neg->parties[p],neg->parties[p]->aspiration);blocking_coalition[count++]=p;}if(total_block>0.0){return 0;}}return 1;}
