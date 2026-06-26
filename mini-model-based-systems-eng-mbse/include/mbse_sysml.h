#ifndef MBSE_SYSML_H
#define MBSE_SYSML_H
#include <stddef.h>
#include <math.h>

/* absolute value element-wise transform */
int mbse_sysml_op0(int n, const double *x, double *out);
int mbse_sysml_batch0(int n, int m, const double *x, double *out);
/* square element-wise transform */
int mbse_sysml_op1(int n, const double *x, double *out);
int mbse_sysml_batch1(int n, int m, const double *x, double *out);
/* cube element-wise transform */
int mbse_sysml_op2(int n, const double *x, double *out);
int mbse_sysml_batch2(int n, int m, const double *x, double *out);
/* logistic sigmoid element-wise transform */
int mbse_sysml_op3(int n, const double *x, double *out);
int mbse_sysml_batch3(int n, int m, const double *x, double *out);
/* ReLU activation element-wise transform */
int mbse_sysml_op4(int n, const double *x, double *out);
int mbse_sysml_batch4(int n, int m, const double *x, double *out);
double mbse_sysml_mean(int n, const double *x);
double mbse_sysml_variance(int n, const double *x);
int mbse_sysml_minmax(int n, const double *x, double *out);
double mbse_sysml_sum(int n, const double *x);
double mbse_sysml_norm_l2(int n, const double *x);
double mbse_sysml_norm_l1(int n, const double *x);
double mbse_sysml_norm_linf(int n, const double *x);

#endif