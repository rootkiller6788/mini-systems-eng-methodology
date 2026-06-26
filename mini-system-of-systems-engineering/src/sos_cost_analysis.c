#include "sos_cost_analysis.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

SoSCostEstimate* sos_cost_create(const char* name) {
    SoSCostEstimate* sce = calloc(1, sizeof(SoSCostEstimate));
    if (!sce) return NULL;
    sce->cost_model_name = name ? malloc(strlen(name) + 1) : NULL;
    if (name && sce->cost_model_name) strcpy(sce->cost_model_name, name);
    sce->years_of_operation = 10.0;
    return sce;
}

void sos_cost_free(SoSCostEstimate* sce) { if (sce) { free(sce->cost_model_name); free(sce); } }

double sos_cost_total(const SoSCostEstimate* sce) {
    if (!sce) return 0.0;
    return sce->development_cost + sce->integration_cost + sce->testing_cost
         + sce->deployment_cost
         + (sce->operations_cost_per_year + sce->maintenance_cost_per_year) * sce->years_of_operation
         + sce->disposal_cost;
}

double sos_cost_per_system(const SoSCostEstimate* sce, int n) {
    double t = sos_cost_total(sce);
    return (n > 0) ? t / (double)n : 0.0;
}

double sos_cost_per_year(const SoSCostEstimate* sce) {
    double t = sos_cost_total(sce);
    return (sce->years_of_operation > 0) ? t / sce->years_of_operation : 0.0;
}

SoSCostEstimate* sos_cost_estimate_from_architecture(const SoSArchitecture* arch, int n, double years) {
    if (!arch) return NULL;
    SoSCostEstimate* sce = sos_cost_create("Architecture-based");
    sce->integration_cost = (double)arch->n_connections * 100000.0;
    sce->operations_cost_per_year = (double)n * 50000.0;
    sce->maintenance_cost_per_year = (double)n * 20000.0;
    sce->years_of_operation = years;
    sce->total_lifecycle_cost = sos_cost_total(sce);
    return sce;
}

void sos_cost_print(const SoSCostEstimate* sce) {
    if (!sce) return;
    printf("=== Cost: %s ===\n", sce->cost_model_name ? sce->cost_model_name : "?");
    printf("Dev: %.0f  Integ: %.0f  Test: %.0f  Deploy: %.0f\n",
           sce->development_cost, sce->integration_cost, sce->testing_cost, sce->deployment_cost);
    printf("Ops/yr: %.0f  Maint/yr: %.0f  Total LCC: %.0f\n",
           sce->operations_cost_per_year, sce->maintenance_cost_per_year, sos_cost_total(sce));
}

CostBenefitAnalysis* sos_cba_create(double rate) {
    CostBenefitAnalysis* cba = calloc(1, sizeof(CostBenefitAnalysis));
    if (cba) cba->discount_rate = (rate > 0) ? rate : 0.05;
    return cba;
}

void sos_cba_free(CostBenefitAnalysis* cba) { free(cba); }
void sos_cba_set_current(CostBenefitAnalysis* cba, SoSCostEstimate* sce) { if (cba) cba->current = sce; }
void sos_cba_set_proposed(CostBenefitAnalysis* cba, SoSCostEstimate* sce) { if (cba) cba->proposed = sce; }

void sos_cba_compute(CostBenefitAnalysis* cba) {
    if (!cba || !cba->current || !cba->proposed) return;
    double cur = sos_cost_total(cba->current);
    double prop = sos_cost_total(cba->proposed);
    cba->cost_savings = cur - prop;
    cba->benefit_cost_ratio = (prop > 0) ? cur / prop : 0.0;
    cba->net_present_value = cba->cost_savings / pow(1.0 + cba->discount_rate, 10.0);
}

bool sos_cba_is_worthwhile(const CostBenefitAnalysis* cba) {
    return cba && cba->benefit_cost_ratio > 1.0;
}

void sos_cba_print(const CostBenefitAnalysis* cba) {
    if (!cba) return;
    printf("=== CBA: Savings=%.0f B/C=%.2f NPV=%.0f Worthwhile=%s ===\n",
           cba->cost_savings, cba->benefit_cost_ratio, cba->net_present_value,
           sos_cba_is_worthwhile(cba) ? "YES" : "NO");
}

BudgetTracker* sos_budget_create(double total) {
    BudgetTracker* bt = calloc(1, sizeof(BudgetTracker));
    if (bt) { bt->budget_total = total; bt->budget_remaining = total; }
    return bt;
}

void sos_budget_free(BudgetTracker* bt) {
    if (!bt) return;
    for (int i = 0; i < bt->n_cost_drivers; i++) free(bt->cost_driver_names[i]);
    free(bt->cost_driver_names); free(bt->cost_driver_impacts); free(bt);
}

void sos_budget_spend(BudgetTracker* bt, double amount) {
    if (!bt) return;
    bt->budget_spent += amount;
    bt->budget_remaining = bt->budget_total - bt->budget_spent;
    if (bt->budget_remaining < 0) { bt->budget_remaining = 0; bt->cost_overrun_risk = 1.0; }
    else bt->cost_overrun_risk = bt->budget_spent / bt->budget_total;
}

bool sos_budget_is_exceeded(const BudgetTracker* bt) { return bt && bt->budget_remaining <= 0; }

double sos_budget_burn_rate(const BudgetTracker* bt, double elapsed) {
    return (bt && elapsed > 0) ? bt->budget_spent / elapsed : 0.0;
}

void sos_budget_add_cost_driver(BudgetTracker* bt, const char* name, double impact) {
    if (!bt || !name) return;
    int nc = bt->n_cost_drivers + 1;
    char** nn = realloc(bt->cost_driver_names, (size_t)nc * sizeof(char*));
    if (!nn) return;
    bt->cost_driver_names = nn;
    bt->cost_driver_names[bt->n_cost_drivers] = malloc(strlen(name) + 1);
    if (bt->cost_driver_names[bt->n_cost_drivers])
        strcpy(bt->cost_driver_names[bt->n_cost_drivers], name);
    bt->n_cost_drivers++;
}

void sos_budget_print(const BudgetTracker* bt) {
    if (!bt) return;
    printf("=== Budget: Total=%.0f Spent=%.0f Remaining=%.0f Risk=%.2f ===\n",
           bt->budget_total, bt->budget_spent, bt->budget_remaining, bt->cost_overrun_risk);
}