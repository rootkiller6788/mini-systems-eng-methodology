#include "sos_types.h"
#include "sos_architecture.h"
#include "sos_emergence.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ==============================================================
 * SoS Integration Management
 *
 * Integration metrics:
 *   - Technical integration: interface compatibility, data formats
 *   - Semantic integration: shared meaning of data
 *   - Organizational integration: aligned processes
 *   - Temporal integration: synchronized operations
 * ============================================================== */

typedef struct {
    double technical;
    double semantic;
    double organizational;
    double temporal;
    double overall;
} IntegrationVector;

static IntegrationVector sos_compute_integration_vector(SystemOfSystems* sos, SoSArchitecture* arch) {
    IntegrationVector iv = {0,0,0,0,0};
    if (!sos || !arch) return iv;
    int n = sos_n_constituents(sos);
    if (n < 2) return iv;
    iv.technical = sos_arch_interoperability_score(arch);
    iv.semantic = sos->chars.diversity * 0.7;
    iv.organizational = sos->chars.belonging;
    iv.temporal = 1.0 - fabs(sos->chars.autonomy - 0.5);
    iv.overall = 0.35*iv.technical + 0.25*iv.semantic + 0.25*iv.organizational + 0.15*iv.temporal;
    return iv;
}

double sos_integration_overall(SystemOfSystems* sos, SoSArchitecture* arch) {
    IntegrationVector iv = sos_compute_integration_vector(sos, arch);
    return iv.overall;
}

double sos_integration_technical(SystemOfSystems* sos, SoSArchitecture* arch) {
    return sos_compute_integration_vector(sos, arch).technical;
}

double sos_integration_semantic(SystemOfSystems* sos, SoSArchitecture* arch) {
    return sos_compute_integration_vector(sos, arch).semantic;
}

double sos_integration_gap(SystemOfSystems* sos, SoSArchitecture* arch) {
    IntegrationVector iv = sos_compute_integration_vector(sos, arch);
    double max_possible = 1.0;
    return max_possible - iv.overall;
}

int sos_integration_bottleneck_dimension(SystemOfSystems* sos, SoSArchitecture* arch) {
    IntegrationVector iv = sos_compute_integration_vector(sos, arch);
    double min_val = iv.technical;
    int min_idx = 0;
    double vals[] = {iv.technical, iv.semantic, iv.organizational, iv.temporal};
    for (int i = 1; i < 4; i++) if (vals[i] < min_val) { min_val = vals[i]; min_idx = i; }
    return min_idx;
}

const char* sos_integration_dimension_name(int dim) {
    switch (dim) {
        case 0: return "Technical"; case 1: return "Semantic";
        case 2: return "Organizational"; case 3: return "Temporal";
        default: return "Unknown";
    }
}

void sos_integration_print(SystemOfSystems* sos, SoSArchitecture* arch) {
    if (!sos || !arch) return;
    IntegrationVector iv = sos_compute_integration_vector(sos, arch);
    printf("=== SoS Integration ===\n");
    printf("Technical: %.2f  Semantic: %.2f  Organizational: %.2f  Temporal: %.2f\n",
           iv.technical, iv.semantic, iv.organizational, iv.temporal);
    printf("Overall: %.2f  Gap: %.2f\n", iv.overall, sos_integration_gap(sos, arch));
}

/* SoS Configuration Management */
typedef struct {
    char* config_name;
    int* system_ids;
    int n_systems;
    SoSArchitectureType arch_type;
    double timestamp;
    bool is_active;
} SoSConfiguration;

typedef struct {
    SoSConfiguration* configs;
    int n_configs;
    int cap;
    int current_idx;
} ConfigurationHistory;

SoSConfiguration* sos_config_create(const char* name, int n_systems) {
    SoSConfiguration* cfg = calloc(1, sizeof(SoSConfiguration));
    if (!cfg) return NULL;
    cfg->config_name = name ? malloc(strlen(name) + 1) : NULL;
    if (name && cfg->config_name) strcpy(cfg->config_name, name);
    cfg->system_ids = malloc((size_t)n_systems * sizeof(int));
    cfg->n_systems = n_systems;
    cfg->is_active = true;
    return cfg;
}

void sos_config_free(SoSConfiguration* cfg) {
    if (!cfg) return;
    free(cfg->config_name); free(cfg->system_ids); free(cfg);
}

ConfigurationHistory* sos_config_history_create(void) {
    return calloc(1, sizeof(ConfigurationHistory));
}

void sos_config_history_free(ConfigurationHistory* ch) {
    if (!ch) return;
    for (int i = 0; i < ch->n_configs; i++) sos_config_free(&ch->configs[i]);
    free(ch->configs); free(ch);
}

int sos_config_history_add(ConfigurationHistory* ch, SoSConfiguration* cfg) {
    if (!ch || !cfg) return -1;
    if (ch->n_configs >= ch->cap) {
        int nc = (ch->cap == 0) ? 8 : ch->cap * 2;
        SoSConfiguration* nc2 = realloc(ch->configs, (size_t)nc * sizeof(SoSConfiguration));
        if (!nc2) return -1;
        ch->configs = nc2; ch->cap = nc;
    }
    ch->configs[ch->n_configs] = *cfg;
    ch->current_idx = ch->n_configs;
    return ch->n_configs++;
}

void sos_config_history_rollback(ConfigurationHistory* ch, int idx) {
    if (!ch || idx < 0 || idx >= ch->n_configs) return;
    for (int i = idx + 1; i < ch->n_configs; i++) ch->configs[i].is_active = false;
    ch->current_idx = idx;
}

void sos_config_history_print(const ConfigurationHistory* ch) {
    if (!ch) return;
    printf("=== Configuration History: %d snapshots ===\n", ch->n_configs);
    for (int i = 0; i < ch->n_configs; i++)
        printf("  [%d] %s systems=%d %s\n", i, ch->configs[i].config_name,
               ch->configs[i].n_systems, ch->configs[i].is_active ? "(active)" : "(rolled back)");
}
/* Integration verification and validation */
bool sos_integration_verify_consistency(const SystemOfSystems* sos, const SoSArchitecture* arch) {
    if (!sos || !arch) return false;
    int n = sos_n_constituents(sos);
    int connections = sos_arch_n_connections((SoSArchitecture*)arch);
    double expected_min = (double)(n - 1);
    double expected_max = (double)(n * (n - 1)) / 2.0;
    if (n <= 1) return true;
    return (double)connections >= expected_min * 0.5;
}

double sos_integration_efficiency(const SystemOfSystems* sos, const SoSArchitecture* arch) {
    if (!sos || !arch) return 0.0;
    int n = sos_n_constituents(sos);
    if (n <= 1) return 1.0;
    int max_conn = n * (n - 1) / 2;
    int actual = sos_arch_n_connections((SoSArchitecture*)arch);
    return (max_conn > 0) ? (double)actual / (double)max_conn : 0.0;
}
