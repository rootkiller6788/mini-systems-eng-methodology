#ifndef STAKEHOLDER_MCDM_H
#define STAKEHOLDER_MCDM_H
#include <stddef.h>
#include <math.h>

/* absolute value element-wise transform */
int stakeholder_mcdm_op0(int n, const double *x, double *out);
int stakeholder_mcdm_batch0(int n, int m, const double *x, double *out);
/* square element-wise transform */
int stakeholder_mcdm_op1(int n, const double *x, double *out);
int stakeholder_mcdm_batch1(int n, int m, const double *x, double *out);
/* cube element-wise transform */
int stakeholder_mcdm_op2(int n, const double *x, double *out);
int stakeholder_mcdm_batch2(int n, int m, const double *x, double *out);
/* logistic sigmoid element-wise transform */
int stakeholder_mcdm_op3(int n, const double *x, double *out);
int stakeholder_mcdm_batch3(int n, int m, const double *x, double *out);
/* ReLU activation element-wise transform */
int stakeholder_mcdm_op4(int n, const double *x, double *out);
int stakeholder_mcdm_batch4(int n, int m, const double *x, double *out);
double stakeholder_mcdm_mean(int n, const double *x);
double stakeholder_mcdm_variance(int n, const double *x);
int stakeholder_mcdm_minmax(int n, const double *x, double *out);
double stakeholder_mcdm_sum(int n, const double *x);
double stakeholder_mcdm_norm_l2(int n, const double *x);
double stakeholder_mcdm_norm_l1(int n, const double *x);
double stakeholder_mcdm_norm_linf(int n, const double *x);

#endif