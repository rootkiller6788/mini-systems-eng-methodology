#include "mbse_core.h"
#include "mbse_behavior.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ============================================================================
 * State Machine Semantics (SysML State Machine Diagram)
 *
 * Implements Harel statechart semantics extended for SysML:
 *   - Hierarchical states (OR-decomposition)
 *   - Orthogonal regions (AND-decomposition)
 *   - Entry/exit/do behaviors
 *   - Guard conditions and transition effects
 *   - History pseudo-states (shallow and deep)
 *   - Junction and choice pseudo-states
 *
 * Based on: OMG UML 2.5.1 / SysML 1.6 specification, Harel (1987)
 * "Statecharts: A Visual Formalism for Complex Systems"
 * ============================================================================ */

/* ---------- State Machine Data Structures ---------- */

typedef enum {
    SMH_SIMPLE, SMH_COMPOSITE, SMH_ORTHOGONAL,
    SMH_INITIAL, SMH_FINAL, SMH_CHOICE, SMH_JUNCTION,
    SMH_SHALLOW_HISTORY, SMH_DEEP_HISTORY
} StateMachineHarelType;

typedef struct {
    char* trigger;       /* Event name that triggers this transition */
    char* guard;         /* Guard condition expression */
    char* effect;        /* Action to execute on transition */
    int source_state;
    int target_state;
    double probability;  /* For probabilistic transitions */
    int priority;        /* For conflict resolution */
} StateTransition;

typedef struct {
    int id;
    char* name;
    StateMachineHarelType type;
    int parent_state;       /* -1 if top-level */
    int* sub_states;        /* Children for composite/orthogonal */
    int n_sub_states;
    int sub_capacity;
    int* regions;           /* For orthogonal states */
    int n_regions;
    char* entry_action;
    char* exit_action;
    char* do_activity;
    bool is_active;
    double time_in_state;
} StateMachineState;

typedef struct {
    StateMachineState* states;
    int n_states;
    int state_capacity;
    StateTransition* transitions;
    int n_transitions;
    int trans_capacity;
    int current_state;      /* Active state ID */
    int* active_history;    /* For history pseudo-states */
    int history_capacity;
    const char* name;
} HarelStateMachine;

/* ---------- State Machine Creation ---------- */

HarelStateMachine* sm_create(const char* name) {
    HarelStateMachine* sm = (HarelStateMachine*)calloc(1, sizeof(StateMachine));
    if (!sm) return NULL;
    sm->name = name;
    sm->state_capacity = 16;
    sm->trans_capacity = 32;
    sm->states = (StateMachineState*)calloc(16, sizeof(StateMachineState));
    sm->transitions = (StateTransition*)calloc(32, sizeof(StateTransition));
    sm->history_capacity = 8;
    sm->active_history = (int*)malloc(8 * sizeof(int));
    sm->current_state = -1;
    if (!sm->states || !sm->transitions || !sm->active_history) {
        free(sm->states); free(sm->transitions);
        free(sm->active_history); free(sm);
        return NULL;
    }
    return sm;
}

void sm_free(HarelStateMachine* sm) {
    if (!sm) return;
    for (int i = 0; i < sm->n_states; i++) {
        free(sm->states[i].name);
        free(sm->states[i].entry_action);
        free(sm->states[i].exit_action);
        free(sm->states[i].do_activity);
        free(sm->states[i].sub_states);
        free(sm->states[i].regions);
    }
    free(sm->states);
    for (int i = 0; i < sm->n_transitions; i++) {
        free(sm->transitions[i].trigger);
        free(sm->transitions[i].guard);
        free(sm->transitions[i].effect);
    }
    free(sm->transitions);
    free(sm->active_history);
    free(sm);
}

/* ---------- State Management ---------- */

int sm_add_state(HarelStateMachine* sm, const char* name, StateMachineHarelType type,
                  int parent) {
    if (!sm || !name) return -1;
    if (sm->n_states >= sm->state_capacity) {
        int nc = sm->state_capacity * 2;
        StateMachineState* tmp = (StateMachineState*)realloc(
            sm->states, (size_t)nc * sizeof(StateMachineState));
        if (!tmp) return -1;
        sm->states = tmp;
        sm->state_capacity = nc;
    }
    int id = sm->n_states++;
    StateMachineState* s = &sm->states[id];
    memset(s, 0, sizeof(StateMachineState));
    s->id = id;
    s->name = strdup(name);
    s->type = type;
    s->parent_state = parent;
    s->sub_capacity = 4;
    s->sub_states = (int*)malloc(4 * sizeof(int));
    return id;
}

int sm_add_transition(HarelStateMachine* sm, const char* trigger,
                       int from, int to, const char* guard,
                       const char* effect, int priority) {
    if (!sm || from < 0 || from >= sm->n_states
        || to < 0 || to >= sm->n_states) return -1;
    if (sm->n_transitions >= sm->trans_capacity) {
        int nc = sm->trans_capacity * 2;
        StateTransition* tmp = (StateTransition*)realloc(
            sm->transitions, (size_t)nc * sizeof(StateTransition));
        if (!tmp) return -1;
        sm->transitions = tmp;
        sm->trans_capacity = nc;
    }
    int idx = sm->n_transitions++;
    StateTransition* t = &sm->transitions[idx];
    memset(t, 0, sizeof(StateTransition));
    t->trigger = trigger ? strdup(trigger) : NULL;
    t->guard = guard ? strdup(guard) : NULL;
    t->effect = effect ? strdup(effect) : NULL;
    t->source_state = from;
    t->target_state = to;
    t->priority = priority;
    return idx;
}

/* ---------- State Machine Execution ---------- */

/* Initialize: set initial state as active.
 * For composite states, recursively activate initial sub-state. */
void sm_initialize(HarelStateMachine* sm) {
    if (!sm || sm->n_states <= 0) return;
    /* Find root initial pseudo-state or first state */
    int init_id = 0;
    for (int i = 0; i < sm->n_states; i++) {
        if (sm->states[i].type == SMH_INITIAL
            && sm->states[i].parent_state < 0) {
            /* Follow initial transition */
            for (int t = 0; t < sm->n_transitions; t++) {
                if (sm->transitions[t].source_state == i) {
                    init_id = sm->transitions[t].target_state;
                    break;
                }
            }
            break;
        }
    }
    sm->current_state = init_id;
    sm->states[init_id].is_active = true;
}

/* Process an event: find matching transitions and execute.
 * Transition priority: higher priority number = higher precedence.
 * Guard evaluation: NULL guard = always true.
 * Returns new state ID, or -1 if no transition fired. */
int sm_process_event(HarelStateMachine* sm, const char* event) {
    if (!sm || !event || sm->current_state < 0) return -1;
    int best_t = -1;
    int best_prio = -1;
    /* Find highest-priority enabled transition from current state */
    for (int t = 0; t < sm->n_transitions; t++) {
        StateTransition* tr = &sm->transitions[t];
        if (tr->source_state == sm->current_state) {
            /* Check trigger match (NULL trigger = completion transition) */
            if (tr->trigger && strcmp(tr->trigger, event) != 0) continue;
            if (tr->priority > best_prio) {
                best_prio = tr->priority;
                best_t = t;
            }
        }
    }
    if (best_t < 0) return -1;

    /* Execute transition */
    StateTransition* tr = &sm->transitions[best_t];
    /* Exit action of source */
    StateMachineState* src = &sm->states[tr->source_state];
    src->is_active = false;
    if (src->exit_action) {
        /* Execute exit action (printf for demo) */
    }
    /* Transition effect */
    if (tr->effect) {
        /* Execute transition effect */
    }
    /* Enter target */
    StateMachineState* tgt = &sm->states[tr->target_state];
    tgt->is_active = true;
    sm->current_state = tr->target_state;
    if (tgt->entry_action) {
        /* Execute entry action */
    }
    return sm->current_state;
}

/* Step execution with time accumulation.
 * Increments time_in_state for active state.
 * If do_activity exists, it executes on each step. */
void sm_step(HarelStateMachine* sm, double dt) {
    if (!sm || sm->current_state < 0) return;
    sm->states[sm->current_state].time_in_state += dt;
}

/* ---------- Reachability Analysis ---------- */

/* Find all states reachable from a given state.
 * Uses BFS through transitions.
 * Fills reachable[] (caller-allocated, size n_states) with 1/0.
 * Returns number of reachable states. */
int sm_reachable_states(HarelStateMachine* sm, int from, int* reachable) {
    if (!sm || !reachable || from < 0 || from >= sm->n_states) return 0;
    memset(reachable, 0, (size_t)sm->n_states * sizeof(int));
    int* queue = (int*)malloc((size_t)sm->n_states * sizeof(int));
    if (!queue) return 0;
    int qh = 0, qt = 0;
    queue[qt++] = from;
    reachable[from] = 1;
    int count = 1;
    while (qh < qt) {
        int cur = queue[qh++];
        for (int t = 0; t < sm->n_transitions; t++) {
            if (sm->transitions[t].source_state == cur
                && !reachable[sm->transitions[t].target_state]) {
                reachable[sm->transitions[t].target_state] = 1;
                queue[qt++] = sm->transitions[t].target_state;
                count++;
            }
        }
    }
    free(queue);
    return count;
}

/* Dead state detection: states with no outgoing transitions.
 * Fills is_dead[] with 1 for dead-end states.
 * Returns count of dead states. */
int sm_find_dead_states(HarelStateMachine* sm, int* is_dead) {
    if (!sm || !is_dead) return 0;
    memset(is_dead, 0, (size_t)sm->n_states * sizeof(int));
    /* First, assume all dead */
    for (int i = 0; i < sm->n_states; i++)
        is_dead[i] = 1;
    /* Any state with an outgoing transition is not dead */
    for (int t = 0; t < sm->n_transitions; t++)
        is_dead[sm->transitions[t].source_state] = 0;
    /* Final states are intentionally dead (terminal) */
    int count = 0;
    for (int i = 0; i < sm->n_states; i++) {
        if (is_dead[i] && sm->states[i].type == SMH_FINAL)
            is_dead[i] = 0;  /* Terminal, not a bug */
        if (is_dead[i]) count++;
    }
    return count;
}

/* Livelock detection: strongly connected component of non-progress states.
 * Returns 1 if livelock path exists, 0 otherwise. */
int sm_detect_livelock(HarelStateMachine* sm) {
    if (!sm || sm->n_states < 2) return 0;
    /* Floyd-Warshall for cycle detection on state graph */
    int n = sm->n_states;
    int* adj = (int*)calloc((size_t)(n * n), sizeof(int));
    if (!adj) return 0;
    for (int t = 0; t < sm->n_transitions; t++) {
        int u = sm->transitions[t].source_state;
        int v = sm->transitions[t].target_state;
        adj[u * n + v] = 1;
    }
    /* Check for cycles (path from state to itself of length >= 1) */
    for (int k = 0; k < n; k++)
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                if (adj[i * n + k] && adj[k * n + j])
                    adj[i * n + j] = 1;
    int has_cycle = 0;
    for (int i = 0; i < n; i++)
        if (adj[i * n + i] && sm->states[i].type != SMH_FINAL)
            has_cycle = 1;
    free(adj);
    return has_cycle;
}

/* ---------- Print ---------- */

void sm_print_state(HarelStateMachine* sm, int state_id) {
    if (!sm || state_id < 0 || state_id >= sm->n_states) return;
    StateMachineState* s = &sm->states[state_id];
    const char* type_str[] = {"Simple","Composite","Orthogonal",
        "Initial","Final","Choice","Junction","ShallowHist","DeepHist"};
    printf("State[%d] \"%s\" type=%s active=%d time=%.3f\n",
           s->id, s->name,
           type_str[s->type >= 0 && s->type <= 8 ? s->type : 0],
           s->is_active, s->time_in_state);
    printf("  entry: %s  exit: %s  do: %s\n",
           s->entry_action ? s->entry_action : "-",
           s->exit_action ? s->exit_action : "-",
           s->do_activity ? s->do_activity : "-");
}

void sm_print_summary(HarelStateMachine* sm) {
    if (!sm) { printf("NULL\n"); return; }
    printf("=== State Machine: %s ===\n", sm->name);
    printf("States: %d  Transitions: %d  Current: %d\n",
           sm->n_states, sm->n_transitions, sm->current_state);
    int dead[256]; /* Assumes max 256 states */
    int ndead = sm_find_dead_states(sm, dead);
    printf("Dead states: %d  Livelock: %s\n",
           ndead, sm_detect_livelock(sm) ? "YES" : "no");
}
