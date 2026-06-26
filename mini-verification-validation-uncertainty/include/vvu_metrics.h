#ifndef VVU_METRICS_H
#define VVU_METRICS_H
#include <stddef.h>
#include <math.h>

/* absolute value element-wise transform */
int vvu_metrics_op0(int n, const double *x, double *out);
int vvu_metrics_batch0(int n, int m, const double *x, double *out);
/* square element-wise transform */
int vvu_metrics_op1(int n, const double *x, double *out);
int vvu_metrics_batch1(int n, int m, const double *x, double *out);
/* cube element-wise transform */
int vvu_metrics_op2(int n, const double *x, double *out);
int vvu_metrics_batch2(int n, int m, const double *x, double *out);
/* logistic sigmoid element-wise transform */
int vvu_metrics_op3(int n, const double *x, double *out);
int vvu_metrics_batch3(int n, int m, const double *x, double *out);
/* ReLU activation element-wise transform */
int vvu_metrics_op4(int n, const double *x, double *out);
int vvu_metrics_batch4(int n, int m, const double *x, double *out);
/* hyperbolic tangent element-wise transform */
int vvu_metrics_op5(int n, const double *x, double *out);
int vvu_metrics_batch5(int n, int m, const double *x, double *out);
/* softplus element-wise transform */
int vvu_metrics_op6(int n, const double *x, double *out);
int vvu_metrics_batch6(int n, int m, const double *x, double *out);
/* Gaussian RBF element-wise transform */
int vvu_metrics_op7(int n, const double *x, double *out);
int vvu_metrics_batch7(int n, int m, const double *x, double *out);
double vvu_metrics_mean(int n, const double *x);
double vvu_metrics_variance(int n, const double *x);
int vvu_metrics_minmax(int n, const double *x, double *out);
double vvu_metrics_sum(int n, const double *x);
double vvu_metrics_norm_l2(int n, const double *x);
double vvu_metrics_norm_l1(int n, const double *x);
double vvu_metrics_norm_linf(int n, const double *x);

#endif