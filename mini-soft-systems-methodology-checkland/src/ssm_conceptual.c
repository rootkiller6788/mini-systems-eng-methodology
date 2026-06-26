#include "ssm_core.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

ConceptualModel* ssm_conceptual_create(const char* rd_text) {
    ConceptualModel* cm = (ConceptualModel*)calloc(1, sizeof(ConceptualModel));
    if (!cm) return NULL;
    cm->root_definition_text = rd_text ? strdup(rd_text) : NULL;
    cm->activity_capacity = 8;
    cm->activities = (ConceptualActivity**)calloc(8, sizeof(ConceptualActivity*));
    return cm;
}
void ssm_conceptual_free(ConceptualModel* cm) {
    if (!cm) return;
    free(cm->root_definition_text);
    for (int i = 0; i < cm->n_activities; i++) {
        ConceptualActivity* a = cm->activities[i];
        if (a) {
            free(a->name); free(a->description);
            free(a->depends_on); free(a->enables);
            if (a->performance_indicators)
                for (int j = 0; j < a->n_indicators; j++) free(a->performance_indicators[j]);
            free(a->performance_indicators); free(a->indicator_targets);
            free(a);
        }
    }
    free(cm->activities); free(cm);
}
int ssm_conceptual_add_activity(ConceptualModel* cm, const char* name, const char* desc) {
    if (!cm || !name) return -1;
    if (cm->n_activities >= cm->activity_capacity) {
        cm->activity_capacity *= 2;
        ConceptualActivity** nn = (ConceptualActivity**)realloc(cm->activities,
            (size_t)cm->activity_capacity * sizeof(ConceptualActivity*));
        if (!nn) return -1; cm->activities = nn;
    }
    ConceptualActivity* a = (ConceptualActivity*)calloc(1, sizeof(ConceptualActivity));
    if (!a) return -1;
    a->id = cm->n_activities; a->name = strdup(name);
    a->description = desc ? strdup(desc) : NULL;
    a->dep_capacity = 4; a->depends_on = (int*)calloc(4, sizeof(int));
    a->en_capacity = 4; a->enables = (int*)calloc(4, sizeof(int));
    a->monitoring_frequency = SSM_DEFAULT_MONITORING;
    cm->activities[cm->n_activities++] = a;
    return a->id;
}
ConceptualActivity* ssm_conceptual_get_activity(ConceptualModel* cm, int id) {
    if (!cm || id < 0 || id >= cm->n_activities) return NULL;
    return cm->activities[id];
}
void ssm_conceptual_add_dependency(ConceptualModel* cm, int from_id, int to_id) {
    if (!cm || from_id < 0 || from_id >= cm->n_activities
        || to_id < 0 || to_id >= cm->n_activities) return;
    ConceptualActivity* from = cm->activities[from_id];
    ConceptualActivity* to = cm->activities[to_id];
    if (from->n_enables >= from->en_capacity) {
        from->en_capacity *= 2;
        int* nn = (int*)realloc(from->enables, (size_t)from->en_capacity * sizeof(int));
        if (!nn) return; from->enables = nn;
    }
    from->enables[from->n_enables++] = to_id;
    if (to->n_dependencies >= to->dep_capacity) {
        to->dep_capacity *= 2;
        int* nn = (int*)realloc(to->depends_on, (size_t)to->dep_capacity * sizeof(int));
        if (!nn) return; to->depends_on = nn;
    }
    to->depends_on[to->n_dependencies++] = from_id;
}
void ssm_activity_set_level(ConceptualActivity* act, int level) { if(act) act->level=level; }
void ssm_activity_add_indicator(ConceptualActivity* act, const char* indicator, double target) {
    if (!act || !indicator) return;
    int n = act->n_indicators;
    char** nn = (char**)realloc(act->performance_indicators, (size_t)(n+1)*sizeof(char*));
    double* vt = (double*)realloc(act->indicator_targets, (size_t)(n+1)*sizeof(double));
    if (!nn || !vt) { free(nn); free(vt); return; }
    act->performance_indicators = nn; act->indicator_targets = vt;
    act->performance_indicators[n] = strdup(indicator);
    act->indicator_targets[n] = target;
    act->n_indicators = n + 1;
}
void ssm_activity_set_monitoring(ConceptualActivity* act, double freq) { if(act)act->monitoring_frequency=freq; }
int ssm_conceptual_count_levels(ConceptualModel* cm) {
    if (!cm) return 0; int max_l = 0;
    for (int i = 0; i < cm->n_activities; i++)
        if (cm->activities[i]->level > max_l) max_l = cm->activities[i]->level;
    return (cm->max_level = max_l) + 1;
}
double ssm_conceptual_coverage(ConceptualModel* cm) {
    if (!cm || !cm->root_definition_text) return 0.0;
    cm->logical_coverage = (double)cm->n_activities / fmax((double)cm->n_activities + 5.0, 1.0);
    return cm->logical_coverage;
}
double ssm_conceptual_complexity(ConceptualModel* cm) {
    if (!cm || cm->n_activities < 2) return 0.0;
    int total_deps = 0;
    for (int i = 0; i < cm->n_activities; i++) total_deps += cm->activities[i]->n_dependencies;
    double max_deps = (double)cm->n_activities * ((double)cm->n_activities - 1.0);
    cm->structural_complexity = (max_deps > 0) ? (double)total_deps / max_deps : 0.0;
    return cm->structural_complexity;
}
double ssm_conceptual_connectivity(const ConceptualModel* cm) {
    if (!cm || cm->n_activities == 0) return 0.0;
    int edges = 0;
    for (int i = 0; i < cm->n_activities; i++) edges += cm->activities[i]->n_enables;
    return (double)edges / (double)cm->n_activities;
}
bool ssm_conceptual_is_connected(const ConceptualModel* cm) {
    if (!cm || cm->n_activities <= 1) return true;
    bool* visited = (bool*)calloc((size_t)cm->n_activities, sizeof(bool));
    int* queue = (int*)malloc((size_t)cm->n_activities * sizeof(int));
    if (!visited || !queue) { free(visited); free(queue); return false; }
    int front = 0, back = 0; visited[0] = true; queue[back++] = 0;
    while (front < back) {
        int u = queue[front++];
        for (int i = 0; i < cm->activities[u]->n_enables; i++) {
            int v = cm->activities[u]->enables[i];
            if (!visited[v]) { visited[v] = true; queue[back++] = v; }
        }
    }
    bool conn = true;
    for (int i = 0; i < cm->n_activities; i++)
        if (!visited[i]) { conn = false; break; }
    free(visited); free(queue); return conn;
}
int* ssm_conceptual_topological_sort(ConceptualModel* cm, int* n_out) {
    if (!cm || !n_out) return NULL;
    int* indeg = (int*)calloc((size_t)cm->n_activities, sizeof(int));
    for (int i = 0; i < cm->n_activities; i++) indeg[i] = cm->activities[i]->n_dependencies;
    int* order = (int*)malloc((size_t)cm->n_activities * sizeof(int));
    int idx = 0;
    for (int iter = 0; iter < cm->n_activities; iter++) {
        int found = -1;
        for (int i = 0; i < cm->n_activities; i++)
            if (indeg[i] == 0) { found = i; break; }
        if (found < 0) break;
        order[idx++] = found; indeg[found] = -1;
        for (int i = 0; i < cm->activities[found]->n_enables; i++)
            indeg[cm->activities[found]->enables[i]]--;
    }
    free(indeg); *n_out = idx; return order;
}
double ssm_conceptual_monitorability(const ConceptualModel* cm) {
    if (!cm || cm->n_activities == 0) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < cm->n_activities; i++)
        if (cm->activities[i]->n_indicators > 0) sum += 1.0;
    return sum / (double)cm->n_activities;
}
bool ssm_conceptual_is_complete(const ConceptualModel* cm) {
    return cm && ssm_conceptual_connectivity(cm) > 0.3 && ssm_conceptual_is_connected(cm);
}
void ssm_conceptual_print(const ConceptualModel* cm) {
    if (!cm) { printf("(null CM)\n"); return; }
    printf("CM: %s\n  Activities=%d Coverage=%.2f Complexity=%.2f\n",
        cm->root_definition_text ? cm->root_definition_text : "?",
        cm->n_activities, cm->logical_coverage, cm->structural_complexity);
    for (int i = 0; i < cm->n_activities; i++) {
        ConceptualActivity* a = cm->activities[i];
        printf("  [%d] %s L%d deps:", a->id, a->name ? a->name : "?", a->level);
        for (int j = 0; j < a->n_dependencies; j++) printf(" %d", a->depends_on[j]);
        printf(" en:");
        for (int j = 0; j < a->n_enables; j++) printf(" %d", a->enables[j]);
        printf("\n");
    }
}
void ssm_conceptual_print_activity(const ConceptualActivity* act) {
    if (!act) { printf("(null CA)\n"); return; }
    printf("Activity[%d]: %s\n  %s\n  Level=%d Deps=%d Enables=%d Mon=%.2f\n",
        act->id, act->name, act->description ? act->description : "",
        act->level, act->n_dependencies, act->n_enables, act->monitoring_frequency);
}
/* Advanced model metrics */
double ssm_conceptual_bottleneck_score(ConceptualModel* cm) {
    if (!cm || cm->n_activities < 1) return 0.0;
    int max_deps = 0;
    for (int i = 0; i < cm->n_activities; i++)
        if (cm->activities[i]->n_dependencies > max_deps) max_deps = cm->activities[i]->n_dependencies;
    return (double)max_deps / fmax((double)cm->n_activities, 1.0);
}
double ssm_conceptual_parallelism(const ConceptualModel* cm) {
    if (!cm || cm->n_activities < 2) return 0.0;
    int independent = 0;
    for (int i = 0; i < cm->n_activities; i++)
        if (cm->activities[i]->n_dependencies == 0 && cm->activities[i]->n_enables == 0) independent++;
    return (double)independent / (double)cm->n_activities;
}
double ssm_conceptual_critical_path_length(const ConceptualModel* cm) {
    if (!cm || cm->n_activities < 1) return 0.0;
    int max_level = 0;
    for (int i = 0; i < cm->n_activities; i++)
        if (cm->activities[i]->level > max_level) max_level = cm->activities[i]->level;
    return (double)max_level;
}
double ssm_conceptual_monitoring_deficit(const ConceptualModel* cm) {
    if (!cm) return 1.0;
    return 1.0 - ssm_conceptual_monitorability(cm);
}
bool ssm_conceptual_validate_monitoring(const ConceptualModel* cm, double threshold) {
    return ssm_conceptual_monitorability(cm) >= threshold;
}
void ssm_conceptual_generate_monitoring_plan(const ConceptualModel* cm, int* activity_ids, double* frequencies, int* n_plan) {
    if (!cm || !activity_ids || !frequencies || !n_plan) return;
    int count = 0;
    for (int i = 0; i < cm->n_activities && count < 20; i++) {
        activity_ids[count] = cm->activities[i]->id;
        frequencies[count] = cm->activities[i]->monitoring_frequency;
        count++;
    }
    *n_plan = count;
}
double ssm_conceptual_resource_intensity(const ConceptualModel* cm) {
    if (!cm || cm->n_activities < 1) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < cm->n_activities; i++)
        sum += (double)cm->activities[i]->n_indicators;
    return sum / fmax((double)cm->n_activities, 1.0);
}
int ssm_conceptual_find_orphans(const ConceptualModel* cm, int* orphan_ids) {
    if (!cm || !orphan_ids) return 0;
    int count = 0;
    for (int i = 0; i < cm->n_activities; i++)
        if (cm->activities[i]->n_dependencies == 0 && cm->activities[i]->n_enables == 0)
            orphan_ids[count++] = i;
    return count;
}
bool ssm_conceptual_has_sufficient_monitoring(const ConceptualModel* cm) {
    return cm && ssm_conceptual_monitorability(cm) > 0.3;
}

bool ssm_conceptual_validate_minimal(const ConceptualModel* cm) {
    return cm && cm->n_activities >= 2 && ssm_conceptual_is_connected(cm) && ssm_conceptual_connectivity(cm) > 0.2;
}
double ssm_conceptual_activity_density(const ConceptualModel* cm) {
    if (!cm || cm->n_activities < 2) return 0.0;
    return (double)(cm->n_activities) / fmax((double)(cm->max_level + 1), 1.0);
}
