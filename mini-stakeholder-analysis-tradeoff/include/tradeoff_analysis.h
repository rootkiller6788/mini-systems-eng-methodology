#ifndef TRADEOFF_ANALYSIS_H
#define TRADEOFF_ANALYSIS_H
#include "stakeholder_model.h"

typedef struct { char* name; double value,min_acceptable,max_desired,weight; bool is_maximize; char* units; } Objective;
typedef struct { char* name; Objective** objectives; int n_objectives; double** alternative_scores; char** alternative_names; int n_alternatives; int* pareto_front; int n_pareto; double* utopia_point,*nadir_point; } TradeoffProblem;
typedef struct { int index_a,index_b; double tradeoff_ratio; char* explanation; } TradeoffPair;
TradeoffProblem* to_create(const char* name,int n_obj,const char** names,double* weights,bool* maximize);
void to_free(TradeoffProblem* tp);
int to_add_alternative(TradeoffProblem* tp,const char* name,double* scores);
void to_compute_pareto_front(TradeoffProblem* tp);
bool to_is_dominated(TradeoffProblem* tp,int a,int b);
double to_tradeoff_ratio(TradeoffProblem* tp,int oa,int ob,int alt);
int to_find_best_compromise(TradeoffProblem* tp,double* weights);
void to_compute_utopia_nadir(TradeoffProblem* tp);
double to_distance_to_utopia(TradeoffProblem* tp,int alt);
void to_print_pareto_front(TradeoffProblem* tp);
double to_evaluate_alternative(TradeoffProblem* tp,int idx,double* weights);
void to_set_objective_limits(TradeoffProblem* tp,int idx,double min,double max,const char* units);
void to_normalize_scores(TradeoffProblem* tp);
int to_find_knee_point(TradeoffProblem* tp);
void to_tradeoff_matrix(TradeoffProblem* tp,double** matrix);
double to_pareto_hypervolume(TradeoffProblem* tp,double* ref_point);
int to_weight_sensitivity(TradeoffProblem* tp,int alt_idx,double* orig_weights,double* ranges,int n_steps);
int to_find_robust_alternatives(TradeoffProblem* tp,int* robust_idxs,int max_robust);
double to_opportunity_cost(TradeoffProblem* tp,int chosen,int forgone,int obj_idx);
void to_value_path(TradeoffProblem* tp,int alt_idx,double* path);
double to_compromise_distance(TradeoffProblem* tp,int alt_idx,double p_norm);
void to_objective_variation(TradeoffProblem* tp,double* cv);
bool to_remove_alternative(TradeoffProblem* tp,int idx);
double to_rank_distance(TradeoffProblem* tp,int a,int b);
double to_pairwise_distance(TradeoffProblem* tp,int a,int b);
void to_hierarchical_cluster(TradeoffProblem* tp,int* merge_pairs,int* n_merges);
double to_objective_conflict(TradeoffProblem* tp,int obj_a,int obj_b);
int to_compromise_solution(TradeoffProblem* tp,double p,double* weights);
double to_value_of_information(TradeoffProblem* tp,double* current_belief,int uncertain_obj);

#define TO_MAX_ALTERNATIVES 32
#define TO_MAX_OBJECTIVES 12
#define TO_EPSILON 1e-6
#endif
