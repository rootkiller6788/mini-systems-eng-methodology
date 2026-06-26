#include "hsm_logic.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

HSMLogicActivity* hsm_logic_create(HSMLogicStep step) {
    HSMLogicActivity* la = calloc(1, sizeof(HSMLogicActivity));
    if (!la) return NULL;
    la->step = step; la->alt_capacity = 8;
    la->alternatives = malloc(8 * sizeof(HSMAlternative));
    la->best_alternative = -1;
    return la;
}
void hsm_logic_free(HSMLogicActivity* la) {
    if (!la) return;
    for (int i = 0; i < la->n_alternatives; i++) free(la->alternatives[i].description);
    free(la->alternatives);
    for (int i = 0; i < la->n_criteria; i++) free(la->criteria_names[i]);
    free(la->criteria_names); free(la->criteria_weights);
    free(la);
}
int hsm_logic_add_alternative(HSMLogicActivity* la, const char* desc, double cost, double benefit, double risk) {
    if (!la || !desc) return -1;
    if (la->n_alternatives >= la->alt_capacity) { la->alt_capacity *= 2; la->alternatives = realloc(la->alternatives, la->alt_capacity * sizeof(HSMAlternative)); }
    int idx = la->n_alternatives;
    HSMAlternative* alt = &la->alternatives[idx];
    memset(alt, 0, sizeof(HSMAlternative));
    alt->description = strdup(desc); alt->cost_estimate = cost; alt->benefit_estimate = benefit; alt->risk_level = risk;
    la->n_alternatives++; return idx;
}
void hsm_logic_add_criterion(HSMLogicActivity* la, const char* name, double weight) {
    if (!la || !name) return;
    la->n_criteria++;
    la->criteria_names = realloc(la->criteria_names, la->n_criteria * sizeof(char*));
    la->criteria_weights = realloc(la->criteria_weights, la->n_criteria * sizeof(double));
    la->criteria_names[la->n_criteria - 1] = strdup(name);
    la->criteria_weights[la->n_criteria - 1] = weight;
}
void hsm_logic_set_score(HSMLogicActivity* la, int alt_idx, int crit_idx, double score) {
    if (la && alt_idx >= 0 && alt_idx < la->n_alternatives && crit_idx >= 0 && crit_idx < la->n_criteria)
        la->alternatives[alt_idx].score += score * la->criteria_weights[crit_idx];
}
int hsm_logic_select_best(HSMLogicActivity* la) {
    if (!la || la->n_alternatives == 0) return -1;
    int best = 0;
    for (int i = 1; i < la->n_alternatives; i++)
        if (la->alternatives[i].score > la->alternatives[best].score) best = i;
    for (int i = 0; i < la->n_alternatives; i++) la->alternatives[i].is_selected = (i == best);
    la->best_alternative = best;
    return best;
}
void hsm_logic_print_alternatives(HSMLogicActivity* la) {
    if (!la) return;
    printf("Logic Activity: %s (%d alternatives, %d criteria)\n", hsm_step_name(la->step), la->n_alternatives, la->n_criteria);
    for (int i = 0; i < la->n_alternatives; i++) {
        HSMAlternative* a = &la->alternatives[i];
        printf("  [%d] %s: score=%.3f cost=%.1f benefit=%.1f risk=%.2f %s\n",
               i, a->description, a->score, a->cost_estimate, a->benefit_estimate, a->risk_level,
               a->is_selected ? "<< SELECTED" : "");
    }
}
HSMDecisionMatrix* hsm_dm_create(int n_alts, int n_criteria, double* weights) {
    HSMDecisionMatrix* dm = calloc(1, sizeof(HSMDecisionMatrix));
    if (!dm) return NULL;
    dm->n_alternatives = n_alts; dm->n_criteria = n_criteria;
    dm->weights = malloc(n_criteria * sizeof(double));
    memcpy(dm->weights, weights, n_criteria * sizeof(double));
    dm->scores = malloc(n_alts * sizeof(double*));
    dm->total_scores = calloc(n_alts, sizeof(double));
    dm->ranking = malloc(n_alts * sizeof(int));
    for (int i = 0; i < n_alts; i++) { dm->scores[i] = calloc(n_criteria, sizeof(double)); dm->ranking[i] = i; }
    return dm;
}
void hsm_dm_free(HSMDecisionMatrix* dm) { if(dm){for(int i=0;i<dm->n_alternatives;i++)free(dm->scores[i]);free(dm->scores);free(dm->weights);free(dm->total_scores);free(dm->ranking);free(dm);} }
void hsm_dm_set_score(HSMDecisionMatrix* dm, int alt, int crit, double s) { if(dm&&alt>=0&&alt<dm->n_alternatives&&crit>=0&&crit<dm->n_criteria)dm->scores[alt][crit]=s; }
void hsm_dm_compute_ranking(HSMDecisionMatrix* dm) {
    if (!dm) return;
    for (int i = 0; i < dm->n_alternatives; i++) { dm->total_scores[i] = 0.0; for (int j = 0; j < dm->n_criteria; j++) dm->total_scores[i] += dm->scores[i][j] * dm->weights[j]; }
    for (int i = 0; i < dm->n_alternatives; i++) dm->ranking[i] = i;
    for (int i = 0; i < dm->n_alternatives - 1; i++)
        for (int j = i + 1; j < dm->n_alternatives; j++)
            if (dm->total_scores[dm->ranking[j]] > dm->total_scores[dm->ranking[i]]) { int t = dm->ranking[i]; dm->ranking[i] = dm->ranking[j]; dm->ranking[j] = t; }
}
int hsm_dm_best(HSMDecisionMatrix* dm) { if(!dm||dm->n_alternatives==0)return -1; hsm_dm_compute_ranking(dm); return dm->ranking[0]; }
/* Saaty Consistency Ratio for Decision Matrix. Computes CR = CI/RI where CI = (lambda_max - n)/(n-1). CR < 0.10 => acceptable. */
double hsm_dm_consistency_ratio(HSMDecisionMatrix* dm) {
    if (!dm || dm->n_criteria < 2) return 0.0;
    static const double ri[] = {0.0,0.0,0.58,0.90,1.12,1.24,1.32,1.41,1.45,1.49};
    int n = dm->n_criteria;
    double lambda_max = 0.0;
    for (int i = 0; i < dm->n_alternatives && i < 10; i++) {
        double weighted_sum = 0.0;
        for (int j = 0; j < n; j++)
            weighted_sum += dm->scores[i][j] * dm->weights[j];
        double wi = dm->total_scores[i];
        if (wi > 1e-12) lambda_max += weighted_sum / wi;
    }
    int m = (dm->n_alternatives < 10) ? dm->n_alternatives : 10;
    if (m < 2) return 0.0;
    lambda_max /= (double)m;
    double ci = (lambda_max - (double)n) / (double)(n - 1);
    double r = (n <= 9) ? ri[n] : 1.49;
    return (r > 0.0) ? ci / r : 0.0;
}
void hsm_dm_print(HSMDecisionMatrix* dm) {
    if (!dm) return;
    printf("Decision Matrix (%d x %d)\n", dm->n_alternatives, dm->n_criteria);
    printf("Weights: "); for(int j=0;j<dm->n_criteria;j++) printf("%.2f ",dm->weights[j]); printf("\n");
    for(int i=0;i<dm->n_alternatives;i++) { printf(" Alt%d: score=%.3f [%d]",i,dm->total_scores[i],dm->ranking[i]); for(int j=0;j<dm->n_criteria;j++) printf(" %.2f",dm->scores[i][j]); printf("\n"); }
}
double* hsm_ahp_weights(double** pairwise, int n) {
    double* w = calloc(n, sizeof(double));
    for (int i = 0; i < n; i++) { double prod = 1.0; for (int j = 0; j < n; j++) prod *= pairwise[i][j]; w[i] = pow(prod, 1.0/n); }
    double sum = 0.0; for (int i = 0; i < n; i++) sum += w[i];
    if (sum > 0) for (int i = 0; i < n; i++) w[i] /= sum;
    return w;
}
/* Saaty AHP Consistency Ratio (Saaty, 1980). CR = CI/RI where CI = (lambda_max - n)/(n-1). */
	double hsm_ahp_consistency(double** pairwise, double* weights, int n) {
	    if (!pairwise || !weights || n < 2) return 0.0;
	    static const double ri[] = {0.0,0.0,0.58,0.90,1.12,1.24,1.32,1.41,1.45,1.49,1.51};
	    double lambda_max = 0.0;
	    for (int i = 0; i < n; i++) {
	        double row_sum = 0.0;
	        for (int j = 0; j < n; j++)
	            row_sum += pairwise[i][j] * weights[j];
	        if (weights[i] > 1e-12)
	            lambda_max += row_sum / weights[i];
	    }
	    lambda_max /= (double)n;
	    double ci = (lambda_max - (double)n) / (double)(n - 1);
	    double r = (n <= 10) ? ri[n] : 1.49;
	    return (r > 0.0) ? ci / r : 0.0;
	}
