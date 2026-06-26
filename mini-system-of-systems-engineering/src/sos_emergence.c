#include "sos_emergence.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

EmergentBehavior* sos_emerge_create(const char* name, const char* desc,
                                     EmergenceType type) {
    if (!name) return NULL;
    EmergentBehavior* eb = calloc(1, sizeof(EmergentBehavior));
    if (!eb) return NULL;
    eb->name = malloc(strlen(name) + 1);
    if (eb->name) strcpy(eb->name, name);
    eb->description = desc ? malloc(strlen(desc) + 1) : NULL;
    if (desc && eb->description) strcpy(eb->description, desc);
    eb->type = type; eb->strength = 0.5; eb->predictability = 0.5;
    eb->contributing_systems = NULL; eb->n_contributing = 0;
    eb->emergence_threshold = 0.3; eb->is_active = false;
    eb->is_beneficial = (type != EMERGE_PATHOLOGICAL);
    return eb;
}

void sos_emerge_free(EmergentBehavior* eb) {
    if (!eb) return;
    free(eb->name); free(eb->description);
    free(eb->contributing_systems); free(eb);
}

void sos_emerge_add_contributor(EmergentBehavior* eb, int sys_idx) {
    if (!eb || sys_idx < 0) return;
    int nc = eb->n_contributing + 1;
    int* ns = realloc(eb->contributing_systems, (size_t)nc * sizeof(int));
    if (!ns) return;
    eb->contributing_systems = ns;
    eb->contributing_systems[eb->n_contributing] = sys_idx;
    eb->n_contributing++;
}

double sos_emerge_assess(EmergentBehavior* eb, SystemOfSystems* sos,
                          SoSArchitecture* arch) {
    if (!eb || !sos || !arch) return 0.0;
    double contrib_sum = 0;
    for (int i = 0; i < eb->n_contributing; i++) {
        int idx = eb->contributing_systems[i];
        if (idx >= 0 && idx < sos->constituents->n_systems)
            contrib_sum += 1.0;
    }
    double coverage = (eb->n_contributing > 0)
        ? contrib_sum / (double)eb->n_contributing : 0.0;
    eb->strength = coverage * sos->chars.emergence;
    eb->is_active = (eb->strength >= eb->emergence_threshold);
    return eb->strength;
}

EmergenceProfile* sos_emerge_profile_create(void) {
    EmergenceProfile* ep = calloc(1, sizeof(EmergenceProfile));
    return ep;
}

void sos_emerge_profile_free(EmergenceProfile* ep) {
    if (!ep) return;
    for (int i = 0; i < ep->n_behaviors; i++) {
        EmergentBehavior* eb = &ep->behaviors[i];
        free(eb->name); free(eb->description);
        free(eb->contributing_systems);
    }
    free(ep->behaviors); free(ep);
}

void sos_emerge_profile_add(EmergenceProfile* ep, EmergentBehavior* eb) {
    if (!ep || !eb) return;
    if (ep->n_behaviors >= ep->cap) {
        int nc = (ep->cap == 0) ? 8 : ep->cap * 2;
        EmergentBehavior* nb = realloc(ep->behaviors, (size_t)nc * sizeof(EmergentBehavior));
        if (!nb) return;
        ep->behaviors = nb; ep->cap = nc;
    }
    /* Deep copy */
    EmergentBehavior* dest = &ep->behaviors[ep->n_behaviors];
    memset(dest, 0, sizeof(EmergentBehavior));
    dest->name = eb->name ? malloc(strlen(eb->name) + 1) : NULL;
    if (dest->name) strcpy(dest->name, eb->name);
    dest->description = eb->description ? malloc(strlen(eb->description) + 1) : NULL;
    if (dest->description) strcpy(dest->description, eb->description);
    dest->type = eb->type; dest->strength = eb->strength;
    dest->predictability = eb->predictability;
    dest->emergence_threshold = eb->emergence_threshold;
    dest->is_active = eb->is_active; dest->is_beneficial = eb->is_beneficial;
    if (eb->n_contributing > 0 && eb->contributing_systems) {
        dest->contributing_systems = malloc((size_t)eb->n_contributing * sizeof(int));
        memcpy(dest->contributing_systems, eb->contributing_systems,
               (size_t)eb->n_contributing * sizeof(int));
        dest->n_contributing = eb->n_contributing;
    }
    ep->n_behaviors++;
}

void sos_emerge_profile_analyze(EmergenceProfile* ep, SystemOfSystems* sos,
                                 SoSArchitecture* arch) {
    if (!ep || !sos || !arch) return;
    int total = 0, nominal = 0;
    ep->emergence_index = 0;
    for (int i = 0; i < ep->n_behaviors; i++) {
        double s = sos_emerge_assess(&ep->behaviors[i], sos, arch);
        ep->emergence_index += s;
        total++;
        if (ep->behaviors[i].is_beneficial) nominal++;
    }
    ep->emergence_index = (total > 0) ? ep->emergence_index / (double)total : 0.0;
    ep->nominal_ratio = (total > 0) ? (double)nominal / (double)total : 1.0;
    ep->surprise_index = 1.0 - ep->nominal_ratio;
}

EmergentBehavior* sos_emerge_profile_dominant(EmergenceProfile* ep) {
    if (!ep || ep->n_behaviors == 0) return NULL;
    int best = 0;
    for (int i = 1; i < ep->n_behaviors; i++)
        if (ep->behaviors[i].strength > ep->behaviors[best].strength) best = i;
    return &ep->behaviors[best];
}

int sos_emerge_count_by_type(EmergenceProfile* ep, EmergenceType type) {
    if (!ep) return 0;
    int n = 0;
    for (int i = 0; i < ep->n_behaviors; i++)
        if (ep->behaviors[i].type == type) n++;
    return n;
}

double sos_emerge_pathological_risk(EmergenceProfile* ep) {
    if (!ep || ep->n_behaviors == 0) return 0.0;
    int n_path = sos_emerge_count_by_type(ep, EMERGE_PATHOLOGICAL);
    double risk = (double)n_path / (double)ep->n_behaviors;
    for (int i = 0; i < ep->n_behaviors; i++)
        if (ep->behaviors[i].type == EMERGE_PATHOLOGICAL && ep->behaviors[i].is_active)
            risk += ep->behaviors[i].strength * 0.5;
    return fmin(risk, 1.0);
}

void sos_emerge_profile_print(EmergenceProfile* ep) {
    if (!ep) { printf("EmergenceProfile: NULL\n"); return; }
    printf("=== Emergence Profile ===\n");
    printf("Behaviors: %d  Index: %.2f  Nominal: %.0f%%  Risk: %.2f\n",
           ep->n_behaviors, ep->emergence_index,
           ep->nominal_ratio * 100.0, sos_emerge_pathological_risk(ep));
    for (int i = 0; i < ep->n_behaviors; i++) {
        EmergentBehavior* eb = &ep->behaviors[i];
        printf("  [%d] %s: %.2f %s %s\n", i, eb->name, eb->strength,
               eb->is_active ? "[ACTIVE]" : "[dormant]",
               eb->is_beneficial ? "[BENEFICIAL]" : "[HARMFUL]");
    }
}

double sos_emergence_formula(int n_systems, double connectivity,
                              double diversity, double integration) {
    if (n_systems < 2) return 0.0;
    double scale = 1.0 - 1.0 / sqrt((double)n_systems);
    return scale * connectivity * diversity * (0.5 + 0.5 * integration);
}

bool sos_detect_novel_emergence(SystemOfSystems* sos, SoSArchitecture* arch,
                                 double threshold) {
    if (!sos || !arch) return false;
    sos_compute_characteristics(sos);
    double e = sos_emergence_formula(sos_n_constituents(sos),
                                     sos->chars.connectivity,
                                     sos->chars.diversity,
                                     sos->integration_level);
    return e > threshold;
}

/* Extended emergence analysis */
double sos_emergence_threshold_crossed(const EmergenceProfile* ep) {
    if (!ep || ep->n_behaviors == 0) return 0.0;
    double max_s = 0;
    for (int i = 0; i < ep->n_behaviors; i++)
        if (ep->behaviors[i].strength > max_s) max_s = ep->behaviors[i].strength;
    return max_s;
}

int sos_emergence_predict_new_behaviors(const EmergenceProfile* ep,
                                         double connectivity_increase) {
    if (!ep) return 0;
    int n_latent = 0;
    for (int i = 0; i < ep->n_behaviors; i++)
        if (!ep->behaviors[i].is_active
            && ep->behaviors[i].strength + connectivity_increase * 0.3
               >= ep->behaviors[i].emergence_threshold)
            n_latent++;
    return n_latent;
}

double sos_emergence_benefit_cost_ratio(const EmergenceProfile* ep) {
    if (!ep || ep->n_behaviors == 0) return 1.0;
    double benefit = 0, cost = 0;
    for (int i = 0; i < ep->n_behaviors; i++) {
        if (ep->behaviors[i].is_beneficial) benefit += ep->behaviors[i].strength;
        else cost += ep->behaviors[i].strength;
    }
    double total = benefit + cost;
    return (total > 1e-12) ? benefit / total : 1.0;
}

void sos_emergence_set_threshold(EmergentBehavior* eb, double threshold) {
    if (!eb || threshold < 0 || threshold > 1.0) return;
    eb->emergence_threshold = threshold;
    eb->is_active = (eb->strength >= threshold);
}

double sos_emergence_cascade_risk(const EmergenceProfile* ep, SoSArchitecture* arch) {
    if (!ep || !arch || ep->n_behaviors < 2) return 0.0;
    int n_pathological = sos_emerge_count_by_type((EmergenceProfile*)ep, EMERGE_PATHOLOGICAL);
    double connectivity = arch->connectivity_density;
    return (double)n_pathological * connectivity / (double)(ep->n_behaviors + 1);
}