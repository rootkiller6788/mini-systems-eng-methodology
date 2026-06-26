#ifndef CST_SOSM_H
#define CST_SOSM_H
#include "cst_core.h"

/* System of Systems Methodologies (Jackson & Keys, 1984; Flood & Jackson, 1991)
 * Grid: complexity (simple/complex) x relationship (unitary/pluralist/coercive) */
typedef struct {
    CSTComplexity complexity;
    CSTRelationship relationship;
    char* cell_name;
    char* ideal_methodology;
    char* fallback_methodology;
    double fit_score;
    int case_count;
} CSTSOSMCell;

typedef struct {
    CSTSOSMCell grid[3][3];  /* [complexity][relationship] */
    CSTSystem* sys;
    int best_row, best_col;
} CSTSOSM;

CSTSOSM* cst_sosm_create(CSTSystem* sys);
void cst_sosm_free(CSTSOSM* sosm);
void cst_sosm_classify(CSTSOSM* sosm);
void cst_sosm_find_best_cell(CSTSOSM* sosm);
const char* cst_sosm_best_methodology(const CSTSOSM* sosm);
double cst_sosm_fit(const CSTSOSM* sosm, int row, int col);
bool cst_sosm_needs_critical(const CSTSOSM* sosm);
bool cst_sosm_needs_soft(const CSTSOSM* sosm);
void cst_sosm_print(const CSTSOSM* sosm);
void cst_sosm_print_grid(const CSTSOSM* sosm);
void cst_sosm_track_context_shift(CSTSOSM* sosm, CSTComplexity nc, CSTRelationship nr);
int cst_sosm_methodology_count(void);
bool cst_sosm_is_coercive_cell(const CSTSOSM* sosm);
bool cst_sosm_is_pluralist_cell(const CSTSOSM* sosm);
bool cst_sosm_is_unitary_cell(const CSTSOSM* sosm);
double cst_sosm_context_stability(const CSTSOSM* sosm);

#endif /* CST_SOSM_H */