#include "ssm_core.h"
#include "ssm_eval.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

SSMEvaluation* ssm_evaluation_create(const char* name) {
    SSMEvaluation* ev = (SSMEvaluation*)calloc(1, sizeof(SSMEvaluation));
    if (!ev) return NULL; ev->intervention_name = strdup(name);
    ev->metrics = (SSMMetric**)calloc(16, sizeof(SSMMetric*));
    ev->lessons_learned = (char**)calloc(16, sizeof(char*));
    return ev;
}
void ssm_evaluation_free(SSMEvaluation* ev) {
    if (!ev) return; free(ev->intervention_name);
    for (int i = 0; i < ev->n_metrics; i++) { free(ev->metrics[i]->name); free(ev->metrics[i]->qualitative_evidence); free(ev->metrics[i]); }
    free(ev->metrics);
    for (int i = 0; i < ev->n_lessons; i++) free(ev->lessons_learned[i]); free(ev->lessons_learned); free(ev);
}
int ssm_eval_add_metric(SSMEvaluation* ev, const char* name, double before, double after, const char* evidence) {
    if (!ev || !name) return -1;
    if (ev->n_metrics >= 16) {
        SSMMetric** nn = (SSMMetric**)realloc(ev->metrics, (size_t)(ev->n_metrics + 8) * sizeof(SSMMetric*));
        if (!nn) return -1; ev->metrics = nn;
    }
    SSMMetric* m = (SSMMetric*)calloc(1, sizeof(SSMMetric)); if (!m) return -1;
    m->name = strdup(name); m->before_score = before; m->after_score = after;
    m->improvement = (before > 1e-10) ? (after - before) / before : (after - before);
    m->qualitative_evidence = evidence ? strdup(evidence) : NULL;
    ev->metrics[ev->n_metrics++] = m; return ev->n_metrics - 1;
}
double ssm_eval_compute_improvement(SSMEvaluation* ev) {
    if (!ev || ev->n_metrics < 1) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < ev->n_metrics; i++) sum += ev->metrics[i]->improvement;
    ev->overall_improvement = sum / (double)ev->n_metrics; return ev->overall_improvement;
}
void ssm_eval_add_lesson(SSMEvaluation* ev, const char* lesson) {
    if (!ev || !lesson) return; int n = ev->n_lessons;
    char** nn = (char**)realloc(ev->lessons_learned, (size_t)(n + 1) * sizeof(char*));
    if (!nn) return; ev->lessons_learned = nn;
    ev->lessons_learned[ev->n_lessons++] = strdup(lesson);
}
void ssm_eval_print(const SSMEvaluation* ev) {
    if (!ev) { printf("(null EV)\n"); return; }
    printf("Evaluation: %s\n  Metrics: %d  Improvement: %.2f  Sustainable: %s\n",
        ev->intervention_name, ev->n_metrics, ev->overall_improvement, ev->sustainable ? "YES" : "NO");
    for (int i = 0; i < ev->n_metrics; i++) { SSMMetric* m = ev->metrics[i];
        printf("  [%s] %.2f -> %.2f (%.0f%%) %s\n", m->name, m->before_score, m->after_score, m->improvement*100, m->qualitative_evidence ? m->qualitative_evidence : ""); }
    printf("  Lessons(%d):\n", ev->n_lessons);
    for (int i = 0; i < ev->n_lessons; i++) printf("    - %s\n", ev->lessons_learned[i]);
}

SSMFeedbackSummary* ssm_feedback_create(const char* name) {
    SSMFeedbackSummary* fs = (SSMFeedbackSummary*)calloc(1, sizeof(SSMFeedbackSummary));
    if (!fs) return NULL; fs->situation_name = strdup(name);
    fs->feedback = (StakeholderFeedback**)calloc(16, sizeof(StakeholderFeedback*)); return fs;
}
void ssm_feedback_free(SSMFeedbackSummary* fs) {
    if (!fs) return; free(fs->situation_name);
    for (int i = 0; i < fs->n_feedback; i++) { StakeholderFeedback* sf = fs->feedback[i];
        if (sf) { free(sf->stakeholder); for (int j = 0; j < sf->n_concerns; j++) free(sf->concerns[j]); free(sf->concerns); free(sf); } }
    free(fs->feedback); free(fs);
}
int ssm_feedback_add_stakeholder(SSMFeedbackSummary* fs, const char* name, double satisfaction, double fairness) {
    if (!fs || !name) return -1;
    if (fs->n_feedback >= 16) {
        StakeholderFeedback** nn = (StakeholderFeedback**)realloc(fs->feedback, (size_t)(fs->n_feedback + 8) * sizeof(StakeholderFeedback*));
        if (!nn) return -1; fs->feedback = nn;
    }
    StakeholderFeedback* sf = (StakeholderFeedback*)calloc(1, sizeof(StakeholderFeedback)); if (!sf) return -1;
    sf->stakeholder = strdup(name); sf->satisfaction = satisfaction; sf->perceived_fairness = fairness; sf->willingness_to_continue = 0.5;
    sf->concerns = (char**)calloc(8, sizeof(char*));
    fs->feedback[fs->n_feedback++] = sf; return fs->n_feedback - 1;
}
void ssm_feedback_add_concern(SSMFeedbackSummary* fs, int stk_id, const char* concern) {
    if (!fs || !concern || stk_id < 0 || stk_id >= fs->n_feedback) return;
    StakeholderFeedback* sf = fs->feedback[stk_id]; int n = sf->n_concerns;
    char** nn = (char**)realloc(sf->concerns, (size_t)(n + 1) * sizeof(char*));
    if (!nn) return; sf->concerns = nn; sf->concerns[sf->n_concerns++] = strdup(concern);
}
double ssm_feedback_compute_satisfaction(SSMFeedbackSummary* fs) {
    if (!fs || fs->n_feedback < 1) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < fs->n_feedback; i++) sum += fs->feedback[i]->satisfaction;
    fs->overall_satisfaction = sum / (double)fs->n_feedback; return fs->overall_satisfaction;
}
double ssm_feedback_compute_legitimacy(SSMFeedbackSummary* fs) {
    if (!fs || fs->n_feedback < 1) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < fs->n_feedback; i++) sum += fs->feedback[i]->perceived_fairness;
    fs->process_legitimacy = sum / (double)fs->n_feedback; return fs->process_legitimacy;
}
void ssm_feedback_print(const SSMFeedbackSummary* fs) {
    if (!fs) return;
    printf("Feedback: %s\n  Respondents: %d  Satisfaction: %.2f  Legitimacy: %.2f\n",
        fs->situation_name, fs->n_feedback, fs->overall_satisfaction, fs->process_legitimacy);
}

LearningHistory* ssm_history_create(int cycle) {
    LearningHistory* lh = (LearningHistory*)calloc(1, sizeof(LearningHistory));
    if (!lh) return NULL; lh->cycle_number = cycle;
    lh->key_insights = (char**)calloc(16, sizeof(char*)); return lh;
}
void ssm_history_free(LearningHistory* lh) {
    if (!lh) return;
    for (int i = 0; i < lh->n_insights; i++) free(lh->key_insights[i]); free(lh->key_insights); free(lh);
}
void ssm_history_add_insight(LearningHistory* lh, const char* insight) {
    if (!lh || !insight) return; int n = lh->n_insights;
    char** nn = (char**)realloc(lh->key_insights, (size_t)(n + 1) * sizeof(char*));
    if (!nn) return; lh->key_insights = nn; lh->key_insights[lh->n_insights++] = strdup(insight);
}
void ssm_history_record_accommodation(LearningHistory* lh, double accommodation) { if (lh) lh->accommodation_achieved = accommodation; }
bool ssm_history_is_improving(const LearningHistory* lh) { return lh && lh->accommodation_achieved > 0.5; }
double ssm_history_learning_rate(const LearningHistory* lh, const LearningHistory* prev) {
    if (!lh || !prev || prev->accommodation_achieved < 1e-10) return 0.0;
    return (lh->accommodation_achieved - prev->accommodation_achieved) / prev->accommodation_achieved;
}
void ssm_history_print(const LearningHistory* lh) {
    if (!lh) return;
    printf("Learning Cycle %d: accommodation=%.2f insights=%d conflicts=%d/%d\n",
        lh->cycle_number, lh->accommodation_achieved, lh->n_insights, lh->conflicts_resolved, lh->conflicts_remaining);
    for (int i = 0; i < lh->n_insights; i++) printf("  - %s\n", lh->key_insights[i]);
}

/* Cross-metric correlation analysis */
double ssm_eval_correlation(SSMEvaluation* ev, int metric_a, int metric_b) {
    if (!ev || metric_a < 0 || metric_a >= ev->n_metrics || metric_b < 0 || metric_b >= ev->n_metrics) return 0.0;
    return (ev->metrics[metric_a]->improvement + ev->metrics[metric_b]->improvement) / 2.0;
}
double ssm_eval_sustainability_score(SSMEvaluation* ev) {
    if (!ev || ev->n_metrics < 1) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < ev->n_metrics; i++) sum += (ev->metrics[i]->after_score > ev->metrics[i]->before_score) ? 1.0 : 0.0;
    ev->sustainable = (sum / (double)ev->n_metrics) > 0.6;
    return sum / (double)ev->n_metrics;
}
bool ssm_eval_was_effective(SSMEvaluation* ev) { return ev && ssm_eval_compute_improvement(ev) > 0.1; }
void ssm_eval_generate_report(const SSMEvaluation* ev) {
    if (!ev) return;
    printf("=== SSM Evaluation Report ===\n");
    printf("Intervention: %s\n", ev->intervention_name);
    printf("Overall improvement: %.1f%%\n", ev->overall_improvement * 100.0);
    printf("Sustainable: %s\n", ev->sustainable ? "YES" : "NO");
    printf("Lessons learned (%d):\n", ev->n_lessons);
    for (int i = 0; i < ev->n_lessons; i++) printf("  %d. %s\n", i + 1, ev->lessons_learned[i]);
}

/* Feedback: identify most/least satisfied */
int ssm_feedback_most_satisfied(const SSMFeedbackSummary* fs) {
    if (!fs || fs->n_feedback < 1) return -1; int best = 0;
    for (int i = 1; i < fs->n_feedback; i++)
        if (fs->feedback[i]->satisfaction > fs->feedback[best]->satisfaction) best = i;
    return best;
}
int ssm_feedback_least_satisfied(const SSMFeedbackSummary* fs) {
    if (!fs || fs->n_feedback < 1) return -1; int worst = 0;
    for (int i = 1; i < fs->n_feedback; i++)
        if (fs->feedback[i]->satisfaction < fs->feedback[worst]->satisfaction) worst = i;
    return worst;
}
double ssm_feedback_concern_density(const SSMFeedbackSummary* fs) {
    if (!fs || fs->n_feedback < 1) return 0.0; int total = 0;
    for (int i = 0; i < fs->n_feedback; i++) total += fs->feedback[i]->n_concerns;
    return (double)total / (double)fs->n_feedback;
}

/* Learning history trajectory analysis */
double ssm_history_trend(LearningHistory** histories, int n) {
    if (!histories || n < 2) return 0.0;
    double first = histories[0]->accommodation_achieved;
    double last = histories[n - 1]->accommodation_achieved;
    return (last - first) / fmax(first, 0.01);
}
int ssm_history_count_insights(LearningHistory** histories, int n) { int sum = 0; for (int i = 0; i < n; i++) sum += histories[i]->n_insights; return sum; }
bool ssm_history_converged(LearningHistory** histories, int n) {
    if (!histories || n < 3) return false;
    double d1 = histories[n-1]->accommodation_achieved - histories[n-2]->accommodation_achieved;
    double d2 = histories[n-2]->accommodation_achieved - histories[n-3]->accommodation_achieved;
    return fabs(d1) < 0.05 && fabs(d2) < 0.05;
}

/* Cross-evaluation synthesis */
void ssm_eval_synthesize(SSMEvaluation** evals, int n, SSMEvaluation* synthesis) {
    if (!evals || n < 1 || !synthesis) return;
    for (int e = 0; e < n; e++)
        for (int m = 0; m < evals[e]->n_metrics; m++)
            ssm_eval_add_metric(synthesis, evals[e]->metrics[m]->name, evals[e]->metrics[m]->before_score, evals[e]->metrics[m]->after_score, "Synthesized");
    ssm_eval_compute_improvement(synthesis);
    for (int e = 0; e < n; e++)
        for (int l = 0; l < evals[e]->n_lessons; l++)
            ssm_eval_add_lesson(synthesis, evals[e]->lessons_learned[l]);
}

double ssm_eval_compare_before_after(SSMEvaluation* ev, int metric_id) {
    if (!ev || metric_id < 0 || metric_id >= ev->n_metrics) return 0.0;
    return ev->metrics[metric_id]->improvement;
}

int ssm_eval_best_metric(SSMEvaluation* ev) {
    if (!ev || ev->n_metrics < 1) return -1; int best = 0;
    for (int i = 1; i < ev->n_metrics; i++)
        if (ev->metrics[i]->improvement > ev->metrics[best]->improvement) best = i;
    return best;
}

int ssm_eval_worst_metric(SSMEvaluation* ev) {
    if (!ev || ev->n_metrics < 1) return -1; int worst = 0;
    for (int i = 1; i < ev->n_metrics; i++)
        if (ev->metrics[i]->improvement < ev->metrics[worst]->improvement) worst = i;
    return worst;
}

void ssm_eval_identify_patterns(SSMEvaluation* ev, char** patterns, int* n_patterns) {
    if (!ev || !patterns || !n_patterns) return; *n_patterns = 0;
    int improved = 0; for (int i = 0; i < ev->n_metrics; i++) if (ev->metrics[i]->improvement > 0) improved++;
    if (improved == ev->n_metrics) patterns[(*n_patterns)++] = strdup("Universal improvement");
    else if (improved == 0) patterns[(*n_patterns)++] = strdup("No improvement detected");
    else patterns[(*n_patterns)++] = strdup("Mixed results");
    if (ev->sustainable) patterns[(*n_patterns)++] = strdup("Improvements appear sustainable");
}

/* Stakeholder feedback longitudinal */
void ssm_feedback_compare_cycles(const SSMFeedbackSummary* before, const SSMFeedbackSummary* after, double* satisfaction_change, double* legitimacy_change) {
    if (!before || !after) return;
    *satisfaction_change = after->overall_satisfaction - before->overall_satisfaction;
    *legitimacy_change = after->process_legitimacy - before->process_legitimacy;
}

double ssm_feedback_net_promoter(const SSMFeedbackSummary* fs) {
    if (!fs || fs->n_feedback < 1) return 0.0;
    int promoters = 0, detractors = 0;
    for (int i = 0; i < fs->n_feedback; i++) {
        if (fs->feedback[i]->satisfaction >= 0.8) promoters++;
        else if (fs->feedback[i]->satisfaction <= 0.4) detractors++;
    }
    double n = (double)fs->n_feedback;
    return (promoters / n) - (detractors / n);
}

/* SSM evaluation: assess the quality of a soft systems intervention.
 * Checkland's criteria: E1 (efficacy), E2 (efficiency), E3 (effectiveness). */
int ssm_evaluate_intervention(double efficacy, double efficiency, double effectiveness,
                               double* overall_score, char** recommendation) {
    if (!overall_score || !recommendation) return -1;
    *overall_score = (efficacy + efficiency + effectiveness) / 3.0;
    if (*overall_score > 0.8) *recommendation = strdup("Implement changes");
    else if (*overall_score > 0.5) *recommendation = strdup("Refine and retest");
    else *recommendation = strdup("Rethink problem framing");
    return 0;
}

/* Stakeholder satisfaction distribution across CATWOE elements.
 * Identifies which elements have the lowest consensus. */
int ssm_satisfaction_gap(const double* satisfaction, int n_elements,
                          double* min_sat, int* min_idx) {
    if (!satisfaction || !min_sat || !min_idx || n_elements <= 0) return -1;
    *min_idx = 0; *min_sat = satisfaction[0];
    for (int i = 1; i < n_elements; i++)
        if (satisfaction[i] < *min_sat) { *min_sat = satisfaction[i]; *min_idx = i; }
    return 0;
}
