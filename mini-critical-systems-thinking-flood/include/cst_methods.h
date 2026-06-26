#ifndef CST_METHODS_H
#define CST_METHODS_H
#include <stddef.h>
#include <math.h>

/* absolute value element-wise transform */
int cst_methods_op0(int n, const double *x, double *out);
int cst_methods_batch0(int n, int m, const double *x, double *out);
/* square element-wise transform */
int cst_methods_op1(int n, const double *x, double *out);
int cst_methods_batch1(int n, int m, const double *x, double *out);
/* cube element-wise transform */
int cst_methods_op2(int n, const double *x, double *out);
int cst_methods_batch2(int n, int m, const double *x, double *out);
/* logistic sigmoid element-wise transform */
int cst_methods_op3(int n, const double *x, double *out);
int cst_methods_batch3(int n, int m, const double *x, double *out);
/* ReLU activation element-wise transform */
int cst_methods_op4(int n, const double *x, double *out);
int cst_methods_batch4(int n, int m, const double *x, double *out);
/* hyperbolic tangent element-wise transform */
int cst_methods_op5(int n, const double *x, double *out);
int cst_methods_batch5(int n, int m, const double *x, double *out);
/* softplus element-wise transform */
int cst_methods_op6(int n, const double *x, double *out);
int cst_methods_batch6(int n, int m, const double *x, double *out);
/* Gaussian RBF element-wise transform */
int cst_methods_op7(int n, const double *x, double *out);
int cst_methods_batch7(int n, int m, const double *x, double *out);
/* swish activation element-wise transform */
int cst_methods_op8(int n, const double *x, double *out);
int cst_methods_batch8(int n, int m, const double *x, double *out);
/* GELU approx element-wise transform */
int cst_methods_op9(int n, const double *x, double *out);
int cst_methods_batch9(int n, int m, const double *x, double *out);
double cst_methods_mean(int n, const double *x);
double cst_methods_variance(int n, const double *x);
int cst_methods_minmax(int n, const double *x, double *out);
double cst_methods_sum(int n, const double *x);
double cst_methods_norm_l2(int n, const double *x);
double cst_methods_norm_l1(int n, const double *x);
double cst_methods_norm_linf(int n, const double *x);

#endif