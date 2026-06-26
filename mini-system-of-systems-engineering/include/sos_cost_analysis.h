#ifndef SOS_COST_ANALYSIS_H
#define SOS_COST_ANALYSIS_H
#include "sos_types.h"
#include "sos_architecture.h"
#include <stdbool.h>

typedef struct {
    double development_cost, integration_cost, testing_cost;
    double deployment_cost, operations_cost_per_year;
    double maintenance_cost_per_year, disposal_cost;
    double total_lifecycle_cost, years_of_operation;
    char* cost_model_name;
} SoSCostEstimate;

typedef struct {
    SoSCostEstimate* current, *proposed;
    double cost_savings, benefit_cost_ratio;
    double net_present_value, discount_rate;
    int payback_period_years;
} CostBenefitAnalysis;

typedef struct {
    double budget_total, budget_spent, budget_remaining;
    double cost_overrun_risk;
    int n_cost_drivers; char** cost_driver_names; double* cost_driver_impacts;
} BudgetTracker;

SoSCostEstimate* sos_cost_create(const char* name);
void sos_cost_free(SoSCostEstimate* sce);
double sos_cost_total(const SoSCostEstimate* sce);
double sos_cost_per_system(const SoSCostEstimate* sce, int n);
double sos_cost_per_year(const SoSCostEstimate* sce);
void sos_cost_print(const SoSCostEstimate* sce);
SoSCostEstimate* sos_cost_estimate_from_architecture(const SoSArchitecture* arch, int n, double years);

CostBenefitAnalysis* sos_cba_create(double rate);
void sos_cba_free(CostBenefitAnalysis* cba);
void sos_cba_set_current(CostBenefitAnalysis* cba, SoSCostEstimate* sce);
void sos_cba_set_proposed(CostBenefitAnalysis* cba, SoSCostEstimate* sce);
void sos_cba_compute(CostBenefitAnalysis* cba);
bool sos_cba_is_worthwhile(const CostBenefitAnalysis* cba);
void sos_cba_print(const CostBenefitAnalysis* cba);

BudgetTracker* sos_budget_create(double total);
void sos_budget_free(BudgetTracker* bt);
void sos_budget_spend(BudgetTracker* bt, double amount);
bool sos_budget_is_exceeded(const BudgetTracker* bt);
double sos_budget_burn_rate(const BudgetTracker* bt, double elapsed);
void sos_budget_add_cost_driver(BudgetTracker* bt, const char* name, double impact);
void sos_budget_print(const BudgetTracker* bt);
#endif