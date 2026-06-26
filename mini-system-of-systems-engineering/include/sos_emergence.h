#ifndef SOS_EMERGENCE_H
#define SOS_EMERGENCE_H
#include "gst_core.h"
#include "sos_types.h"
#include "sos_architecture.h"

/* ==============================================================
 * sos_emergence.h — Emergent Behavior in SoS
 *
 * Emergence: properties/behaviors that arise from interactions
 * among constituent systems but are not present in any individual
 * system.
 *
 * Types:
 *   Weak emergence: predictable from component properties
 *   Strong emergence: irreducible to component properties
 *   Nominal emergence: intended/designed
 *   Pathological emergence: unintended/harmful
 * ============================================================== */

typedef enum {
    EMERGE_WEAK = 0,
    EMERGE_STRONG = 1,
    EMERGE_NOMINAL = 2,
    EMERGE_PATHOLOGICAL = 3
} EmergenceType;

typedef struct {
    char* name;
    char* description;
    EmergenceType type;
    double strength;             /* 0-1 intensity */
    double predictability;       /* 0-1 how predictable */
    int* contributing_systems;   /* indices of systems involved */
    int n_contributing;
    double emergence_threshold;  /* minimum condition for emergence */
    bool is_active;
    bool is_beneficial;
} EmergentBehavior;

typedef struct {
    EmergentBehavior* behaviors;
    int n_behaviors;
    int cap;
    double emergence_index;       /* overall emergence level [0-1] */
    double nominal_ratio;         /* fraction that is beneficial */
    double surprise_index;        /* unexpected emergence measure */
} EmergenceProfile;

/* API */
EmergentBehavior* sos_emerge_create(const char* name, const char* desc,
                                     EmergenceType type);
void sos_emerge_free(EmergentBehavior* eb);
void sos_emerge_add_contributor(EmergentBehavior* eb, int sys_idx);
double sos_emerge_assess(EmergentBehavior* eb, SystemOfSystems* sos,
                          SoSArchitecture* arch);

EmergenceProfile* sos_emerge_profile_create(void);
void sos_emerge_profile_free(EmergenceProfile* ep);
void sos_emerge_profile_add(EmergenceProfile* ep, EmergentBehavior* eb);
void sos_emerge_profile_analyze(EmergenceProfile* ep, SystemOfSystems* sos,
                                 SoSArchitecture* arch);
EmergentBehavior* sos_emerge_profile_dominant(EmergenceProfile* ep);
int sos_emerge_count_by_type(EmergenceProfile* ep, EmergenceType type);
double sos_emerge_pathological_risk(EmergenceProfile* ep);
void sos_emerge_profile_print(EmergenceProfile* ep);

double sos_emergence_formula(int n_systems, double connectivity,
                              double diversity, double integration);
bool sos_detect_novel_emergence(SystemOfSystems* sos, SoSArchitecture* arch,
                                 double threshold);

#endif
