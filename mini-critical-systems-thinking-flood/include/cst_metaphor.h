#ifndef CST_METAPHOR_H
#define CST_METAPHOR_H
#include "cst_core.h"

/* Metaphor Analysis (Morgan, 1986; Flood & Jackson, 1991)
 * Organizational metaphors as lenses for creative problem structuring */
typedef struct {
    CSTSystem* sys;
    double machine_score, organism_score, brain_score;
    double culture_score, political_score, psychic_prison_score;
    double flux_score, instrument_score;
    CSTMetaphor dominant_metaphor;
    double dominant_confidence;
    int n_metaphors_used;
} CSTMetaphorAnalysis;

CSTMetaphorAnalysis* cst_metaphor_create(CSTSystem* sys);
void cst_metaphor_free(CSTMetaphorAnalysis* ma);
void cst_metaphor_assess_all(CSTMetaphorAnalysis* ma);
void cst_metaphor_select_dominant(CSTMetaphorAnalysis* ma);
const char* cst_metaphor_name(CSTMetaphor m);
const char* cst_metaphor_implication(CSTMetaphor m);
double cst_metaphor_pluralism(const CSTMetaphorAnalysis* ma);
bool cst_metaphor_is_novel(const CSTMetaphorAnalysis* ma, CSTMetaphor m);
void cst_metaphor_print(const CSTMetaphorAnalysis* ma);
#endif
