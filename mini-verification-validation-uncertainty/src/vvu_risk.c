#include "vvu_risk.h"
#include "vvu_core.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

VVU_RiskRegister* vvu_risk_register_create(void) {
    VVU_RiskRegister* rr = calloc(1, sizeof(VVU_RiskRegister));
    if (!rr) return NULL;
    rr->risk_capacity = 64;
    rr->risks = calloc(64, sizeof(VVU_RiskItem));
    rr->fmea_capacity = 32;
    rr->fmea_items = calloc(32, sizeof(VVU_FMEAItem));
    return rr;
}

void vvu_risk_register_free(VVU_RiskRegister* rr) {
    if (!rr) return;
    for (int i = 0; i < rr->n_risks; i++) {
        free(rr->risks[i].id); free(rr->risks[i].description);
        free(rr->risks[i].mitigation);
    }
    for (int i = 0; i < rr->n_fmea; i++) {
        free(rr->fmea_items[i].failure_mode);
        free(rr->fmea_items[i].effect);
        free(rr->fmea_items[i].cause);
        free(rr->fmea_items[i].recommended_action);
    }
    free(rr->risks); free(rr->fmea_items); free(rr);
}

int vvu_risk_add(VVU_RiskRegister* rr, const char* id, const char* desc,
                  VVU_Likelihood like, VVU_Consequence conseq) {
    if (!rr || !id || rr->n_risks >= rr->risk_capacity) return -1;
    int idx = rr->n_risks;
    VVU_RiskItem* r = &rr->risks[idx];
    memset(r, 0, sizeof(VVU_RiskItem));
    r->id = strdup(id); r->description = strdup(desc ? desc : "");
    r->likelihood = like; r->consequence = conseq;
    r->risk_level = vvu_risk_matrix_level(like, conseq);
    r->probability = (double)(like + 1) / 5.0;
    r->impact_score = (double)(conseq + 1) / 5.0;
    r->is_acceptable = (r->risk_level <= VVU_RISK_LOW);
    rr->n_risks++;
    return idx;
}

void vvu_risk_set_mitigation(VVU_RiskRegister* rr, int idx,
    const char* mitigation, double residual_prob) {
    if (!rr || idx < 0 || idx >= rr->n_risks) return;
    free(rr->risks[idx].mitigation);
    rr->risks[idx].mitigation = strdup(mitigation ? mitigation : "");
    rr->risks[idx].residual_probability = residual_prob;
    rr->risks[idx].residual_impact = rr->risks[idx].impact_score * 0.5;
    rr->risks[idx].is_acceptable = (residual_prob < 0.2);
}

int vvu_fmea_add(VVU_RiskRegister* rr, const char* mode, const char* effect,
                  const char* cause, int sev, int occ, int det) {
    if (!rr || !mode || rr->n_fmea >= rr->fmea_capacity) return -1;
    int idx = rr->n_fmea;
    VVU_FMEAItem* f = &rr->fmea_items[idx];
    memset(f, 0, sizeof(VVU_FMEAItem));
    f->failure_mode = strdup(mode); f->effect = strdup(effect ? effect : "");
    f->cause = strdup(cause ? cause : "");
    f->severity = sev; f->occurrence = occ; f->detection = det;
    f->rpn = sev * occ * det;
    rr->n_fmea++;
    return idx;
}

void vvu_risk_compute_metrics(VVU_RiskRegister* rr) {
    if (!rr) return;
    double total = 0.0, residual = 0.0;
    for (int i = 0; i < rr->n_risks; i++) {
        VVU_RiskItem* r = &rr->risks[i];
        double expo = r->probability * r->impact_score;
        total += expo;
        residual += r->residual_probability * r->residual_impact;
    }
    rr->total_risk_exposure = total;
    rr->residual_risk_exposure = residual;
    rr->risk_reduction_ratio = (total > 1e-10) ? (total - residual) / total : 0.0;
}

VVU_RiskLevel vvu_risk_matrix_level(VVU_Likelihood l, VVU_Consequence c) {
    int score = (int)l + (int)c;
    if (score >= 7) return VVU_RISK_CATASTROPHIC;
    if (score >= 5) return VVU_RISK_CRITICAL;
    if (score >= 3) return VVU_RISK_HIGH;
    if (score >= 1) return VVU_RISK_MEDIUM;
    return VVU_RISK_LOW;
}

double vvu_risk_exposure(const VVU_RiskRegister* rr) {
    return rr ? rr->total_risk_exposure : 0.0;
}

int vvu_risk_count_by_level(const VVU_RiskRegister* rr, VVU_RiskLevel level) {
    if (!rr) return 0;
    int count = 0;
    for (int i = 0; i < rr->n_risks; i++)
        if (rr->risks[i].risk_level == level) count++;
    return count;
}

void vvu_risk_print(const VVU_RiskRegister* rr) {
    if (!rr) { printf("RiskRegister: NULL\n"); return; }
    printf("=== Risk Register (%d risks, %d FMEA) ===\n", rr->n_risks, rr->n_fmea);
    printf("Total exposure: %.4f, Residual: %.4f, Reduction: %.1f%%\n",
           rr->total_risk_exposure, rr->residual_risk_exposure,
           rr->risk_reduction_ratio * 100);
    printf("%-10s %-6s %-6s %-8s %-8s %s\n",
           "ID", "Prob", "Impact", "Level", "Accept?", "Description");
    for (int i = 0; i < rr->n_risks; i++) {
        VVU_RiskItem* r = &rr->risks[i];
        const char* lvl = "LOW";
        switch (r->risk_level) { case VVU_RISK_CATASTROPHIC: lvl="CATA"; break;
            case VVU_RISK_CRITICAL: lvl="CRIT"; break;
            case VVU_RISK_HIGH: lvl="HIGH"; break;
            case VVU_RISK_MEDIUM: lvl="MED"; break; }
        printf("%-10s %5.2f %6.2f %-8s %-8s %s\n",
               r->id, r->probability, r->impact_score, lvl,
               r->is_acceptable ? "YES" : "NO", r->description);
    }
}

void vvu_risk_matrix_print(const VVU_RiskRegister* rr) {
    if (!rr) return;
    printf("=== Risk Matrix (5x5 Heat Map) ===\n");
    int matrix[5][5]; memset(matrix, 0, sizeof(matrix));
    for (int i = 0; i < rr->n_risks; i++) {
        VVU_RiskItem* r = &rr->risks[i];
        int li = (int)r->likelihood; if (li >= 5) li = 4;
        int ci = (int)r->consequence; if (ci >= 5) ci = 4;
        matrix[4-ci][li]++;
    }
    const char* clabels[] = {"CATA", "MAJOR", "MOD", "MINOR", "NEGL"};
    const char* llabels[] = {"RARE","UNLIKE","POSS","LIKELY","CERTAIN"};
    printf("C\L       ");
    for (int j = 0; j < 5; j++) printf("%-8s", llabels[j]);
    printf("\n");
    for (int i = 0; i < 5; i++) {
        printf("%-10s", clabels[i]);
        for (int j = 0; j < 5; j++) printf("  %2d    ", matrix[i][j]);
        printf("\n");
    }
}

void vvu_fmea_print(const VVU_RiskRegister* rr) {
    if (!rr || rr->n_fmea == 0) { printf("FMEA: empty\n"); return; }
    printf("=== FMEA (%d items) ===\n", rr->n_fmea);
    printf("%-15s %-20s %-15s %3s %3s %3s %5s\n",
           "Mode", "Effect", "Cause", "S", "O", "D", "RPN");
    for (int i = 0; i < rr->n_fmea; i++) {
        VVU_FMEAItem* f = &rr->fmea_items[i];
        printf("%-15s %-20s %-15s %3d %3d %3d %5d\n",
               f->failure_mode, f->effect, f->cause,
               f->severity, f->occurrence, f->detection, f->rpn);
    }
}

