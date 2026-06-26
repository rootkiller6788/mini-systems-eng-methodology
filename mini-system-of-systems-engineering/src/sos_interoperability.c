#include "sos_interoperability.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

InteroperabilityGap* sos_interop_gap_create(InteroperabilityLevel cur, InteroperabilityLevel target) {
    InteroperabilityGap* gap = calloc(1, sizeof(InteroperabilityGap));
    if (!gap) return NULL;
    gap->current_level = cur; gap->target_level = target;
    gap->gap_score = (double)(target - cur) / (double)INTEROP_ENTERPRISE;
    gap->cost_estimate = gap->gap_score * 0.5;
    gap->time_estimate = gap->gap_score * 0.3;
    return gap;
}

void sos_interop_gap_free(InteroperabilityGap* gap) {
    if (!gap) return;
    for (int i = 0; i < gap->n_blockers; i++) free(gap->blocker_descriptions[i]);
    free(gap->blocker_descriptions); free(gap);
}

void sos_interop_gap_add_blocker(InteroperabilityGap* gap, const char* desc) {
    if (!gap || !desc) return;
    int nc = gap->n_blockers + 1;
    char** nb = realloc(gap->blocker_descriptions, (size_t)nc * sizeof(char*));
    if (!nb) return;
    gap->blocker_descriptions = nb;
    gap->blocker_descriptions[gap->n_blockers] = malloc(strlen(desc) + 1);
    if (gap->blocker_descriptions[gap->n_blockers])
        strcpy(gap->blocker_descriptions[gap->n_blockers], desc);
    gap->n_blockers++;
}

double sos_interop_gap_score(const InteroperabilityGap* gap) { return gap ? gap->gap_score : 0.0; }

void sos_interop_gap_print(const InteroperabilityGap* gap) {
    if (!gap) return;
    printf("Interop Gap: L%d->L%d score=%.2f\n", gap->current_level, gap->target_level, gap->gap_score);
}

InteroperabilityAssessment* sos_interop_assess_create(SoSArchitecture* arch) {
    InteroperabilityAssessment* ia = calloc(1, sizeof(InteroperabilityAssessment));
    if (ia) ia->arch = arch;
    return ia;
}

void sos_interop_assess_free(InteroperabilityAssessment* ia) {
    if (!ia) return;
    for (int i = 0; i < ia->n_gaps; i++) sos_interop_gap_free(&ia->gaps[i]);
    free(ia->gaps); free(ia);
}

void sos_interop_assess_analyze(InteroperabilityAssessment* ia) {
    if (!ia || !ia->arch) return;
    ia->overall_interop_maturity = sos_arch_interoperability_score(ia->arch);
    ia->weakest_link_level = INTEROP_ENTERPRISE;
    for (int i = 0; i < ia->arch->n_connections; i++) {
        if (ia->arch->connections[i].is_active) {
            InteroperabilityLevel lv = ia->arch->connections[i].level;
            if (lv < ia->weakest_link_level) ia->weakest_link_level = lv;
            double s = 1.0 - (double)lv / (double)INTEROP_ENTERPRISE;
            if (s > ia->bottleneck_severity) {
                ia->bottleneck_severity = s; ia->bottleneck_connection = i;
            }
        }
    }
}

double sos_interop_maturity_index(const InteroperabilityAssessment* ia) {
    return ia ? ia->overall_interop_maturity : 0.0;
}

int sos_interop_find_bottleneck(const InteroperabilityAssessment* ia) {
    return ia ? ia->bottleneck_connection : -1;
}

void sos_interop_assess_print(const InteroperabilityAssessment* ia) {
    if (!ia) return;
    printf("=== Interop Assessment ===\n");
    printf("Maturity: %.2f  Bottleneck: conn #%d (%.2f)\n",
           ia->overall_interop_maturity, ia->bottleneck_connection, ia->bottleneck_severity);
}

StandardsProfile* sos_standards_create(void) { return calloc(1, sizeof(StandardsProfile)); }

void sos_standards_free(StandardsProfile* sp) {
    if (!sp) return;
    for (int i = 0; i < sp->n_standards; i++) { free(sp->standards[i].standard_name); free(sp->standards[i].domain); }
    free(sp->standards); free(sp);
}

void sos_standards_add(StandardsProfile* sp, const char* name, const char* domain, InteroperabilityLevel level, bool mandatory) {
    if (!sp || !name) return;
    if (sp->n_standards >= sp->cap) {
        int nc = (sp->cap == 0) ? 8 : sp->cap * 2;
        InterfaceStandard* ns = realloc(sp->standards, (size_t)nc * sizeof(InterfaceStandard));
        if (!ns) return;
        sp->standards = ns; sp->cap = nc;
    }
    InterfaceStandard* s = &sp->standards[sp->n_standards];
    s->standard_name = malloc(strlen(name) + 1);
    if (s->standard_name) strcpy(s->standard_name, name);
    s->domain = domain ? malloc(strlen(domain) + 1) : NULL;
    if (domain && s->domain) strcpy(s->domain, domain);
    s->min_level_required = level; s->is_mandatory = mandatory; s->adoption_rate = 0.5;
    sp->n_standards++;
}

double sos_standards_compliance(const StandardsProfile* sp, const InteroperabilityAssessment* ia) {
    return (sp && ia && sp->n_standards > 0) ? 0.5 * ia->overall_interop_maturity : 0.0;
}

void sos_standards_print(const StandardsProfile* sp) {
    if (!sp) return;
    printf("=== Standards: %d  Compliance: %.2f ===\n", sp->n_standards, sp->compliance_score);
}