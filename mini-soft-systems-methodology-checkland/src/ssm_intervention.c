#include "ssm_core.h"
#include "ssm_intervention.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

InterventionPlan* ssm_intervention_create(const char* name) {
    InterventionPlan* ip = (InterventionPlan*)calloc(1, sizeof(InterventionPlan));
    if (!ip) return NULL;
    ip->name = strdup(name);
    ip->changes = (SystemicChange**)calloc(16, sizeof(SystemicChange*));
    return ip;
}
void ssm_intervention_free(InterventionPlan* ip) {
    if (!ip) return;
    free(ip->name);
    for (int i = 0; i < ip->n_changes; i++) {
        SystemicChange* sc = ip->changes[i];
        if (sc) { free(sc->description);
            for (int j = 0; j < sc->n_affected; j++) free(sc->stakeholders_affected[j]);
            free(sc->stakeholders_affected); free(sc); }
    }
    free(ip->changes); free(ip);
}
int ssm_intervention_add_change(InterventionPlan* ip, const char* desc, ChangeType type, double desirability, double feasibility) {
    if (!ip || !desc) return -1;
    if (ip->n_changes >= 16) {
        SystemicChange** nn = (SystemicChange**)realloc(ip->changes, (size_t)(ip->n_changes + 8) * sizeof(SystemicChange*));
        if (!nn) return -1; ip->changes = nn;
    }
    SystemicChange* sc = (SystemicChange*)calloc(1, sizeof(SystemicChange)); if (!sc) return -1;
    sc->id = ip->n_changes; sc->description = strdup(desc); sc->type = type;
    sc->desirability = desirability; sc->feasibility = feasibility; sc->sustainability = 0.5;
    sc->stakeholders_affected = (char**)calloc(8, sizeof(char*)); sc->implementation_time = 1.0; sc->risk_level = 0.3;
    ip->changes[ip->n_changes++] = sc; return sc->id;
}
void ssm_intervention_add_affected_stakeholder(InterventionPlan* ip, int change_id, const char* stakeholder) {
    if (!ip || !stakeholder || change_id < 0 || change_id >= ip->n_changes) return;
    SystemicChange* sc = ip->changes[change_id];
    int n = sc->n_affected;
    char** nn = (char**)realloc(sc->stakeholders_affected, (size_t)(n + 1) * sizeof(char*));
    if (!nn) return; sc->stakeholders_affected = nn;
    sc->stakeholders_affected[sc->n_affected++] = strdup(stakeholder);
}
double ssm_intervention_compute_priority(InterventionPlan* ip) {
    if (!ip || ip->n_changes < 1) return 0.0;
    double sum_d = 0.0, sum_f = 0.0, sum_s = 0.0;
    for (int i = 0; i < ip->n_changes; i++) {
        sum_d += ip->changes[i]->desirability;
        sum_f += ip->changes[i]->feasibility;
        sum_s += ip->changes[i]->sustainability;
    }
    double n = (double)ip->n_changes;
    ip->overall_desirability = sum_d / n;
    ip->overall_feasibility = sum_f / n;
    ip->implementation_priority = (ip->overall_desirability * 0.4 + ip->overall_feasibility * 0.6);
    return ip->implementation_priority;
}
void ssm_intervention_sort_by_priority(InterventionPlan* ip) {
    if (!ip || ip->n_changes < 2) return;
    for (int i = 0; i < ip->n_changes - 1; i++)
        for (int j = i + 1; j < ip->n_changes; j++) {
            double pi = ip->changes[i]->desirability * 0.4 + ip->changes[i]->feasibility * 0.6;
            double pj = ip->changes[j]->desirability * 0.4 + ip->changes[j]->feasibility * 0.6;
            if (pj > pi) { SystemicChange* t = ip->changes[i]; ip->changes[i] = ip->changes[j]; ip->changes[j] = t; }
        }
}
double ssm_intervention_implementation_risk(InterventionPlan* ip) {
    if (!ip || ip->n_changes < 1) return 1.0;
    double risk = 0.0;
    for (int i = 0; i < ip->n_changes; i++)
        risk += ip->changes[i]->risk_level * (1.0 - ip->changes[i]->feasibility);
    return risk / (double)ip->n_changes;
}
bool ssm_intervention_is_actionable(const InterventionPlan* ip) {
    if (!ip) return false;
    double priority = ssm_intervention_compute_priority((InterventionPlan*)ip);
    double risk = ssm_intervention_implementation_risk((InterventionPlan*)ip);
    return priority > 0.4 && risk < 0.7;
}
double ssm_intervention_expected_impact(const InterventionPlan* ip) {
    if (!ip || ip->n_changes < 1) return 0.0;
    double impact = 0.0;
    for (int i = 0; i < ip->n_changes; i++)
        impact += ip->changes[i]->desirability * ip->changes[i]->feasibility * (double)ip->changes[i]->n_affected;
    return impact / (double)ip->n_changes;
}
void ssm_intervention_generate_roadmap(const InterventionPlan* ip, int* ordering, int* n_steps) {
    if (!ip || !ordering || !n_steps) return;
    InterventionPlan* sorted = (InterventionPlan*)malloc(sizeof(InterventionPlan));
    if (!sorted) return;
    memcpy(sorted, ip, sizeof(InterventionPlan));
    sorted->changes = (SystemicChange**)malloc((size_t)ip->n_changes * sizeof(SystemicChange*));
    for (int i = 0; i < ip->n_changes; i++) sorted->changes[i] = ip->changes[i];
    ssm_intervention_sort_by_priority(sorted);
    int n = (ip->n_changes < 10) ? ip->n_changes : 10;
    for (int i = 0; i < n; i++) ordering[i] = sorted->changes[i]->id;
    *n_steps = n;
    free(sorted->changes); free(sorted);
}
void ssm_intervention_print(const InterventionPlan* ip) {
    if (!ip) { printf("(null IP)\n"); return; }
    printf("Intervention Plan: %s\n  Changes: %d  Desirability: %.2f  Feasibility: %.2f  Priority: %.2f\n",
        ip->name, ip->n_changes, ip->overall_desirability, ip->overall_feasibility, ip->implementation_priority);
    printf("  Actionable: %s  Risk: %.2f  Impact: %.2f\n",
        ssm_intervention_is_actionable(ip) ? "YES" : "NO",
        ssm_intervention_implementation_risk(ip), ssm_intervention_expected_impact(ip));
    for (int i = 0; i < ip->n_changes; i++) {
        SystemicChange* sc = ip->changes[i];
        printf("  [%d] %-40s type=%d D=%.2f F=%.2f S=%.2f risk=%.2f affected=%d\n",
            sc->id, sc->description, sc->type, sc->desirability, sc->feasibility, sc->sustainability, sc->risk_level, sc->n_affected);
    }
}

/* Implementation roadmap with dependencies */
void ssm_intervention_dependency_map(const InterventionPlan* ip, int** dep_matrix) {
    if (!ip || !dep_matrix || ip->n_changes < 2) return;
    for (int i = 0; i < ip->n_changes; i++)
        for (int j = 0; j < ip->n_changes; j++)
            dep_matrix[i][j] = (i < j) ? 1 : 0;
}
double ssm_intervention_cost_estimate(const InterventionPlan* ip) {
    if (!ip) return 0.0;
    double cost = 0.0;
    for (int i = 0; i < ip->n_changes; i++) {
        switch (ip->changes[i]->type) {
            case SSM_CHANGE_STRUCTURAL: cost += 100.0; break;
            case SSM_CHANGE_PROCEDURAL: cost += 30.0; break;
            case SSM_CHANGE_ATTITUDINAL: cost += 10.0; break;
            case SSM_CHANGE_SYSTEMIC: cost += 200.0; break;
        }
    }
    return cost;
}
double ssm_intervention_time_estimate(const InterventionPlan* ip) {
    if (!ip || ip->n_changes < 1) return 0.0;
    double total = 0.0;
    for (int i = 0; i < ip->n_changes; i++) total += ip->changes[i]->implementation_time;
    return total;
}
void ssm_intervention_risk_assessment(const InterventionPlan* ip, double* technical_risk, double* social_risk, double* political_risk) {
    if (!ip) return;
    *technical_risk = 0.0; *social_risk = 0.0; *political_risk = 0.0;
    for (int i = 0; i < ip->n_changes; i++) {
        switch (ip->changes[i]->type) {
            case SSM_CHANGE_STRUCTURAL: *political_risk += ip->changes[i]->risk_level; break;
            case SSM_CHANGE_PROCEDURAL: *technical_risk += ip->changes[i]->risk_level; break;
            case SSM_CHANGE_ATTITUDINAL: *social_risk += ip->changes[i]->risk_level; break;
            case SSM_CHANGE_SYSTEMIC: *political_risk += ip->changes[i]->risk_level * 0.5; *social_risk += ip->changes[i]->risk_level * 0.5; break;
        }
    }
    if (ip->n_changes > 0) { *technical_risk /= (double)ip->n_changes; *social_risk /= (double)ip->n_changes; *political_risk /= (double)ip->n_changes; }
}
bool ssm_intervention_is_sustainable(const InterventionPlan* ip) {
    if (!ip || ip->n_changes < 1) return false;
    double avg_sustainability = 0.0;
    for (int i = 0; i < ip->n_changes; i++) avg_sustainability += ip->changes[i]->sustainability;
    return (avg_sustainability / (double)ip->n_changes) > 0.5;
}
int ssm_intervention_quick_wins(const InterventionPlan* ip, int* quick_win_ids) {
    if (!ip || !quick_win_ids) return 0;
    int count = 0;
    for (int i = 0; i < ip->n_changes; i++)
        if (ip->changes[i]->feasibility > 0.7 && ip->changes[i]->implementation_time < 1.0)
            quick_win_ids[count++] = ip->changes[i]->id;
    return count;
}

/* Phase-based implementation planning */
void ssm_intervention_phase_plan(const InterventionPlan* ip, int** phase_assignments, int* n_phases) {
    if (!ip || !phase_assignments || !n_phases || ip->n_changes < 1) return;
    *n_phases = (ip->n_changes < 3) ? ip->n_changes : 3;
    for (int i = 0; i < ip->n_changes; i++) phase_assignments[0][i] = i % (*n_phases) + 1;
}

