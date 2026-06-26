#ifndef MULTI_CRITERIA_DECISION_H
#define MULTI_CRITERIA_DECISION_H
#include "tradeoff_analysis.h"

typedef enum { MCDA_WEIGHTED_SUM=0,MCDA_TOPSIS=1,MCDA_AHP=2,MCDA_ELECTRE=3,MCDA_PROMETHEE=4,MCDA_N_METHODS=5 } MCDAMethod;
typedef struct { int n_criteria; double** pairwise_matrix; double* criteria_weights; double consistency_ratio; bool is_consistent; } AHPResult;
typedef struct { double* scores; int* ranking; int n_alternatives; MCDAMethod method; double consistency; } MCDAResult;
MCDAResult* mcda_weighted_sum(TradeoffProblem* tp,double* weights);
MCDAResult* mcda_topsis(TradeoffProblem* tp,double* weights);
AHPResult* mcda_ahp_compute(int n,double** pairwise);
void mcda_ahp_free(AHPResult* ahp);
MCDAResult* mcda_rank(TradeoffProblem* tp,MCDAMethod method,double* weights);
void mcda_result_free(MCDAResult* r);
void mcda_print_ranking(MCDAResult* r,TradeoffProblem* tp);
double mcda_concordance_index(TradeoffProblem* tp,int a,int b,double* weights);
double mcda_discordance_index(TradeoffProblem* tp,int a,int b);
double mcda_kendall_tau(MCDAResult* r1,MCDAResult* r2);
double mcda_spearman_rho(MCDAResult* r1,MCDAResult* r2);
bool mcda_electre_outranks(TradeoffProblem* tp,int a,int b,double* weights,double concord_threshold,double discord_threshold);
void mcda_random_weights(int n,double* weights);
void mcda_dominance_counts(TradeoffProblem* tp,int* counts);
void mcda_entropy_weights(TradeoffProblem* tp,double* weights);
double mcda_credibility_index(TradeoffProblem* tp,int a,int b,double* weights);
double mcda_preference_function(double diff,int type,double p,double q);
void mcda_promethee_flow(TradeoffProblem* tp,double* weights,double* pos_flow,double* neg_flow,double* net_flow);
double mcda_fuzzy_concordance(TradeoffProblem* tp,int a,int b,double* weights,double* thresholds);
void mcda_vikor(TradeoffProblem* tp,double* weights,double v,double* S,double* R,double* Q);
void mcda_interval_topsis(TradeoffProblem* tp,double* weights,double** lower,double** upper,double* scores,double* robustness);
void mcda_electre_iii(TradeoffProblem* tp,double* weights,double* q,double* p,double* v,double** credibility);
void mcda_smaa(TradeoffProblem* tp,int n_samples,int n_ranks,double*** smaa_indices);
void mcda_group_aggregation(int n_criteria,int n_decision_makers,double*** individual_matrices,double* dm_weights,double** group_matrix);

#define AHP_CONSISTENCY_THRESHOLD 0.10
#define AHP_RI_10 1.49
#endif
