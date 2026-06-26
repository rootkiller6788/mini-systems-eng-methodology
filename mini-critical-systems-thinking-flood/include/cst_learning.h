#ifndef CST_LEARNING_H
#define CST_LEARNING_H
#include "cst_core.h"

typedef enum { CST_LOOP_SINGLE=0, CST_LOOP_DOUBLE=1, CST_LOOP_TRIPLE=2 } CSTLearningLoop;
typedef struct {
    CSTSystem* sys; CSTLearningLoop dominant_loop;
    double single_loop_depth, double_loop_depth, triple_loop_depth;
    double learning_rate, unlearning_rate; int n_mental_models_surfaced; bool is_learning_organization;
} CSTLearningSystem;

CSTLearningSystem* cst_learning_create(CSTSystem* sys);
void cst_learning_free(CSTLearningSystem* ls);
void cst_learning_assess(CSTLearningSystem* ls);
CSTLearningLoop cst_learning_dominant(const CSTLearningSystem* ls);
double cst_learning_senge_score(const CSTLearningSystem* ls);
bool cst_learning_is_learning_org(const CSTLearningSystem* ls);
void cst_learning_print(const CSTLearningSystem* ls);
#endif
