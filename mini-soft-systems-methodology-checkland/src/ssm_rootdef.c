#include "ssm_core.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

RootDefinition* ssm_rootdef_create(const char* name, RootDefType type) {
    RootDefinition* rd = (RootDefinition*)calloc(1, sizeof(RootDefinition));
    if (!rd) return NULL;
    rd->name = strdup(name); rd->type = type;
    rd->coherence_score = 0.5; rd->relevance_rank = 5;
    return rd;
}
void ssm_rootdef_free(RootDefinition* rd) {
    if (!rd) return;
    free(rd->name); free(rd->root_definition);
    free(rd->what_to_do); free(rd->how_to_do); free(rd->why_do_it);
    free(rd);
}
/* Deep clone a root definition, including all CATWOE and PQR fields.
 * Used for generating variant root definitions in Stage 3. */
RootDefinition* ssm_rootdef_clone(const RootDefinition* src) {
    if (!src) return NULL;
    RootDefinition* rd = ssm_rootdef_create(src->name, src->type);
    if (!rd) return NULL;
    rd->root_definition = src->root_definition ? strdup(src->root_definition) : NULL;
    rd->what_to_do = src->what_to_do ? strdup(src->what_to_do) : NULL;
    rd->how_to_do = src->how_to_do ? strdup(src->how_to_do) : NULL;
    rd->why_do_it = src->why_do_it ? strdup(src->why_do_it) : NULL;
    rd->efficacy = src->efficacy;
    rd->efficiency = src->efficiency;
    rd->effectiveness = src->effectiveness;
    rd->ethicality = src->ethicality;
    rd->elegance = src->elegance;
    rd->coherence_score = src->coherence_score;
    rd->relevance_rank = src->relevance_rank;
    if (src->catwoe) {
        rd->catwoe = (CATWOE*)calloc(1, sizeof(CATWOE));
        if (rd->catwoe) {
            rd->catwoe->customers = src->catwoe->customers ? strdup(src->catwoe->customers) : NULL;
            rd->catwoe->actors = src->catwoe->actors ? strdup(src->catwoe->actors) : NULL;
            rd->catwoe->transformation_input = src->catwoe->transformation_input ? strdup(src->catwoe->transformation_input) : NULL;
            rd->catwoe->transformation_output = src->catwoe->transformation_output ? strdup(src->catwoe->transformation_output) : NULL;
            rd->catwoe->weltanschauung = src->catwoe->weltanschauung ? strdup(src->catwoe->weltanschauung) : NULL;
            rd->catwoe->owners = src->catwoe->owners ? strdup(src->catwoe->owners) : NULL;
            rd->catwoe->transform_type = src->catwoe->transform_type;
        }
    }
    return rd;
}
void ssm_rootdef_set_pqr(RootDefinition* rd, const char* what,
    const char* how, const char* why) {
    if (!rd) return;
    free(rd->what_to_do); rd->what_to_do = what ? strdup(what) : NULL;
    free(rd->how_to_do); rd->how_to_do = how ? strdup(how) : NULL;
    free(rd->why_do_it); rd->why_do_it = why ? strdup(why) : NULL;
    ssm_rootdef_synthesize(rd);
}
void ssm_rootdef_synthesize(RootDefinition* rd) {
    if (!rd) return;
    free(rd->root_definition);
    char buf[1024];
    snprintf(buf, sizeof(buf), "A system to %s by %s in order to achieve %s",
        rd->what_to_do ? rd->what_to_do : "[P]",
        rd->how_to_do ? rd->how_to_do : "[Q]",
        rd->why_do_it ? rd->why_do_it : "[R]");
    rd->root_definition = strdup(buf);
}
void ssm_rootdef_parse(RootDefinition* rd, const char* text) {
    if (!rd || !text) return;
    free(rd->root_definition); rd->root_definition = strdup(text);
}
void ssm_rootdef_set_catwoe(RootDefinition* rd, CATWOE* catwoe) {
    if (!rd) return; rd->catwoe = catwoe;
}
double ssm_rootdef_catwoe_fit(const RootDefinition* rd) {
    if (!rd || !rd->catwoe) return 0.0;
    return ssm_catwoe_completeness(rd->catwoe) * ssm_catwoe_consistency(rd->catwoe);
}
void ssm_rootdef_set_efficacy(RootDefinition* rd, double v) { if(rd) rd->efficacy=v; }
void ssm_rootdef_set_efficiency(RootDefinition* rd, double v) { if(rd) rd->efficiency=v; }
void ssm_rootdef_set_effectiveness(RootDefinition* rd, double v) { if(rd) rd->effectiveness=v; }
void ssm_rootdef_set_ethicality(RootDefinition* rd, double v) { if(rd) rd->ethicality=v; }
void ssm_rootdef_set_elegance(RootDefinition* rd, double v) { if(rd) rd->elegance=v; }

double ssm_rootdef_5e_composite(const RootDefinition* rd) {
    if (!rd) return 0.0;
    return (rd->efficacy + rd->efficiency + rd->effectiveness
            + rd->ethicality + rd->elegance) / 5.0;
}
bool ssm_rootdef_is_viable(const RootDefinition* rd) {
    if (!rd) return false;
    return rd->efficacy >= SSM_EFFICACY_MIN
        && rd->efficiency >= SSM_EFFICIENCY_MIN
        && rd->effectiveness >= SSM_EFFECTIVENESS_MIN
        && rd->ethicality >= SSM_ETHICALITY_MIN;
}
double ssm_rootdef_internal_coherence(const RootDefinition* rd) {
    if (!rd) return 0.0;
    double score = 1.0;
    if (!rd->what_to_do && !rd->root_definition) score -= 0.3;
    if (!rd->how_to_do) score -= 0.2;
    if (!rd->why_do_it) score -= 0.2;
    return fmax(score, 0.0);
}
bool ssm_rootdef_monitorable(const RootDefinition* rd) {
    return rd && rd->efficacy >= 0.3 && rd->efficiency >= 0.3;
}

double ssm_rootdef_pair_conflict(const RootDefinition* a, const RootDefinition* b) {
    if (!a || !b) return 0.0;
    if (!a->catwoe || !b->catwoe) return 0.0;
    if (!a->catwoe->w_ref || !b->catwoe->w_ref) return 0.0;
    return ssm_w_conflict_score(a->catwoe->w_ref, b->catwoe->w_ref);
}
void ssm_rootdef_rank_by_relevance(RootDefinition** rds, int n) {
    for (int i = 0; i < n - 1; i++)
        for (int j = i + 1; j < n; j++)
            if (rds[i] && rds[j] && rds[j]->relevance_rank < rds[i]->relevance_rank) {
                RootDefinition* tmp = rds[i]; rds[i] = rds[j]; rds[j] = tmp;
            }
}
int ssm_rootdef_select_primary(RootDefinition** rds, int n) {
    if (!rds || n <= 0) return -1;
    ssm_rootdef_rank_by_relevance(rds, n);
    return 0;
}
void ssm_rootdef_print(const RootDefinition* rd) {
    if (!rd) { printf("(null RD)\n"); return; }
    printf("RootDef[%d]: %s\n", rd->id, rd->name ? rd->name : "?");
    printf("  RD: %s\n", rd->root_definition ? rd->root_definition : "?");
    printf("  PQR: [%s] by [%s] for [%s]\n",
        rd->what_to_do ? rd->what_to_do : "?",
        rd->how_to_do ? rd->how_to_do : "?",
        rd->why_do_it ? rd->why_do_it : "?");
    printf("  5E: E1=%.2f E2=%.2f E3=%.2f E4=%.2f E5=%.2f\n",
        rd->efficacy, rd->efficiency, rd->effectiveness,
        rd->ethicality, rd->elegance);
    printf("  Coherence=%.2f  Rank=%d\n",
        rd->coherence_score, rd->relevance_rank);
}
/* Advanced Root Definition Analysis */
RootDefinition** ssm_rootdef_generate_variants(const RootDefinition* base, const Weltanschauung** ws, int n_w, int* n_out) {
    if (!base || !ws || n_w < 1 || !n_out) return NULL;
    RootDefinition** variants = (RootDefinition**)calloc((size_t)n_w, sizeof(RootDefinition*));
    if (!variants) { *n_out = 0; return NULL; }
    for (int i = 0; i < n_w; i++) {
        variants[i] = ssm_rootdef_clone(base);
        if (variants[i] && variants[i]->catwoe)
            variants[i]->catwoe->w_ref = (Weltanschauung*)ws[i];
    }
    *n_out = n_w; return variants;
}
void ssm_rootdef_free_variants(RootDefinition** variants, int n) {
    if (!variants) return;
    for (int i = 0; i < n; i++) ssm_rootdef_free(variants[i]);
    free(variants);
}
double ssm_rootdef_consensus_score(RootDefinition** rds, int n) {
    if (!rds || n < 2) return 1.0;
    double total_conflict = 0.0; int pairs = 0;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++) {
            total_conflict += ssm_rootdef_pair_conflict(rds[i], rds[j]);
            pairs++;
        }
    return (pairs > 0) ? 1.0 - total_conflict / (double)pairs : 1.0;
}
bool ssm_rootdef_is_contentious(RootDefinition** rds, int n) {
    return ssm_rootdef_consensus_score(rds, n) < 0.5;
}
int ssm_rootdef_find_accommodation(RootDefinition** rds, int n, RootDefinition** result) {
    if (!rds || n < 2 || !result) return -1;
    int best_pair_i = 0, best_pair_j = 1;
    double min_conflict = 1.0;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++) {
            double conf = ssm_rootdef_pair_conflict(rds[i], rds[j]);
            if (conf < min_conflict) { min_conflict = conf; best_pair_i = i; best_pair_j = j; }
        }
    *result = ssm_rootdef_clone(rds[best_pair_i]);
    return (min_conflict < 0.3) ? 0 : 1;
}
double ssm_rootdef_stakeholder_coverage(const RootDefinition* rd, const ProblemSituation* ps) {
    if (!rd || !ps || ps->n_stakeholders < 1) return 0.0;
    int covered = 0;
    for (int i = 0; i < ps->n_stakeholders; i++) {
        const char* stk = ps->stakeholders[i];
        if (rd->catwoe && rd->catwoe->customers && strstr(rd->catwoe->customers, stk)) covered++;
        else if (rd->catwoe && rd->catwoe->actors && strstr(rd->catwoe->actors, stk)) covered++;
    }
    return (double)covered / (double)ps->n_stakeholders;
}

double ssm_rootdef_coverage_matrix(RootDefinition** rds, int n, double** matrix) {
    if (!rds || !matrix || n < 2) return 0.0;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            matrix[i][j] = (i == j) ? 1.0 : (1.0 - ssm_rootdef_pair_conflict(rds[i], rds[j]));
    return ssm_rootdef_consensus_score(rds, n);
}

void ssm_rootdef_classify_by_type(RootDefinition** rds, int n, int* primary_task, int* issue_based, int* exploratory, int* critical) {
    if (!rds) return;
    *primary_task = *issue_based = *exploratory = *critical = 0;
    for (int i = 0; i < n; i++) {
        switch (rds[i]->type) {
            case SSM_RD_PRIMARY_TASK: (*primary_task)++; break;
            case SSM_RD_ISSUE_BASED: (*issue_based)++; break;
            case SSM_RD_EXPLORATORY: (*exploratory)++; break;
            case SSM_RD_CRITICAL: (*critical)++; break;
        }
    }
}
