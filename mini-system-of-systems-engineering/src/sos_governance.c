#include "sos_governance.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

SoSGovernance* sos_gov_create(GovernanceModel model) {
    SoSGovernance* gov = calloc(1, sizeof(SoSGovernance));
    if (!gov) return NULL;
    gov->model = model;
    gov->control_effectiveness = 0.5;
    gov->compliance_rate = 0.5;
    gov->agility = 0.5;
    gov->decision_latency = 1.0;
    gov->policy_names = NULL; gov->policy_descriptions = NULL;
    gov->policy_effectiveness = NULL; gov->n_policies = 0;
    gov->stakeholders = NULL; gov->n_stakeholders = 0; gov->stake_cap = 0;
    return gov;
}

void sos_gov_free(SoSGovernance* gov) {
    if (!gov) return;
    for (int i = 0; i < gov->n_policies; i++) {
        free(gov->policy_names[i]); free(gov->policy_descriptions[i]);
    }
    free(gov->policy_names); free(gov->policy_descriptions);
    free(gov->policy_effectiveness);
    for (int i = 0; i < gov->n_stakeholders; i++) {
        free(gov->stakeholders[i].name);
        for (int j = 0; j < gov->stakeholders[i].n_requirements; j++)
            free(gov->stakeholders[i].requirements[j]);
        free(gov->stakeholders[i].requirements);
    }
    free(gov->stakeholders); free(gov);
}

void sos_gov_add_policy(SoSGovernance* gov, const char* name,
                         const char* desc, double effectiveness) {
    if (!gov || !name) return;
    int nc = gov->n_policies + 1;
    char** nn = realloc(gov->policy_names, (size_t)nc * sizeof(char*));
    char** nd = realloc(gov->policy_descriptions, (size_t)nc * sizeof(char*));
    double* ne = realloc(gov->policy_effectiveness, (size_t)nc * sizeof(double));
    if (!nn || !nd || !ne) { free(nn); free(nd); free(ne); return; }
    gov->policy_names = nn; gov->policy_descriptions = nd;
    gov->policy_effectiveness = ne;
    gov->policy_names[gov->n_policies] = malloc(strlen(name) + 1);
    if (gov->policy_names[gov->n_policies])
        strcpy(gov->policy_names[gov->n_policies], name);
    gov->policy_descriptions[gov->n_policies] = desc ? malloc(strlen(desc) + 1) : NULL;
    if (desc && gov->policy_descriptions[gov->n_policies])
        strcpy(gov->policy_descriptions[gov->n_policies], desc);
    gov->policy_effectiveness[gov->n_policies] = effectiveness;
    gov->n_policies++;
}

double sos_gov_policy_impact(SoSGovernance* gov, int policy_idx,
                              SystemOfSystems* sos) {
    if (!gov || policy_idx < 0 || policy_idx >= gov->n_policies || !sos) return 0.0;
    return gov->policy_effectiveness[policy_idx] * gov->compliance_rate
           * sos->integration_level;
}

int sos_gov_add_stakeholder(SoSGovernance* gov, const char* name,
                             StakeholderType type, double influence,
                             double interest) {
    if (!gov || !name) return -1;
    if (gov->n_stakeholders >= gov->stake_cap) {
        int nc = (gov->stake_cap == 0) ? 8 : gov->stake_cap * 2;
        Stakeholder* ns = realloc(gov->stakeholders, (size_t)nc * sizeof(Stakeholder));
        if (!ns) return -1;
        gov->stakeholders = ns; gov->stake_cap = nc;
    }
    Stakeholder* s = &gov->stakeholders[gov->n_stakeholders];
    s->name = malloc(strlen(name) + 1);
    if (s->name) strcpy(s->name, name);
    s->type = type; s->influence = influence; s->interest = interest;
    s->requirements = NULL; s->n_requirements = 0; s->satisfaction = 0.5;
    return gov->n_stakeholders++;
}

void sos_gov_add_requirement(SoSGovernance* gov, int stake_idx, const char* req) {
    if (!gov || stake_idx < 0 || stake_idx >= gov->n_stakeholders || !req) return;
    Stakeholder* s = &gov->stakeholders[stake_idx];
    int nc = s->n_requirements + 1;
    char** nr = realloc(s->requirements, (size_t)nc * sizeof(char*));
    if (!nr) return;
    s->requirements = nr;
    s->requirements[s->n_requirements] = malloc(strlen(req) + 1);
    if (s->requirements[s->n_requirements])
        strcpy(s->requirements[s->n_requirements], req);
    s->n_requirements++;
}

double sos_gov_stakeholder_satisfaction(SoSGovernance* gov, int stake_idx,
                                         SystemOfSystems* sos) {
    if (!gov || stake_idx < 0 || stake_idx >= gov->n_stakeholders || !sos) return 0.0;
    Stakeholder* s = &gov->stakeholders[stake_idx];
    double base = sos_readiness(sos) * s->interest;
    return base * (0.5 + 0.5 * gov->compliance_rate);
}

double sos_gov_overall_satisfaction(SoSGovernance* gov, SystemOfSystems* sos) {
    if (!gov || gov->n_stakeholders == 0) return 0.0;
    double total = 0, total_inf = 0;
    for (int i = 0; i < gov->n_stakeholders; i++) {
        double inf = gov->stakeholders[i].influence;
        total += sos_gov_stakeholder_satisfaction(gov, i, sos) * inf;
        total_inf += inf;
    }
    return (total_inf > 0) ? total / total_inf : 0.0;
}

void sos_gov_assess(SoSGovernance* gov, SystemOfSystems* sos) {
    if (!gov || !sos) return;
    int n = sos_n_constituents(sos);
    switch (gov->model) {
        case GOV_CENTRALIZED:
            gov->control_effectiveness = 0.8; gov->agility = 0.3; break;
        case GOV_FEDERATED:
            gov->control_effectiveness = 0.6; gov->agility = 0.6; break;
        case GOV_DISTRIBUTED:
            gov->control_effectiveness = 0.3; gov->agility = 0.9; break;
        case GOV_MARKET:
            gov->control_effectiveness = 0.4; gov->agility = 0.7; break;
    }
    gov->compliance_rate = (n > 0) ? 0.5 + 0.3 * sos->integration_level : 0.0;
}

bool sos_gov_is_effective(SoSGovernance* gov) {
    if (!gov) return false;
    return gov->control_effectiveness > 0.4 && gov->compliance_rate > 0.3;
}

double sos_gov_governance_fitness(SoSGovernance* gov, SoSType sos_type) {
    if (!gov) return 0.0;
    double ideal_control = (sos_type == SOS_DIRECTED) ? 0.8 :
                           (sos_type == SOS_VIRTUAL) ? 0.2 : 0.5;
    double ideal_agility = (sos_type == SOS_VIRTUAL) ? 0.9 :
                           (sos_type == SOS_DIRECTED) ? 0.3 : 0.6;
    double control_fit = 1.0 - fabs(gov->control_effectiveness - ideal_control);
    double agility_fit = 1.0 - fabs(gov->agility - ideal_agility);
    return (control_fit + agility_fit) / 2.0;
}

void sos_gov_print(SoSGovernance* gov) {
    if (!gov) { printf("SoSGovernance: NULL\n"); return; }
    printf("=== SoS Governance: %s ===\n", sos_gov_model_name(gov->model));
    printf("Control: %.2f  Compliance: %.2f  Agility: %.2f  Latency: %.2f\n",
           gov->control_effectiveness, gov->compliance_rate,
           gov->agility, gov->decision_latency);
}

const char* sos_gov_model_name(GovernanceModel model) {
    switch (model) {
        case GOV_CENTRALIZED: return "Centralized";
        case GOV_FEDERATED: return "Federated";
        case GOV_DISTRIBUTED: return "Distributed";
        case GOV_MARKET: return "Market-based";
        default: return "Unknown";
    }
}

/* Extended governance analysis */
double sos_gov_conflict_index(const SoSGovernance* gov) {
    if (!gov || gov->n_stakeholders < 2) return 0.0;
    double conflict = 0;
    for (int i = 0; i < gov->n_stakeholders; i++)
        for (int j = i + 1; j < gov->n_stakeholders; j++) {
            double diff = fabs(gov->stakeholders[i].influence - gov->stakeholders[j].influence);
            conflict += diff * gov->stakeholders[i].interest * gov->stakeholders[j].interest;
        }
    double n_pairs = (double)(gov->n_stakeholders * (gov->n_stakeholders - 1)) / 2.0;
    return (n_pairs > 0) ? conflict / n_pairs : 0.0;
}

double sos_gov_policy_coverage(const SoSGovernance* gov) {
    if (!gov || gov->n_policies == 0) return 0.0;
    double total_eff = 0;
    for (int i = 0; i < gov->n_policies; i++)
        total_eff += gov->policy_effectiveness[i];
    return fmin(total_eff / (double)gov->n_policies, 1.0);
}

double sos_gov_resilience_to_change(const SoSGovernance* gov) {
    if (!gov) return 0.0;
    return gov->agility * (1.0 - gov->control_effectiveness * 0.3);
}

void sos_gov_compare_models(GovernanceModel m1, GovernanceModel m2, double* similarity) {
    if (!similarity) return;
    double c1[] = {0.8, 0.6, 0.3, 0.4};
    double a1[] = {0.3, 0.6, 0.9, 0.7};
    if (m1 == m2) *similarity = 1.0;
    else {
        double dc = fabs(c1[m1] - c1[m2]);
        double da = fabs(a1[m1] - a1[m2]);
        *similarity = 1.0 - (dc + da) / 2.0;
    }
}