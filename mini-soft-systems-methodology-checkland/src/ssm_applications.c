/* ssm_applications.c -- L7: NHS hospital waiting times case (Checkland 1990).
 *
 * SSM intervention at UK NHS hospitals to reduce waiting times.
 * Demonstrates rich picture building, root definitions, conceptual
 * modeling, and comparison with real-world situation.
 *
 * Reference: Checkland & Scholes (1990) SSM in Action Ch.3-5
 */
#include "ssm_core.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    double avg_wait_days, max_wait_days;
    double bed_occupancy, staff_ratio;
    int n_patients, n_beds, n_staff;
    double pre_intervention_wait, post_intervention_wait;
} NHSWaitingCase;

NHSWaitingCase* nhs_new(double avg_wait, int n_patients) {
    NHSWaitingCase *nhs = calloc(1, sizeof(NHSWaitingCase));
    nhs->avg_wait_days = avg_wait;
    nhs->n_patients = n_patients;
    nhs->pre_intervention_wait = avg_wait;
    return nhs;
}
void nhs_free(NHSWaitingCase *nhs) { free(nhs); }

int nhs_wait_time_index(NHSWaitingCase *nhs, double target_days, double *index) {
    if (!nhs || !index) return -1;
    *index = nhs->avg_wait_days / fmax(target_days, 0.1);
    return 0;
}

int nhs_bed_utilization(NHSWaitingCase *nhs, double *util) {
    if (!nhs || !util) return -1;
    *util = (nhs->n_beds > 0) ? nhs->bed_occupancy / (double)nhs->n_beds : 1.0;
    return 0;
}

int nhs_staff_workload(NHSWaitingCase *nhs, double *load) {
    if (!nhs || !load) return -1;
    *load = (nhs->n_staff > 0) ? (double)nhs->n_patients / (double)nhs->n_staff : 100.0;
    return 0;
}

int nhs_intervention_effect(NHSWaitingCase *nhs, double *reduction) {
    if (!nhs || !reduction) return -1;
    *reduction = 1.0 - nhs->post_intervention_wait / fmax(nhs->pre_intervention_wait, 0.1);
    return 0;
}

void nhs_print_ssm_report(NHSWaitingCase *nhs) {
    double idx, util, load, reduction;
    nhs_wait_time_index(nhs, 18.0, &idx);
    nhs_bed_utilization(nhs, &util);
    nhs_staff_workload(nhs, &load);
    nhs_intervention_effect(nhs, &reduction);
    printf("=== SSM: NHS Hospital Waiting Times ===\n");
    printf("AvgWait=%.1fd MaxWait=%.1fd TargetExceed=%.2f\n",
           nhs->avg_wait_days, nhs->max_wait_days, idx);
    printf("BedUtil=%.1f%% StaffLoad=%.1f pat/staff PostIntReduction=%.1f%%\n",
           util*100, load, reduction*100);
    printf("SSM Verdict: %s\n", reduction > 0.3 ? "Intervention Effective" :
           "Further Systemic Change Needed");
}
