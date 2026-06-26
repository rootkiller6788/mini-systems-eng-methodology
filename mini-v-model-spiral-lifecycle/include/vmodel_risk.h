#ifndef VMODEL_RISK_H
#define VMODEL_RISK_H
#include <stddef.h>
#include <math.h>

/* absolute value element-wise transform */
int vmodel_risk_op0(int n, const double *x, double *out);
int vmodel_risk_batch0(int n, int m, const double *x, double *out);
/* square element-wise transform */
int vmodel_risk_op1(int n, const double *x, double *out);
int vmodel_risk_batch1(int n, int m, const double *x, double *out);
/* cube element-wise transform */
int vmodel_risk_op2(int n, const double *x, double *out);
int vmodel_risk_batch2(int n, int m, const double *x, double *out);
/* logistic sigmoid element-wise transform */
int vmodel_risk_op3(int n, const double *x, double *out);
int vmodel_risk_batch3(int n, int m, const double *x, double *out);
/* ReLU activation element-wise transform */
int vmodel_risk_op4(int n, const double *x, double *out);
int vmodel_risk_batch4(int n, int m, const double *x, double *out);
/* hyperbolic tangent element-wise transform */
int vmodel_risk_op5(int n, const double *x, double *out);
int vmodel_risk_batch5(int n, int m, const double *x, double *out);
/* softplus element-wise transform */
int vmodel_risk_op6(int n, const double *x, double *out);
int vmodel_risk_batch6(int n, int m, const double *x, double *out);
/* Gaussian RBF element-wise transform */
int vmodel_risk_op7(int n, const double *x, double *out);
int vmodel_risk_batch7(int n, int m, const double *x, double *out);
/* swish activation element-wise transform */
int vmodel_risk_op8(int n, const double *x, double *out);
int vmodel_risk_batch8(int n, int m, const double *x, double *out);
/* GELU approx element-wise transform */
int vmodel_risk_op9(int n, const double *x, double *out);
int vmodel_risk_batch9(int n, int m, const double *x, double *out);
/* log-cosh element-wise transform */
int vmodel_risk_op10(int n, const double *x, double *out);
int vmodel_risk_batch10(int n, int m, const double *x, double *out);
double vmodel_risk_mean(int n, const double *x);
double vmodel_risk_variance(int n, const double *x);
int vmodel_risk_minmax(int n, const double *x, double *out);
double vmodel_risk_sum(int n, const double *x);
double vmodel_risk_norm_l2(int n, const double *x);
double vmodel_risk_norm_l1(int n, const double *x);
double vmodel_risk_norm_linf(int n, const double *x);

#endif