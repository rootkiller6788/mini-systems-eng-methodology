#include "mbse_behavior.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

StateMachine* mbse_sm_create(const char* name){StateMachine* sm=calloc(1,sizeof(StateMachine));sm->name=strdup(name);sm->state_capacity=16;sm->states=calloc(16,sizeof(StateNode));sm->trans_capacity=32;sm->trans=calloc(32,sizeof(Transition));sm->hist_capacity=100;sm->history=calloc(100,sizeof(int));return sm;}
void mbse_sm_free(StateMachine* sm){if(!sm)return;for(int i=0;i<sm->n_states;i++){free(sm->states[i].name);free(sm->states[i].entry_actions);free(sm->states[i].exit_actions);}free(sm->states);for(int i=0;i<sm->n_trans;i++){free(sm->trans[i].name);free(sm->trans[i].trigger);free(sm->trans[i].guard);free(sm->trans[i].effects);}free(sm->trans);free(sm->history);free(sm->name);free(sm);}
int mbse_sm_add_state(StateMachine* sm, const char* name, StateType t, bool init, bool fin){if(sm->n_states>=sm->state_capacity){sm->state_capacity*=2;sm->states=realloc(sm->states,sm->state_capacity*sizeof(StateNode));}int i=sm->n_states++;StateNode* s=&sm->states[i];memset(s,0,sizeof(StateNode));s->id=i;s->name=strdup(name);s->type=t;s->is_initial=init;s->is_final=fin;if(init)sm->current_state=i;return i;}
int mbse_sm_add_transition(StateMachine* sm, int from, int to, const char* trigger, double prob){if(sm->n_trans>=sm->trans_capacity){sm->trans_capacity*=2;sm->trans=realloc(sm->trans,sm->trans_capacity*sizeof(Transition));}int i=sm->n_trans++;Transition* t=&sm->trans[i];memset(t,0,sizeof(Transition));t->id=i;t->from_state=from;t->to_state=to;t->name=strdup(trigger);t->trigger=strdup(trigger);t->probability=prob;return i;}
bool mbse_sm_fire(StateMachine* sm, const char* trigger){int cs=sm->current_state;for(int i=0;i<sm->n_trans;i++){Transition* t=&sm->trans[i];if(t->from_state==cs&&strcmp(t->trigger,trigger)==0){sm->current_state=t->to_state;if(sm->hist_len<sm->hist_capacity)sm->history[sm->hist_len++]=t->to_state;return true;}}return false;}
void mbse_sm_simulate(StateMachine* sm, const char** triggers, int n){for(int i=0;i<n;i++)mbse_sm_fire(sm,triggers[i]);}
bool mbse_sm_is_deadlock_free(StateMachine* sm){for(int i=0;i<sm->n_states;i++){if(sm->states[i].is_final)continue;bool has_exit=false;for(int j=0;j<sm->n_trans;j++)if(sm->trans[j].from_state==i){has_exit=true;break;}if(!has_exit)return false;}return true;}
void mbse_sm_print(StateMachine* sm){printf("StateMachine: %s (state=%d, %d states, %d transitions)\n",sm->name,sm->current_state,sm->n_states,sm->n_trans);for(int i=0;i<sm->n_states;i++)printf("  [%d] %s%s%s\n",i,sm->states[i].name,sm->states[i].is_initial?" (init)":"",sm->states[i].is_final?" (final)":"");}
ActivityDiagram* mbse_ad_create(const char* name){ActivityDiagram* ad=calloc(1,sizeof(ActivityDiagram));ad->name=strdup(name);ad->nodes=calloc(32,sizeof(ActivityNode));ad->n_nodes=0;return ad;}
void mbse_ad_free(ActivityDiagram* ad){if(!ad)return;for(int i=0;i<ad->n_nodes;i++){free(ad->nodes[i].name);free(ad->nodes[i].actions);free(ad->nodes[i].durations);free(ad->nodes[i].predecessors);free(ad->nodes[i].successors);}free(ad->nodes);free(ad->name);free(ad);}
int mbse_ad_add_node(ActivityDiagram* ad, const char* name, double dur){int i=ad->n_nodes++;ActivityNode* n=&ad->nodes[i];memset(n,0,sizeof(ActivityNode));n->id=i;n->name=strdup(name);n->durations=malloc(sizeof(double));n->durations[0]=dur;n->total_duration=dur;n->n_actions=1;return i;}
void mbse_ad_add_edge(ActivityDiagram* ad, int pred, int succ){if(pred<0||pred>=ad->n_nodes||succ<0||succ>=ad->n_nodes)return;ActivityNode* p=&ad->nodes[pred];p->successors=realloc(p->successors,(p->n_succ+1)*sizeof(int));p->successors[p->n_succ++]=succ;ActivityNode* s=&ad->nodes[succ];s->predecessors=realloc(s->predecessors,(s->n_pred+1)*sizeof(int));s->predecessors[s->n_pred++]=pred;}
double mbse_ad_critical_path(ActivityDiagram* ad){double* earliest=calloc(ad->n_nodes,sizeof(double));for(int i=0;i<ad->n_nodes;i++){double max_pred=0.0;for(int j=0;j<ad->nodes[i].n_pred;j++){int p=ad->nodes[i].predecessors[j];if(earliest[p]+ad->nodes[p].total_duration>max_pred)max_pred=earliest[p]+ad->nodes[p].total_duration;}earliest[i]=max_pred;}double cp=0.0;for(int i=0;i<ad->n_nodes;i++){double finish=earliest[i]+ad->nodes[i].total_duration;if(finish>cp)cp=finish;}ad->critical_path_duration=cp;free(earliest);return cp;}
void mbse_ad_print(ActivityDiagram* ad){printf("ActivityDiagram: %s (%d nodes, CP=%.2f)\n",ad->name,ad->n_nodes,ad->critical_path_duration);}
bool mbse_sm_is_reachable(StateMachine* sm, int target_state){bool* visited=calloc(sm->n_states,sizeof(bool));int* queue=calloc(sm->n_states,sizeof(int));int front=0,back=0;queue[back++]=sm->current_state;visited[sm->current_state]=true;while(front<back){int u=queue[front++];if(u==target_state){free(visited);free(queue);return true;}for(int i=0;i<sm->n_trans;i++){if(sm->trans[i].from_state==u&&!visited[sm->trans[i].to_state]){visited[sm->trans[i].to_state]=true;queue[back++]=sm->trans[i].to_state;}}}free(visited);free(queue);return false;}
int mbse_sm_reachable_states_count(StateMachine* sm){int count=0;for(int i=0;i<sm->n_states;i++)if(mbse_sm_is_reachable(sm,i))count++;return count;}
int mbse_sm_outgoing_transitions(StateMachine* sm, int state){int c=0;for(int i=0;i<sm->n_trans;i++)if(sm->trans[i].from_state==state)c++;return c;}
int mbse_ad_longest_chain(ActivityDiagram* ad){int max_chain=0;for(int i=0;i<ad->n_nodes;i++){int chain=1;ActivityNode* n=&ad->nodes[i];while(n->n_succ>0){chain++;n=&ad->nodes[n->successors[0]];}if(chain>max_chain)max_chain=chain;}return max_chain;}
double mbse_ad_total_duration(ActivityDiagram* ad){double sum=0.0;for(int i=0;i<ad->n_nodes;i++)sum+=ad->nodes[i].total_duration;return sum;}
int mbse_ad_parallel_paths(ActivityDiagram* ad){int max_parallel=0;for(int i=0;i<ad->n_nodes;i++){if(ad->nodes[i].n_succ>max_parallel)max_parallel=ad->nodes[i].n_succ;}return max_parallel;}

static int _validate_dims(int rows, int cols) {
    return (rows > 0 && cols > 0 && rows <= 1024 && cols <= 1024);
}
static double _clip_value(double x, double lo, double hi) {
    if (x < lo) return lo; if (x > hi) return hi; return x;
}
static int _count_nonzero(const double* data, int n) {
    int cnt = 0; for (int i = 0; i < n; i++) if (fabs(data[i]) > 1e-12) cnt++;
    return cnt;
}
static double _frobenius_norm(const double* data, int n) {
    double sum = 0.0; for (int i = 0; i < n; i++) sum += data[i] * data[i];
    return sqrt(sum);
}
static int _sign(double x) { return (x > 1e-12) ? 1 : ((x < -1e-12) ? -1 : 0); }

/* ── Condition number estimation ── */
double _estimate_condition_1norm(const double* A, int n) {
    if (n <= 0 || !A) return 1.0;
    double normA = 0.0;
    for (int j = 0; j < n; j++) {
        double col_sum = 0.0;
        for (int i = 0; i < n; i++) col_sum += fabs(A[i * n + j]);
        if (col_sum > normA) normA = col_sum;
    }
    return (normA > 1e-12) ? normA : 1.0;
}

/* ── Simple iterative refinement for linear systems ── */
int _iterative_refine(double* A, double* b, double* x, int n, int max_iter) {
    if (n <= 0 || !A || !b || !x || max_iter <= 0) return -1;
    double* r = (double*)calloc((size_t)n, sizeof(double));
    if (!r) return -1;
    for (int iter = 0; iter < max_iter; iter++) {
        double max_r = 0.0;
        for (int i = 0; i < n; i++) {
            r[i] = b[i];
            for (int j = 0; j < n; j++) r[i] -= A[i * n + j] * x[j];
            if (fabs(r[i]) > max_r) max_r = fabs(r[i]);
        }
        if (max_r < 1e-10) { free(r); return iter; }
        for (int i = 0; i < n; i++) x[i] += r[i];
    }
    free(r); return max_iter;
}

/* ── Vector/matrix normalization ── */
void _normalize_vector(double* v, int n) {
    double norm = _frobenius_norm(v, n);
    if (norm > 1e-12) for (int i = 0; i < n; i++) v[i] /= norm;
}
double _inner_product(const double* a, const double* b, int n) {
    double sum = 0.0; for (int i = 0; i < n; i++) sum += a[i] * b[i];
    return sum;
}

/* ============================================================================
 * Activity Diagram Analysis — CPM, PERT, Resource Loading
 *
 * Extends basic activity diagram with Critical Path Method (CPM),
 * Program Evaluation and Review Technique (PERT) three-point
 * estimation, and resource leveling.
 * ============================================================================ */

/* PERT three-point estimate for activity duration.
 * expected = (optimistic + 4*most_likely + pessimistic) / 6
 * variance = ((pessimistic - optimistic) / 6)^2 */
void mbse_pert_duration(double optimistic, double most_likely,
                          double pessimistic, double* expected,
                          double* variance) {
    if (expected) *expected = (optimistic + 4.0 * most_likely
                                + pessimistic) / 6.0;
    if (variance) {
        double std = (pessimistic - optimistic) / 6.0;
        *variance = std * std;
    }
}

/* Forward pass: compute earliest start (ES) and earliest finish (EF).
 * ES[i] = max(EF[pred]) for all predecessors.
 * EF[i] = ES[i] + duration[i].
 * Fills es[] and ef[] (caller alloc, size n).
 * Returns project duration (max EF). */
double mbse_forward_pass(ActivityDiagram* ad, double* es, double* ef) {
    if (!ad || !es || !ef || ad->n_nodes <= 0) return 0.0;
    int n = ad->n_nodes;
    memset(es, 0, (size_t)n * sizeof(double));
    memset(ef, 0, (size_t)n * sizeof(double));
    double max_finish = 0.0;
    /* Topological pass (nodes assumed ordered) */
    for (int i = 0; i < n; i++) {
        double max_es = 0.0;
        for (int p = 0; p < ad->nodes[i].n_pred; p++) {
            int pred = ad->nodes[i].predecessors[p];
            if (pred >= 0 && pred < n && ef[pred] > max_es)
                max_es = ef[pred];
        }
        es[i] = max_es;
        ef[i] = es[i] + ad->nodes[i].total_duration;
        if (ef[i] > max_finish) max_finish = ef[i];
    }
    return max_finish;
}

/* Backward pass: compute latest start (LS) and latest finish (LF).
 * LF[i] = min(LS[succ]) for all successors.
 * LS[i] = LF[i] - duration[i].
 * project_duration: from forward pass.
 * Fills ls[] and lf[] (caller alloc, size n). */
void mbse_backward_pass(ActivityDiagram* ad, double* ls, double* lf,
                          double project_duration) {
    if (!ad || !ls || !lf || ad->n_nodes <= 0) return;
    int n = ad->n_nodes;
    for (int i = 0; i < n; i++) {
        lf[i] = project_duration;
        ls[i] = project_duration;
    }
    /* Reverse topological pass */
    for (int i = n - 1; i >= 0; i--) {
        double min_lf = project_duration;
        for (int s = 0; s < ad->nodes[i].n_succ; s++) {
            int succ = ad->nodes[i].successors[s];
            if (succ >= 0 && succ < n && ls[succ] < min_lf)
                min_lf = ls[succ];
        }
        lf[i] = min_lf;
        ls[i] = lf[i] - ad->nodes[i].total_duration;
        if (ls[i] < 0.0) ls[i] = 0.0;
    }
}

/* Critical path identification: activities where ES = LS (zero float).
 * Fills is_critical[] (caller alloc, size n) with 1/0.
 * Returns number of critical activities. */
int mbse_critical_path(ActivityDiagram* ad, int* is_critical) {
    if (!ad || !is_critical || ad->n_nodes <= 0) return 0;
    int n = ad->n_nodes;
    double* es = (double*)calloc((size_t)n, sizeof(double));
    double* ef = (double*)calloc((size_t)n, sizeof(double));
    double* ls = (double*)calloc((size_t)n, sizeof(double));
    double* lf = (double*)calloc((size_t)n, sizeof(double));
    if (!es || !ef || !ls || !lf) {
        free(es); free(ef); free(ls); free(lf); return 0;
    }
    double pd = mbse_forward_pass(ad, es, ef);
    mbse_backward_pass(ad, ls, lf, pd);
    /* Float = LS - ES (zero for critical) */
    int crit_count = 0;
    for (int i = 0; i < n; i++) {
        double flt = ls[i] - es[i];
        is_critical[i] = (fabs(flt) < 1e-10) ? 1 : 0;
        if (is_critical[i]) crit_count++;
    }
    free(es); free(ef); free(ls); free(lf);
    return crit_count;
}

/* Float (slack) for each activity: LS - ES.
 * Fills float_vals[] (caller alloc, size n). */
void mbse_activity_floats(ActivityDiagram* ad, double* float_vals) {
    if (!ad || !float_vals || ad->n_nodes <= 0) return;
    int n = ad->n_nodes;
    double* es = (double*)calloc((size_t)n, sizeof(double));
    double* ef = (double*)calloc((size_t)n, sizeof(double));
    double* ls = (double*)calloc((size_t)n, sizeof(double));
    double* lf = (double*)calloc((size_t)n, sizeof(double));
    if (!es || !ef || !ls || !lf) {
        free(es); free(ef); free(ls); free(lf); return;
    }
    double pd = mbse_forward_pass(ad, es, ef);
    mbse_backward_pass(ad, ls, lf, pd);
    for (int i = 0; i < n; i++)
        float_vals[i] = ls[i] - es[i];
    free(es); free(ef); free(ls); free(lf);
}

/* State coverage analysis: which states are visited in
 * N random simulation steps. Returns fraction of states visited.
 * Uses uniform random transition selection. */
double mbse_state_coverage_sim(StateMachine* sm, int n_steps) {
    if (!sm || sm->n_states <= 0 || n_steps <= 0) return 0.0;
    int* visited = (int*)calloc((size_t)sm->n_states, sizeof(int));
    if (!visited) return 0.0;
    int current = sm->current_state;
    visited[current] = 1;
    /* Simple random walk through transitions */
    for (int step = 0; step < n_steps; step++) {
        int n_out = 0;
        int* out_trans = (int*)malloc((size_t)sm->n_trans * sizeof(int));
        if (!out_trans) break;
        for (int t = 0; t < sm->n_trans; t++) {
            if (sm->trans[t].from_state == current)
                out_trans[n_out++] = sm->trans[t].to_state;
        }
        if (n_out > 0) {
            int pick = step % n_out;  /* Deterministic pseudo-random */
            current = out_trans[pick];
            visited[current] = 1;
        }
        free(out_trans);
    }
    int count = 0;
    for (int i = 0; i < sm->n_states; i++)
        if (visited[i]) count++;
    free(visited);
    return (double)count / (double)sm->n_states;
}


/* State machine coverage: compute the fraction of states and transitions
 * that have been verified through simulation or testing.
 * Essential for DO-178C and ISO 26262 compliance. */
int mbse_sm_coverage(int n_states, int n_transitions, int n_visited_states,
                      int n_traversed_transitions, double* state_cov, double* trans_cov) {
    if (!state_cov || !trans_cov) return -1;
    *state_cov = (n_states > 0) ? (double)n_visited_states / (double)n_states : 1.0;
    *trans_cov = (n_transitions > 0) ? (double)n_traversed_transitions / (double)n_transitions : 1.0;
    return (*state_cov >= 0.95 && *trans_cov >= 0.90) ? 1 : 0;
}

/* Interface compatibility matrix: check that all component interfaces
 * are compatible (matching types, protocols, and data formats).
 * Returns number of incompatible interface pairs. */
int mbse_check_interfaces(int n_components, const int* interface_matrix, int* incompatible) {
    if (!interface_matrix || !incompatible || n_components <= 0) return -1;
    *incompatible = 0;
    for (int i = 0; i < n_components; i++)
        for (int j = i + 1; j < n_components; j++)
            if (interface_matrix[i * n_components + j] <= 0) (*incompatible)++;
    return 0;
}
