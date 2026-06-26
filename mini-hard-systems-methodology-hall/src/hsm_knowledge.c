#include "hsm_knowledge.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

HSMKnowledgeBase* hsm_kb_create(void) {
    HSMKnowledgeBase* kb = calloc(1, sizeof(HSMKnowledgeBase));
    if (!kb) return NULL;
    kb->tool_coverage = malloc(HSM_NUM_PHASES * sizeof(double*));
    for (int p = 0; p < HSM_NUM_PHASES; p++) { kb->tool_coverage[p] = calloc(HSM_NUM_STEPS, sizeof(double)); }
    return kb;
}
void hsm_kb_free(HSMKnowledgeBase* kb) {
    if (!kb) return;
    for (int d = 0; d < HSM_NUM_DISCIPLINES; d++) { for (int t = 0; t < kb->profiles[d].n_tools; t++) { free(kb->profiles[d].tools[t].name); free(kb->profiles[d].tools[t].description); free(kb->profiles[d].tools[t].category); } free(kb->profiles[d].tools); free(kb->profiles[d].body_of_knowledge); free(kb->profiles[d].standards); }
    for (int p = 0; p < HSM_NUM_PHASES; p++) free(kb->tool_coverage[p]);
    free(kb->tool_coverage); free(kb);
}
void hsm_kb_init_profile(HSMKnowledgeBase* kb, HSMDiscipline disc, const char* bok, const char* std) {
    if (!kb || disc < 0 || disc >= HSM_NUM_DISCIPLINES) return;
    HSMDisciplineProfile* p = &kb->profiles[disc];
    p->discipline = disc; p->body_of_knowledge = bok ? strdup(bok) : NULL; p->standards = std ? strdup(std) : NULL;
    p->tool_capacity = 16; p->tools = malloc(16 * sizeof(HSMTool));
}
HSMTool* hsm_kb_add_tool(HSMKnowledgeBase* kb, HSMDiscipline disc, const char* name, const char* desc, const char* cat) {
    if (!kb || disc < 0 || disc >= HSM_NUM_DISCIPLINES || !name) return NULL;
    HSMDisciplineProfile* p = &kb->profiles[disc];
    if (p->n_tools >= p->tool_capacity) { p->tool_capacity *= 2; p->tools = realloc(p->tools, p->tool_capacity * sizeof(HSMTool)); }
    HSMTool* t = &p->tools[p->n_tools++];
    memset(t, 0, sizeof(HSMTool));
    t->name = strdup(name); t->description = desc ? strdup(desc) : NULL; t->category = cat ? strdup(cat) : NULL;
    return t;
}
void hsm_kb_set_tool_applicability(HSMTool* t, HSMPhase phase, HSMLogicStep step, double app) { if (t && phase >= 0 && phase < HSM_NUM_PHASES && step >= 0 && step < HSM_NUM_STEPS) t->applicability[phase][step] = app; }
int hsm_kb_tools_for_cell(HSMKnowledgeBase* kb, HSMPhase p, HSMLogicStep s, HSMTool** results, int max) {
    if (!kb || !results || max <= 0) return 0; int count = 0;
    for (int d = 0; d < HSM_NUM_DISCIPLINES && count < max; d++)
        for (int t = 0; t < kb->profiles[d].n_tools && count < max; t++)
            if (kb->profiles[d].tools[t].applicability[p][s] > 0.5) results[count++] = &kb->profiles[d].tools[t];
    return count;
}
double hsm_kb_coverage(HSMKnowledgeBase* kb, HSMPhase p, HSMLogicStep s) {
    if (!kb) return 0.0; int total = 0; for (int d = 0; d < HSM_NUM_DISCIPLINES; d++) total += kb->profiles[d].n_tools;
    int found = 0;
    for (int d = 0; d < HSM_NUM_DISCIPLINES; d++) for (int t = 0; t < kb->profiles[d].n_tools; t++) if (kb->profiles[d].tools[t].applicability[p][s] > 0.5) found++;
    return total > 0 ? (double)found / total : 0.0;
}
void hsm_kb_print_coverage_matrix(HSMKnowledgeBase* kb) {
    if (!kb) return;
    printf("=== Knowledge Coverage Matrix ===\n%22s", "Phase/Step");
    for (int s = 0; s < HSM_NUM_STEPS; s++) printf(" S%d", s + 1); printf("\n");
    for (int p = 0; p < HSM_NUM_PHASES; p++) {
        printf("%-22s", hsm_phase_name((HSMPhase)p));
        for (int s = 0; s < HSM_NUM_STEPS; s++) printf(" %2.0f", hsm_kb_coverage(kb, (HSMPhase)p, (HSMLogicStep)s) * 100);
        printf("\n");
    }
}
void hsm_kb_print_discipline(HSMKnowledgeBase* kb, HSMDiscipline disc) {
    if (!kb || disc < 0 || disc >= HSM_NUM_DISCIPLINES) return;
    HSMDisciplineProfile* p = &kb->profiles[disc];
    printf("Discipline: %s\n  BoK: %s\n  Standards: %s\n  Tools: %d\n",
           hsm_discipline_name(disc),
           p->body_of_knowledge ? p->body_of_knowledge : "N/A",
           p->standards ? p->standards : "N/A", p->n_tools);
    for (int i = 0; i < p->n_tools; i++)
        printf("    %s (%s)\n", p->tools[i].name,
               p->tools[i].category ? p->tools[i].category : "");
}

/* ============================================================================
 * Extended Knowledge Base Engineering (KBE) Functions
 *
 * KBE extends the basic knowledge base with:
 *   - Cross-discipline synergy analysis
 *   - Technology readiness assessment (TRA)
 *   - Knowledge gap identification
 *   - Tool maturity evaluation
 * ============================================================================ */

/* Technology Readiness Level (TRL) mapping.
 * TRL 1: Basic principles observed
 * TRL 2: Technology concept formulated
 * TRL 3: Experimental proof of concept
 * TRL 4: Technology validated in lab
 * TRL 5: Technology validated in relevant environment
 * TRL 6: Technology demonstrated in relevant environment
 * TRL 7: System prototype in operational environment
 * TRL 8: System complete and qualified
 * TRL 9: Actual system proven in operational environment
 */
const char* hsm_trl_description(int trl) {
    static const char* desc[] = {
        "Basic principles observed",
        "Technology concept formulated",
        "Experimental proof of concept",
        "Validated in laboratory",
        "Validated in relevant environment",
        "Demonstrated in relevant environment",
        "Prototype in operational environment",
        "System complete and qualified",
        "Actual system proven in operations"
    };
    if (trl < 1 || trl > 9) return "Unknown TRL";
    return desc[trl - 1];
}

/* Assess overall TRL of the knowledge base for a given phase.
 * Averages TRLs of all tools applicable to that phase.
 * Returns value in [1.0, 9.0]. */
double hsm_kb_phase_trl(HSMKnowledgeBase* kb, HSMPhase phase) {
    if (!kb) return 1.0;
    double sum = 0.0;
    int count = 0;
    for (int d = 0; d < HSM_NUM_DISCIPLINES; d++) {
        for (int t = 0; t < kb->profiles[d].n_tools; t++) {
            /* Tools applied to at least one step in this phase count */
            bool used = false;
            for (int s = 0; s < HSM_NUM_STEPS; s++) {
                if (kb->profiles[d].tools[t].applicability[phase][s] > 0.5)
                    used = true;
            }
            if (used) {
                sum += (double)kb->profiles[d].tools[t].maturity_level;
                count++;
            }
        }
    }
    return (count > 0) ? sum / (double)count : 1.0;
}

/* Identify knowledge gaps: cells with coverage below threshold.
 * Fills gap_phase[] and gap_step[] arrays (caller-allocated, size max_gaps).
 * Returns number of gaps found. */
int hsm_kb_identify_gaps(HSMKnowledgeBase* kb, double threshold,
                          HSMPhase* gap_phase, HSMLogicStep* gap_step,
                          int max_gaps) {
    if (!kb || !gap_phase || !gap_step || max_gaps <= 0) return 0;
    int count = 0;
    for (int p = 0; p < HSM_NUM_PHASES && count < max_gaps; p++) {
        for (int s = 0; s < HSM_NUM_STEPS && count < max_gaps; s++) {
            double cov = hsm_kb_coverage(kb, (HSMPhase)p, (HSMLogicStep)s);
            if (cov < threshold) {
                gap_phase[count] = (HSMPhase)p;
                gap_step[count] = (HSMLogicStep)s;
                count++;
            }
        }
    }
    return count;
}

/* Cross-discipline synergy: measure how many tools from different
 * disciplines are jointly applicable to the same cell.
 * Returns average synergy score in [0, 1]. */
double hsm_kb_cross_discipline_synergy(HSMKnowledgeBase* kb) {
    if (!kb) return 0.0;
    double total = 0.0;
    int cells = 0;
    for (int p = 0; p < HSM_NUM_PHASES; p++) {
        for (int s = 0; s < HSM_NUM_STEPS; s++) {
            int disciplines_used = 0;
            for (int d = 0; d < HSM_NUM_DISCIPLINES; d++) {
                for (int t = 0; t < kb->profiles[d].n_tools; t++) {
                    if (kb->profiles[d].tools[t].applicability[p][s] > 0.5) {
                        disciplines_used++;
                        break;
                    }
                }
            }
            total += (double)disciplines_used
                   / (double)HSM_NUM_DISCIPLINES;
            cells++;
        }
    }
    return (cells > 0) ? total / (double)cells : 0.0;
}


/* Knowledge transfer metrics: evaluate how well systems engineering
 * knowledge propagates across project phases (Hall 1969).
 * Returns transfer efficiency in [0, 1]. */
double knowledge_transfer_efficiency(int n_phases, const double* knowledge_at_phase,
                                      const double* knowledge_required) {
    if (!knowledge_at_phase || !knowledge_required || n_phases <= 0) return 0.0;
    double total_gap = 0.0;
    for (int i = 0; i < n_phases; i++) {
        double gap = knowledge_required[i] - knowledge_at_phase[i];
        if (gap > 0.0) total_gap += gap;
    }
    return 1.0 / (1.0 + total_gap / (double)n_phases);
}
