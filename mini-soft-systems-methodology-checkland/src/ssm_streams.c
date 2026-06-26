#include "ssm_core.h"
#include "ssm_streams.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

RichPicture* ssm_rich_picture_create(const char* name) {
    RichPicture* rp = (RichPicture*)calloc(1, sizeof(RichPicture));
    if (!rp) return NULL;
    rp->situation_name = strdup(name);
    rp->node_capacity = 16;
    rp->nodes = (RichPictureNode**)calloc(16, sizeof(RichPictureNode*));
    return rp;
}
void ssm_rich_picture_free(RichPicture* rp) {
    if (!rp) return;
    free(rp->situation_name);
    for (int i = 0; i < rp->n_nodes; i++) {
        RichPictureNode* n = rp->nodes[i];
        if (n) { free(n->label); free(n->description); free(n->connected_to); free(n); }
    }
    free(rp->nodes); free(rp);
}
int ssm_rp_add_node(RichPicture* rp, const char* label, RichPictureElement type, double x, double y) {
    if (!rp || !label) return -1;
    if (rp->n_nodes >= rp->node_capacity) {
        rp->node_capacity *= 2;
        RichPictureNode** nn = (RichPictureNode**)realloc(rp->nodes, (size_t)rp->node_capacity * sizeof(RichPictureNode*));
        if (!nn) return -1; rp->nodes = nn;
    }
    RichPictureNode* n = (RichPictureNode*)calloc(1, sizeof(RichPictureNode));
    if (!n) return -1;
    n->id = rp->n_nodes; n->label = strdup(label); n->type = type;
    n->x_position = x; n->y_position = y; n->salience = 0.5;
    n->conn_capacity = 8; n->connected_to = (int*)calloc(8, sizeof(int));
    rp->nodes[rp->n_nodes++] = n;
    return n->id;
}
void ssm_rp_connect(RichPicture* rp, int from_id, int to_id) {
    if (!rp || from_id < 0 || from_id >= rp->n_nodes || to_id < 0 || to_id >= rp->n_nodes) return;
    RichPictureNode* n = rp->nodes[from_id];
    if (n->n_connections >= n->conn_capacity) { n->conn_capacity *= 2;
        int* nn = (int*)realloc(n->connected_to, (size_t)n->conn_capacity * sizeof(int));
        if (!nn) return; n->connected_to = nn; }
    n->connected_to[n->n_connections++] = to_id;
}
void ssm_rp_set_salience(RichPicture* rp, int node_id, double salience) {
    if (rp && node_id >= 0 && node_id < rp->n_nodes) rp->nodes[node_id]->salience = salience;
}
double ssm_rp_compute_complexity(RichPicture* rp) {
    if (!rp || rp->n_nodes < 2) return 0.0;
    int edges = 0;
    for (int i = 0; i < rp->n_nodes; i++) edges += rp->nodes[i]->n_connections;
    double max_edges = (double)rp->n_nodes * (double)(rp->n_nodes - 1);
    rp->complexity_index = (max_edges > 0) ? (double)edges / max_edges : 0.0;
    return rp->complexity_index;
}
double ssm_rp_compute_conflict_density(RichPicture* rp) {
    if (!rp || rp->n_nodes < 1) return 0.0;
    int conflicts = 0;
    for (int i = 0; i < rp->n_nodes; i++)
        if (rp->nodes[i]->type == SSM_ELEM_CONFLICT) conflicts++;
    rp->conflict_density = (double)conflicts / (double)rp->n_nodes;
    return rp->conflict_density;
}
void ssm_rp_print(const RichPicture* rp) {
    if (!rp) { printf("(null RP)\n"); return; }
    printf("Rich Picture: %s\n  Nodes: %d  Complexity: %.3f  Conflict: %.3f\n",
        rp->situation_name, rp->n_nodes, rp->complexity_index, rp->conflict_density);
    for (int i = 0; i < rp->n_nodes; i++) {
        RichPictureNode* n = rp->nodes[i];
        printf("  [%d] %-20s type=%d pos=(%.2f,%.2f) sal=%.2f conn=%d\n",
            i, n->label, n->type, n->x_position, n->y_position, n->salience, n->n_connections);
    }
}

SocialSystemAnalysis* ssm_social_create(const char* name) {
    SocialSystemAnalysis* ssa = (SocialSystemAnalysis*)calloc(1, sizeof(SocialSystemAnalysis));
    if (!ssa) return NULL;
    ssa->situation_name = strdup(name);
    ssa->role_capacity = 8; ssa->roles = (SocialRole**)calloc(8, sizeof(SocialRole*));
    ssa->em_capacity = 8; ssa->emergent_properties = (char**)calloc(8, sizeof(char*));
    return ssa;
}
void ssm_social_free(SocialSystemAnalysis* ssa) {
    if (!ssa) return; free(ssa->situation_name);
    for (int i = 0; i < ssa->n_roles; i++) { SocialRole* r = ssa->roles[i]; if(r){free(r->name);free(r->description);
        for(int j=0;j<r->n_role_holders;j++)free(r->role_holders[j]);free(r->role_holders);
        for(int j=0;j<r->n_norms;j++)free(r->expected_behaviors[j]);free(r->expected_behaviors);
        for(int j=0;j<r->n_values;j++)free(r->underlying_values[j]);free(r->underlying_values);free(r);}}
    free(ssa->roles);
    for(int i=0;i<ssa->n_emergent;i++)free(ssa->emergent_properties[i]);free(ssa->emergent_properties);free(ssa);
}
int ssm_social_add_role(SocialSystemAnalysis* ssa, const char* name, const char* desc) {
    if (!ssa || !name) return -1;
    if (ssa->n_roles >= ssa->role_capacity) { ssa->role_capacity *= 2;
        SocialRole** nn = (SocialRole**)realloc(ssa->roles, (size_t)ssa->role_capacity * sizeof(SocialRole*));
        if (!nn) return -1; ssa->roles = nn; }
    SocialRole* r = (SocialRole*)calloc(1, sizeof(SocialRole)); if (!r) return -1;
    r->name = strdup(name); r->description = strdup(desc); r->role_power = 0.5; r->role_clarity = 0.5;
    r->rh_capacity = 4; r->role_holders = (char**)calloc(4, sizeof(char*));
    r->norm_capacity = 4; r->expected_behaviors = (char**)calloc(4, sizeof(char*));
    r->val_capacity = 4; r->underlying_values = (char**)calloc(4, sizeof(char*));
    r->role_power = 0.5; r->role_clarity = 0.5;
    ssa->roles[ssa->n_roles++] = r; return ssa->n_roles - 1;
}
void ssm_social_add_role_holder(SocialSystemAnalysis* ssa, int role_id, const char* holder) {
    if (!ssa || !holder || role_id < 0 || role_id >= ssa->n_roles) return;
    SocialRole* r = ssa->roles[role_id];
    if (r->n_role_holders >= r->rh_capacity) { r->rh_capacity *= 2;
        char** nn = (char**)realloc(r->role_holders, (size_t)r->rh_capacity * sizeof(char*));
        if (!nn) return; r->role_holders = nn; }
    r->role_holders[r->n_role_holders++] = strdup(holder);
}
void ssm_social_add_norm(SocialSystemAnalysis* ssa, int role_id, const char* norm) {
    if (!ssa || !norm || role_id < 0 || role_id >= ssa->n_roles) return;
    SocialRole* r = ssa->roles[role_id];
    if (r->n_norms >= r->norm_capacity) { r->norm_capacity *= 2;
        char** nn = (char**)realloc(r->expected_behaviors, (size_t)r->norm_capacity * sizeof(char*));
        if (!nn) return; r->expected_behaviors = nn; }
    r->expected_behaviors[r->n_norms++] = strdup(norm);
}
void ssm_social_add_value(SocialSystemAnalysis* ssa, int role_id, const char* value) {
    if (!ssa || !value || role_id < 0 || role_id >= ssa->n_roles) return;
    SocialRole* r = ssa->roles[role_id];
    if (r->n_values >= r->val_capacity) { r->val_capacity *= 2;
        char** nn = (char**)realloc(r->underlying_values, (size_t)r->val_capacity * sizeof(char*));
        if (!nn) return; r->underlying_values = nn; }
    r->underlying_values[r->n_values++] = strdup(value);
}
double ssm_social_compute_cohesion(SocialSystemAnalysis* ssa) {
    if (!ssa || ssa->n_roles < 1) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < ssa->n_roles; i++) sum += ssa->roles[i]->role_clarity * ssa->roles[i]->role_power;
    ssa->social_cohesion = sum / fmax((double)ssa->n_roles, 1.0);
    return ssa->social_cohesion;
}
double ssm_social_compute_normative_conflict(SocialSystemAnalysis* ssa) {
    if (!ssa || ssa->n_roles < 2) return 0.0;
    int conflicts = 0, pairs = 0;
    for (int i = 0; i < ssa->n_roles; i++) for (int j = i + 1; j < ssa->n_roles; j++) { pairs++;
        for (int ni = 0; ni < ssa->roles[i]->n_norms; ni++) for (int nj = 0; nj < ssa->roles[j]->n_norms; nj++)
            if (ssa->roles[i]->expected_behaviors[ni] && ssa->roles[j]->expected_behaviors[nj]
                && strcmp(ssa->roles[i]->expected_behaviors[ni], ssa->roles[j]->expected_behaviors[nj]) == 0) conflicts++; }
    ssa->normative_conflict = (pairs > 0) ? (double)conflicts / (double)(pairs * 2) : 0.0;
    return ssa->normative_conflict;
}
void ssm_social_print(const SocialSystemAnalysis* ssa) {
    if (!ssa) { printf("(null SSA)\n"); return; }
    printf("Social System: %s\n  Roles: %d  Cohesion: %.2f  Conflict: %.2f\n",
        ssa->situation_name, ssa->n_roles, ssa->social_cohesion, ssa->normative_conflict);
    for (int i = 0; i < ssa->n_roles; i++) { SocialRole* r = ssa->roles[i];
        printf("  Role[%d]: %s (power=%.2f clarity=%.2f)\n", i, r->name, r->role_power, r->role_clarity);
        printf("    Holders:"); for (int j = 0; j < r->n_role_holders; j++) printf(" [%s]", r->role_holders[j]);
        printf("\n    Norms:"); for (int j = 0; j < r->n_norms; j++) printf(" [%s]", r->expected_behaviors[j]);
        printf("\n    Values:"); for (int j = 0; j < r->n_values; j++) printf(" [%s]", r->underlying_values[j]);
        printf("\n");
    }
}

PoliticalSystemAnalysis* ssm_political_create(const char* name) {
    PoliticalSystemAnalysis* psa = (PoliticalSystemAnalysis*)calloc(1, sizeof(PoliticalSystemAnalysis));
    if (!psa) return NULL; psa->situation_name = strdup(name);
    psa->com_capacity = 8; psa->commodities = (PoliticalCommodity**)calloc(8, sizeof(PoliticalCommodity*));
    psa->st_capacity = 8; psa->power_struggles = (char**)calloc(8, sizeof(char*));
    psa->str_capacity = 8; psa->accommodation_strategies = (char**)calloc(8, sizeof(char*));
    return psa;
}
void ssm_political_free(PoliticalSystemAnalysis* psa) {
    if (!psa) return; free(psa->situation_name);
    for (int i = 0; i < psa->n_commodities; i++) { PoliticalCommodity* c = psa->commodities[i];
        if (c) { free(c->description); free(c->primary_holder);
            for (int j = 0; j < c->n_secondary; j++) free(c->secondary_holders[j]); free(c->secondary_holders); free(c); } }
    free(psa->commodities);
    for (int i = 0; i < psa->n_struggles; i++) free(psa->power_struggles[i]); free(psa->power_struggles);
    for (int i = 0; i < psa->n_strategies; i++) free(psa->accommodation_strategies[i]); free(psa->accommodation_strategies);
    free(psa);
}
int ssm_political_add_commodity(PoliticalSystemAnalysis* psa, CommodityType type, const char* desc, const char* holder) {
    if (!psa || !desc || !holder) return -1;
    if (psa->n_commodities >= psa->com_capacity) { psa->com_capacity *= 2;
        PoliticalCommodity** nn = (PoliticalCommodity**)realloc(psa->commodities, (size_t)psa->com_capacity * sizeof(PoliticalCommodity*));
        if (!nn) return -1; psa->commodities = nn; }
    PoliticalCommodity* c = (PoliticalCommodity*)calloc(1, sizeof(PoliticalCommodity)); if (!c) return -1;
    c->type = type; c->description = strdup(desc); c->primary_holder = strdup(holder);
    c->sec_capacity = 4; c->secondary_holders = (char**)calloc(4, sizeof(char*));
    c->concentration = 0.5; c->contestation = 0.3;
    psa->commodities[psa->n_commodities++] = c; return psa->n_commodities - 1;
}
void ssm_political_add_secondary_holder(PoliticalSystemAnalysis* psa, int com_id, const char* holder) {
    if (!psa || !holder || com_id < 0 || com_id >= psa->n_commodities) return;
    PoliticalCommodity* c = psa->commodities[com_id];
    if (c->n_secondary >= c->sec_capacity) { c->sec_capacity *= 2;
        char** nn = (char**)realloc(c->secondary_holders, (size_t)c->sec_capacity * sizeof(char*));
        if (!nn) return; c->secondary_holders = nn; }
    c->secondary_holders[c->n_secondary++] = strdup(holder);
}
void ssm_political_add_power_struggle(PoliticalSystemAnalysis* psa, const char* struggle) {
    if (!psa || !struggle) return;
    if (psa->n_struggles >= psa->st_capacity) { psa->st_capacity *= 2;
        char** nn = (char**)realloc(psa->power_struggles, (size_t)psa->st_capacity * sizeof(char*));
        if (!nn) return; psa->power_struggles = nn; }
    psa->power_struggles[psa->n_struggles++] = strdup(struggle);
}
void ssm_political_add_strategy(PoliticalSystemAnalysis* psa, const char* strategy) {
    if (!psa || !strategy) return;
    if (psa->n_strategies >= psa->str_capacity) { psa->str_capacity *= 2;
        char** nn = (char**)realloc(psa->accommodation_strategies, (size_t)psa->str_capacity * sizeof(char*));
        if (!nn) return; psa->accommodation_strategies = nn; }
    psa->accommodation_strategies[psa->n_strategies++] = strdup(strategy);
}
double ssm_political_compute_concentration(PoliticalSystemAnalysis* psa) {
    if (!psa || psa->n_commodities < 1) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < psa->n_commodities; i++) sum += psa->commodities[i]->concentration;
    psa->power_concentration = sum / (double)psa->n_commodities; return psa->power_concentration;
}
double ssm_political_compute_feasibility(PoliticalSystemAnalysis* psa) {
    if (!psa) return 0.0;
    double concentration = ssm_political_compute_concentration(psa);
    double struggle_penalty = (double)psa->n_struggles / fmax((double)(psa->n_struggles + 3), 1.0);
    double strategy_bonus = (double)psa->n_strategies / fmax((double)(psa->n_strategies + 5), 1.0);
    psa->political_feasibility = 1.0 - 0.5 * concentration - 0.3 * struggle_penalty + 0.2 * strategy_bonus;
    if (psa->political_feasibility > 1.0) psa->political_feasibility = 1.0;
    if (psa->political_feasibility < 0.0) psa->political_feasibility = 0.0;
    return psa->political_feasibility;
}
void ssm_political_print(const PoliticalSystemAnalysis* psa) {
    if (!psa) { printf("(null PSA)\n"); return; }
    printf("Political System: %s\n  Commodities: %d  PowerConc: %.2f  Feasibility: %.2f\n",
        psa->situation_name, psa->n_commodities, psa->power_concentration, psa->political_feasibility);
    for (int i = 0; i < psa->n_commodities; i++) { PoliticalCommodity* c = psa->commodities[i];
        printf("  [%d] type=%d holder=%s conc=%.2f contest=%.2f asym=%s\n",
            i, c->type, c->primary_holder, c->concentration, c->contestation, c->is_asymmetric ? "YES" : "NO"); }
    printf("  Struggles(%d):", psa->n_struggles);
    for (int i = 0; i < psa->n_struggles; i++) printf(" [%s]", psa->power_struggles[i]);
    printf("\n  Strategies(%d):", psa->n_strategies);
    for (int i = 0; i < psa->n_strategies; i++) printf(" [%s]", psa->accommodation_strategies[i]);
    printf("\n");
}

/* Rich Picture layout optimization */
void ssm_rp_auto_layout(RichPicture* rp) {
    if (!rp || rp->n_nodes < 1) return;
    double angle_step = 2.0 * 3.14159 / (double)rp->n_nodes;
    for (int i = 0; i < rp->n_nodes; i++) {
        rp->nodes[i]->x_position = 0.5 + 0.4 * cos(angle_step * (double)i);
        rp->nodes[i]->y_position = 0.5 + 0.4 * sin(angle_step * (double)i);
    }
}
double ssm_rp_node_centrality(const RichPicture* rp, int node_id) {
    if (!rp || node_id < 0 || node_id >= rp->n_nodes) return 0.0;
    return (double)rp->nodes[node_id]->n_connections / fmax((double)(rp->n_nodes - 1), 1.0);
}
int ssm_rp_find_most_connected(const RichPicture* rp) {
    if (!rp || rp->n_nodes < 1) return -1;
    int best = 0;
    for (int i = 1; i < rp->n_nodes; i++)
        if (rp->nodes[i]->n_connections > rp->nodes[best]->n_connections) best = i;
    return best;
}
void ssm_rp_get_elements_by_type(const RichPicture* rp, RichPictureElement type, int* ids, int* n_found) {
    if (!rp || !ids || !n_found) return;
    *n_found = 0;
    for (int i = 0; i < rp->n_nodes; i++)
        if (rp->nodes[i]->type == type) ids[(*n_found)++] = i;
}

/* Social system: detect emergent roles */
int ssm_social_detect_emergent_roles(SocialSystemAnalysis* ssa) {
    if (!ssa) return 0;
    int count = 0;
    for (int i = 0; i < ssa->n_roles; i++)
        if (ssa->roles[i]->role_clarity < 0.3) count++;
    return count;
}
double ssm_social_power_distribution(const SocialSystemAnalysis* ssa) {
    if (!ssa || ssa->n_roles < 1) return 0.0;
    double sum = 0.0, sum_sq = 0.0;
    for (int i = 0; i < ssa->n_roles; i++) { sum += ssa->roles[i]->role_power; sum_sq += ssa->roles[i]->role_power * ssa->roles[i]->role_power; }
    double n = (double)ssa->n_roles;
    return (sum_sq / n) / ((sum / n) * (sum / n));
}
void ssm_social_identify_value_conflicts(const SocialSystemAnalysis* ssa, int* conflict_pairs, int* n_conflicts) {
    if (!ssa || !conflict_pairs || !n_conflicts) return;
    *n_conflicts = 0;
    for (int i = 0; i < ssa->n_roles; i++)
        for (int j = i + 1; j < ssa->n_roles; j++) {
            bool conflict = false;
            for (int vi = 0; !conflict && vi < ssa->roles[i]->n_values; vi++)
                for (int vj = 0; !conflict && vj < ssa->roles[j]->n_values; vj++)
                    if (ssa->roles[i]->underlying_values[vi] && ssa->roles[j]->underlying_values[vj]
                        && strcmp(ssa->roles[i]->underlying_values[vi], ssa->roles[j]->underlying_values[vj]) != 0)
                        conflict = true;
            if (conflict) { conflict_pairs[(*n_conflicts) * 2] = i; conflict_pairs[(*n_conflicts) * 2 + 1] = j; (*n_conflicts)++; }
        }
}

/* Political system: find power imbalances */
int ssm_political_count_asymmetries(const PoliticalSystemAnalysis* psa) {
    if (!psa) return 0; int count = 0;
    for (int i = 0; i < psa->n_commodities; i++) if (psa->commodities[i]->is_asymmetric) count++;
    return count;
}
double ssm_political_stability_index(const PoliticalSystemAnalysis* psa) {
    if (!psa) return 0.0;
    double conc = ssm_political_compute_concentration((PoliticalSystemAnalysis*)psa);
    int asym = ssm_political_count_asymmetries(psa);
    double struggle_factor = (double)psa->n_struggles / fmax((double)(psa->n_struggles + 5), 1.0);
    return 1.0 - 0.4 * conc - 0.3 * ((double)asym / fmax((double)psa->n_commodities, 1.0)) - 0.3 * struggle_factor;
}
bool ssm_political_is_stable(const PoliticalSystemAnalysis* psa) {
    return ssm_political_stability_index(psa) > 0.5;
}
void ssm_political_rank_commodities_by_contestation(const PoliticalSystemAnalysis* psa, int* ranking) {
    if (!psa || !ranking) return;
    for (int i = 0; i < psa->n_commodities; i++) ranking[i] = i;
    for (int i = 0; i < psa->n_commodities - 1; i++)
        for (int j = i + 1; j < psa->n_commodities; j++)
            if (psa->commodities[ranking[j]]->contestation > psa->commodities[ranking[i]]->contestation)
                { int t = ranking[i]; ranking[i] = ranking[j]; ranking[j] = t; }
}

/* Rich Picture narrative generation */
void ssm_rp_generate_narrative(const RichPicture* rp) {
    if (!rp) return;
    printf("=== Rich Picture Narrative: %s ===\n", rp->situation_name);
    printf("The situation involves %d key elements:\n", rp->n_nodes);
    for (int i = 0; i < rp->n_nodes; i++) {
        RichPictureNode* n = rp->nodes[i];
        const char* type_names[] = {"Structure","Process","Relationship","Concern","Boundary","Worldview","Conflict","Uncertainty"};
        printf("  - %s (%s) at position (%.2f, %.2f) with salience %.2f\n",
            n->label, type_names[n->type], n->x_position, n->y_position, n->salience);
        if (n->n_connections > 0) {
            printf("    Connected to:");
            for (int j = 0; j < n->n_connections; j++) printf(" [%s]", rp->nodes[n->connected_to[j]]->label);
            printf("\n");
        }
    }
    printf("Overall complexity: %.2f, Conflict density: %.2f\n", rp->complexity_index, rp->conflict_density);
}

int ssm_rp_count_by_type(const RichPicture* rp, RichPictureElement type) {
    if (!rp) return 0; int count = 0;
    for (int i = 0; i < rp->n_nodes; i++) if (rp->nodes[i]->type == type) count++;
    return count;
}

/* Social system intervention design */
void ssm_social_design_intervention(const SocialSystemAnalysis* ssa, char** recommendations, int* n_recs) {
    if (!ssa || !recommendations || !n_recs) return;
    *n_recs = 0;
    if (ssa->normative_conflict > 0.5) recommendations[(*n_recs)++] = strdup("Facilitate norm negotiation workshop");
    if (ssa->social_cohesion < 0.4) recommendations[(*n_recs)++] = strdup("Strengthen shared values through team building");
    if (ssa->value_alignment < 0.5) recommendations[(*n_recs)++] = strdup("Conduct value clarification exercise");
}
