#ifndef CST_POWER_H
#define CST_POWER_H
#include "cst_core.h"

typedef enum { CST_POWER_COERCIVE=0, CST_POWER_REWARD=1, CST_POWER_LEGITIMATE=2, CST_POWER_EXPERT=3, CST_POWER_REFERENT=4, CST_POWER_INFORMATIONAL=5 } CSTPowerType;
typedef struct { CSTSystem* sys; double* power_scores; int n_scores; double power_concentration; double power_distance; int n_dominant_actors; } CSTPowerAnalysis;

CSTPowerAnalysis* cst_power_create(CSTSystem* sys);
void cst_power_free(CSTPowerAnalysis* pa);
void cst_power_analyze(CSTPowerAnalysis* pa);
double cst_power_concentration_index(const CSTPowerAnalysis* pa);
double cst_power_distance_index(const CSTPowerAnalysis* pa);
int cst_power_dominant_count(const CSTPowerAnalysis* pa);
bool cst_power_is_democratic(const CSTPowerAnalysis* pa);
void cst_power_print(const CSTPowerAnalysis* pa);
#endif
