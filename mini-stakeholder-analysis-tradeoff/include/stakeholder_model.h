#ifndef STAKEHOLDER_MODEL_H
#define STAKEHOLDER_MODEL_H
#include <stdbool.h>
#include <stddef.h>

typedef enum { STAKE_LOW_POWER_LOW_INTEREST=0, STAKE_LOW_POWER_HIGH_INTEREST=1,
  STAKE_HIGH_POWER_LOW_INTEREST=2, STAKE_HIGH_POWER_HIGH_INTEREST=3, STAKE_N_CATEGORIES=4
} StakeholderCategory;
typedef enum { STYPE_PRIMARY=0, STYPE_SECONDARY=1, STYPE_KEY=2,
  STYPE_REGULATORY=3, STYPE_COMMUNITY=4, STYPE_N_TYPES=5 } StakeholderType;
typedef enum { INFLUENCE_NEGATIVE=-1, INFLUENCE_NEUTRAL=0, INFLUENCE_POSITIVE=1 } InfluenceDirection;
typedef struct { char* name; StakeholderType type; double power, interest, influence;
  double urgency, legitimacy; char** requirements; int n_requirements;
  double* requirement_weights; double satisfaction; InfluenceDirection stance; } Stakeholder;
typedef struct { Stakeholder** stakeholders; int n_stakeholders, capacity;
  double** influence_matrix; double* betweenness; double network_density, centralization; } StakeholderNetwork;
Stakeholder* stk_create(const char* name, StakeholderType type, double power, double interest, double influence, double urgency, double legitimacy);
void stk_free(Stakeholder* s);
void stk_add_requirement(Stakeholder* s, const char* req, double weight);
double stk_salience(const Stakeholder* s);
StakeholderCategory stk_classify(const Stakeholder* s);
double stk_mitchell_score(const Stakeholder* s);
void stk_print(const Stakeholder* s);
StakeholderNetwork* stk_net_create(void);
void stk_net_free(StakeholderNetwork* net);
int stk_net_add(StakeholderNetwork* net, Stakeholder* s);
void stk_net_set_influence(StakeholderNetwork* net, int from, int to, double val);
void stk_net_compute_metrics(StakeholderNetwork* net);
void stk_net_find_key_players(StakeholderNetwork* net, int* indices, int* n);
double stk_net_conflict_index(StakeholderNetwork* net);
void stk_net_print(StakeholderNetwork* net);
#define STK_POWER_THRESHOLD 0.5
#define STK_INTEREST_THRESHOLD 0.5
#define STK_SALIENCE_WEIGHT_POWER 0.4
#define STK_SALIENCE_WEIGHT_LEGITIMACY 0.3
#define STK_SALIENCE_WEIGHT_URGENCY 0.3
/* Advanced stakeholder analysis functions */
void stk_net_grid_counts(StakeholderNetwork* net, int counts[4]);
int stk_net_most_influential(StakeholderNetwork* net);
const char* stk_engagement_strategy(const Stakeholder* s);
double stk_net_satisfaction(StakeholderNetwork* net);
double stk_net_alignment(StakeholderNetwork* net);
void stk_net_propagate_influence(StakeholderNetwork* net);
int stk_net_find_coalitions(StakeholderNetwork* net, int* coalition_ids);
double stk_requirement_overlap(const Stakeholder* a, const Stakeholder* b);
void stk_set_stance(Stakeholder* s, int stance);
int stk_generate_report(const Stakeholder* s, char* buf, int bufsize);
double stk_net_diversity(StakeholderNetwork* net);
void stk_net_influence_vector(StakeholderNetwork* net, double* vector);
void stk_net_prioritize(StakeholderNetwork* net, int* ranking);
double stk_power_distance(const Stakeholder* a, const Stakeholder* b);
double stk_net_max_power_distance(StakeholderNetwork* net);
double stk_net_influence_gini(StakeholderNetwork* net);
const char* stk_type_name(StakeholderType t);

/* Stakeholder mapping: connect stakeholder positions to system variables */
typedef struct { Stakeholder* stakeholder; int* variable_indices; double* position_values; int n_variables; } StakeholderMapping;
StakeholderMapping* stk_map_create(Stakeholder* s, int n_vars);
void stk_map_free(StakeholderMapping* m);
void stk_map_set_position(StakeholderMapping* m, int var_idx, double value);
double stk_map_distance(const StakeholderMapping* a, const StakeholderMapping* b);

#define STK_MAX_STAKEHOLDERS 64
#define STK_REPORT_BUFFER_SIZE 512
#endif
