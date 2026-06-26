#ifndef MBSE_BEHAVIOR_H
#define MBSE_BEHAVIOR_H
#include "mbse_core.h"
/* ===================================================================
 * Behavioral modeling captures system dynamics through:
 *   - State Machines: event-driven transitions with trigger/guard
 *   - Activity Diagrams: control flow, data flow, critical path
 *   - Deadlock detection: every non-final state must have egress
 *   - Reachability analysis: DFS from any initial state
 *
 * State semantics follow OMG SysML v2 state machine formalism.
 * Activity nodes model tasks with deterministic durations,
 * supporting critical path method (CPM) for schedule analysis.
 *
 * The critical path duration estimates earliest project
 * completion time assuming unlimited resources.
 * =================================================================== */
typedef enum { SM_IDLE=0, SM_ACTIVE=1, SM_ERROR=2, SM_OFF=3, SM_INIT=4 } StateType;
/*
 * Behavioral modeling captures system dynamics through:
 *   - State Machines: event-driven behavior with triggers
 *   - Activity Diagrams: control flow and data flow
 *   - Sequence-based simulation of scenarios
 *
 * Key operations: deadlock detection, reachability analysis,
 * critical path computation for timing analysis.
 */
typedef struct { int id; char* name; StateType type; bool is_initial, is_final; int* entry_actions, n_entry, *exit_actions, n_exit; } StateNode;
typedef struct { int id, from_state, to_state; char* name, *trigger, *guard; double probability; int* effects; int n_effects; } Transition;
typedef struct { int id, current_state; char* name; StateNode* states; int n_states, state_capacity; Transition* trans; int n_trans, trans_capacity; int* history; int hist_len, hist_capacity; } StateMachine;
typedef struct { int id; char* name; int* actions, n_actions; double* durations, total_duration; int* predecessors, n_pred, *successors, n_succ; } ActivityNode;
typedef struct { int id, start_node, end_node; char* name; ActivityNode* nodes; int n_nodes; double critical_path_duration; } ActivityDiagram;
StateMachine* mbse_sm_create(const char* name);
void mbse_sm_free(StateMachine* sm);
int mbse_sm_add_state(StateMachine* sm, const char* name, StateType t, bool init, bool fin);
int mbse_sm_add_transition(StateMachine* sm, int from, int to, const char* trigger, double prob);
bool mbse_sm_fire(StateMachine* sm, const char* trigger);
void mbse_sm_simulate(StateMachine* sm, const char** triggers, int n);
bool mbse_sm_is_deadlock_free(StateMachine* sm);
void mbse_sm_print(StateMachine* sm);
ActivityDiagram* mbse_ad_create(const char* name);
void mbse_ad_free(ActivityDiagram* ad);
int mbse_ad_add_node(ActivityDiagram* ad, const char* name, double dur);
void mbse_ad_add_edge(ActivityDiagram* ad, int pred, int succ);
double mbse_ad_critical_path(ActivityDiagram* ad);
void mbse_ad_print(ActivityDiagram* ad);
#endif
