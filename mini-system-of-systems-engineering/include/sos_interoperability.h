#ifndef SOS_INTEROPERABILITY_H
#define SOS_INTEROPERABILITY_H
#include "sos_architecture.h"
#include "sos_types.h"
#include <stdbool.h>

typedef struct {
    InteroperabilityLevel current_level, target_level;
    double gap_score, cost_estimate, time_estimate;
    int n_blockers; char** blocker_descriptions;
} InteroperabilityGap;

typedef struct {
    InteroperabilityGap* gaps; int n_gaps, cap;
    SoSArchitecture* arch;
    double overall_interop_maturity, weakest_link_level, bottleneck_severity;
    int bottleneck_connection;
} InteroperabilityAssessment;

typedef struct {
    char* standard_name, *domain;
    InteroperabilityLevel min_level_required;
    double adoption_rate; bool is_mandatory; int year_introduced;
} InterfaceStandard;

typedef struct {
    InterfaceStandard* standards; int n_standards, cap;
    double compliance_score;
} StandardsProfile;

InteroperabilityGap* sos_interop_gap_create(InteroperabilityLevel cur, InteroperabilityLevel target);
void sos_interop_gap_free(InteroperabilityGap* gap);
void sos_interop_gap_add_blocker(InteroperabilityGap* gap, const char* desc);
double sos_interop_gap_score(const InteroperabilityGap* gap);
void sos_interop_gap_print(const InteroperabilityGap* gap);

InteroperabilityAssessment* sos_interop_assess_create(SoSArchitecture* arch);
void sos_interop_assess_free(InteroperabilityAssessment* ia);
void sos_interop_assess_analyze(InteroperabilityAssessment* ia);
double sos_interop_maturity_index(const InteroperabilityAssessment* ia);
int sos_interop_find_bottleneck(const InteroperabilityAssessment* ia);
void sos_interop_recommend_upgrades(const InteroperabilityAssessment* ia, InteroperabilityLevel* targets, int n);
void sos_interop_assess_print(const InteroperabilityAssessment* ia);

StandardsProfile* sos_standards_create(void);
void sos_standards_free(StandardsProfile* sp);
void sos_standards_add(StandardsProfile* sp, const char* name, const char* domain, InteroperabilityLevel level, bool mandatory);
double sos_standards_compliance(const StandardsProfile* sp, const InteroperabilityAssessment* ia);
void sos_standards_print(const StandardsProfile* sp);

#endif