/* vvu_applications.c -- L7: Mars Climate Orbiter unit error + 737 MAX MCAS V&V.
 * Reference: NASA MCO Mishap Report (1999), NTSB 737 MAX Reports
 */
#include "vvu_core.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    char *mission_name, *failure_type;
    double magnitude_expected, magnitude_actual;
    double unit_conversion_error, detection_probability;
    int n_checkpoints, n_passed;
} VVFailure;

VVFailure* vvf_new(const char *name, const char *fail_type) {
    VVFailure *v = calloc(1, sizeof(VVFailure));
    v->mission_name = strdup(name);
    v->failure_type = strdup(fail_type);
    return v;
}
void vvf_free(VVFailure *v) {
    if (v) { free(v->mission_name); free(v->failure_type); free(v); }
}

int vvf_error_magnitude(VVFailure *v, double *ratio) {
    if (!v || !ratio) return -1;
    *ratio = v->magnitude_actual / fmax(v->magnitude_expected, 1e-6);
    return 0;
}

int vvf_detection_gap(VVFailure *v, double *gap) {
    if (!v || !gap) return -1;
    *gap = 1.0 - v->detection_probability;
    return 0;
}

int vvf_checkpoint_pass_rate(VVFailure *v, double *rate) {
    if (!v || !rate) return -1;
    *rate = v->n_checkpoints > 0 ?
        (double)v->n_passed / (double)v->n_checkpoints : 0.0;
    return 0;
}

int vvf_mars_lesson(VVFailure *v, double *unit_risk) {
    if (!v || !unit_risk) return -1;
    *unit_risk = v->unit_conversion_error *
                 (1.0 - v->detection_probability);
    return 0;
}

int vvf_mcas_lesson(VVFailure *v, double *single_point_risk) {
    if (!v || !single_point_risk) return -1;
    *single_point_risk = 1.0 - v->detection_probability;
    return 0;
}

void vvf_print(VVFailure *v) {
    double ratio, gap, rate, unit_risk, sp_risk;
    vvf_error_magnitude(v, &ratio);
    vvf_detection_gap(v, &gap);
    vvf_checkpoint_pass_rate(v, &rate);
    vvf_mars_lesson(v, &unit_risk);
    vvf_mcas_lesson(v, &sp_risk);
    printf("=== V&V Failure: %s ===\n", v->mission_name);
    printf("Type=%s ErrorRatio=%.1fx DetectionGap=%.2f\n",
           v->failure_type, ratio, gap);
    printf("Checkpoints=%d/%d UnitRisk=%.3f SinglePointRisk=%.3f\n",
           v->n_passed, v->n_checkpoints, unit_risk, sp_risk);
    printf("V&V Lesson: Independent verification %s\n",
           gap > 0.3 ? "CRITICAL" : "recommended");
}
