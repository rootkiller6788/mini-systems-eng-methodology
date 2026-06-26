#ifndef SOS_TYPES_H
#define SOS_TYPES_H
#include "gst_core.h"

/* ==============================================================
 * sos_types.h — System of Systems Classification
 * Ref: Maier (1998), Boardman & Sauser (2006)
 *
 * SoS Types:
 *   Directed: built and managed to fulfill specific purposes
 *   Acknowledged: recognized objectives, centralized management
 *   Collaborative: voluntary interactions, collective goals
 *   Virtual: lacks central management, emergent behavior
 *
 * Five SoS Characteristics:
 *   Autonomy, Belonging, Connectivity, Diversity, Emergence
 * ============================================================== */

typedef enum {
    SOS_DIRECTED = 0,
    SOS_ACKNOWLEDGED = 1,
    SOS_COLLABORATIVE = 2,
    SOS_VIRTUAL = 3,
    SOS_N_TYPES = 4
} SoSType;

typedef enum {
    SOS_PHASE_FORMING = 0,
    SOS_PHASE_DEVELOPING = 1,
    SOS_PHASE_OPERATING = 2,
    SOS_PHASE_EVOLVING = 3,
    SOS_PHASE_DISSOLVING = 4
} SoSLifecyclePhase;

typedef struct {
    double autonomy;         /* independence of constituent systems [0-1] */
    double belonging;        /* commitment to SoS goals [0-1] */
    double connectivity;     /* inter-system links and data flow [0-1] */
    double diversity;        /* heterogeneity of constituent systems [0-1] */
    double emergence;        /* emergent capability beyond sum of parts [0-1] */
} SoSCharacteristics;

typedef struct {
    char* name;
    SoSType type;
    SoSLifecyclePhase phase;
    SoSCharacteristics chars;
    SystemSet* constituents;
    char* mission;
    double formation_year;
    double operational_readiness;
    double integration_level;
    int n_emergent_behaviors;
    char** emergent_behavior_names;
} SystemOfSystems;

/* API */
SystemOfSystems* sos_create(const char* name, SoSType type, const char* mission);
void sos_free(SystemOfSystems* sos);
void sos_add_constituent(SystemOfSystems* sos, ConstituentSystem* cs);
int sos_remove_constituent(SystemOfSystems* sos, int idx);
int sos_n_constituents(SystemOfSystems* sos);
void sos_compute_characteristics(SystemOfSystems* sos);
double sos_readiness(SystemOfSystems* sos);
double sos_integration_index(SystemOfSystems* sos);
double sos_complexity(SystemOfSystems* sos);
bool sos_is_viable(SystemOfSystems* sos);
void sos_print(SystemOfSystems* sos);
const char* sos_type_name(SoSType type);
const char* sos_phase_name(SoSLifecyclePhase phase);

#endif
