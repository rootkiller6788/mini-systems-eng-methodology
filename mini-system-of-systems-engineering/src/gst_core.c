#include "gst_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

ConstituentSystem* gst_system_create(const char* name, BouldingLevel level) {
    if (!name) return NULL;
    ConstituentSystem* cs = calloc(1, sizeof(ConstituentSystem));
    if (!cs) return NULL;
    cs->name = malloc(strlen(name) + 1);
    if (!cs->name) { free(cs); return NULL; }
    strcpy(cs->name, name);
    cs->level = level;
    cs->openness = GST_OPEN;
    cs->complexity = 1.0;
    cs->reliability = 1.0;
    cs->autonomy_level = 0.5;
    cs->interface_names = NULL; cs->interface_types = NULL;
    cs->n_interfaces = 0;
    cs->state = NULL; cs->n_states = 0;
    cs->capabilities = NULL; cs->n_capabilities = 0;
    cs->is_operational = true;
    return cs;
}

void gst_system_free_contents(ConstituentSystem* cs) {
    if (!cs) return;
    free(cs->name); cs->name = NULL;
    for (int i = 0; i < cs->n_interfaces; i++) free(cs->interface_names[i]);
    free(cs->interface_names); cs->interface_names = NULL;
    free(cs->interface_types); cs->interface_types = NULL;
    free(cs->state); cs->state = NULL;
    free(cs->capabilities); cs->capabilities = NULL;
    cs->n_interfaces = 0; cs->n_states = 0; cs->n_capabilities = 0;
}

void gst_system_free(ConstituentSystem* cs) {
    if (!cs) return;
    gst_system_free_contents(cs);
    free(cs);
}

void gst_system_set_state(ConstituentSystem* cs, int idx, double val) {
    if (!cs || idx < 0 || idx >= cs->n_states) return;
    cs->state[idx] = val;
}

double gst_system_get_state(ConstituentSystem* cs, int idx) {
    if (!cs || idx < 0 || idx >= cs->n_states) return 0.0;
    return cs->state[idx];
}

int gst_system_add_capability(ConstituentSystem* cs, double cap) {
    if (!cs) return -1;
    int nc = cs->n_capabilities + 1;
    double* ncaps = realloc(cs->capabilities, (size_t)nc * sizeof(double));
    if (!ncaps) return -1;
    cs->capabilities = ncaps;
    cs->capabilities[cs->n_capabilities] = cap;
    return cs->n_capabilities++;
}

int gst_system_add_interface(ConstituentSystem* cs, const char* name, int type) {
    if (!cs || !name) return -1;
    int nc = cs->n_interfaces + 1;
    char** nn = realloc(cs->interface_names, (size_t)nc * sizeof(char*));
    int* nt = realloc(cs->interface_types, (size_t)nc * sizeof(int));
    if (!nn || !nt) { free(nn); free(nt); return -1; }
    cs->interface_names = nn; cs->interface_types = nt;
    cs->interface_names[cs->n_interfaces] = malloc(strlen(name) + 1);
    if (cs->interface_names[cs->n_interfaces])
        strcpy(cs->interface_names[cs->n_interfaces], name);
    cs->interface_types[cs->n_interfaces] = type;
    return cs->n_interfaces++;
}

double gst_system_operational_readiness(ConstituentSystem* cs) {
    if (!cs || !cs->is_operational) return 0.0;
    double r = cs->reliability;
    for (int i = 0; i < cs->n_states; i++) r *= gst_sigmoid(cs->state[i], 1.0, 0.5);
    return r;
}

void gst_system_print(ConstituentSystem* cs) {
    if (!cs) { printf("ConstituentSystem: NULL\n"); return; }
    printf("%s: level=%d autonomy=%.2f reliability=%.2f interfaces=%d op=%s\n",
           cs->name, cs->level, cs->autonomy_level, cs->reliability,
           cs->n_interfaces, cs->is_operational ? "YES" : "NO");
}

SystemSet* gst_set_create(void) {
    SystemSet* set = calloc(1, sizeof(SystemSet));
    return set;
}

void gst_set_free(SystemSet* set) {
    if (!set) return;
    for (int i = 0; i < set->n_systems; i++) gst_system_free_contents(&set->systems[i]);
    free(set->systems); free(set);
}

int gst_set_add(SystemSet* set, ConstituentSystem* cs) {
    if (!set || !cs) return -1;
    if (set->n_systems >= set->sys_cap) {
        int nc = (set->sys_cap == 0) ? 8 : set->sys_cap * 2;
        ConstituentSystem* ns = realloc(set->systems, (size_t)nc * sizeof(ConstituentSystem));
        if (!ns) return -1;
        set->systems = ns; set->sys_cap = nc;
    }
    /* Deep copy: allocate independent copies of all heap fields */
    ConstituentSystem* dest = &set->systems[set->n_systems];
    memset(dest, 0, sizeof(ConstituentSystem));
    dest->name = cs->name ? malloc(strlen(cs->name) + 1) : NULL;
    if (dest->name) strcpy(dest->name, cs->name);
    dest->level = cs->level; dest->openness = cs->openness;
    dest->complexity = cs->complexity;
    dest->reliability = cs->reliability;
    dest->autonomy_level = cs->autonomy_level;
    dest->is_operational = cs->is_operational;
    if (cs->n_interfaces > 0) {
        dest->interface_names = calloc((size_t)cs->n_interfaces, sizeof(char*));
        dest->interface_types = calloc((size_t)cs->n_interfaces, sizeof(int));
        for (int i = 0; i < cs->n_interfaces; i++) {
            if (cs->interface_names[i]) {
                dest->interface_names[i] = malloc(strlen(cs->interface_names[i]) + 1);
                if (dest->interface_names[i]) strcpy(dest->interface_names[i], cs->interface_names[i]);
            }
            dest->interface_types[i] = cs->interface_types[i];
        }
        dest->n_interfaces = cs->n_interfaces;
    }
    if (cs->n_states > 0) {
        dest->state = malloc((size_t)cs->n_states * sizeof(double));
        memcpy(dest->state, cs->state, (size_t)cs->n_states * sizeof(double));
        dest->n_states = cs->n_states;
    }
    if (cs->n_capabilities > 0) {
        dest->capabilities = malloc((size_t)cs->n_capabilities * sizeof(double));
        memcpy(dest->capabilities, cs->capabilities, (size_t)cs->n_capabilities * sizeof(double));
        dest->n_capabilities = cs->n_capabilities;
    }
    return set->n_systems++;
}

ConstituentSystem* gst_set_get(SystemSet* set, int idx) {
    if (!set || idx < 0 || idx >= set->n_systems) return NULL;
    return &set->systems[idx];
}

double gst_set_average_complexity(SystemSet* set) {
    if (!set || set->n_systems == 0) return 0.0;
    double sum = 0;
    for (int i = 0; i < set->n_systems; i++) sum += set->systems[i].complexity;
    return sum / (double)set->n_systems;
}

double gst_set_average_autonomy(SystemSet* set) {
    if (!set || set->n_systems == 0) return 0.0;
    double sum = 0;
    for (int i = 0; i < set->n_systems; i++) sum += set->systems[i].autonomy_level;
    return sum / (double)set->n_systems;
}

double gst_sigmoid(double x, double k, double x0) {
    return 1.0 / (1.0 + exp(-k * (x - x0)));
}

double gst_entropy(double* probs, int n) {
    if (!probs || n <= 0) return 0.0;
    double h = 0;
    for (int i = 0; i < n; i++)
        if (probs[i] > 1e-12) h -= probs[i] * log2(probs[i]);
    return h;
}

double gst_normalize(double x, double min, double max) {
    if (fabs(max - min) < 1e-12) return 0.5;
    return (x - min) / (max - min);
}

/* Extended core utilities for SoS */
double gst_system_interface_density(const ConstituentSystem* cs) {
    return cs ? (double)cs->n_interfaces / 10.0 : 0.0;
}

double gst_system_capability_score(const ConstituentSystem* cs) {
    if (!cs || cs->n_capabilities == 0) return 0.0;
    double s = 0;
    for (int i = 0; i < cs->n_capabilities; i++) s += cs->capabilities[i];
    return s / (double)cs->n_capabilities;
}

int gst_system_compare_complexity(const void* a, const void* b) {
    const ConstituentSystem* ca = (const ConstituentSystem*)a;
    const ConstituentSystem* cb = (const ConstituentSystem*)b;
    if (ca->complexity < cb->complexity) return -1;
    if (ca->complexity > cb->complexity) return 1;
    return 0;
}

void gst_system_update_reliability(ConstituentSystem* cs, double degradation) {
    if (!cs) return;
    cs->reliability *= (1.0 - degradation);
    if (cs->reliability < 0.0) cs->reliability = 0.0;
    if (cs->reliability < 0.5) cs->is_operational = false;
}

void gst_system_reset(ConstituentSystem* cs) {
    if (!cs) return;
    cs->reliability = 1.0;
    cs->is_operational = true;
    for (int i = 0; i < cs->n_states; i++) cs->state[i] = 0.5;
}

void gst_set_sort_by_complexity(SystemSet* set) {
    if (!set || set->n_systems <= 1) return;
    qsort(set->systems, (size_t)set->n_systems, sizeof(ConstituentSystem), gst_system_compare_complexity);
}

double gst_set_entropy(SystemSet* set) {
    if (!set || set->n_systems <= 1) return 0.0;
    double total_c = 0;
    for (int i = 0; i < set->n_systems; i++) total_c += set->systems[i].complexity;
    if (total_c < 1e-12) return 0.0;
    double* probs = malloc((size_t)set->n_systems * sizeof(double));
    if (!probs) return 0.0;
    for (int i = 0; i < set->n_systems; i++) probs[i] = set->systems[i].complexity / total_c;
    double h = gst_entropy(probs, set->n_systems);
    free(probs);
    return h;
}

double gst_set_diversity_index(SystemSet* set) {
    if (!set || set->n_systems < 2) return 0.0;
    int n_levels[10] = {0};
    for (int i = 0; i < set->n_systems; i++)
        if (set->systems[i].level > 0 && set->systems[i].level < 10)
            n_levels[set->systems[i].level]++;
    double* probs = malloc(10 * sizeof(double));
    if (!probs) return 0.0;
    for (int i = 0; i < 10; i++) probs[i] = (double)n_levels[i] / (double)set->n_systems;
    double h = gst_entropy(probs, 10);
    free(probs);
    return h / log2(10.0);
}

int gst_set_find_by_name(const SystemSet* set, const char* name) {
    if (!set || !name) return -1;
    for (int i = 0; i < set->n_systems; i++)
        if (set->systems[i].name && strcmp(set->systems[i].name, name) == 0) return i;
    return -1;
}

bool gst_system_is_compatible(const ConstituentSystem* a, const ConstituentSystem* b) {
    if (!a || !b) return false;
    for (int i = 0; i < a->n_interfaces; i++)
        for (int j = 0; j < b->n_interfaces; j++)
            if (a->interface_types[i] == b->interface_types[j]) return true;
    return false;
}

double gst_system_similarity(const ConstituentSystem* a, const ConstituentSystem* b) {
    if (!a || !b) return 0.0;
    double sim = 0; int matches = 0;
    if (a->level == b->level) { sim += 1.0; matches++; }
    double auto_diff = 1.0 - fabs(a->autonomy_level - b->autonomy_level);
    sim += auto_diff; matches++;
    double rel_diff = 1.0 - fabs(a->reliability - b->reliability);
    sim += rel_diff; matches++;
    return (matches > 0) ? sim / (double)matches : 0.0;
}

double gst_set_similarity_matrix(const SystemSet* set, double** matrix) {
    if (!set || !matrix) return 0.0;
    int n = set->n_systems;
    double total = 0; int count = 0;
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            double s = gst_system_similarity(&set->systems[i], &set->systems[j]);
            matrix[i][j] = matrix[j][i] = s;
            total += s; count++;
        }
    }
    return (count > 0) ? total / (double)count : 0.0;
}