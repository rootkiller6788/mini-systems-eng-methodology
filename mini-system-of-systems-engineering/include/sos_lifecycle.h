#ifndef SOS_LIFECYCLE_H
#define SOS_LIFECYCLE_H
#include "sos_types.h"
#include "sos_architecture.h"
#include <stdbool.h>
/* SoS Lifecycle: tracks system evolution from formation through dissolution.
   Patterns: Organic growth, Staged deployment, Technology refresh. */
typedef enum { EVOLVE_ORGANIC=0,EVOLVE_STAGED=1,EVOLVE_REFRESH=2,EVOLVE_MISSION=3 } EvolutionPattern;
typedef struct { double time; int n_systems; double readiness,integration,emergence; SoSLifecyclePhase phase; } LifecycleSnapshot;
typedef struct { LifecycleSnapshot* snapshots; int n_snapshots,cap; SystemOfSystems* sos; EvolutionPattern pattern; double lifetime,current_time; int n_systems_added,n_systems_removed,n_architecture_changes; double growth_rate,decay_rate; bool is_maturing; } SoSLifecycle;
SoSLifecycle* sos_lifecycle_create(SystemOfSystems* sos,EvolutionPattern pattern);
void sos_lifecycle_free(SoSLifecycle* lc);
void sos_lifecycle_advance(SoSLifecycle* lc,double dt);
void sos_lifecycle_snapshot(SoSLifecycle* lc);
void sos_lifecycle_add_system(SoSLifecycle* lc,ConstituentSystem* cs);
int sos_lifecycle_remove_system(SoSLifecycle* lc,int idx);
void sos_lifecycle_change_architecture(SoSLifecycle* lc,SoSArchitectureType t);
double sos_lifecycle_time_to_maturity(SoSLifecycle* lc);
bool sos_lifecycle_is_sustainable(SoSLifecycle* lc);
double sos_lifecycle_growth_rate(const SoSLifecycle* lc);
double sos_lifecycle_churn_rate(const SoSLifecycle* lc);
int sos_lifecycle_predict_size(const SoSLifecycle* lc,double t);
void sos_lifecycle_print(const SoSLifecycle* lc);
#endif