#ifndef SOS_GOVERNANCE_H
#define SOS_GOVERNANCE_H
#include "gst_core.h"
#include "sos_types.h"

/* ==============================================================
 * sos_governance.h — SoS Governance and Management
 *
 * Governance models for Systems of Systems:
 *   Centralized: single authority controls all systems
 *   Federated: shared authority with central coordination
 *   Distributed: no central authority, peer agreements
 *   Market-based: economic incentives drive cooperation
 *
 * Key metrics: control effectiveness, compliance, agility
 * ============================================================== */

typedef enum {
    GOV_CENTRALIZED = 0,
    GOV_FEDERATED = 1,
    GOV_DISTRIBUTED = 2,
    GOV_MARKET = 3
} GovernanceModel;

typedef enum {
    STAKE_SYSTEM_OWNER = 0,
    STAKE_SOS_MANAGER = 1,
    STAKE_END_USER = 2,
    STAKE_REGULATOR = 3,
    STAKE_DEVELOPER = 4,
    STAKE_N_TYPES = 5
} StakeholderType;

typedef struct {
    char* name;
    StakeholderType type;
    double influence;            /* 0-1 */
    double interest;             /* 0-1 */
    char** requirements;
    int n_requirements;
    double satisfaction;         /* 0-1 */
} Stakeholder;

typedef struct {
    GovernanceModel model;
    double control_effectiveness;    /* 0-1 */
    double compliance_rate;          /* 0-1 */
    double agility;                  /* 0-1 */
    double decision_latency;         /* time units */
    int n_policies;
    char** policy_names;
    char** policy_descriptions;
    double* policy_effectiveness;
    Stakeholder* stakeholders;
    int n_stakeholders;
    int stake_cap;
} SoSGovernance;

/* API */
SoSGovernance* sos_gov_create(GovernanceModel model);
void sos_gov_free(SoSGovernance* gov);
void sos_gov_add_policy(SoSGovernance* gov, const char* name,
                         const char* desc, double effectiveness);
double sos_gov_policy_impact(SoSGovernance* gov, int policy_idx,
                              SystemOfSystems* sos);
int sos_gov_add_stakeholder(SoSGovernance* gov, const char* name,
                             StakeholderType type, double influence,
                             double interest);
void sos_gov_add_requirement(SoSGovernance* gov, int stake_idx,
                              const char* req);
double sos_gov_stakeholder_satisfaction(SoSGovernance* gov, int stake_idx,
                                         SystemOfSystems* sos);
double sos_gov_overall_satisfaction(SoSGovernance* gov, SystemOfSystems* sos);
void sos_gov_assess(SoSGovernance* gov, SystemOfSystems* sos);
bool sos_gov_is_effective(SoSGovernance* gov);
double sos_gov_governance_fitness(SoSGovernance* gov, SoSType sos_type);
void sos_gov_print(SoSGovernance* gov);
const char* sos_gov_model_name(GovernanceModel model);

#endif
