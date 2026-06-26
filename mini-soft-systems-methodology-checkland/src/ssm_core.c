#include "ssm_core.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

CATWOE* ssm_catwoe_create(void) {
    CATWOE* c = (CATWOE*)calloc(1, sizeof(CATWOE));
    if (!c) return NULL;
    c->constraint_capacity = 4;
    c->environment_constraints = (char**)calloc(4, sizeof(char*));
    c->transform_measurability = SSM_DEFAULT_MEASURABILITY;
    return c;
}
void ssm_catwoe_free(CATWOE* c) {
    if (!c) return;
    free(c->customers); free(c->actors);
    free(c->transformation_input); free(c->transformation_output);
    free(c->weltanschauung); free(c->owners);
    if (c->customer_list)
        for (int i = 0; i < c->n_customer_types; i++) free(c->customer_list[i]);
    free(c->customer_list);
    if (c->actor_list)
        for (int i = 0; i < c->n_actor_types; i++) free(c->actor_list[i]);
    free(c->actor_list);
    if (c->owner_list)
        for (int i = 0; i < c->n_owner_types; i++) free(c->owner_list[i]);
    free(c->owner_list);
    if (c->environment_constraints)
        for (int i = 0; i < c->n_constraints; i++) free(c->environment_constraints[i]);
    free(c->environment_constraints);
    free(c);
}
void ssm_catwoe_set_customer(CATWOE* c, const char* cust) {
    if (!c) return; free(c->customers); c->customers = strdup(cust);
}
void ssm_catwoe_add_customer(CATWOE* c, const char* cust) {
    if (!c || !cust) return;
    int n = c->n_customer_types;
    char** nn = (char**)realloc(c->customer_list, (size_t)(n + 1) * sizeof(char*));
    if (!nn) return; c->customer_list = nn;
    c->customer_list[n] = strdup(cust); c->n_customer_types = n + 1;
}
void ssm_catwoe_set_actor(CATWOE* c, const char* actor) {
    if (!c) return; free(c->actors); c->actors = strdup(actor);
}
void ssm_catwoe_add_actor(CATWOE* c, const char* actor) {
    if (!c || !actor) return;
    int n = c->n_actor_types;
    char** nn = (char**)realloc(c->actor_list, (size_t)(n + 1) * sizeof(char*));
    if (!nn) return; c->actor_list = nn;
    c->actor_list[n] = strdup(actor); c->n_actor_types = n + 1;
}
void ssm_catwoe_set_transformation(CATWOE* c, const char* input,
    const char* output, TransformationType t) {
    if (!c) return;
    free(c->transformation_input); c->transformation_input = strdup(input);
    free(c->transformation_output); c->transformation_output = strdup(output);
    c->transform_type = t;
}
void ssm_catwoe_set_worldview(CATWOE* c, const char* wv) {
    if (!c) return; free(c->weltanschauung); c->weltanschauung = strdup(wv);
}
void ssm_catwoe_set_owner(CATWOE* c, const char* owner) {
    if (!c) return; free(c->owners); c->owners = strdup(owner);
}
void ssm_catwoe_add_owner(CATWOE* c, const char* owner) {
    if (!c || !owner) return;
    int n = c->n_owner_types;
    char** nn = (char**)realloc(c->owner_list, (size_t)(n + 1) * sizeof(char*));
    if (!nn) return; c->owner_list = nn;
    c->owner_list[n] = strdup(owner); c->n_owner_types = n + 1;
}
void ssm_catwoe_add_constraint(CATWOE* c, const char* constraint) {
    if (!c || !constraint) return;
    if (c->n_constraints >= c->constraint_capacity) {
        c->constraint_capacity *= 2;
        char** nn = (char**)realloc(c->environment_constraints,
            (size_t)c->constraint_capacity * sizeof(char*));
        if (!nn) return; c->environment_constraints = nn;
    }
    c->environment_constraints[c->n_constraints++] = strdup(constraint);
}
void ssm_catwoe_set_efficacy(CATWOE* c, double v) { if (c) c->efficacy_criterion = v; }
void ssm_catwoe_set_efficiency(CATWOE* c, double v) { if (c) c->efficiency_criterion = v; }
void ssm_catwoe_set_effectiveness(CATWOE* c, double v) { if (c) c->effectiveness_criterion = v; }

double ssm_catwoe_completeness(const CATWOE* c) {
    if (!c) return 0.0;
    int filled = 0;
    if (c->customers || c->n_customer_types > 0) filled++;
    if (c->actors || c->n_actor_types > 0) filled++;
    if (c->transformation_input && c->transformation_output) filled++;
    if (c->weltanschauung) filled++;
    if (c->owners || c->n_owner_types > 0) filled++;
    if (c->n_constraints > 0) filled++;
    return (double)filled / 6.0;
}
double ssm_catwoe_consistency(const CATWOE* c) {
    if (!c) return 0.0;
    double score = 1.0;
    if (!c->transformation_input || !c->transformation_output) score -= 0.3;
    return fmax(score, 0.0);
}
bool ssm_catwoe_is_well_formulated(const CATWOE* c) {
    return ssm_catwoe_completeness(c) > 0.75 && ssm_catwoe_consistency(c) > 0.6;
}

Weltanschauung* ssm_weltanschauung_create(const char* name, const char* desc) {
    Weltanschauung* w = (Weltanschauung*)calloc(1, sizeof(Weltanschauung));
    if (!w) return NULL;
    w->name = strdup(name); w->description = strdup(desc);
    w->assumption_capacity = 4;
    w->assumptions = (char**)calloc(4, sizeof(char*));
    w->implication_capacity = 4;
    w->implications = (char**)calloc(4, sizeof(char*));
    w->type = SSM_W_EXPLICIT; w->influence_weight = 0.5;
    return w;
}
void ssm_weltanschauung_free(Weltanschauung* w) {
    if (!w) return;
    free(w->name); free(w->description);
    for (int i = 0; i < w->n_assumptions; i++) free(w->assumptions[i]);
    free(w->assumptions);
    for (int i = 0; i < w->n_implications; i++) free(w->implications[i]);
    free(w->implications);
    free(w);
}
void ssm_w_add_assumption(Weltanschauung* w, const char* a) {
    if (!w || !a) return;
    if (w->n_assumptions >= w->assumption_capacity) {
        w->assumption_capacity *= 2;
        char** nn = (char**)realloc(w->assumptions,
            (size_t)w->assumption_capacity * sizeof(char*));
        if (!nn) return; w->assumptions = nn;
    }
    w->assumptions[w->n_assumptions++] = strdup(a);
}
void ssm_w_add_implication(Weltanschauung* w, const char* imp) {
    if (!w || !imp) return;
    if (w->n_implications >= w->implication_capacity) {
        w->implication_capacity *= 2;
        char** nn = (char**)realloc(w->implications,
            (size_t)w->implication_capacity * sizeof(char*));
        if (!nn) return; w->implications = nn;
    }
    w->implications[w->n_implications++] = strdup(imp);
}
double ssm_w_conflict_score(const Weltanschauung* w1, const Weltanschauung* w2) {
    if (!w1 || !w2) return 0.0;
    double conflicts = 0.0; int total = 0;
    for (int i = 0; i < w1->n_assumptions && i < w2->n_assumptions; i++) {
        total++;
        if (w1->assumptions[i] && w2->assumptions[i] &&
            strcmp(w1->assumptions[i], w2->assumptions[i]) != 0) conflicts += 1.0;
    }
    return (total > 0) ? conflicts / (double)total : 0.0;
}
double ssm_w_accommodation_potential(const Weltanschauung* w1,
    const Weltanschauung* w2) {
    return 1.0 - ssm_w_conflict_score(w1, w2);
}
void ssm_w_print(const Weltanschauung* w) {
    if (!w) { printf("(null W)\n"); return; }
    printf("W: %s\n  %s\n  Assumptions(%d):",
        w->name, w->description, w->n_assumptions);
    for (int i = 0; i < w->n_assumptions; i++)
        printf(" [%s]", w->assumptions[i]);
    printf("\n  type=%d influence=%.2f\n", w->type, w->influence_weight);
}
void ssm_catwoe_print(const CATWOE* c) {
    if (!c) { printf("(null CATWOE)\n"); return; }
    printf("CATWOE:\n  C: %s", c->customers ? c->customers : "(?)");
    printf("\n  A: %s", c->actors ? c->actors : "(?)");
    printf("\n  T: %s -> %s [type=%d]\n",
        c->transformation_input ? c->transformation_input : "?",
        c->transformation_output ? c->transformation_output : "?",
        c->transform_type);
    printf("  W: %s\n  O: %s\n",
        c->weltanschauung ? c->weltanschauung : "?",
        c->owners ? c->owners : "?");
    printf("  E(%d):", c->n_constraints);
    for (int i = 0; i < c->n_constraints; i++)
        printf(" [%s]", c->environment_constraints[i]);
    printf("\n  3E: E1=%.2f E2=%.2f E3=%.2f\n",
        c->efficacy_criterion, c->efficiency_criterion,
        c->effectiveness_criterion);
}
/* =============================================================
 * Advanced Weltanschauung Analysis
 * ============================================================= */

double ssm_w_influence_matrix(const Weltanschauung** ws, int n, double** matrix_out) {
    if (!ws || n < 2 || !matrix_out) return 0.0;
    double total_conflict = 0.0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j) { matrix_out[i][j] = 1.0; continue; }
            double conf = ssm_w_conflict_score(ws[i], ws[j]);
            matrix_out[i][j] = 1.0 - conf;  /* Compatibility matrix */
            total_conflict += conf;
        }
    }
    return total_conflict / (double)(n * (n - 1));
}

int ssm_w_identify_dominant(const Weltanschauung** ws, int n) {
    if (!ws || n < 1) return -1;
    int dominant = 0;
    for (int i = 1; i < n; i++)
        if (ws[i]->influence_weight > ws[dominant]->influence_weight) dominant = i;
    return dominant;
}

int ssm_w_identify_aligned(const Weltanschauung** ws, int n, int* aligned_indices) {
    if (!ws || n < 2 || !aligned_indices) return 0;
    int dominant = ssm_w_identify_dominant(ws, n);
    int count = 0;
    for (int i = 0; i < n; i++) {
        if (i == dominant) { aligned_indices[count++] = i; continue; }
        if (ssm_w_accommodation_potential(ws[dominant], ws[i]) > 0.7)
            aligned_indices[count++] = i;
    }
    return count;
}

double ssm_w_diversity_index(const Weltanschauung** ws, int n) {
    if (!ws || n < 1) return 0.0;
    double total_pairs = (double)(n * (n - 1)) / 2.0;
    if (total_pairs < 1.0) return 0.0;
    double sum_conflict = 0.0;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            sum_conflict += ssm_w_conflict_score(ws[i], ws[j]);
    return sum_conflict / total_pairs;
}

bool ssm_w_can_accommodate(const Weltanschauung** ws, int n) {
    if (!ws || n < 2) return true;
    double diversity = ssm_w_diversity_index(ws, n);
    double avg_potential = 0.0;
    int pairs = 0;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++) {
            avg_potential += ssm_w_accommodation_potential(ws[i], ws[j]);
            pairs++;
        }
    avg_potential /= (double)pairs;
    return avg_potential >= SSM_ACCOMMODATION_THRESHOLD && diversity <= SSM_CONFLICT_TOLERABLE;
}

/* =============================================================
 * CATWOE Cross-Stakeholder Validation
 * ============================================================= */

double ssm_catwoe_stakeholder_alignment(const CATWOE* c1, const CATWOE* c2) {
    if (!c1 || !c2) return 0.0;
    double alignment = 0.0;
    int checks = 0;
    if (c1->weltanschauung && c2->weltanschauung) {
        if (strcmp(c1->weltanschauung, c2->weltanschauung) == 0) alignment += 1.0;
        checks++;
    }
    if (c1->transformation_output && c2->transformation_output) {
        if (strcmp(c1->transformation_output, c2->transformation_output) == 0) alignment += 1.0;
        checks++;
    }
    return (checks > 0) ? alignment / (double)checks : 0.0;
}

double ssm_catwoe_stakeholder_conflict_index(const CATWOE** catwoes, int n) {
    if (!catwoes || n < 2) return 0.0;
    double total_misalignment = 0.0;
    int pairs = 0;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++) {
            total_misalignment += 1.0 - ssm_catwoe_stakeholder_alignment(catwoes[i], catwoes[j]);
            pairs++;
        }
    return (pairs > 0) ? total_misalignment / (double)pairs : 0.0;
}

void ssm_catwoe_identify_tensions(const CATWOE** catwoes, int n, char*** tension_list, int* n_tensions) {
    if (!catwoes || n < 2 || !tension_list || !n_tensions) return;
    *n_tensions = 0;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (ssm_catwoe_stakeholder_alignment(catwoes[i], catwoes[j]) < 0.5)
                (*n_tensions)++;
}

/* =============================================================
 * Rich Picture to CATWOE Bridge
 * ============================================================= */

int ssm_rp_extract_catwoe_elements(const RichPicture* rp, int* structure_ids, int* process_ids, int* concern_ids) {
    if (!rp) return 0;
    int s = 0, p = 0, c = 0;
    for (int i = 0; i < rp->n_nodes; i++) {
        RichPictureNode* n = rp->nodes[i];
        if (n->type == SSM_ELEM_STRUCTURE && structure_ids) structure_ids[s++] = i;
        else if (n->type == SSM_ELEM_PROCESS && process_ids) process_ids[p++] = i;
        else if (n->type == SSM_ELEM_CONCERN && concern_ids) concern_ids[c++] = i;
    }
    return s + p + c;
}

/* =============================================================
 * Systemic Desirability and Cultural Feasibility
 * ============================================================= */

double ssm_change_desirability(const Comparison* comp, int agenda_idx) {
    if (!comp || agenda_idx < 0 || agenda_idx >= comp->n_agenda) return 0.0;
    return 0.7;  /* Default: desirable if it reached the agenda */
}

double ssm_change_feasibility(const Comparison* comp, int agenda_idx) {
    if (!comp || agenda_idx < 0 || agenda_idx >= comp->n_agenda) return 0.0;
    return 0.5;  /* Default moderate feasibility */
}

double ssm_change_priority(const Comparison* comp, int agenda_idx) {
    double d = ssm_change_desirability(comp, agenda_idx);
    double f = ssm_change_feasibility(comp, agenda_idx);
    return d * 0.4 + f * 0.6;
}

/* =============================================================
 * SSM Debrief and Reflection
 * ============================================================= */

void ssm_cycle_reflect(SSMCycle* cycle) {
    if (!cycle) return;
    char buf[256];
    snprintf(buf, sizeof(buf), "Stage %d complete: accommodation=%.2f, %d root defs, %d models",
        (int)cycle->current_stage, cycle->accommodation_index,
        cycle->n_root_defs, cycle->n_models);
    ssm_cycle_add_learning(cycle, buf);
}

bool ssm_cycle_should_iterate(const SSMCycle* cycle) {
    return cycle && !ssm_accommodation_achieved(cycle) && cycle->iteration_count < SSM_MAX_ITERATIONS;
}

double ssm_cycle_progress_metric(const SSMCycle* cycle) {
    if (!cycle) return 0.0;
    double stage_progress = (double)cycle->current_stage / 7.0;
    double accom_progress = cycle->accommodation_index;
    return (stage_progress + accom_progress) / 2.0;
}

/* =============================================================
 * Cross-module integration helpers
 * ============================================================= */

double ssm_worldview_alignment_matrix(const ProblemSituation* ps, double** matrix) {
    if (!ps || !matrix || ps->n_worldviews < 2) return 0.0;
    for (int i = 0; i < ps->n_worldviews; i++)
        for (int j = 0; j < ps->n_worldviews; j++)
            matrix[i][j] = (i == j) ? 1.0 : ssm_w_accommodation_potential(ps->worldviews[i], ps->worldviews[j]);
    return ssm_w_diversity_index((const Weltanschauung**)ps->worldviews, ps->n_worldviews);
}

void ssm_generate_rich_picture_from_situation(const ProblemSituation* ps, RichPicture* rp) {
    if (!ps || !rp) return;
    for (int i = 0; i < ps->n_stakeholders; i++)
        ssm_rp_add_node(rp, ps->stakeholders[i], SSM_ELEM_STRUCTURE, 0.2 + 0.6 * ((double)i / fmax((double)ps->n_stakeholders, 1.0)), 0.3);
    for (int i = 0; i < ps->n_issues; i++)
        ssm_rp_add_node(rp, ps->issues[i], SSM_ELEM_CONCERN, 0.2 + 0.6 * ((double)i / fmax((double)ps->n_issues, 1.0)), 0.7);
}

int ssm_extract_key_concerns(const ProblemSituation* ps, char** concerns, int max_n) {
    if (!ps || !concerns || max_n < 1) return 0;
    int n = (ps->n_issues < max_n) ? ps->n_issues : max_n;
    for (int i = 0; i < n; i++) concerns[i] = ps->issues[i];
    return n;
}

/* =============================================================
 * SSM Process Quality Metrics
 * ============================================================= */

double ssm_process_rigor_score(const SSMCycle* cycle) {
    if (!cycle) return 0.0;
    double score = 0.0;
    if (cycle->current_stage >= SSM_STAGE_2_EXPRESSION) score += 0.2;
    if (cycle->current_stage >= SSM_STAGE_3_ROOT_DEFS && cycle->n_root_defs > 0) score += 0.3;
    if (cycle->current_stage >= SSM_STAGE_4_MODELS && cycle->n_models > 0) score += 0.2;
    if (cycle->current_stage >= SSM_STAGE_5_COMPARISON && cycle->n_comparisons > 0) score += 0.2;
    score += 0.1 * cycle->accommodation_index;
    return score;
}

double ssm_process_efficiency(const SSMCycle* cycle) {
    if (!cycle || cycle->iteration_count < 1) return 0.0;
    return cycle->accommodation_index / fmax((double)cycle->iteration_count, 1.0);
}

bool ssm_process_is_methodologically_sound(const SSMCycle* cycle) {
    return ssm_process_rigor_score(cycle) > 0.5 && ssm_process_efficiency(cycle) > 0.1;
}






































































