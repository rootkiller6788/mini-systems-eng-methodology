#include "sos_assessment.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

SoSAssessment* sos_assess_create(void) {
    SoSAssessment* sa = calloc(1, sizeof(SoSAssessment));
    if (!sa) return NULL;
    sa->maturity_level = MATURITY_INITIAL;
    sa->recommendations = NULL; sa->n_recommendations = 0;
    sa->dimension_scores = malloc(6 * sizeof(double));
    sa->n_dimensions = 6;
    if (sa->dimension_scores) memset(sa->dimension_scores, 0, 6 * sizeof(double));
    return sa;
}

void sos_assess_free(SoSAssessment* sa) {
    if (!sa) return;
    for (int i = 0; i < sa->n_recommendations; i++) free(sa->recommendations[i]);
    free(sa->recommendations); free(sa->dimension_scores); free(sa);
}

void sos_assess_readiness(SoSAssessment* sa, SystemOfSystems* sos) {
    if (!sa || !sos) return;
    sa->readiness_score = sos_readiness(sos);
}

void sos_assess_effectiveness(SoSAssessment* sa, SystemOfSystems* sos,
                               SoSArchitecture* arch) {
    if (!sa || !sos || !arch) return;
    sos_compute_characteristics(sos);
    sa->effectiveness_score = sos_effectiveness_formula(
        sa->readiness_score, sos->integration_level,
        sos->chars.emergence, 0.5);
}

void sos_assess_resilience(SoSAssessment* sa, SystemOfSystems* sos,
                            SoSArchitecture* arch) {
    if (!sa || !sos || !arch) return;
    double redundancy = (sos_n_constituents(sos) > 1) ? 1.0 / (double)sos_n_constituents(sos) : 0.0;
    double adaptability = sos->chars.autonomy;
    double recovery = sos_arch_robustness(arch);
    sa->resilience_score = sos_resilience_formula(redundancy, adaptability, recovery);
}

void sos_assess_maturity(SoSAssessment* sa, int n_best_practices,
                          int n_implemented) {
    if (!sa) return;
    double ratio = (n_best_practices > 0)
        ? (double)n_implemented / (double)n_best_practices : 0.0;
    sa->maturity_score = ratio;
    if (ratio < 0.2) sa->maturity_level = MATURITY_INITIAL;
    else if (ratio < 0.4) sa->maturity_level = MATURITY_MANAGED;
    else if (ratio < 0.6) sa->maturity_level = MATURITY_DEFINED;
    else if (ratio < 0.8) sa->maturity_level = MATURITY_QUANTITATIVE;
    else sa->maturity_level = MATURITY_OPTIMIZING;
}

void sos_assess_compute_overall(SoSAssessment* sa) {
    if (!sa) return;
    double weights[] = {0.25, 0.15, 0.25, 0.15, 0.10, 0.10};
    double scores[] = {sa->readiness_score, sa->effectiveness_score,
                       sa->resilience_score, sa->maturity_score,
                       sa->affordability_score, sa->sustainability_score};
    sa->overall_score = 0;
    for (int i = 0; i < 6; i++) sa->overall_score += weights[i] * scores[i];
}

void sos_assess_add_recommendation(SoSAssessment* sa, const char* rec) {
    if (!sa || !rec) return;
    int nc = sa->n_recommendations + 1;
    char** nr = realloc(sa->recommendations, (size_t)nc * sizeof(char*));
    if (!nr) return;
    sa->recommendations = nr;
    sa->recommendations[sa->n_recommendations] = malloc(strlen(rec) + 1);
    if (sa->recommendations[sa->n_recommendations])
        strcpy(sa->recommendations[sa->n_recommendations], rec);
    sa->n_recommendations++;
}

void sos_assess_print(SoSAssessment* sa) {
    if (!sa) { printf("SoSAssessment: NULL\n"); return; }
    printf("=== SoS Assessment ===\n");
    printf("Readiness: %.2f  Effectiveness: %.2f  Resilience: %.2f\n",
           sa->readiness_score, sa->effectiveness_score, sa->resilience_score);
    printf("Maturity: %.2f (L%d)  Overall: %.2f\n",
           sa->maturity_score, sa->maturity_level, sa->overall_score);
    if (sa->n_recommendations > 0) {
        printf("Recommendations:\n");
        for (int i = 0; i < sa->n_recommendations; i++)
            printf("  %d. %s\n", i + 1, sa->recommendations[i]);
    }
}

ResilienceScenario* sos_resilience_create(const char* name, int n_systems) {
    if (!name || n_systems < 1) return NULL;
    ResilienceScenario* rs = calloc(1, sizeof(ResilienceScenario));
    if (!rs) return NULL;
    rs->scenario_name = malloc(strlen(name) + 1);
    if (rs->scenario_name) strcpy(rs->scenario_name, name);
    rs->impact_vector = calloc((size_t)n_systems, sizeof(double));
    rs->n_impacts = n_systems;
    rs->system_survives = true;
    return rs;
}

void sos_resilience_free(ResilienceScenario* rs) {
    if (!rs) return;
    free(rs->scenario_name); free(rs->impact_vector); free(rs);
}

void sos_resilience_set_impact(ResilienceScenario* rs, int sys_idx, double impact) {
    if (!rs || sys_idx < 0 || sys_idx >= rs->n_impacts) return;
    rs->impact_vector[sys_idx] = impact;
}

bool sos_resilience_evaluate(ResilienceScenario* rs, SystemOfSystems* sos,
                              SoSArchitecture* arch) {
    if (!rs || !sos || !arch) return true;
    double total_impact = 0;
    for (int i = 0; i < rs->n_impacts; i++)
        total_impact += rs->impact_vector[i];
    rs->degradation = total_impact / (double)rs->n_impacts;
    rs->system_survives = (rs->degradation < 0.7);
    rs->recovery_time = rs->degradation * 10.0;
    rs->cascading_failure = (rs->degradation > 0.5 && arch->connectivity_density > 0.6);
    return rs->system_survives;
}

ResilienceAnalysis* sos_resilience_analysis_create(void) {
    return calloc(1, sizeof(ResilienceAnalysis));
}

void sos_resilience_analysis_free(ResilienceAnalysis* ra) {
    if (!ra) return;
    for (int i = 0; i < ra->n_scenarios; i++) {
        free(ra->scenarios[i].scenario_name);
        free(ra->scenarios[i].impact_vector);
    }
    free(ra->scenarios); free(ra);
}

void sos_resilience_analysis_add(ResilienceAnalysis* ra, ResilienceScenario* rs) {
    if (!ra || !rs) return;
    if (ra->n_scenarios >= ra->cap) {
        int nc = (ra->cap == 0) ? 8 : ra->cap * 2;
        ResilienceScenario* ns = realloc(ra->scenarios, (size_t)nc * sizeof(ResilienceScenario));
        if (!ns) return;
        ra->scenarios = ns; ra->cap = nc;
    }
    /* Deep copy */
    ResilienceScenario* dest = &ra->scenarios[ra->n_scenarios];
    memset(dest, 0, sizeof(ResilienceScenario));
    dest->scenario_name = rs->scenario_name ? malloc(strlen(rs->scenario_name) + 1) : NULL;
    if (dest->scenario_name) strcpy(dest->scenario_name, rs->scenario_name);
    dest->n_impacts = rs->n_impacts;
    if (rs->n_impacts > 0 && rs->impact_vector) {
        dest->impact_vector = malloc((size_t)rs->n_impacts * sizeof(double));
        memcpy(dest->impact_vector, rs->impact_vector, (size_t)rs->n_impacts * sizeof(double));
    }
    dest->recovery_time = rs->recovery_time;
    dest->degradation = rs->degradation;
    dest->cascading_failure = rs->cascading_failure;
    dest->system_survives = rs->system_survives;
    ra->n_scenarios++;
}

void sos_resilience_analysis_compute(ResilienceAnalysis* ra,
                                      SystemOfSystems* sos, SoSArchitecture* arch) {
    if (!ra || ra->n_scenarios == 0) return;
    double total_deg = 0, total_rec = 0, worst = 0;
    for (int i = 0; i < ra->n_scenarios; i++) {
        sos_resilience_evaluate(&ra->scenarios[i], sos, arch);
        total_deg += ra->scenarios[i].degradation;
        total_rec += ra->scenarios[i].recovery_time;
        if (ra->scenarios[i].degradation > worst) worst = ra->scenarios[i].degradation;
    }
    ra->resilience_index = 1.0 - worst;
    ra->mean_recovery_time = total_rec / (double)ra->n_scenarios;
    ra->worst_case_degradation = worst;
}

void sos_resilience_analysis_print(ResilienceAnalysis* ra) {
    if (!ra) { printf("ResilienceAnalysis: NULL\n"); return; }
    printf("=== Resilience Analysis (%d scenarios) ===\n", ra->n_scenarios);
    printf("Index: %.2f  Recovery: %.2f  WorstCase: %.2f\n",
           ra->resilience_index, ra->mean_recovery_time, ra->worst_case_degradation);
}

double sos_effectiveness_formula(double readiness, double integration,
                                  double emergence, double satisfaction) {
    return 0.3 * readiness + 0.3 * integration + 0.2 * emergence + 0.2 * satisfaction;
}

double sos_resilience_formula(double redundancy, double adaptability,
                               double recovery_speed) {
    return 0.4 * redundancy + 0.3 * adaptability + 0.3 * recovery_speed;
}

/* Extended assessment */
typedef struct { double score; char* name; } AssessmentDimension;

double sos_assess_dimension_score(const SoSAssessment* sa, int dim) {
    if (!sa || dim < 0 || dim >= sa->n_dimensions) return 0.0;
    return sa->dimension_scores[dim];
}

void sos_assess_set_affordability(SoSAssessment* sa, double budget_used, double budget_total) {
    if (!sa || budget_total <= 0) return;
    sa->affordability_score = 1.0 - fmin(budget_used / budget_total, 1.0);
}

void sos_assess_set_sustainability(SoSAssessment* sa, double lifetime, double design_life) {
    if (!sa || design_life <= 0) return;
    sa->sustainability_score = fmin(lifetime / design_life, 1.0);
}

void sos_assess_risk_assessment(SoSAssessment* sa, SystemOfSystems* sos, SoSArchitecture* arch) {
    if (!sa || !sos || !arch) return;
    double tech_risk = 1.0 - sos_readiness(sos);
    double integ_risk = 1.0 - sos->integration_level;
    double arch_risk = 1.0 - sos_arch_robustness(arch);
    double overall_risk = (tech_risk + integ_risk + arch_risk) / 3.0;
    if (overall_risk > 0.5)
        sos_assess_add_recommendation(sa, "High risk assessment: review architecture");
}

void sos_assess_compare(const SoSAssessment* a, const SoSAssessment* b, double* delta) {
    if (!delta) return;
    *delta = (a && b) ? a->overall_score - b->overall_score : 0.0;
}