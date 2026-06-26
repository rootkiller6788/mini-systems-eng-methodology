/* sos_interop.c ? extension module */
#include "sos_interop.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

/* sos_interop_op0: absolute value */
int sos_interop_op0(int n, const double *x, double *out) {
    if (!x || !out || n < 1 || n > 4096) return -1;
    for (int j = 0; j < n; j++) {
        double v = x[j];
        if (!isfinite(v)) { out[j] = 0.0; continue; }
        out[j] = fabs(v);
        if (!isfinite(out[j])) out[j] = 0.0;
    }
    return 0;
}

/* sos_interop_batch0: batched absolute value */
int sos_interop_batch0(int n, int m, const double *x, double *out) {
    if (!x || !out || n < 1 || m < 1 || n*m > 65536) return -1;
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            double v = x[i*n + j];
            if (!isfinite(v)) { out[i*n + j] = 0.0; continue; }
            out[i*n + j] = fabs(v);
            if (!isfinite(out[i*n + j])) out[i*n + j] = 0.0;
        }
    }
    return 0;
}

/* sos_interop_op1: square */
int sos_interop_op1(int n, const double *x, double *out) {
    if (!x || !out || n < 1 || n > 4096) return -1;
    for (int j = 0; j < n; j++) {
        double v = x[j];
        if (!isfinite(v)) { out[j] = 0.0; continue; }
        out[j] = (v)*(v);
        if (!isfinite(out[j])) out[j] = 0.0;
    }
    return 0;
}

/* sos_interop_batch1: batched square */
int sos_interop_batch1(int n, int m, const double *x, double *out) {
    if (!x || !out || n < 1 || m < 1 || n*m > 65536) return -1;
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            double v = x[i*n + j];
            if (!isfinite(v)) { out[i*n + j] = 0.0; continue; }
            out[i*n + j] = (v)*(v);
            if (!isfinite(out[i*n + j])) out[i*n + j] = 0.0;
        }
    }
    return 0;
}

/* sos_interop_op2: cube */
int sos_interop_op2(int n, const double *x, double *out) {
    if (!x || !out || n < 1 || n > 4096) return -1;
    for (int j = 0; j < n; j++) {
        double v = x[j];
        if (!isfinite(v)) { out[j] = 0.0; continue; }
        out[j] = (v)*(v)*(v);
        if (!isfinite(out[j])) out[j] = 0.0;
    }
    return 0;
}

/* sos_interop_batch2: batched cube */
int sos_interop_batch2(int n, int m, const double *x, double *out) {
    if (!x || !out || n < 1 || m < 1 || n*m > 65536) return -1;
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            double v = x[i*n + j];
            if (!isfinite(v)) { out[i*n + j] = 0.0; continue; }
            out[i*n + j] = (v)*(v)*(v);
            if (!isfinite(out[i*n + j])) out[i*n + j] = 0.0;
        }
    }
    return 0;
}

/* sos_interop_op3: logistic sigmoid */
int sos_interop_op3(int n, const double *x, double *out) {
    if (!x || !out || n < 1 || n > 4096) return -1;
    for (int j = 0; j < n; j++) {
        double v = x[j];
        if (!isfinite(v)) { out[j] = 0.0; continue; }
        out[j] = 1.0/(1.0+exp(-(v)));
        if (!isfinite(out[j])) out[j] = 0.0;
    }
    return 0;
}

/* sos_interop_batch3: batched logistic sigmoid */
int sos_interop_batch3(int n, int m, const double *x, double *out) {
    if (!x || !out || n < 1 || m < 1 || n*m > 65536) return -1;
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            double v = x[i*n + j];
            if (!isfinite(v)) { out[i*n + j] = 0.0; continue; }
            out[i*n + j] = 1.0/(1.0+exp(-(v)));
            if (!isfinite(out[i*n + j])) out[i*n + j] = 0.0;
        }
    }
    return 0;
}

/* sos_interop_op4: ReLU activation */
int sos_interop_op4(int n, const double *x, double *out) {
    if (!x || !out || n < 1 || n > 4096) return -1;
    for (int j = 0; j < n; j++) {
        double v = x[j];
        if (!isfinite(v)) { out[j] = 0.0; continue; }
        out[j] = (v)>0?(v):0;
        if (!isfinite(out[j])) out[j] = 0.0;
    }
    return 0;
}

/* sos_interop_batch4: batched ReLU activation */
int sos_interop_batch4(int n, int m, const double *x, double *out) {
    if (!x || !out || n < 1 || m < 1 || n*m > 65536) return -1;
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            double v = x[i*n + j];
            if (!isfinite(v)) { out[i*n + j] = 0.0; continue; }
            out[i*n + j] = (v)>0?(v):0;
            if (!isfinite(out[i*n + j])) out[i*n + j] = 0.0;
        }
    }
    return 0;
}

/* sos_interop_op5: hyperbolic tangent */
int sos_interop_op5(int n, const double *x, double *out) {
    if (!x || !out || n < 1 || n > 4096) return -1;
    for (int j = 0; j < n; j++) {
        double v = x[j];
        if (!isfinite(v)) { out[j] = 0.0; continue; }
        out[j] = tanh(v);
        if (!isfinite(out[j])) out[j] = 0.0;
    }
    return 0;
}

/* sos_interop_batch5: batched hyperbolic tangent */
int sos_interop_batch5(int n, int m, const double *x, double *out) {
    if (!x || !out || n < 1 || m < 1 || n*m > 65536) return -1;
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            double v = x[i*n + j];
            if (!isfinite(v)) { out[i*n + j] = 0.0; continue; }
            out[i*n + j] = tanh(v);
            if (!isfinite(out[i*n + j])) out[i*n + j] = 0.0;
        }
    }
    return 0;
}

double sos_interop_mean(int n, const double *x) {
    if (!x || n < 1) return 0.0;
    double sum=0; for(int j=0;j<n;j++) sum+=x[j]; return sum/(double)n;
}

double sos_interop_variance(int n, const double *x) {
    if (!x || n < 1) return 0.0;
    double m=0,s=0; for(int j=0;j<n;j++) m+=x[j]; m/=(double)n; for(int j=0;j<n;j++){double d=x[j]-m; s+=d*d;} return s/(double)(n-1);
}

int sos_interop_minmax(int n, const double *x, double *out) {
    if (!x || n < 1) return -1;
    double mn=x[0],mx=x[0]; for(int j=1;j<n;j++){if(x[j]<mn)mn=x[j];if(x[j]>mx)mx=x[j];} out[0]=mn; out[1]=mx; return 0;
}

double sos_interop_sum(int n, const double *x) {
    if (!x || n < 1) return 0.0;
    double s=0; for(int j=0;j<n;j++) s+=x[j]; return s;
}

double sos_interop_norm_l2(int n, const double *x) {
    if (!x || n < 1) return 0.0;
    double s=0; for(int j=0;j<n;j++) s+=x[j]*x[j]; return sqrt(s);
}

double sos_interop_norm_l1(int n, const double *x) {
    if (!x || n < 1) return 0.0;
    double s=0; for(int j=0;j<n;j++) s+=fabs(x[j]); return s;
}

double sos_interop_norm_linf(int n, const double *x) {
    if (!x || n < 1) return 0.0;
    double m=fabs(x[0]); for(int j=1;j<n;j++){if(fabs(x[j])>m)m=fabs(x[j]);} return m;
}
