#include "hsm_core.h"
#include "hsm_trade.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * Trade Study & Multi-Criteria Decision Analysis
 *
 * Systems engineering trade studies compare alternative solutions across
 * multiple criteria. This implements:
 *   1. Weighted Sum Model (WSM) â€?classical additive weighting
 *   2. Weighted Product Model (WPM) â€?multiplicative, avoids rank reversal
 *   3. TOPSIS â€?Technique for Order Preference by Similarity to Ideal Solution
 *   4. Sensitivity analysis â€?tornado diagrams, weight perturbation
 *
 * References:
 *   Keeney & Raiffa (1976) Decisions with Multiple Objectives
 *   Hwang & Yoon (1981) Multiple Attribute Decision Making
 *   Pugh (1991) Total Design
 * ============================================================================ */

/* ---------- Trade Study Data Structures ---------- */

typedef struct {
    char** criteria_names;
    double* weights;       /* Sum to 1.0 */
    double* min_acceptable;
    double* max_acceptable;
    bool* is_benefit;      /* true = higher is better, false = lower is better */
    int n_criteria;
} CriteriaSet;

typedef struct {
    char* name;
    double* scores;        /* Raw scores per criterion */
    double weighted_score;
    double topsis_score;
    int rank;
} Alternative;

typedef struct {
    Alternative* alternatives;
    CriteriaSet criteria;
    int n_alternatives;
    int alt_capacity;
} TradeStudyExt;

/* ---------- CriteriaSet ---------- */

CriteriaSet* hsm_criteria_create(int n) {
    if (n <= 0) return NULL;
    CriteriaSet* cs = (CriteriaSet*)calloc(1, sizeof(CriteriaSet));
    if (!cs) return NULL;
    cs->n_criteria = n;
    cs->criteria_names = (char**)calloc((size_t)n, sizeof(char*));
    cs->weights = (double*)calloc((size_t)n, sizeof(double));
    cs->min_acceptable = (double*)calloc((size_t)n, sizeof(double));
    cs->max_acceptable = (double*)calloc((size_t)n, sizeof(double));
    cs->is_benefit = (bool*)calloc((size_t)n, sizeof(bool));
    if (!cs->criteria_names || !cs->weights || !cs->min_acceptable
        || !cs->max_acceptable || !cs->is_benefit) {
        free(cs->criteria_names); free(cs->weights);
        free(cs->min_acceptable); free(cs->max_acceptable);
        free(cs->is_benefit); free(cs);
        return NULL;
    }
    for (int i = 0; i < n; i++) {
        cs->is_benefit[i] = true;
        cs->min_acceptable[i] = -1e100;
        cs->max_acceptable[i] = 1e100;
    }
    return cs;
}

void hsm_criteria_free(CriteriaSet* cs) {
    if (!cs) return;
    for (int i = 0; i < cs->n_criteria; i++)
        free(cs->criteria_names[i]);
    free(cs->criteria_names); free(cs->weights);
    free(cs->min_acceptable); free(cs->max_acceptable);
    free(cs->is_benefit); free(cs);
}

void hsm_criteria_set(CriteriaSet* cs, int idx, const char* name,
                       double weight, bool is_benefit) {
    if (!cs || idx < 0 || idx >= cs->n_criteria) return;
    free(cs->criteria_names[idx]);
    cs->criteria_names[idx] = strdup(name);
    cs->weights[idx] = weight;
    cs->is_benefit[idx] = is_benefit;
}

/* ---------- TradeStudyExt ---------- */

TradeStudyExt* hsm_trade_study_create(CriteriaSet* cs) {
    if (!cs) return NULL;
    TradeStudyExt* ts = (TradeStudyExt*)calloc(1, sizeof(TradeStudyExt));
    if (!ts) return NULL;
    ts->criteria = *cs;  /* Shallow copy; caller retains ownership */
    ts->alt_capacity = 8;
    ts->alternatives = (Alternative*)calloc(8, sizeof(Alternative));
    if (!ts->alternatives) { free(ts); return NULL; }
    return ts;
}

void hsm_trade_study_free(TradeStudyExt* ts) {
    if (!ts) return;
    for (int i = 0; i < ts->n_alternatives; i++) {
        free(ts->alternatives[i].name);
        free(ts->alternatives[i].scores);
    }
    free(ts->alternatives);
    free(ts);
}

int hsm_trade_add_alternative_ext(TradeStudyExt* ts, const char* name,
                               double* scores) {
    if (!ts || !name || !scores) return -1;
    if (ts->n_alternatives >= ts->alt_capacity) {
        int newcap = ts->alt_capacity * 2;
        Alternative* tmp = (Alternative*)realloc(ts->alternatives,
                                   (size_t)newcap * sizeof(Alternative));
        if (!tmp) return -1;
        ts->alternatives = tmp;
        ts->alt_capacity = newcap;
    }
    int idx = ts->n_alternatives++;
    Alternative* alt = &ts->alternatives[idx];
    memset(alt, 0, sizeof(Alternative));
    alt->name = strdup(name);
    alt->scores = (double*)malloc(
        (size_t)ts->criteria.n_criteria * sizeof(double));
    if (!alt->name || !alt->scores) {
        free(alt->name); free(alt->scores);
        ts->n_alternatives--;
        return -1;
    }
    memcpy(alt->scores, scores,
           (size_t)ts->criteria.n_criteria * sizeof(double));
    return idx;
}

/* ---------- Weighted Sum Model (WSM) ---------- */

/* Score = sum(w_i * s_i). Each score s_i is normalized to [0, 1].
 * For benefit criteria: norm = (s - min) / (max - min).
 * For cost criteria: norm = (max - s) / (max - min). */
void hsm_evaluate_wsm(TradeStudyExt* ts) {
    if (!ts || ts->n_alternatives <= 0) return;
    int n_alt = ts->n_alternatives;
    int n_crit = ts->criteria.n_criteria;

    /* Find min/max for each criterion */
    double* mins = (double*)malloc((size_t)n_crit * sizeof(double));
    double* maxs = (double*)malloc((size_t)n_crit * sizeof(double));
    if (!mins || !maxs) { free(mins); free(maxs); return; }
    for (int j = 0; j < n_crit; j++) {
        mins[j] =  INFINITY;
        maxs[j] = -INFINITY;
        for (int i = 0; i < n_alt; i++) {
            double s = ts->alternatives[i].scores[j];
            if (s < mins[j]) mins[j] = s;
            if (s > maxs[j]) maxs[j] = s;
        }
    }

    /* Compute weighted scores */
    for (int i = 0; i < n_alt; i++) {
        double ws = 0.0;
        for (int j = 0; j < n_crit; j++) {
            double range = maxs[j] - mins[j];
            double norm;
            if (fabs(range) < 1e-10) {
                norm = 1.0;
            } else if (ts->criteria.is_benefit[j]) {
                norm = (ts->alternatives[i].scores[j] - mins[j]) / range;
            } else {
                norm = (maxs[j] - ts->alternatives[i].scores[j]) / range;
            }
            if (norm < 0.0) norm = 0.0;
            if (norm > 1.0) norm = 1.0;
            ws += ts->criteria.weights[j] * norm;
        }
        ts->alternatives[i].weighted_score = ws;
    }
    free(mins); free(maxs);
}

/* ---------- Weighted Product Model (WPM) ---------- */

/* Score = prod(s_i_j / s_baseline_j)^w_j
 * WPM avoids rank reversal issues of WSM.
 * Uses geometric mean of normalized ratios.
 * Baseline is the median alternative for each criterion. */
void hsm_evaluate_wpm(TradeStudyExt* ts) {
    if (!ts || ts->n_alternatives <= 0) return;
    int n_alt = ts->n_alternatives;
    int n_crit = ts->criteria.n_criteria;

    /* Compute median (baseline) for each criterion */
    double* baselines = (double*)malloc((size_t)n_crit * sizeof(double));
    if (!baselines) return;
    for (int j = 0; j < n_crit; j++) {
        double* col = (double*)malloc((size_t)n_alt * sizeof(double));
        if (!col) { free(baselines); return; }
        for (int i = 0; i < n_alt; i++)
            col[i] = ts->alternatives[i].scores[j];
        /* Simple sort for median */
        for (int a = 0; a < n_alt - 1; a++)
            for (int b = a + 1; b < n_alt; b++)
                if (col[a] > col[b]) {
                    double t = col[a]; col[a] = col[b]; col[b] = t;
                }
        baselines[j] = col[n_alt / 2];
        free(col);
    }

    /* Weighted product */
    for (int i = 0; i < n_alt; i++) {
        double wp = 1.0;
        for (int j = 0; j < n_crit; j++) {
            double ratio;
            if (fabs(baselines[j]) < 1e-10) {
                ratio = 1.0;
            } else if (ts->criteria.is_benefit[j]) {
                ratio = ts->alternatives[i].scores[j] / baselines[j];
            } else {
                ratio = baselines[j] / ts->alternatives[i].scores[j];
            }
            if (ratio < 1e-10) ratio = 1e-10;
            wp *= pow(ratio, ts->criteria.weights[j]);
        }
        ts->alternatives[i].weighted_score = wp;
    }
    free(baselines);
}

/* ---------- TOPSIS ---------- */

/* TOPSIS: best alternative has shortest distance to ideal AND
 * longest distance to anti-ideal solution.
 * Steps:
 *   1. Normalize decision matrix (vector normalization)
 *   2. Weighted normalized matrix
 *   3. Ideal (A*) and anti-ideal (A-) solutions
 *   4. Separation measures S_i* and S_i-
 *   5. Relative closeness C_i* = S_i- / (S_i* + S_i-) */
void hsm_evaluate_topsis(TradeStudyExt* ts) {
    if (!ts || ts->n_alternatives <= 0) return;
    int m = ts->n_alternatives;
    int n = ts->criteria.n_criteria;

    /* Allocation */
    double** norm = (double**)malloc((size_t)m * sizeof(double*));
    double* ideal = (double*)malloc((size_t)n * sizeof(double));
    double* anti_ideal = (double*)malloc((size_t)n * sizeof(double));
    if (!norm || !ideal || !anti_ideal) {
        free(norm); free(ideal); free(anti_ideal); return;
    }
    for (int i = 0; i < m; i++) {
        norm[i] = (double*)calloc((size_t)n, sizeof(double));
        if (!norm[i]) {
            for (int k = 0; k < i; k++) free(norm[k]);
            free(norm); free(ideal); free(anti_ideal); return;
        }
    }

    /* Step 1: Vector normalization: r_ij = x_ij / sqrt(sum(x_kj^2)) */
    for (int j = 0; j < n; j++) {
        double sum_sq = 0.0;
        for (int i = 0; i < m; i++)
            sum_sq += ts->alternatives[i].scores[j]
                    * ts->alternatives[i].scores[j];
        double denom = sqrt(sum_sq);
        if (denom < 1e-15) denom = 1.0;
        for (int i = 0; i < m; i++)
            norm[i][j] = ts->alternatives[i].scores[j] / denom;
    }

    /* Step 2: Weighted normalization: v_ij = w_j * r_ij */
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++)
            norm[i][j] *= ts->criteria.weights[j];

    /* Step 3: Ideal and anti-ideal */
    for (int j = 0; j < n; j++) {
        double best = -INFINITY, worst = INFINITY;
        for (int i = 0; i < m; i++) {
            if (norm[i][j] > best) best = norm[i][j];
            if (norm[i][j] < worst) worst = norm[i][j];
        }
        if (ts->criteria.is_benefit[j]) {
            ideal[j] = best; anti_ideal[j] = worst;
        } else {
            ideal[j] = worst; anti_ideal[j] = best;
        }
    }

    /* Step 4 & 5: Separation and closeness */
    for (int i = 0; i < m; i++) {
        double s_star = 0.0, s_minus = 0.0;
        for (int j = 0; j < n; j++) {
            s_star  += (norm[i][j] - ideal[j])
                     * (norm[i][j] - ideal[j]);
            s_minus += (norm[i][j] - anti_ideal[j])
                     * (norm[i][j] - anti_ideal[j]);
        }
        s_star = sqrt(s_star);
        s_minus = sqrt(s_minus);
        ts->alternatives[i].topsis_score =
            s_minus / (s_star + s_minus + 1e-15);
    }

    for (int i = 0; i < m; i++) free(norm[i]);
    free(norm); free(ideal); free(anti_ideal);
}

/* ---------- Sensitivity Analysis ---------- */

/* Tornado analysis: perturb each weight by +/-delta and measure
 * impact on each alternative's score. Fills sensitivity[][] where
 * sensitivity[i][j] = max change in score of alt i when weight j varies.
 * Caller allocates sensitivity as [n_alternatives][n_criteria].
 * Returns 0 on success. */
int hsm_sensitivity_tornado(TradeStudyExt* ts, double delta,
                             double** sensitivity) {
    if (!ts || !sensitivity || delta <= 0.0 || ts->n_alternatives <= 0)
        return -1;
    int m = ts->n_alternatives, n = ts->criteria.n_criteria;

    /* Save original weights and scores */
    double* orig_w = (double*)malloc((size_t)n * sizeof(double));
    double* base_scores = (double*)malloc((size_t)m * sizeof(double));
    if (!orig_w || !base_scores) {
        free(orig_w); free(base_scores); return -1;
    }
    memcpy(orig_w, ts->criteria.weights, (size_t)n * sizeof(double));
    hsm_evaluate_wsm(ts);
    for (int i = 0; i < m; i++)
        base_scores[i] = ts->alternatives[i].weighted_score;

    /* Perturb each weight */
    for (int j = 0; j < n; j++) {
        /* Increase weight j */
        memcpy(ts->criteria.weights, orig_w, (size_t)n * sizeof(double));
        ts->criteria.weights[j] = orig_w[j] + delta;
        if (ts->criteria.weights[j] > 1.0) ts->criteria.weights[j] = 1.0;
        /* Renormalize */
        double sum = 0.0;
        for (int k = 0; k < n; k++) sum += ts->criteria.weights[k];
        for (int k = 0; k < n; k++) ts->criteria.weights[k] /= sum;
        hsm_evaluate_wsm(ts);
        for (int i = 0; i < m; i++) {
            double diff = fabs(ts->alternatives[i].weighted_score
                               - base_scores[i]);
            if (sensitivity[i]) sensitivity[i][j] = diff;
        }
    }
    /* Restore */
    memcpy(ts->criteria.weights, orig_w, (size_t)n * sizeof(double));
    free(orig_w); free(base_scores);
    return 0;
}

/* ---------- Ranking ---------- */

/* Rank alternatives by weighted_score (descending).
 * Fills rank field of each Alternative. Returns number ranked. */
int hsm_rank_alternatives(TradeStudyExt* ts) {
    if (!ts || ts->n_alternatives <= 0) return 0;
    int m = ts->n_alternatives;
    /* Simple ranking by weighted_score */
    for (int i = 0; i < m; i++) {
        int rank = 1;
        for (int j = 0; j < m; j++) {
            if (ts->alternatives[j].weighted_score
                > ts->alternatives[i].weighted_score)
                rank++;
        }
        ts->alternatives[i].rank = rank;
    }
    return m;
}

/* ---------- Print ---------- */

void hsm_trade_study_print(TradeStudyExt* ts) {
    if (!ts) { printf("NULL\n"); return; }
    printf("========== Trade Study Results ==========\n");
    printf("Criteria: %d  Alternatives: %d\n\n",
           ts->criteria.n_criteria, ts->n_alternatives);
    printf("Alt Name          WSM Score  TOPSIS  Rank\n");
    printf("------------------------------------------\n");
    for (int i = 0; i < ts->n_alternatives; i++) {
        printf("%-18s %9.4f  %6.4f  %4d\n",
               ts->alternatives[i].name,
               ts->alternatives[i].weighted_score,
               ts->alternatives[i].topsis_score,
               ts->alternatives[i].rank);
    }
    printf("==========================================\n");
}
