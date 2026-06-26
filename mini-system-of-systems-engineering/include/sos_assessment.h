#ifndef SOS_ASSESSMENT_H
#define SOS_ASSESSMENT_H
#include "gst_core.h"
#include "sos_types.h"
#include "sos_architecture.h"

/* ==============================================================
 * sos_assessment.h — SoS Readiness, Maturity, and Effectiveness
 *
 * Assessment dimensions:
 *   Readiness: operational capability to perform mission
 *   Maturity: process capability of SoS engineering
 *   Effectiveness: degree of mission accomplishment
 *   Resilience: ability to withstand and recover from disruptions
 * ============================================================== */

typedef enum {
    MATURITY_INITIAL = 0,
    MATURITY_MANAGED = 1,
    MATURITY_DEFINED = 2,
    MATURITY_QUANTITATIVE = 3,
    MATURITY_OPTIMIZING = 4,
    MATURITY_N_LEVELS = 5
} MaturityLevel;

typedef struct {
    double readiness_score;          /* 0-1 */
    double maturity_score;           /* 0-1 */
    double effectiveness_score;      /* 0-1 */
    double resilience_score;         /* 0-1 */
    double affordability_score;      /* 0-1 */
    double sustainability_score;     /* 0-1 */
    double overall_score;            /* weighted average */
    MaturityLevel maturity_level;
    char** recommendations;
    int n_recommendations;
    double* dimension_scores;
    int n_dimensions;
} SoSAssessment;

typedef struct {
    char* scenario_name;
    double* impact_vector;           /* impact on each constituent */
    int n_impacts;
    double recovery_time;
    double degradation;
    bool cascading_failure;
    bool system_survives;
} ResilienceScenario;

typedef struct {
    ResilienceScenario* scenarios;
    int n_scenarios;
    int cap;
    double resilience_index;
    double mean_recovery_time;
    double worst_case_degradation;
} ResilienceAnalysis;

/* API */
SoSAssessment* sos_assess_create(void);
void sos_assess_free(SoSAssessment* sa);
void sos_assess_readiness(SoSAssessment* sa, SystemOfSystems* sos);
void sos_assess_effectiveness(SoSAssessment* sa, SystemOfSystems* sos,
                               SoSArchitecture* arch);
void sos_assess_resilience(SoSAssessment* sa, SystemOfSystems* sos,
                            SoSArchitecture* arch);
void sos_assess_maturity(SoSAssessment* sa, int n_best_practices,
                          int n_implemented);
void sos_assess_compute_overall(SoSAssessment* sa);
void sos_assess_add_recommendation(SoSAssessment* sa, const char* rec);
void sos_assess_print(SoSAssessment* sa);

ResilienceScenario* sos_resilience_create(const char* name, int n_systems);
void sos_resilience_free(ResilienceScenario* rs);
void sos_resilience_set_impact(ResilienceScenario* rs, int sys_idx,
                                double impact);
bool sos_resilience_evaluate(ResilienceScenario* rs, SystemOfSystems* sos,
                              SoSArchitecture* arch);

ResilienceAnalysis* sos_resilience_analysis_create(void);
void sos_resilience_analysis_free(ResilienceAnalysis* ra);
void sos_resilience_analysis_add(ResilienceAnalysis* ra,
                                  ResilienceScenario* rs);
void sos_resilience_analysis_compute(ResilienceAnalysis* ra,
                                      SystemOfSystems* sos,
                                      SoSArchitecture* arch);
void sos_resilience_analysis_print(ResilienceAnalysis* ra);

double sos_effectiveness_formula(double readiness, double integration,
                                  double emergence, double satisfaction);
double sos_resilience_formula(double redundancy, double adaptability,
                               double recovery_speed);

#endif
