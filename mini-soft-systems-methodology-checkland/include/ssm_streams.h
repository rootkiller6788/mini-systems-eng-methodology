#ifndef SSM_STREAMS_H
#define SSM_STREAMS_H
#include "ssm_types.h"

/* Rich Picture Element Types */
typedef enum { SSM_ELEM_STRUCTURE=0, SSM_ELEM_PROCESS=1, SSM_ELEM_RELATIONSHIP=2, SSM_ELEM_CONCERN=3, SSM_ELEM_BOUNDARY=4, SSM_ELEM_WORLDVIEW=5, SSM_ELEM_CONFLICT=6, SSM_ELEM_UNCERTAINTY=7 } RichPictureElement;

typedef struct { int id; char* label; RichPictureElement type; double x_position; double y_position; char* description; int* connected_to; int n_connections; int conn_capacity; double salience; } RichPictureNode;
typedef struct { char* situation_name; RichPictureNode** nodes; int n_nodes; int node_capacity; double complexity_index; double conflict_density; int dominant_worldview_count; } RichPicture;

typedef struct { char* name; char* description; char** role_holders; int n_role_holders; int rh_capacity; char** expected_behaviors; int n_norms; int norm_capacity; char** underlying_values; int n_values; int val_capacity; double role_power; double role_clarity; } SocialRole;
typedef struct { char* situation_name; SocialRole** roles; int n_roles; int role_capacity; double social_cohesion; double normative_conflict; double value_alignment; char** emergent_properties; int n_emergent; int em_capacity; } SocialSystemAnalysis;

typedef enum { SSM_COMM_AUTHORITY=0, SSM_COMM_INFORMATION=1, SSM_COMM_RESOURCES=2, SSM_COMM_EXPERTISE=3, SSM_COMM_REPUTATION=4, SSM_COMM_ACCESS=5, SSM_COMM_CHARISMA=6, SSM_COMM_LEGITIMACY=7 } CommodityType;
typedef struct { CommodityType type; char* description; char* primary_holder; char** secondary_holders; int n_secondary; int sec_capacity; double concentration; double contestation; bool is_asymmetric; } PoliticalCommodity;
typedef struct { char* situation_name; PoliticalCommodity** commodities; int n_commodities; int com_capacity; double power_concentration; double political_feasibility; char** power_struggles; int n_struggles; int st_capacity; char** accommodation_strategies; int n_strategies; int str_capacity; } PoliticalSystemAnalysis;

/* Rich Picture */
RichPicture* ssm_rich_picture_create(const char* name);
void ssm_rich_picture_free(RichPicture* rp);
int ssm_rp_add_node(RichPicture* rp, const char* label, RichPictureElement type, double x, double y);
void ssm_rp_connect(RichPicture* rp, int from_id, int to_id);
double ssm_rp_compute_complexity(RichPicture* rp);
double ssm_rp_compute_conflict_density(RichPicture* rp);
void ssm_rp_print(const RichPicture* rp);

/* Social System (Analysis 2) */
SocialSystemAnalysis* ssm_social_create(const char* name);
void ssm_social_free(SocialSystemAnalysis* ssa);
int ssm_social_add_role(SocialSystemAnalysis* ssa, const char* name, const char* desc);
void ssm_social_add_role_holder(SocialSystemAnalysis* ssa, int role_id, const char* holder);
void ssm_social_add_norm(SocialSystemAnalysis* ssa, int role_id, const char* norm);
void ssm_social_add_value(SocialSystemAnalysis* ssa, int role_id, const char* value);
double ssm_social_compute_cohesion(SocialSystemAnalysis* ssa);
double ssm_social_compute_normative_conflict(SocialSystemAnalysis* ssa);
void ssm_social_print(const SocialSystemAnalysis* ssa);

/* Political System (Analysis 3) */
PoliticalSystemAnalysis* ssm_political_create(const char* name);
void ssm_political_free(PoliticalSystemAnalysis* psa);
int ssm_political_add_commodity(PoliticalSystemAnalysis* psa, CommodityType type, const char* desc, const char* holder);
void ssm_political_add_secondary_holder(PoliticalSystemAnalysis* psa, int com_id, const char* holder);
void ssm_political_add_power_struggle(PoliticalSystemAnalysis* psa, const char* struggle);
void ssm_political_add_strategy(PoliticalSystemAnalysis* psa, const char* strategy);
double ssm_political_compute_concentration(PoliticalSystemAnalysis* psa);
double ssm_political_compute_feasibility(PoliticalSystemAnalysis* psa);
void ssm_political_print(const PoliticalSystemAnalysis* psa);

#endif
