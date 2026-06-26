#ifndef MBSE_PARAMETRIC_H
#define MBSE_PARAMETRIC_H
#include "mbse_core.h"
/* ===================================================================
 * Parametric analysis for systems engineering decision support:
 *
 * Constraint evaluation: sum, product, power, ratio, average, max
 *   - Constraints capture mathematical relationships between
 *     system properties (mass budget, power budget, etc.)
 *
 * Trade Studies: weighted sum model (WSM) for multi-criteria
 * decision analysis. Each alternative scored 1-10 on each
 * criterion; weighted scores summed; ranking produced.
 * Sensitivity analysis available for weight perturbation.
 *
 * Measure of Effectiveness (MOE): aggregate metric computed
 * from the parametric model's constraint network.
 *
 * References: NASA SE Handbook Ch.5, INCOSE Decision Mgmt
 * =================================================================== */
typedef struct { int id, *input_ids, n_inputs, output_id; char* name, *expression; double* input_values; double result; } ParametricConstraint;
/*
 * Parametric analysis enables:
 *   - Multi-criteria trade studies with weighted scoring
 *   - Constraint definition and evaluation
 *   - Measures of Effectiveness (MOE) computation
 *   - Sensitivity analysis for design optimization
 *
 * Typical use: evaluate 3-5 design alternatives against
 * cost, performance, reliability, and schedule criteria.
 */
typedef struct { int id; char* name, *unit; double value, min, max; } Parameter;
typedef struct { int id; char* name; Parameter** params; int n_params; ParametricConstraint** constraints; int n_constraints; double overall_moe; } ParametricModel;
typedef struct { int n_alternatives, n_criteria; char** alt_names; double** scores, *weights, *total_score; int* ranking; } TradeStudy;
Parameter* mbse_param_create(const char* name, double val, double min, double max, const char* unit);
void mbse_param_free(Parameter* p);
ParametricConstraint* mbse_constraint_create(const char* name, const char* expr);
void mbse_constraint_free(ParametricConstraint* c);
double mbse_constraint_evaluate(ParametricConstraint* c, double* inputs);
ParametricModel* mbse_param_model_create(const char* name);
void mbse_param_model_free(ParametricModel* pm);
void mbse_param_model_add_param(ParametricModel* pm, Parameter* p);
void mbse_param_model_add_constraint(ParametricModel* pm, ParametricConstraint* c);
double mbse_param_model_evaluate_moe(ParametricModel* pm);
TradeStudy* mbse_trade_create(int n_alts, int n_criteria, const char** names);
void mbse_trade_free(TradeStudy* ts);
void mbse_trade_set_weight(TradeStudy* ts, int crit, double w);
void mbse_trade_set_score(TradeStudy* ts, int alt, int crit, double s);
void mbse_trade_compute(TradeStudy* ts);
int mbse_trade_best(TradeStudy* ts);
void mbse_trade_print(TradeStudy* ts);
#endif
