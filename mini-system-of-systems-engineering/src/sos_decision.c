#include "sos_types.h"
#include "sos_architecture.h"
#include "sos_governance.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* SoS Decision Support */

typedef struct {
    char* option_name;
    double cost;
    double benefit;
    double risk;
    double time_to_implement;
    int* affected_systems;
    int n_affected;
    double score;
} SoSDecisionOption;

typedef struct {
    SoSDecisionOption* options;
    int n_options;
    int cap;
    double* weights; /* cost, benefit, risk, time */
    int n_weights;
} SoSDecisionMatrix;

SoSDecisionOption* sos_decision_option_create(const char* name, double cost, double benefit, double risk) {
    SoSDecisionOption* opt = calloc(1, sizeof(SoSDecisionOption));
    if (!opt) return NULL;
    opt->option_name = name ? malloc(strlen(name) + 1) : NULL;
    if (name && opt->option_name) strcpy(opt->option_name, name);
    opt->cost = cost; opt->benefit = benefit; opt->risk = risk;
    opt->score = 0;
    return opt;
}

void sos_decision_option_free(SoSDecisionOption* opt) {
    if (!opt) return;
    free(opt->option_name); free(opt->affected_systems); free(opt);
}

SoSDecisionMatrix* sos_decision_matrix_create(void) {
    SoSDecisionMatrix* dm = calloc(1, sizeof(SoSDecisionMatrix));
    if (dm) {
        dm->weights = malloc(4 * sizeof(double));
        dm->n_weights = 4;
        if (dm->weights) {
            dm->weights[0] = -0.3; dm->weights[1] = 0.4;
            dm->weights[2] = -0.2; dm->weights[3] = -0.1;
        }
    }
    return dm;
}

void sos_decision_matrix_free(SoSDecisionMatrix* dm) {
    if (!dm) return;
    for (int i = 0; i < dm->n_options; i++) sos_decision_option_free(&dm->options[i]);
    free(dm->options); free(dm->weights); free(dm);
}

int sos_decision_add_option(SoSDecisionMatrix* dm, SoSDecisionOption* opt) {
    if (!dm || !opt) return -1;
    if (dm->n_options >= dm->cap) {
        int nc = (dm->cap == 0) ? 8 : dm->cap * 2;
        SoSDecisionOption* no = realloc(dm->options, (size_t)nc * sizeof(SoSDecisionOption));
        if (!no) return -1;
        dm->options = no; dm->cap = nc;
    }
    dm->options[dm->n_options] = *opt;
    return dm->n_options++;
}

void sos_decision_evaluate(SoSDecisionMatrix* dm) {
    if (!dm || dm->n_options == 0) return;
    double max_cost = 1, max_benefit = 1, max_risk = 1, max_time = 1;
    for (int i = 0; i < dm->n_options; i++) {
        if (dm->options[i].cost > max_cost) max_cost = dm->options[i].cost;
        if (dm->options[i].benefit > max_benefit) max_benefit = dm->options[i].benefit;
        if (dm->options[i].risk > max_risk) max_risk = dm->options[i].risk;
        if (dm->options[i].time_to_implement > max_time) max_time = dm->options[i].time_to_implement;
    }
    for (int i = 0; i < dm->n_options; i++) {
        SoSDecisionOption* o = &dm->options[i];
        double nc = (max_cost > 0) ? o->cost / max_cost : 0;
        double nb = (max_benefit > 0) ? o->benefit / max_benefit : 0;
        double nr = (max_risk > 0) ? o->risk / max_risk : 0;
        double nt = (max_time > 0) ? o->time_to_implement / max_time : 0;
        o->score = dm->weights[0]*nc + dm->weights[1]*nb + dm->weights[2]*nr + dm->weights[3]*nt;
    }
}

int sos_decision_best_option(const SoSDecisionMatrix* dm) {
    if (!dm || dm->n_options == 0) return -1;
    int best = 0;
    for (int i = 1; i < dm->n_options; i++)
        if (dm->options[i].score > dm->options[best].score) best = i;
    return best;
}

void sos_decision_print(const SoSDecisionMatrix* dm) {
    if (!dm) return;
    printf("=== SoS Decision Matrix ===\n");
    for (int i = 0; i < dm->n_options; i++)
        printf("  [%d] %s: score=%.3f (cost=%.1f benefit=%.1f risk=%.2f)\n",
               i, dm->options[i].option_name, dm->options[i].score,
               dm->options[i].cost, dm->options[i].benefit, dm->options[i].risk);
    int best = sos_decision_best_option(dm);
    if (best >= 0) printf("Best option: %s\n", dm->options[best].option_name);
}
/* Multi-criteria decision analysis extensions */
void sos_decision_set_weight(SoSDecisionMatrix* dm, int idx, double w) {
    if (dm && idx >= 0 && idx < dm->n_weights) dm->weights[idx] = w;
}

double sos_decision_sensitivity(SoSDecisionMatrix* dm, int option_idx, int weight_idx, double delta) {
    if (!dm || option_idx < 0 || option_idx >= dm->n_options) return 0.0;
    double orig = dm->weights[weight_idx];
    dm->weights[weight_idx] += delta;
    sos_decision_evaluate(dm);
    double new_score = dm->options[option_idx].score;
    dm->weights[weight_idx] = orig;
    sos_decision_evaluate(dm);
    return (new_score - dm->options[option_idx].score) / (delta + 1e-12);
}

int sos_decision_count_feasible(const SoSDecisionMatrix* dm, double max_cost, double max_risk) {
    if (!dm) return 0;
    int n = 0;
    for (int i = 0; i < dm->n_options; i++)
        if (dm->options[i].cost <= max_cost && dm->options[i].risk <= max_risk) n++;
    return n;
}

void sos_decision_rank_options(SoSDecisionMatrix* dm, int* ranking) {
    if (!dm || !ranking) return;
    for (int i = 0; i < dm->n_options; i++) {
        ranking[i] = 1;
        for (int j = 0; j < dm->n_options; j++)
            if (dm->options[j].score > dm->options[i].score) ranking[i]++;
    }
}

double sos_decision_robustness(const SoSDecisionMatrix* dm, int option_idx) {
    if (!dm || option_idx < 0) return 0.0;
    double max_sens = 0;
    for (int w = 0; w < dm->n_weights; w++) {
        double s = fabs(sos_decision_sensitivity((SoSDecisionMatrix*)dm, option_idx, w, 0.1));
        if (s > max_sens) max_sens = s;
    }
    return 1.0 / (1.0 + max_sens);
}

/* Additional decision utilities */
SoSDecisionOption* sos_decision_clone_option(const SoSDecisionOption* src) {
    if (!src) return NULL;
    SoSDecisionOption* dst = sos_decision_option_create(src->option_name, src->cost, src->benefit, src->risk);
    if (dst) { dst->time_to_implement = src->time_to_implement; dst->score = src->score; }
    return dst;
}

void sos_decision_option_add_affected(SoSDecisionOption* opt, int sys_id) {
    if (!opt) return;
    int nc = opt->n_affected + 1;
    int* na = realloc(opt->affected_systems, (size_t)nc * sizeof(int));
    if (na) { opt->affected_systems = na; opt->affected_systems[opt->n_affected] = sys_id; opt->n_affected++; }
}

bool sos_decision_option_affects(const SoSDecisionOption* opt, int sys_id) {
    if (!opt) return false;
    for (int i = 0; i < opt->n_affected; i++)
        if (opt->affected_systems[i] == sys_id) return true;
    return false;
}

/* Final decision utilities */
double sos_decision_expected_value(const SoSDecisionOption* opt, double success_prob) {
    if (!opt || success_prob < 0 || success_prob > 1) return 0.0;
    return success_prob * opt->benefit - (1.0 - success_prob) * opt->cost - opt->risk * opt->cost;
}

void sos_decision_option_set_time(SoSDecisionOption* opt, double time) {
    if (opt && time >= 0) opt->time_to_implement = time;
}
