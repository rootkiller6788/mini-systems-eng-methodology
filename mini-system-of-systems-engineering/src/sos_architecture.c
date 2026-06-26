#include "sos_architecture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

SoSArchitecture* sos_arch_create(SoSArchitectureType type, int n_systems) {
    if (n_systems < 1) return NULL;
    SoSArchitecture* arch = calloc(1, sizeof(SoSArchitecture));
    if (!arch) return NULL;
    arch->type = type; arch->n_systems = n_systems;
    arch->connections = NULL; arch->n_connections = 0; arch->conn_cap = 0;
    arch->adjacency = malloc((size_t)n_systems * sizeof(double*));
    if (!arch->adjacency) { free(arch); return NULL; }
    for (int i = 0; i < n_systems; i++) {
        arch->adjacency[i] = calloc((size_t)n_systems, sizeof(double));
        if (!arch->adjacency[i]) { sos_arch_free(arch); return NULL; }
    }
    arch->central_system_idx = -1;
    return arch;
}

void sos_arch_free(SoSArchitecture* arch) {
    if (!arch) return;
    for (int i = 0; i < arch->n_connections; i++) free(arch->connections[i].protocol);
    free(arch->connections);
    if (arch->adjacency) {
        for (int i = 0; i < arch->n_systems; i++) free(arch->adjacency[i]);
        free(arch->adjacency);
    }
    free(arch);
}

int sos_arch_connect(SoSArchitecture* arch, int from, int to,
                      InteroperabilityLevel level, double bw, double lat,
                      const char* protocol) {
    if (!arch || from < 0 || from >= arch->n_systems
        || to < 0 || to >= arch->n_systems || from == to) return -1;
    if (arch->n_connections >= arch->conn_cap) {
        int nc = (arch->conn_cap == 0) ? 32 : arch->conn_cap * 2;
        SoSConnection* nconn = realloc(arch->connections, (size_t)nc * sizeof(SoSConnection));
        if (!nconn) return -1;
        arch->connections = nconn; arch->conn_cap = nc;
    }
    SoSConnection* c = &arch->connections[arch->n_connections];
    c->from_idx = from; c->to_idx = to;
    c->level = level; c->bandwidth = bw; c->latency = lat;
    c->reliability = 0.99;
    c->protocol = protocol ? malloc(strlen(protocol) + 1) : NULL;
    if (protocol && c->protocol) strcpy(c->protocol, protocol);
    c->is_active = true;
    arch->adjacency[from][to] = 1.0;
    arch->adjacency[to][from] = 1.0;
    return arch->n_connections++;
}

void sos_arch_disconnect(SoSArchitecture* arch, int from, int to) {
    if (!arch || from < 0 || to < 0 || from >= arch->n_systems
        || to >= arch->n_systems) return;
    arch->adjacency[from][to] = 0.0;
    arch->adjacency[to][from] = 0.0;
    for (int i = 0; i < arch->n_connections; i++) {
        SoSConnection* c = &arch->connections[i];
        if ((c->from_idx == from && c->to_idx == to)
            || (c->from_idx == to && c->to_idx == from)) c->is_active = false;
    }
}

int sos_arch_n_connections(SoSArchitecture* arch) {
    if (!arch) return 0;
    int n = 0;
    for (int i = 0; i < arch->n_connections; i++)
        if (arch->connections[i].is_active) n++;
    return n;
}

InteroperabilityLevel sos_arch_interop_level(SoSArchitecture* arch, int from, int to) {
    if (!arch || from < 0 || to < 0 || from >= arch->n_systems
        || to >= arch->n_systems) return INTEROP_ISOLATED;
    if (fabs(arch->adjacency[from][to]) < 0.5) return INTEROP_ISOLATED;
    for (int i = 0; i < arch->n_connections; i++)
        if (arch->connections[i].is_active && arch->connections[i].from_idx == from
            && arch->connections[i].to_idx == to)
            return arch->connections[i].level;
    return INTEROP_CONNECTED;
}

void sos_arch_compute_metrics(SoSArchitecture* arch) {
    if (!arch || arch->n_systems < 2) return;
    int n = arch->n_systems;
    int possible = n * (n - 1) / 2, actual = 0;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (arch->adjacency[i][j] > 0.5) actual++;
    arch->connectivity_density = (possible > 0) ? (double)actual / (double)possible : 0.0;

    double** dist = malloc((size_t)n * sizeof(double*));
    for (int i = 0; i < n; i++) {
        dist[i] = malloc((size_t)n * sizeof(double));
        for (int j = 0; j < n; j++)
            dist[i][j] = (i == j) ? 0 : (arch->adjacency[i][j] > 0.5 ? 1 : DBL_MAX/2);
    }
    for (int k = 0; k < n; k++)
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                if (dist[i][k] + dist[k][j] < dist[i][j])
                    dist[i][j] = dist[i][k] + dist[k][j];
    double total = 0; int count = 0;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (dist[i][j] < DBL_MAX/2) { total += dist[i][j]; count++; }
    arch->average_path_length = (count > 0) ? total / (double)count : 0.0;

    arch->centrality_max = 0; arch->central_system_idx = 0;
    for (int i = 0; i < n; i++) {
        double deg = 0;
        for (int j = 0; j < n; j++) deg += arch->adjacency[i][j];
        if (deg > arch->centrality_max) { arch->centrality_max = deg; arch->central_system_idx = i; }
    }
    double clust_sum = 0;
    for (int i = 0; i < n; i++) {
        int neighbors = 0, edges_among = 0;
        for (int j = 0; j < n; j++) if (arch->adjacency[i][j] > 0.5) neighbors++;
        if (neighbors < 2) continue;
        for (int j = 0; j < n; j++)
            for (int k = j + 1; k < n; k++)
                if (arch->adjacency[i][j] > 0.5 && arch->adjacency[i][k] > 0.5
                    && arch->adjacency[j][k] > 0.5) edges_among++;
        clust_sum += (double)(2 * edges_among) / (double)(neighbors * (neighbors - 1));
    }
    arch->clustering_coefficient = clust_sum / (double)n;
    arch->robustness = 1.0 - arch->centrality_max / (double)(n);
    if (arch->robustness < 0) arch->robustness = 0;
    arch->efficiency = (arch->average_path_length > 0) ? 1.0 / arch->average_path_length : 0.0;
    for (int i = 0; i < n; i++) free(dist[i]);
    free(dist);
}

double sos_arch_robustness(SoSArchitecture* arch) {
    if (!arch) return 0.0;
    sos_arch_compute_metrics(arch);
    return arch->robustness;
}

double sos_arch_efficiency(SoSArchitecture* arch) {
    if (!arch) return 0.0;
    sos_arch_compute_metrics(arch);
    return arch->efficiency;
}

double sos_arch_interoperability_score(SoSArchitecture* arch) {
    if (!arch || arch->n_connections == 0) return 0.0;
    double sum = 0; int n = 0;
    for (int i = 0; i < arch->n_connections; i++) {
        if (arch->connections[i].is_active) {
            sum += (double)arch->connections[i].level; n++;
        }
    }
    return (n > 0) ? sum / (double)(n * INTEROP_ENTERPRISE) : 0.0;
}

bool sos_arch_is_connected(SoSArchitecture* arch) {
    if (!arch || arch->n_systems < 2) return false;
    sos_arch_compute_metrics(arch);
    return arch->average_path_length > 0 && arch->connectivity_density > 0;
}

int sos_arch_shortest_path(SoSArchitecture* arch, int from, int to) {
    if (!arch || from < 0 || to < 0 || from >= arch->n_systems
        || to >= arch->n_systems) return -1;
    sos_arch_compute_metrics(arch);
    return (int)arch->average_path_length;
}

int* sos_arch_critical_nodes(SoSArchitecture* arch, int* n_critical) {
    if (!arch || !n_critical) return NULL;
    sos_arch_compute_metrics(arch);
    *n_critical = (arch->centrality_max > (arch->n_systems / 2.0)) ? 1 : 0;
    int* result = malloc((size_t)(*n_critical + 1) * sizeof(int));
    if (result && *n_critical > 0) result[0] = arch->central_system_idx;
    return result;
}

void sos_arch_print(SoSArchitecture* arch) {
    if (!arch) { printf("SoSArchitecture: NULL\n"); return; }
    printf("=== SoS Architecture: %s ===\n", sos_arch_type_name(arch->type));
    printf("Systems: %d  Connections: %d  Density: %.2f  Robustness: %.2f\n",
           arch->n_systems, sos_arch_n_connections(arch),
           arch->connectivity_density, arch->robustness);
}

const char* sos_arch_type_name(SoSArchitectureType type) {
    switch (type) {
        case ARCH_HIERARCHICAL: return "Hierarchical";
        case ARCH_NETWORKED: return "Networked";
        case ARCH_HUB_SPOKE: return "Hub-and-Spoke";
        case ARCH_MESH: return "Mesh";
        case ARCH_HYBRID: return "Hybrid";
        default: return "Unknown";
    }
}

const char* sos_interop_level_name(InteroperabilityLevel level) {
    switch (level) {
        case INTEROP_ISOLATED: return "Isolated";
        case INTEROP_CONNECTED: return "Connected";
        case INTEROP_FUNCTIONAL: return "Functional";
        case INTEROP_DOMAIN: return "Domain";
        case INTEROP_ENTERPRISE: return "Enterprise";
        default: return "Unknown";
    }
}

/* Extended architecture analysis */
double sos_arch_modularity(SoSArchitecture* arch) {
    if (!arch || arch->n_systems < 2) return 0.0;
    int n = arch->n_systems;
    double m = (double)sos_arch_n_connections(arch);
    if (m < 1) return 0.0;
    double Q = 0;
    for (int i = 0; i < n; i++) {
        double ki = 0;
        for (int j = 0; j < n; j++) ki += arch->adjacency[i][j];
        Q += ki * ki / (2.0 * m);
    }
    return 1.0 - Q / (2.0 * m);
}

double sos_arch_assortativity(SoSArchitecture* arch) {
    if (!arch || arch->n_systems < 2) return 0.0;
    int n = arch->n_systems;
    double sum_deg = 0, sum_deg2 = 0, sum_cross = 0;
    for (int i = 0; i < n; i++) {
        double di = 0;
        for (int j = 0; j < n; j++) di += arch->adjacency[i][j];
        sum_deg += di; sum_deg2 += di * di;
        for (int j = 0; j < n; j++)
            if (arch->adjacency[i][j] > 0.5) {
                double dj = 0;
                for (int k = 0; k < n; k++) dj += arch->adjacency[j][k];
                sum_cross += di * dj;
            }
    }
    double m = sum_deg / 2.0;
    if (m < 1) return 0.0;
    double num = sum_cross - sum_deg * sum_deg / (2.0 * m);
    double den = sum_deg2 - sum_deg * sum_deg / (2.0 * m);
    return (fabs(den) > 1e-12) ? num / den : 0.0;
}

int sos_arch_diameter(SoSArchitecture* arch) {
    if (!arch || arch->n_systems < 2) return 0;
    sos_arch_compute_metrics(arch);
    return (int)(arch->average_path_length * 2.0 + 0.5);
}

double sos_arch_vulnerability(SoSArchitecture* arch, int target_node) {
    if (!arch || target_node < 0 || target_node >= arch->n_systems) return 0.0;
    int n = arch->n_systems;
    double orig_deg = 0, alt_deg = 0;
    for (int i = 0; i < n; i++) orig_deg += arch->adjacency[target_node][i];
    alt_deg = (orig_deg > 0) ? orig_deg - 1.0 : 0.0;
    double orig_total = 0;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++) orig_total += arch->adjacency[i][j];
    double delta = orig_deg / (orig_total + 1.0);
    return delta;
}

void sos_arch_add_node(SoSArchitecture* arch) {
    if (!arch) return;
    int n = arch->n_systems + 1;
    double** new_adj = malloc((size_t)n * sizeof(double*));
    if (!new_adj) return;
    for (int i = 0; i < n; i++) {
        new_adj[i] = calloc((size_t)n, sizeof(double));
        if (!new_adj[i]) { for (int j=0;j<i;j++) free(new_adj[j]); free(new_adj); return; }
    }
    for (int i = 0; i < arch->n_systems; i++)
        for (int j = 0; j < arch->n_systems; j++)
            new_adj[i][j] = arch->adjacency[i][j];
    for (int i = 0; i < arch->n_systems; i++) free(arch->adjacency[i]);
    free(arch->adjacency);
    arch->adjacency = new_adj; arch->n_systems = n;
}

SoSArchitecture* sos_arch_create_from_template(SoSArchitectureType type, int n,
                                                 const int* edges_from, const int* edges_to,
                                                 int n_edges) {
    SoSArchitecture* arch = sos_arch_create(type, n);
    if (!arch) return NULL;
    for (int i = 0; i < n_edges; i++)
        sos_arch_connect(arch, edges_from[i], edges_to[i], INTEROP_FUNCTIONAL, 10.0, 0.01, "Generic");
    sos_arch_compute_metrics(arch);
    return arch;
}