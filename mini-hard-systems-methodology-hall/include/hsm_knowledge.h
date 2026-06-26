#ifndef HSM_KNOWLEDGE_H
#define HSM_KNOWLEDGE_H
#include "hsm_core.h"

/* ============================================================================
 * Knowledge Dimension — Disciplines, Tools, and Techniques
 *
 * The third dimension of Hall's morphology represents the entire body
 * of knowledge available to the systems engineer. Each discipline
 * brings specific tools and techniques applicable across phases and steps.
 * ============================================================================ */

typedef struct {
    char* name;
    char* description;
    char* category;           /* Analysis, Design, Management, Testing, etc. */
    int maturity_level;       /* TRL-like maturity (1-9) for this tool */
    double applicability[HSM_NUM_PHASES][HSM_NUM_STEPS];
} HSMTool;

typedef struct {
    HSMDiscipline discipline;
    HSMTool* tools;
    int n_tools;
    int tool_capacity;
    char* body_of_knowledge;
    char* standards;
} HSMDisciplineProfile;

/* Knowledge mapping: which tools apply to which phase/step */
typedef struct {
    HSMDisciplineProfile profiles[HSM_NUM_DISCIPLINES];
    double** tool_coverage;    /* [phase][step] = number of applicable tools */
    int n_disciplines_active;
} HSMKnowledgeBase;

/* API */
HSMKnowledgeBase* hsm_kb_create(void);
void hsm_kb_free(HSMKnowledgeBase* kb);
void hsm_kb_init_profile(HSMKnowledgeBase* kb, HSMDiscipline disc,
                           const char* bok, const char* standards);
HSMTool* hsm_kb_add_tool(HSMKnowledgeBase* kb, HSMDiscipline disc,
                           const char* name, const char* desc,
                           const char* category);
void hsm_kb_set_tool_applicability(HSMTool* tool, HSMPhase phase,
                                     HSMLogicStep step, double applicability);
int hsm_kb_tools_for_cell(HSMKnowledgeBase* kb, HSMPhase phase,
                            HSMLogicStep step, HSMTool** results, int max);
double hsm_kb_coverage(HSMKnowledgeBase* kb, HSMPhase phase,
                         HSMLogicStep step);
void hsm_kb_print_coverage_matrix(HSMKnowledgeBase* kb);
void hsm_kb_print_discipline(HSMKnowledgeBase* kb, HSMDiscipline disc);
#endif
