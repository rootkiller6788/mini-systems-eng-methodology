#include "ssm_core.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

ProblemSituation* ssm_situation_create(const char* name) {
    ProblemSituation* ps = (ProblemSituation*)calloc(1, sizeof(ProblemSituation));
    if (!ps) return NULL;
    ps->situation_name = strdup(name);
    ps->stakeholder_capacity = 8;
    ps->stakeholders = (char**)calloc(8, sizeof(char*));
    ps->issue_capacity = 8;
    ps->issues = (char**)calloc(8, sizeof(char*));
    ps->structures = (char**)calloc(4, sizeof(char*));
    ps->w_capacity = 8;
    ps->worldviews = (Weltanschauung**)calloc(8, sizeof(Weltanschauung*));
    return ps;
}
void ssm_situation_free(ProblemSituation* ps) {
    if (!ps) return;
    free(ps->situation_name); free(ps->rich_description);
    for (int i = 0; i < ps->n_stakeholders; i++) free(ps->stakeholders[i]);
    free(ps->stakeholders);
    for (int i = 0; i < ps->n_issues; i++) free(ps->issues[i]);
    free(ps->issues);
    for (int i = 0; i < ps->n_structures; i++) free(ps->structures[i]);
    free(ps->structures);
    free(ps->worldviews); free(ps);
}
void ssm_situation_add_stakeholder(ProblemSituation* ps, const char* s) {
    if (!ps || !s) return;
    if (ps->n_stakeholders >= ps->stakeholder_capacity) {
        ps->stakeholder_capacity *= 2;
        char** nn = (char**)realloc(ps->stakeholders,
            (size_t)ps->stakeholder_capacity * sizeof(char*));
        if (!nn) return;
        ps->stakeholders = nn;
    }
    ps->stakeholders[ps->n_stakeholders++] = strdup(s);
}
void ssm_situation_add_issue(ProblemSituation* ps, const char* issue) {
    if (!ps || !issue) return;
    if (ps->n_issues >= ps->issue_capacity) {
        ps->issue_capacity *= 2;
        char** nn = (char**)realloc(ps->issues,
            (size_t)ps->issue_capacity * sizeof(char*));
        if (!nn) return;
        ps->issues = nn;
    }
    ps->issues[ps->n_issues++] = strdup(issue);
}
void ssm_situation_add_worldview(ProblemSituation* ps, Weltanschauung* w) {
    if (!ps || !w) return;
    if (ps->n_worldviews >= ps->w_capacity) {
        ps->w_capacity *= 2;
        Weltanschauung** nn = (Weltanschauung**)realloc(ps->worldviews,
            (size_t)ps->w_capacity * sizeof(Weltanschauung*));
        if (!nn) return;
        ps->worldviews = nn;
    }
    ps->worldviews[ps->n_worldviews++] = w;
}
double ssm_situation_complexity(ProblemSituation* ps) {
    if (!ps) return 0.0;
    ps->complexity_score = (double)(ps->n_stakeholders + ps->n_issues) / 20.0;
    if (ps->complexity_score > 1.0) ps->complexity_score = 1.0;
    return ps->complexity_score;
}
double ssm_situation_conflict_level(ProblemSituation* ps) {
    if (!ps || ps->n_worldviews < 2) return 0.0;
    double sum = 0.0; int pairs = 0;
    for (int i = 0; i < ps->n_worldviews; i++)
        for (int j = i + 1; j < ps->n_worldviews; j++) {
            sum += ssm_w_conflict_score(ps->worldviews[i], ps->worldviews[j]);
            pairs++;
        }
    ps->conflict_level = (pairs > 0) ? sum / (double)pairs : 0.0;
    return ps->conflict_level;
}

Comparison* ssm_comparison_create(const char* model_name,
    const char* situation_name, ComparisonMethod method) {
    Comparison* comp = (Comparison*)calloc(1, sizeof(Comparison));
    if (!comp) return NULL;
    comp->model_name = strdup(model_name);
    comp->situation_name = strdup(situation_name);
    comp->method = method;
    comp->gap_capacity = 16;
    comp->gaps = (ActivityGap**)calloc(16, sizeof(ActivityGap*));
    return comp;
}
void ssm_comparison_free(Comparison* comp) {
    if (!comp) return;
    free(comp->model_name); free(comp->situation_name);
    for (int i = 0; i < comp->n_gaps; i++) {
        free(comp->gaps[i]->gap_description);
        free(comp->gaps[i]->recommendation);
        free(comp->gaps[i]);
    }
    free(comp->gaps);
    if (comp->agenda_items) {
        for (int i = 0; i < comp->n_agenda; i++) free(comp->agenda_items[i]);
        free(comp->agenda_items);
    }
    free(comp->agenda_priorities);
    free(comp);
}
int ssm_comparison_add_gap(Comparison* comp, int activity_id,
    const char* desc, double severity, bool exists, bool works_well) {
    if (!comp || !desc) return -1;
    if (comp->n_gaps >= comp->gap_capacity) {
        comp->gap_capacity *= 2;
        ActivityGap** nn = (ActivityGap**)realloc(comp->gaps,
            (size_t)comp->gap_capacity * sizeof(ActivityGap*));
        if (!nn) return -1;
        comp->gaps = nn;
    }
    ActivityGap* g = (ActivityGap*)calloc(1, sizeof(ActivityGap));
    if (!g) return -1;
    g->activity_id = activity_id;
    g->gap_description = strdup(desc);
    g->gap_severity = severity;
    g->exists_in_reality = exists;
    g->works_well = works_well;
    comp->gaps[comp->n_gaps++] = g;
    return comp->n_gaps - 1;
}
void ssm_comparison_set_recommendation(Comparison* comp, int gap_idx,
    const char* rec) {
    if (!comp || gap_idx < 0 || gap_idx >= comp->n_gaps) return;
    free(comp->gaps[gap_idx]->recommendation);
    comp->gaps[gap_idx]->recommendation = strdup(rec);
}
double ssm_comparison_compute_alignment(Comparison* comp) {
    if (!comp || comp->n_gaps == 0) return 1.0;
    double sum = 0.0;
    for (int i = 0; i < comp->n_gaps; i++) {
        ActivityGap* g = comp->gaps[i];
        if (g->exists_in_reality && g->works_well) sum += 1.0;
        else if (g->exists_in_reality) sum += 0.5;
    }
    comp->overall_alignment = sum / (double)comp->n_gaps;
    return comp->overall_alignment;
}

SSMCycle* ssm_cycle_create(void) {
    SSMCycle* cycle = (SSMCycle*)calloc(1, sizeof(SSMCycle));
    if (!cycle) return NULL;
    cycle->current_stage = SSM_STAGE_1_SITUATION;
    return cycle;
}
void ssm_cycle_free(SSMCycle* cycle) {
    if (!cycle) return;
    ssm_situation_free(cycle->situation);
    free(cycle->worldviews);
    for (int i = 0; i < cycle->n_root_defs; i++)
        ssm_rootdef_free(cycle->root_definitions[i]);
    free(cycle->root_definitions);
    for (int i = 0; i < cycle->n_models; i++)
        ssm_conceptual_free(cycle->conceptual_models[i]);
    free(cycle->conceptual_models);
    for (int i = 0; i < cycle->n_comparisons; i++)
        ssm_comparison_free(cycle->comparisons[i]);
    free(cycle->comparisons);
    for (int i = 0; i < cycle->n_log_entries; i++)
        free(cycle->learning_log[i]);
    free(cycle->learning_log);
    free(cycle);
}
void ssm_cycle_advance(SSMCycle* cycle) {
    if (!cycle || cycle->current_stage >= SSM_STAGE_7_ACTION) return;
    cycle->current_stage = (SSMStage)((int)cycle->current_stage + 1);
    cycle->iteration_count++;
}
void ssm_cycle_add_learning(SSMCycle* cycle, const char* insight) {
    if (!cycle || !insight) return;
    int n = cycle->n_log_entries;
    char** nn = (char**)realloc(cycle->learning_log,
        (size_t)(n + 1) * sizeof(char*));
    if (!nn) return;
    cycle->learning_log = nn;
    cycle->learning_log[cycle->n_log_entries++] = strdup(insight);
}
void ssm_cycle_add_rootdef(SSMCycle* cycle, RootDefinition* rd) {
    if (!cycle || !rd) return;
    int n = cycle->n_root_defs;
    RootDefinition** nn = (RootDefinition**)realloc(cycle->root_definitions,
        (size_t)(n + 1) * sizeof(RootDefinition*));
    if (!nn) return;
    cycle->root_definitions = nn;
    cycle->root_definitions[cycle->n_root_defs++] = rd;
}
void ssm_cycle_add_model(SSMCycle* cycle, ConceptualModel* cm) {
    if (!cycle || !cm) return;
    int n = cycle->n_models;
    ConceptualModel** nn = (ConceptualModel**)realloc(cycle->conceptual_models,
        (size_t)(n + 1) * sizeof(ConceptualModel*));
    if (!nn) return;
    cycle->conceptual_models = nn;
    cycle->conceptual_models[cycle->n_models++] = cm;
}
void ssm_cycle_add_comparison(SSMCycle* cycle, Comparison* comp) {
    if (!cycle || !comp) return;
    int n = cycle->n_comparisons;
    Comparison** nn = (Comparison**)realloc(cycle->comparisons,
        (size_t)(n + 1) * sizeof(Comparison*));
    if (!nn) return;
    cycle->comparisons = nn;
    cycle->comparisons[cycle->n_comparisons++] = comp;
}
double ssm_accommodation_index(SSMCycle* cycle) {
    if (!cycle || cycle->n_worldviews < 2) return 1.0;
    double sum = 0.0; int pairs = 0;
    for (int i = 0; i < cycle->n_worldviews; i++)
        for (int j = i + 1; j < cycle->n_worldviews; j++) {
            sum += ssm_w_accommodation_potential(cycle->worldviews[i],
                cycle->worldviews[j]);
            pairs++;
        }
    cycle->accommodation_index = (pairs > 0) ? sum / (double)pairs : 0.0;
    return cycle->accommodation_index;
}
bool ssm_accommodation_achieved(const SSMCycle* cycle) {
    return cycle && cycle->accommodation_index >= SSM_ACCOMMODATION_THRESHOLD;
}
bool ssm_cycle_is_complete(const SSMCycle* cycle) {
    return cycle && cycle->current_stage >= SSM_STAGE_7_ACTION
        && ssm_accommodation_achieved(cycle);
}

void ssm_situation_print(const ProblemSituation* ps) {
    if (!ps) { printf("(null PS)\n"); return; }
    printf("Problem: %s\n  Stakeholders(%d):",
        ps->situation_name, ps->n_stakeholders);
    for (int i = 0; i < ps->n_stakeholders; i++)
        printf(" [%s]", ps->stakeholders[i]);
    printf("\n  Issues(%d):", ps->n_issues);
    for (int i = 0; i < ps->n_issues; i++)
        printf(" [%s]", ps->issues[i]);
    printf("\n  Complexity=%.2f Conflict=%.2f\n",
        ps->complexity_score, ps->conflict_level);
}
void ssm_comparison_print(const Comparison* comp) {
    if (!comp) { printf("(null Comp)\n"); return; }
    printf("Comparison: %s vs %s\n  Alignment=%.2f Gaps=%d\n",
        comp->model_name, comp->situation_name,
        comp->overall_alignment, comp->n_gaps);
}
void ssm_cycle_print(const SSMCycle* cycle) {
    if (!cycle) { printf("(null Cycle)\n"); return; }
    printf("SSM Cycle: stage=%d iter=%d accom=%.2f  "
        "RDs=%d Models=%d Comp=%d\n",
        cycle->current_stage, cycle->iteration_count,
        cycle->accommodation_index,
        cycle->n_root_defs, cycle->n_models, cycle->n_comparisons);
}

/* =============================================================
 * Agenda Execution and Monitoring
 * ============================================================= */

double ssm_agenda_execution_risk(const Comparison* comp) {
    if (!comp || comp->n_agenda < 1) return 1.0;
    double risk = 0.0;
    for (int i = 0; i < comp->n_agenda; i++)
        risk += 1.0 - (double)comp->agenda_priorities[i] / 10.0;
    return risk / (double)comp->n_agenda;
}

void ssm_agenda_generate_timeline(const Comparison* comp, int* ordering, int* durations, int* n_phases) {
    if (!comp || !ordering || !durations || !n_phases) return;
    int n = (comp->n_agenda < 5) ? comp->n_agenda : 5;
    for (int i = 0; i < n; i++) {
        ordering[i] = i;
        durations[i] = 3 + i * 2;  /* 3, 5, 7, 9, 11 months */
    }
    *n_phases = n;
}

double ssm_agenda_stakeholder_support(const Comparison* comp, const ProblemSituation* ps) {
    if (!comp || !ps || comp->n_agenda < 1) return 0.0;
    double support = 0.5;
    if (ps->conflict_level < 0.3) support = 0.8;
    else if (ps->conflict_level < 0.6) support = 0.5;
    else support = 0.2;
    return support;
}

bool ssm_agenda_is_implementable(const Comparison* comp, const ProblemSituation* ps) {
    double risk = ssm_agenda_execution_risk(comp);
    double support = ssm_agenda_stakeholder_support(comp, ps);
    return risk < 0.6 && support > 0.4;
}

/* =============================================================
 * Multi-Situation Portfolio
 * ============================================================= */

typedef struct { ProblemSituation** situations; int n_situations; int capacity; } SituationPortfolio;

SituationPortfolio* ssm_portfolio_create(void) {
    SituationPortfolio* sp = (SituationPortfolio*)calloc(1, sizeof(SituationPortfolio));
    if (!sp) return NULL;
    sp->capacity = 8;
    sp->situations = (ProblemSituation**)calloc(8, sizeof(ProblemSituation*));
    return sp;
}
void ssm_portfolio_free(SituationPortfolio* sp) {
    if (!sp) return;
    for (int i = 0; i < sp->n_situations; i++) ssm_situation_free(sp->situations[i]);
    free(sp->situations); free(sp);
}
void ssm_portfolio_add(SituationPortfolio* sp, ProblemSituation* ps) {
    if (!sp || !ps) return;
    if (sp->n_situations >= sp->capacity) { sp->capacity *= 2;
        ProblemSituation** nn = (ProblemSituation**)realloc(sp->situations, (size_t)sp->capacity * sizeof(ProblemSituation*));
        if (!nn) return; sp->situations = nn; }
    sp->situations[sp->n_situations++] = ps;
}

double ssm_portfolio_complexity(const SituationPortfolio* sp) {
    if (!sp || sp->n_situations < 1) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < sp->n_situations; i++)
        sum += ssm_situation_complexity(sp->situations[i]);
    return sum / (double)sp->n_situations;
}

int ssm_portfolio_prioritize(const SituationPortfolio* sp, int* ranking) {
    if (!sp || !ranking || sp->n_situations < 1) return 0;
    for (int i = 0; i < sp->n_situations; i++) ranking[i] = i;
    for (int i = 0; i < sp->n_situations - 1; i++)
        for (int j = i + 1; j < sp->n_situations; j++) {
            double ci = ssm_situation_complexity(sp->situations[i]);
            double cj = ssm_situation_complexity(sp->situations[j]);
            if (cj > ci) { int t = ranking[i]; ranking[i] = ranking[j]; ranking[j] = t; }
        }
    return sp->n_situations;
}

/* ============================================================================
 * Multi-Perspective Rich Picture Analysis
 *
 * SSM emphasizes that different stakeholders see different "realities".
 * These functions analyze agreement/disagreement patterns across
 * stakeholder perspectives, quantifying the richness of the picture.
 * ============================================================================ */

/* Quantify divergence between two stakeholder perspectives.
 * Compares their worldviews as vectors of concern intensities.
 * Returns cosine similarity in [-1, 1] where 1 = identical,
 * 0 = orthogonal, -1 = opposed. */
double ssm_perspective_similarity(StakeholderView* a, StakeholderView* b) {
    if (!a || !b) return 0.0;
    int len = (a->n_concerns < b->n_concerns) ? a->n_concerns : b->n_concerns;
    if (len <= 0) return 0.0;
    double dot = 0.0, na = 0.0, nb = 0.0;
    for (int i = 0; i < len; i++) {
        dot += a->concerns[i].intensity * b->concerns[i].intensity;
        na  += a->concerns[i].intensity * a->concerns[i].intensity;
        nb  += b->concerns[i].intensity * b->concerns[i].intensity;
    }
    double denom = sqrt(na) * sqrt(nb);
    return (denom > 1e-15) ? dot / denom : 0.0;
}

/* Multi-perspective coherence: average pairwise similarity across
 * all stakeholders. High coherence -> consensus; low -> conflict.
 * Returns value in [-1, 1]. */
double ssm_multi_perspective_coherence(ProblemSituation* ps) {
    if (!ps || !ps->stakeholder_views || ps->n_views < 2) return 1.0;
    double total_sim = 0.0;
    int npairs = 0;
    for (int i = 0; i < ps->n_views; i++) {
        for (int j = i + 1; j < ps->n_views; j++) {
            total_sim += ssm_perspective_similarity(
                ps->stakeholder_views[i], ps->stakeholder_views[j]);
            npairs++;
        }
    }
    return (npairs > 0) ? total_sim / (double)npairs : 1.0;
}

/* Identify the most divergent stakeholder pair (minimum similarity).
 * Fills *a_idx and *b_idx with indices of the pair.
 * Returns the similarity value. */
double ssm_most_divergent_pair(ProblemSituation* ps, int* a_idx, int* b_idx) {
    if (!ps || !ps->stakeholder_views || ps->n_views < 2) {
        if (a_idx) *a_idx = -1;
        if (b_idx) *b_idx = -1;
        return 0.0;
    }
    double min_sim = 1.0;
    int ai = 0, bi = 1;
    for (int i = 0; i < ps->n_views; i++) {
        for (int j = i + 1; j < ps->n_views; j++) {
            double sim = ssm_perspective_similarity(
                ps->stakeholder_views[i], ps->stakeholder_views[j]);
            if (sim < min_sim) { min_sim = sim; ai = i; bi = j; }
        }
    }
    if (a_idx) *a_idx = ai;
    if (b_idx) *b_idx = bi;
    return min_sim;
}

/* Conflict intensity score: 1.0 - coherence.
 * High conflict -> SSM intervention needed.
 * Returns value in [0, 2] (0 = full consensus, 2 = total opposition). */
double ssm_conflict_intensity(ProblemSituation* ps) {
    double coh = ssm_multi_perspective_coherence(ps);
    return 1.0 - coh;
}

/* W-matrix similarity: compare two conceptual models quantitatively.
 * Counts matching activities (by label) between the two models.
 * Returns Jaccard similarity in [0, 1]. */
double ssm_conceptual_model_similarity(ConceptualModel* a, ConceptualModel* b) {
    if (!a || !b || a->n_activities <= 0 || b->n_activities <= 0)
        return 0.0;
    int matches = 0;
    for (int i = 0; i < a->n_activities; i++) {
        for (int j = 0; j < b->n_activities; j++) {
            if (a->activities[i]->name && b->activities[j]->name
                && strcmp(a->activities[i]->name,
                          b->activities[j]->name) == 0)
                matches++;
        }
    }
    int total = a->n_activities + b->n_activities - matches;
    return (total > 0) ? (double)matches / (double)total : 0.0;
}

/* Structured comparison of conceptual model vs. real-world situation.
 * SSM Stage 5: Compare model with perceived real situation.
 * Generates agenda of possible changes (Checkland & Scholes, 1990). */
int ssm_compare_activities(int model_activities, int real_activities,
                            int* missing, int* extra, double* alignment_score) {
    if (!missing || !extra || !alignment_score) return -1;
    *missing = (model_activities > real_activities) ? model_activities - real_activities : 0;
    *extra   = (real_activities > model_activities) ? real_activities - model_activities : 0;
    int total = model_activities + real_activities;
    *alignment_score = total > 0 ? (double)(total - *missing - *extra) / (double)total : 1.0;
    return 0;
}

/* Feasibility filter: rank proposed changes by feasibility and desirability. */
int ssm_filter_changes(int n_changes, const double* feasibility,
                        const double* desirability, double threshold, int* accepted) {
    if (!feasibility || !desirability || !accepted || n_changes <= 0) return -1;
    *accepted = 0;
    for (int i = 0; i < n_changes; i++)
        if (feasibility[i] >= threshold && desirability[i] >= threshold)
            (*accepted)++;
    return 0;
}
