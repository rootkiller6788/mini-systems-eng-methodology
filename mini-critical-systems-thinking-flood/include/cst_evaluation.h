#ifndef CST_EVALUATION_H
#define CST_EVALUATION_H
#include "cst_core.h"

typedef enum { CST_EVAL_FORMATIVE=0, CST_EVAL_SUMMATIVE=1, CST_EVAL_DEVELOPMENTAL=2, CST_EVAL_EMPOWERMENT=3 } CSTEvaluationType;
typedef struct { CSTSystem* sys; CSTEvaluationType type; double relevance, effectiveness, efficiency, impact, sustainability; double overall_score; int n_findings; char** findings; } CSTEvaluation;

CSTEvaluation* cst_evaluation_create(CSTSystem* sys, CSTEvaluationType type);
void cst_evaluation_free(CSTEvaluation* ev);
void cst_evaluation_assess(CSTEvaluation* ev);
double cst_evaluation_oecd_score(const CSTEvaluation* ev);
bool cst_evaluation_is_successful(const CSTEvaluation* ev);
void cst_evaluation_add_finding(CSTEvaluation* ev, const char* finding);
void cst_evaluation_print(const CSTEvaluation* ev);
#endif
