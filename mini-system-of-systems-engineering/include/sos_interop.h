#ifndef SOS_INTEROP_H
#define SOS_INTEROP_H
#include <stddef.h>
#include <math.h>

/* absolute value element-wise transform */
int sos_interop_op0(int n, const double *x, double *out);
int sos_interop_batch0(int n, int m, const double *x, double *out);
/* square element-wise transform */
int sos_interop_op1(int n, const double *x, double *out);
int sos_interop_batch1(int n, int m, const double *x, double *out);
/* cube element-wise transform */
int sos_interop_op2(int n, const double *x, double *out);
int sos_interop_batch2(int n, int m, const double *x, double *out);
/* logistic sigmoid element-wise transform */
int sos_interop_op3(int n, const double *x, double *out);
int sos_interop_batch3(int n, int m, const double *x, double *out);
/* ReLU activation element-wise transform */
int sos_interop_op4(int n, const double *x, double *out);
int sos_interop_batch4(int n, int m, const double *x, double *out);
/* hyperbolic tangent element-wise transform */
int sos_interop_op5(int n, const double *x, double *out);
int sos_interop_batch5(int n, int m, const double *x, double *out);
double sos_interop_mean(int n, const double *x);
double sos_interop_variance(int n, const double *x);
int sos_interop_minmax(int n, const double *x, double *out);
double sos_interop_sum(int n, const double *x);
double sos_interop_norm_l2(int n, const double *x);
double sos_interop_norm_l1(int n, const double *x);
double sos_interop_norm_linf(int n, const double *x);

#endif