#include "mbse_core.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ==============================================================
 * MBSE Model Transformations and Consistency Checking
 *
 * SysML-inspired model transformations and cross-view consistency.
 * Implements basic QVT (Query/View/Transformation) patterns.
 * ============================================================== */

/* Compute model coverage: requirements traced to design elements */
double mbse_requirements_coverage(int n_requirements, const int* traced_to_design,
                                   int n_design_elements, int n_traced) {
    if (n_requirements < 1) return 0.0;
    int covered = 0;
    for (int i = 0; i < n_requirements && traced_to_design; i++)
        if (traced_to_design[i] >= 0) covered++;
    return (double)covered / (double)n_requirements * 100.0;
}

/* Interface compatibility check: matching data types and directions */
int mbse_model_interface_compatibility(const char* port1_type, const char* port2_type,
                                  int port1_dir, int port2_dir) {
    if (!port1_type || !port2_type) return -1;
    /* Directions: 0=in, 1=out, 2=inout. Compatible if opposite. */
    bool dir_ok = (port1_dir == 0 && port2_dir == 1) ||
                  (port1_dir == 1 && port2_dir == 0) ||
                  (port1_dir == 2 || port2_dir == 2);
    if (!dir_ok) return 0;
    return (strcmp(port1_type, port2_type) == 0) ? 1 : 0;
}

/* Stereotype application: count model elements with given stereotype */
int mbse_stereotype_count(const char** element_stereotypes, int n_elements,
                           const char* target_stereotype) {
    if (!element_stereotypes || !target_stereotype || n_elements < 1) return 0;
    int count = 0;
    for (int i = 0; i < n_elements; i++)
        if (element_stereotypes[i] && strcmp(element_stereotypes[i], target_stereotype) == 0)
            count++;
    return count;
}

/* View consistency: check that all design elements appear in at least one view */
double mbse_view_consistency(const bool** view_matrices, int n_views,
                              int n_elements) {
    if (!view_matrices || n_views < 1 || n_elements < 1) return 0.0;
    int visible = 0;
    for (int e = 0; e < n_elements; e++) {
        for (int v = 0; v < n_views; v++)
            if (view_matrices[v] && view_matrices[v][e]) { visible++; break; }
    }
    return (double)visible / (double)n_elements * 100.0;
}

/* Allocation analysis: functions allocated to components */
int mbse_allocation_gaps(const int* function_allocations, int n_functions,
                          int n_components) {
    if (!function_allocations || n_functions < 1) return -1;
    int unallocated = 0;
    for (int i = 0; i < n_functions; i++)
        if (function_allocations[i] < 0 || function_allocations[i] >= n_components)
            unallocated++;
    return unallocated;
}

/* Parametric constraint satisfaction: y = f(x) satisfied? */
double mbse_constraint_residual(double (*constraint_func)(double*, int),
                                 double* inputs, int n_inputs, double target) {
    if (!constraint_func || !inputs || n_inputs < 1) return INFINITY;
    double actual = constraint_func(inputs, n_inputs);
    return fabs(actual - target);
}

/* Model maturity index: based on views completed and validated */
double mbse_model_maturity(int n_views_completed, int n_views_total,
                            int n_requirements_validated, int n_requirements_total,
                            int n_interfaces_defined, int n_interfaces_total) {
    double view_score = (n_views_total > 0) ? (double)n_views_completed / (double)n_views_total : 0.0;
    double req_score = (n_requirements_total > 0) ? (double)n_requirements_validated / (double)n_requirements_total : 0.0;
    double iface_score = (n_interfaces_total > 0) ? (double)n_interfaces_defined / (double)n_interfaces_total : 0.0;
    return (view_score * 0.3 + req_score * 0.4 + iface_score * 0.3) * 100.0;
}

/* BDD (Block Definition Diagram) structural analysis: count parts, references, ports */
typedef struct { int n_blocks; int n_ports; int n_connectors; } MBSEBDDStats;
MBSEBDDStats mbse_bdd_statistics(const int* block_port_counts, int n_blocks,
                                   const int* connector_matrix, int n_connectors) {
    MBSEBDDStats stats = {n_blocks, 0, n_connectors};
    for (int i = 0; i < n_blocks && block_port_counts; i++)
        stats.n_ports += block_port_counts[i];
    return stats;
}

/* IBD (Internal Block Definition) connectivity density:
 * ratio of actual to possible connections */
double mbse_connectivity_density(int n_actual_connections, int n_ports) {
    if (n_ports < 2) return 0.0;
    int max_connections = n_ports * (n_ports - 1) / 2;
    return (double)n_actual_connections / (double)max_connections;
}

/* Requirements traceability: count forward/backward traces */
int mbse_trace_gaps(const int* req_to_element, int n_requirements,
                     const int* element_to_req, int n_elements) {
    if (!req_to_element || !element_to_req) return -1;
    int untraced_reqs = 0;
    for (int i = 0; i < n_requirements; i++)
        if (req_to_element[i] < 0) untraced_reqs++;
    return untraced_reqs;
}

/* Activity diagram: compute critical path length */
double mbse_model_critical_path(const double* activity_durations,
                           const int* dependencies, int n_activities) {
    if (!activity_durations || n_activities < 1) return 0.0;
    double max_path = 0.0;
    for (int i = 0; i < n_activities; i++)
        if (activity_durations[i] > max_path) max_path = activity_durations[i];
    return max_path;
}
/* State machine: check if all states are reachable from initial state */
int mbse_state_reachability(const bool* adjacency_matrix, int n_states, int initial_state) {
    if (!adjacency_matrix || n_states < 1 || initial_state < 0) return 0;
    bool* visited = (bool*)calloc((size_t)n_states, sizeof(bool));
    int* queue = (int*)malloc((size_t)n_states * sizeof(int));
    int head = 0, tail = 0;
    queue[tail++] = initial_state; visited[initial_state] = true;
    while (head < tail) {
        int cur = queue[head++];
        for (int next = 0; next < n_states; next++)
            if (adjacency_matrix[cur*n_states+next] && !visited[next]) {
                visited[next] = true; queue[tail++] = next;
            }
    }
    int reachable = 0;
    for (int i = 0; i < n_states; i++) if (visited[i]) reachable++;
    free(visited); free(queue);
    return reachable;
}

/* Sequence diagram: message count by type */
typedef struct { int sync; int async; int reply; int create; int destroy; } MBSEMsgCount;
MBSEMsgCount mbse_message_statistics(const int* message_types, int n_messages) {
    MBSEMsgCount c = {0};
    if (!message_types) return c;
    for (int i = 0; i < n_messages; i++) {
        switch (message_types[i]) {
            case 0: c.sync++; break;
            case 1: c.async++; break;
            case 2: c.reply++; break;
            case 3: c.create++; break;
            case 4: c.destroy++; break;
        }
    }
    return c;
}

/* Use case coverage: % of use cases with allocated sequence diagrams */
double mbse_use_case_coverage(int n_use_cases, const bool* has_sequence_diagram) {
    if (!has_sequence_diagram || n_use_cases < 1) return 0.0;
    int covered = 0;
    for (int i = 0; i < n_use_cases; i++) if (has_sequence_diagram[i]) covered++;
    return (double)covered / (double)n_use_cases * 100.0;
}
/* Parametric diagram: evaluate constraint block */
double mbse_evaluate_constraint(const double* bound_values, int n_values,
                                 const char* operator_type) {
    if (!bound_values || n_values < 2) return 0.0;
    if (operator_type && strcmp(operator_type, "sum") == 0) {
        double s = 0.0; for (int i = 0; i < n_values; i++) s += bound_values[i];
        return s;
    }
    return bound_values[0] + bound_values[1];
}

/* Verify model completeness against meta-model */
int mbse_meta_model_compliance(const bool* required_elements_present,
                                 int n_required) {
    if (!required_elements_present || n_required < 1) return -1;
    for (int i = 0; i < n_required; i++)
        if (!required_elements_present[i]) return 0;
    return 1;
}/* MBSE variant management: count configuration variants */
int mbse_variant_count(const int* feature_selections, int n_features) { if(!feature_selections)return 0; int v=1; for(int i=0;i<n_features;i++) v*=(feature_selections[i]>0?2:1); return v; }
/* MBSE version compatibility check */
int mbse_version_compatible(int v1_major, int v1_minor, int v2_major, int v2_minor) { return (v1_major==v2_major&&v1_minor<=v2_minor)?1:0; }
/* MBSE risk matrix: likelihood * consequence score */
double mbse_risk_score(double likelihood, double consequence) {
    return likelihood * consequence;
}
/* MBSE stakeholder need priority ranking */
int mbse_stakeholder_priority(const double* need_weights, int n_needs, int* ranking) {
    if(!need_weights||!ranking||n_needs<1)return -1;
    for(int i=0;i<n_needs;i++)ranking[i]=i;
    for(int i=0;i<n_needs-1;i++)for(int j=i+1;j<n_needs;j++)
        if(need_weights[ranking[j]]>need_weights[ranking[i]]){int t=ranking[i];ranking[i]=ranking[j];ranking[j]=t;}
    return 0;
}
