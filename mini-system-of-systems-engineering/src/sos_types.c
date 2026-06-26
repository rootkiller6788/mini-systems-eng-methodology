#include "sos_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

SystemOfSystems* sos_create(const char* name, SoSType type, const char* mission) {
    if (!name) return NULL;
    SystemOfSystems* sos = calloc(1, sizeof(SystemOfSystems));
    if (!sos) return NULL;
    sos->name = malloc(strlen(name) + 1);
    if (!sos->name) { free(sos); return NULL; }
    strcpy(sos->name, name);
    sos->type = type;
    sos->phase = SOS_PHASE_FORMING;
    sos->mission = mission ? malloc(strlen(mission) + 1) : NULL;
    if (mission && sos->mission) strcpy(sos->mission, mission);
    sos->constituents = gst_set_create();
    sos->operational_readiness = 0.0;
    sos->integration_level = 0.0;
    sos->emergent_behavior_names = NULL;
    sos->n_emergent_behaviors = 0;
    sos_compute_characteristics(sos);
    return sos;
}

void sos_free(SystemOfSystems* sos) {
    if (!sos) return;
    free(sos->name); free(sos->mission);
    gst_set_free(sos->constituents);
    for (int i = 0; i < sos->n_emergent_behaviors; i++)
        free(sos->emergent_behavior_names[i]);
    free(sos->emergent_behavior_names);
    free(sos);
}

void sos_add_constituent(SystemOfSystems* sos, ConstituentSystem* cs) {
    if (!sos || !cs) return;
    gst_set_add(sos->constituents, cs);
    sos_compute_characteristics(sos);
}

int sos_remove_constituent(SystemOfSystems* sos, int idx) {
    if (!sos || idx < 0 || idx >= sos->constituents->n_systems) return -1;
    gst_system_free_contents(&sos->constituents->systems[idx]);
    for (int i = idx; i < sos->constituents->n_systems - 1; i++)
        sos->constituents->systems[i] = sos->constituents->systems[i + 1];
    sos->constituents->n_systems--;
    sos_compute_characteristics(sos);
    return 0;
}

int sos_n_constituents(SystemOfSystems* sos) {
    if (!sos || !sos->constituents) return 0;
    return sos->constituents->n_systems;
}

void sos_compute_characteristics(SystemOfSystems* sos) {
    if (!sos || !sos->constituents || sos->constituents->n_systems == 0) {
        if (sos) memset(&sos->chars, 0, sizeof(SoSCharacteristics));
        return;
    }
    int n = sos->constituents->n_systems;
    SoSCharacteristics* c = &sos->chars;
    c->autonomy = gst_set_average_autonomy(sos->constituents);
    c->diversity = (n > 1) ? gst_entropy(NULL, 0) * 0.3 + 0.5 : 0.0;
    /* Connectivity = average interfaces per system normalized */
    double total_if = 0;
    for (int i = 0; i < n; i++) total_if += sos->constituents->systems[i].n_interfaces;
    c->connectivity = (n > 1) ? fmin(total_if / (double)(n * (n - 1)), 1.0) : 0.0;
    /* Belonging = inverse of autonomy variance */
    double mean_aut = c->autonomy, var = 0;
    for (int i = 0; i < n; i++) {
        double d = sos->constituents->systems[i].autonomy_level - mean_aut;
        var += d * d;
    }
    var /= (double)n;
    c->belonging = 1.0 / (1.0 + sqrt(var));
    /* Emergence = (1 - 1/sqrt(n)) * connectivity * diversity * (0.5 + 0.5*integration) */
    double scale = (n >= 2) ? 1.0 - 1.0 / sqrt((double)n) : 0.0;
    c->emergence = scale * c->connectivity * c->diversity
                   * (0.5 + 0.5 * sos->integration_level);
}

double sos_readiness(SystemOfSystems* sos) {
    if (!sos || !sos->constituents || sos->constituents->n_systems == 0) return 0.0;
    double r = 0;
    int n = sos->constituents->n_systems;
    for (int i = 0; i < n; i++)
        r += gst_system_operational_readiness(&sos->constituents->systems[i]);
    return r / (double)n;
}

double sos_integration_index(SystemOfSystems* sos) {
    if (!sos) return 0.0;
    return sos->integration_level;
}

double sos_complexity(SystemOfSystems* sos) {
    if (!sos || !sos->constituents || sos->constituents->n_systems == 0) return 0.0;
    double c = gst_set_average_complexity(sos->constituents);
    int n = sos->constituents->n_systems;
    c *= (1.0 + 0.1 * (double)(n - 1));
    c *= (1.0 + sos->chars.emergence);
    return c;
}

bool sos_is_viable(SystemOfSystems* sos) {
    if (!sos) return false;
    double r = sos_readiness(sos);
    return r > 0.5 && sos->chars.connectivity > 0.2;
}

void sos_print(SystemOfSystems* sos) {
    if (!sos) { printf("SystemOfSystems: NULL\n"); return; }
    printf("=== SoS: %s ===\n", sos->name);
    printf("Type: %s  Phase: %s\n", sos_type_name(sos->type),
           sos_phase_name(sos->phase));
    if (sos->mission) printf("Mission: %s\n", sos->mission);
    printf("Constituents: %d  Readiness: %.2f  Integration: %.2f\n",
           sos_n_constituents(sos), sos_readiness(sos), sos->integration_level);
    printf("Characteristics: A=%.2f B=%.2f C=%.2f D=%.2f E=%.2f\n",
           sos->chars.autonomy, sos->chars.belonging, sos->chars.connectivity,
           sos->chars.diversity, sos->chars.emergence);
}

const char* sos_type_name(SoSType type) {
    switch (type) {
        case SOS_DIRECTED: return "Directed";
        case SOS_ACKNOWLEDGED: return "Acknowledged";
        case SOS_COLLABORATIVE: return "Collaborative";
        case SOS_VIRTUAL: return "Virtual";
        default: return "Unknown";
    }
}

const char* sos_phase_name(SoSLifecyclePhase phase) {
    switch (phase) {
        case SOS_PHASE_FORMING: return "Forming";
        case SOS_PHASE_DEVELOPING: return "Developing";
        case SOS_PHASE_OPERATING: return "Operating";
        case SOS_PHASE_EVOLVING: return "Evolving";
        case SOS_PHASE_DISSOLVING: return "Dissolving";
        default: return "Unknown";
    }
}

/* Extended SoS type operations */
void sos_set_mission(SystemOfSystems* sos, const char* mission) {
    if (!sos || !mission) return;
    free(sos->mission);
    sos->mission = malloc(strlen(mission) + 1);
    if (sos->mission) strcpy(sos->mission, mission);
}

void sos_advance_phase(SystemOfSystems* sos) {
    if (!sos || sos->phase >= SOS_PHASE_DISSOLVING) return;
    sos->phase++;
    sos_compute_characteristics(sos);
}

double sos_interoperability_potential(SystemOfSystems* sos) {
    if (!sos) return 0.0;
    return sos->chars.connectivity * sos->chars.diversity;
}

double sos_scalability_index(SystemOfSystems* sos) {
    if (!sos || sos_n_constituents(sos) == 0) return 0.0;
    return 1.0 - 1.0 / sqrt((double)sos_n_constituents(sos) + 1.0);
}

double sos_autonomy_tension(SystemOfSystems* sos) {
    if (!sos) return 0.0;
    return fabs(sos->chars.autonomy - sos->chars.belonging);
}

bool sos_needs_governance_change(SystemOfSystems* sos) {
    if (!sos) return false;
    double tension = sos_autonomy_tension(sos);
    return tension > 0.6;
}

void sos_add_emergent_behavior_name(SystemOfSystems* sos, const char* name) {
    if (!sos || !name) return;
    int nc = sos->n_emergent_behaviors + 1;
    char** nn = realloc(sos->emergent_behavior_names, (size_t)nc * sizeof(char*));
    if (!nn) return;
    sos->emergent_behavior_names = nn;
    sos->emergent_behavior_names[sos->n_emergent_behaviors] = malloc(strlen(name) + 1);
    if (sos->emergent_behavior_names[sos->n_emergent_behaviors])
        strcpy(sos->emergent_behavior_names[sos->n_emergent_behaviors], name);
    sos->n_emergent_behaviors++;
}

const char* sos_constituent_name(const SystemOfSystems* sos, int idx) {
    if (!sos || idx < 0 || idx >= sos_n_constituents(sos)) return NULL;
    return sos->constituents->systems[idx].name;
}