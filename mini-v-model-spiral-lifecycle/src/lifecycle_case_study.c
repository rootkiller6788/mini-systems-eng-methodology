/* lifecycle_app2.c -- L7-2: IEC 62304 medical device software lifecycle Class C (pacemaker).
 * Reference: IEC 62304:2006/Amd1:2015, FDA 21 CFR Part 820
 */
#include "lifecycle_core.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    char *device_name, *device_class;
    int n_requirements, n_hazards, n_mitigations;
    double risk_score, verification_coverage;
    int dev_plan_days, vv_days, total_days;
} MedicalDevice;

MedicalDevice* meddev_new(const char *name, const char *cls) {
    MedicalDevice *m = calloc(1, sizeof(MedicalDevice));
    m->device_name = strdup(name);
    m->device_class = strdup(cls);
    m->risk_score = 0.5;
    return m;
}
void meddev_free(MedicalDevice *m) {
    if (m) { free(m->device_name); free(m->device_class); free(m); }
}

int meddev_safety_integrity(MedicalDevice *m, double *sil) {
    if (!m || !sil) return -1;
    double mitigated = m->n_hazards > 0 ?
        (double)m->n_mitigations / (double)m->n_hazards : 0.0;
    *sil = mitigated * (1.0 - m->risk_score);
    return 0;
}

int meddev_vv_effort(MedicalDevice *m, double *effort_ratio) {
    if (!m || !effort_ratio) return -1;
    *effort_ratio = m->total_days > 0 ?
        (double)m->vv_days / (double)m->total_days : 0.0;
    return 0;
}

int meddev_iec62304_compliance(MedicalDevice *m, double *compliance) {
    if (!m || !compliance) return -1;
    double req_cov = m->n_requirements > 0 ?
        m->verification_coverage / (double)m->n_requirements : 0.0;
    double vv_pct = meddev_vv_effort(m, &(double){0});
    *compliance = 0.4 * req_cov + 0.3 * (1.0 - m->risk_score) + 0.3 * vv_pct;
    return 0;
}

void meddev_print(MedicalDevice *m) {
    double sil, vv, comp;
    meddev_safety_integrity(m, &sil);
    meddev_vv_effort(m, &vv);
    meddev_iec62304_compliance(m, &comp);
    printf("=== IEC 62304 Class C: %s ===\n", m->device_name);
    printf("Hazards=%d Mitigations=%d Risk=%.2f\n",
           m->n_hazards, m->n_mitigations, m->risk_score);
    printf("SIL=%.3f VV_Effort=%.1f%% Compliance=%.2f\n",
           sil, vv*100, comp);
}
