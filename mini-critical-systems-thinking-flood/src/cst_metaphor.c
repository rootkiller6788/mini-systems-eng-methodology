#include "cst_metaphor.h"
#include "cst_core.h"
#include "cst_boundary.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ============================================================================
 * Metaphor Analysis (Morgan 1986, Flood & Jackson 1991)
 *
 * Gareth Morgan's "Images of Organization" (1986) proposed that
 * organizations can be understood through multiple metaphorical lenses,
 * each revealing different aspects while hiding others.
 *
 * Eight core metaphors:
 *   1. Machine — efficiency, standardization, bureaucracy
 *   2. Organism — adaptation, growth, environmental fit
 *   3. Brain — learning, self-organization, holographic
 *   4. Culture — shared meaning, rituals, symbols
 *   5. Political — power, conflict, interests
 *   6. Psychic Prison — unconscious forces, repression
 *   7. Flux & Transformation — chaos, emergence, dialectics
 *   8. Instrument of Domination — exploitation, coercion
 *
 * Metaphor pluralism (using multiple metaphors) enhances
 * creative problem structuring in SSM/CST practice.
 * ============================================================================ */

/* Metaphor scores are assessed by evaluating how well each metaphor
 * describes the system. Machine score high → mechanistic rules;
 * Organism high → adaptive behavior; etc. */

CSTMetaphorAnalysis* cst_metaphor_create(CSTSystem* sys) {
    if (!sys) return NULL;
    CSTMetaphorAnalysis* ma = (CSTMetaphorAnalysis*)calloc(1,
                                   sizeof(CSTMetaphorAnalysis));
    if (!ma) return NULL;
    ma->sys = sys;
    ma->dominant_metaphor = CST_METAPHOR_ORGANISM;
    ma->dominant_confidence = 0.0;
    ma->n_metaphors_used = 8;
    return ma;
}

void cst_metaphor_free(CSTMetaphorAnalysis* ma) {
    free(ma);
}

/* Assess all eight Morgan metaphors for the system.
 * Each score is in [0, 1] based on heuristic criteria derived from
 * the system's boundary judgments, stakeholder diversity, and
 * structural properties. */
void cst_metaphor_assess_all(CSTMetaphorAnalysis* ma) {
    if (!ma || !ma->sys) return;

    CSTSystem* s = ma->sys;
    int n_stk = s->n_stakeholders;

    /* Machine: high standardization, clear hierarchy, rule-driven */
    ma->machine_score = (s->formality_level > 0.7) ? 0.9 :
                        (s->formality_level > 0.4) ? 0.5 : 0.2;

    /* Organism: adaptive, open to environment, growth-oriented */
    ma->organism_score = (s->adaptability > s->rigidity) ? 0.8 :
                          (s->adaptability > 0.3) ? 0.5 : 0.2;

    /* Brain: learning capacity, self-organization, information processing */
    ma->brain_score = (s->learning_capacity > 0.7) ? 0.9 :
                       (s->learning_capacity > 0.4) ? 0.5 : 0.1;

    /* Culture: shared values, symbolic richness, social cohesion */
    ma->culture_score = (n_stk > 3 && s->social_cohesion > 0.5) ? 0.8 :
                         (s->social_cohesion > 0.3) ? 0.5 : 0.2;

    /* Political: power differentials, conflicting interests */
    ma->political_score = (s->power_asymmetry > 0.5) ? 0.8 :
                           (s->power_asymmetry > 0.2) ? 0.5 : 0.1;

    /* Psychic Prison: defense mechanisms, ideology entrapment */
    ma->psychic_prison_score = (s->ideology_rigidity > 0.6) ? 0.8 :
                                (s->ideology_rigidity > 0.3) ? 0.5 : 0.1;

    /* Flux: dynamic instability, emergence, dialectical change */
    ma->flux_score = (s->change_rate > 0.5) ? 0.8 :
                      (s->change_rate > 0.2) ? 0.5 : 0.1;

    /* Instrument of Domination: exploitation, coercion, class divide */
    ma->instrument_score = (s->exploitation_index > 0.5) ? 0.8 :
                            (s->exploitation_index > 0.2) ? 0.5 : 0.1;
}

/* Select dominant metaphor: highest-scoring metaphor.
 * If multiple metaphors have the same max score, the first wins.
 * Confidence = score of dominant / sum of all scores (normalized). */
void cst_metaphor_select_dominant(CSTMetaphorAnalysis* ma) {
    if (!ma) return;
    double scores[] = {
        ma->machine_score, ma->organism_score, ma->brain_score,
        ma->culture_score, ma->political_score,
        ma->psychic_prison_score, ma->flux_score, ma->instrument_score
    };
    CSTMetaphor metaphors[] = {
        CST_METAPHOR_MACHINE, CST_METAPHOR_ORGANISM, CST_METAPHOR_BRAIN,
        CST_METAPHOR_CULTURE, CST_METAPHOR_POLITICAL,
        CST_METAPHOR_PSYCHIC_PRISON, CST_METAPHOR_FLUX,
        CST_METAPHOR_INSTRUMENT
    };
    double max_score = -1.0;
    int max_idx = 0;
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        if (scores[i] > max_score) { max_score = scores[i]; max_idx = i; }
        sum += scores[i];
    }
    ma->dominant_metaphor = metaphors[max_idx];
    ma->dominant_confidence = (sum > 0.0) ? max_score / sum : 0.0;
}

/* Human-readable name of each metaphor. */
const char* cst_metaphor_name(CSTMetaphor m) {
    switch (m) {
        case CST_METAPHOR_MACHINE:        return "Machine";
        case CST_METAPHOR_ORGANISM:       return "Organism";
        case CST_METAPHOR_BRAIN:          return "Brain";
        case CST_METAPHOR_CULTURE:        return "Culture";
        case CST_METAPHOR_POLITICAL:      return "Political System";
        case CST_METAPHOR_PSYCHIC_PRISON: return "Psychic Prison";
        case CST_METAPHOR_FLUX:           return "Flux & Transformation";
        case CST_METAPHOR_INSTRUMENT:     return "Instrument of Domination";
        default:                          return "Unknown";
    }
}

/* Practical implication of each metaphor for intervention design. */
const char* cst_metaphor_implication(CSTMetaphor m) {
    switch (m) {
        case CST_METAPHOR_MACHINE:
            return "Redesign structures, clarify roles, optimize processes";
        case CST_METAPHOR_ORGANISM:
            return "Improve environmental fit, enhance adaptability";
        case CST_METAPHOR_BRAIN:
            return "Enhance learning loops, distribute intelligence";
        case CST_METAPHOR_CULTURE:
            return "Surface shared values, reshape organizational meaning";
        case CST_METAPHOR_POLITICAL:
            return "Address power imbalances, negotiate interests";
        case CST_METAPHOR_PSYCHIC_PRISON:
            return "Surface unconscious constraints, challenge ideology";
        case CST_METAPHOR_FLUX:
            return "Embrace emergence, manage dialectical tensions";
        case CST_METAPHOR_INSTRUMENT:
            return "Confront exploitation, redistribute power";
        default:
            return "No specific implication";
    }
}

/* Metaphor pluralism: entropy of metaphor score distribution.
 * Higher pluralism = more even use of multiple metaphors.
 * Max pluralism = log2(8) when all metaphors equally weighted.
 * Returns value in [0, 1] (1 = most pluralistic). */
double cst_metaphor_pluralism(const CSTMetaphorAnalysis* ma) {
    if (!ma) return 0.0;
    double s[] = {
        ma->machine_score, ma->organism_score, ma->brain_score,
        ma->culture_score, ma->political_score,
        ma->psychic_prison_score, ma->flux_score, ma->instrument_score
    };
    double sum = 0.0;
    for (int i = 0; i < 8; i++) sum += s[i];
    if (sum < 1e-10) return 0.0;
    double H = 0.0;
    for (int i = 0; i < 8; i++) {
        double p = s[i] / sum;
        if (p > 1e-10) H -= p * log2(p);
    }
    double H_max = 3.0;  /* log2(8) */
    return (H_max > 0.0) ? H / H_max : 0.0;
}

/* Check if a metaphor is novel (not yet heavily used) for this analysis.
 * Returns true if the metaphor score is below the average. */
bool cst_metaphor_is_novel(const CSTMetaphorAnalysis* ma, CSTMetaphor m) {
    if (!ma) return false;
    double scores[] = {
        ma->machine_score, ma->organism_score, ma->brain_score,
        ma->culture_score, ma->political_score,
        ma->psychic_prison_score, ma->flux_score, ma->instrument_score
    };
    double sum = 0.0;
    for (int i = 0; i < 8; i++) sum += scores[i];
    double avg = sum / 8.0;
    double target = 0.0;
    switch (m) {
        case CST_METAPHOR_MACHINE:        target = ma->machine_score; break;
        case CST_METAPHOR_ORGANISM:       target = ma->organism_score; break;
        case CST_METAPHOR_BRAIN:          target = ma->brain_score; break;
        case CST_METAPHOR_CULTURE:        target = ma->culture_score; break;
        case CST_METAPHOR_POLITICAL:      target = ma->political_score; break;
        case CST_METAPHOR_PSYCHIC_PRISON: target = ma->psychic_prison_score; break;
        case CST_METAPHOR_FLUX:           target = ma->flux_score; break;
        case CST_METAPHOR_INSTRUMENT:     target = ma->instrument_score; break;
        default: return false;
    }
    return target < avg;
}

/* Print full metaphor analysis. */
void cst_metaphor_print(const CSTMetaphorAnalysis* ma) {
    if (!ma) { printf("NULL\n"); return; }
    printf("===== Metaphor Analysis =====\n");
    printf("Machine:        %.3f\n", ma->machine_score);
    printf("Organism:       %.3f\n", ma->organism_score);
    printf("Brain:          %.3f\n", ma->brain_score);
    printf("Culture:        %.3f\n", ma->culture_score);
    printf("Political:      %.3f\n", ma->political_score);
    printf("PsychicPrison:  %.3f\n", ma->psychic_prison_score);
    printf("Flux:           %.3f\n", ma->flux_score);
    printf("Instrument:     %.3f\n", ma->instrument_score);
    printf("Dominant: %s (confidence: %.2f)\n",
           cst_metaphor_name(ma->dominant_metaphor),
           ma->dominant_confidence);
    printf("Pluralism: %.3f\n", cst_metaphor_pluralism(ma));
    printf("=============================\n");
}
