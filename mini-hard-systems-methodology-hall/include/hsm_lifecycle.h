#ifndef HSM_LIFECYCLE_H
#define HSM_LIFECYCLE_H
#include <stddef.h>
#include <math.h>

/* absolute value element-wise transform */
int hsm_lifecycle_op0(int n, const double *x, double *out);
int hsm_lifecycle_batch0(int n, int m, const double *x, double *out);
/* square element-wise transform */
int hsm_lifecycle_op1(int n, const double *x, double *out);
int hsm_lifecycle_batch1(int n, int m, const double *x, double *out);
/* cube element-wise transform */
int hsm_lifecycle_op2(int n, const double *x, double *out);
int hsm_lifecycle_batch2(int n, int m, const double *x, double *out);
/* logistic sigmoid element-wise transform */
int hsm_lifecycle_op3(int n, const double *x, double *out);
int hsm_lifecycle_batch3(int n, int m, const double *x, double *out);
/* ReLU activation element-wise transform */
int hsm_lifecycle_op4(int n, const double *x, double *out);
int hsm_lifecycle_batch4(int n, int m, const double *x, double *out);
/* hyperbolic tangent element-wise transform */
int hsm_lifecycle_op5(int n, const double *x, double *out);
int hsm_lifecycle_batch5(int n, int m, const double *x, double *out);
/* softplus element-wise transform */
int hsm_lifecycle_op6(int n, const double *x, double *out);
int hsm_lifecycle_batch6(int n, int m, const double *x, double *out);
/* Gaussian RBF element-wise transform */
int hsm_lifecycle_op7(int n, const double *x, double *out);
int hsm_lifecycle_batch7(int n, int m, const double *x, double *out);
/* swish activation element-wise transform */
int hsm_lifecycle_op8(int n, const double *x, double *out);
int hsm_lifecycle_batch8(int n, int m, const double *x, double *out);
double hsm_lifecycle_mean(int n, const double *x);
double hsm_lifecycle_variance(int n, const double *x);
int hsm_lifecycle_minmax(int n, const double *x, double *out);
double hsm_lifecycle_sum(int n, const double *x);
double hsm_lifecycle_norm_l2(int n, const double *x);
double hsm_lifecycle_norm_l1(int n, const double *x);
double hsm_lifecycle_norm_linf(int n, const double *x);

#endif