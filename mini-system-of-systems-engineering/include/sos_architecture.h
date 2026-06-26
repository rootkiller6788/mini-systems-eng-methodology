#ifndef SOS_ARCHITECTURE_H
#define SOS_ARCHITECTURE_H
#include "gst_core.h"
#include "sos_types.h"

/* ==============================================================
 * sos_architecture.h — SoS Architecture and Interoperability
 *
 * Architecture types:
 *   Hierarchical: tree structure, single root
 *   Networked: peer-to-peer with optional hubs
 *   Hub-and-Spoke: central coordinator
 *   Mesh: fully connected
 *
 * Interoperability levels (LISI model):
 *   L0: Isolated, L1: Connected, L2: Functional, L3: Domain, L4: Enterprise
 * ============================================================== */

typedef enum {
    ARCH_HIERARCHICAL = 0,
    ARCH_NETWORKED = 1,
    ARCH_HUB_SPOKE = 2,
    ARCH_MESH = 3,
    ARCH_HYBRID = 4
} SoSArchitectureType;

typedef enum {
    INTEROP_ISOLATED = 0,
    INTEROP_CONNECTED = 1,
    INTEROP_FUNCTIONAL = 2,
    INTEROP_DOMAIN = 3,
    INTEROP_ENTERPRISE = 4,
    INTEROP_N_LEVELS = 5
} InteroperabilityLevel;

typedef struct {
    int from_idx;
    int to_idx;
    InteroperabilityLevel level;
    double bandwidth;
    double latency;
    double reliability;
    char* protocol;
    bool is_active;
} SoSConnection;

typedef struct {
    SoSArchitectureType type;
    SoSConnection* connections;
    int n_connections;
    int conn_cap;
    double** adjacency;
    int n_systems;
    double connectivity_density;
    double average_path_length;
    double clustering_coefficient;
    double centrality_max;
    int central_system_idx;
    double robustness;
    double efficiency;
} SoSArchitecture;

/* API */
SoSArchitecture* sos_arch_create(SoSArchitectureType type, int n_systems);
void sos_arch_free(SoSArchitecture* arch);
int sos_arch_connect(SoSArchitecture* arch, int from, int to,
                      InteroperabilityLevel level, double bw, double lat,
                      const char* protocol);
void sos_arch_disconnect(SoSArchitecture* arch, int from, int to);
int sos_arch_n_connections(SoSArchitecture* arch);
InteroperabilityLevel sos_arch_interop_level(SoSArchitecture* arch,
                                              int from, int to);
void sos_arch_compute_metrics(SoSArchitecture* arch);
double sos_arch_robustness(SoSArchitecture* arch);
double sos_arch_efficiency(SoSArchitecture* arch);
double sos_arch_interoperability_score(SoSArchitecture* arch);
bool sos_arch_is_connected(SoSArchitecture* arch);
int sos_arch_shortest_path(SoSArchitecture* arch, int from, int to);
int* sos_arch_critical_nodes(SoSArchitecture* arch, int* n_critical);
void sos_arch_print(SoSArchitecture* arch);
const char* sos_arch_type_name(SoSArchitectureType type);
const char* sos_interop_level_name(InteroperabilityLevel level);

#endif
