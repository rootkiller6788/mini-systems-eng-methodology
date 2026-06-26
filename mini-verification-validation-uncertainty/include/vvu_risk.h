#ifndef VVU_RISK_H
#define VVU_RISK_H
#include "vvu_core.h"

typedef enum { VVU_RISK_LOW=0, VVU_RISK_MEDIUM=1, VVU_RISK_HIGH=2, VVU_RISK_CRITICAL=3, VVU_RISK_CATASTROPHIC=4 } VVU_RiskLevel;
typedef enum { VVU_LIKE_RARE=0, VVU_LIKE_UNLIKELY=1, VVU_LIKE_POSSIBLE=2, VVU_LIKE_LIKELY=3, VVU_LIKE_CERTAIN=4 } VVU_Likelihood;
typedef enum { VVU_CONS_NEGLIGIBLE=0, VVU_CONS_MINOR=1, VVU_CONS_MODERATE=2, VVU_CONS_MAJOR=3, VVU_CONS_CATASTROPHIC=4 } VVU_Consequence;

typedef struct {
    char* id; char* description; VVU_Likelihood likelihood;
    VVU_Consequence consequence; VVU_RiskLevel risk_level;
    double probability; double impact_score; char* mitigation;
    double residual_probability; double residual_impact; bool is_acceptable;
} VVU_RiskItem;

typedef struct {
    char* failure_mode; char* effect; char* cause;
    int severity; int occurrence; int detection; int rpn;
    char* recommended_action;
} VVU_FMEAItem;

typedef struct {
    VVU_RiskItem* risks; int n_risks; int risk_capacity;
    VVU_FMEAItem* fmea_items; int n_fmea; int fmea_capacity;
    double total_risk_exposure; double residual_risk_exposure;
    double risk_reduction_ratio;
} VVU_RiskRegister;

VVU_RiskRegister* vvu_risk_register_create(void);
void vvu_risk_register_free(VVU_RiskRegister* rr);
int vvu_risk_add(VVU_RiskRegister* rr, const char* id, const char* desc, VVU_Likelihood like, VVU_Consequence conseq);
void vvu_risk_set_mitigation(VVU_RiskRegister* rr, int idx, const char* mitigation, double residual_prob);
int vvu_fmea_add(VVU_RiskRegister* rr, const char* mode, const char* effect, const char* cause, int sev, int occ, int det);
void vvu_risk_compute_metrics(VVU_RiskRegister* rr);
VVU_RiskLevel vvu_risk_matrix_level(VVU_Likelihood l, VVU_Consequence c);
double vvu_risk_exposure(const VVU_RiskRegister* rr);
int vvu_risk_count_by_level(const VVU_RiskRegister* rr, VVU_RiskLevel level);
void vvu_risk_print(const VVU_RiskRegister* rr);
void vvu_risk_matrix_print(const VVU_RiskRegister* rr);
void vvu_fmea_print(const VVU_RiskRegister* rr);

#endif
