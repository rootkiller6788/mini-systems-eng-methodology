#include "mbse_core.h"
#include "mbse_parametric.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
Parameter* mbse_param_create(const char* name, double val, double min, double max, const char* unit){Parameter* p=(Parameter*)calloc(1,sizeof(Parameter));p->name=strdup(name);p->value=val;p->min=min;p->max=max;p->unit=strdup(unit?unit:"");return p;}
void mbse_param_free(Parameter* p){if(!p)return;free(p->name);free(p->unit);free(p);}
ParametricConstraint* mbse_constraint_create(const char* name, const char* expr){ParametricConstraint* c=(ParametricConstraint*)calloc(1,sizeof(ParametricConstraint));c->name=strdup(name);c->expression=strdup(expr?expr:"");return c;}
void mbse_constraint_free(ParametricConstraint* c){if(!c)return;free(c->name);free(c->expression);free(c->input_ids);free(c->input_values);free(c);}
double mbse_constraint_evaluate(ParametricConstraint* c, double* inputs){if(!c||!inputs)return 0.0;if(strstr(c->expression,"sum")){double s=0.0;for(int i=0;i<c->n_inputs;i++)s+=inputs[i];return s;}if(strstr(c->expression,"product")){double p=1.0;for(int i=0;i<c->n_inputs;i++)p*=inputs[i];return p;}if(strstr(c->expression,"power"))return inputs[0]*inputs[0];if(strstr(c->expression,"ratio"))return inputs[1]>0?inputs[0]/inputs[1]:0.0;if(strstr(c->expression,"average")){double s=0.0;for(int i=0;i<c->n_inputs;i++)s+=inputs[i];return s/c->n_inputs;}if(strstr(c->expression,"max")){double m=inputs[0];for(int i=1;i<c->n_inputs;i++)if(inputs[i]>m)m=inputs[i];return m;}return inputs[0];}
ParametricModel* mbse_param_model_create(const char* name){ParametricModel* pm=(ParametricModel*)calloc(1,sizeof(ParametricModel));pm->name=strdup(name);pm->params=(Parameter**)malloc(16*sizeof(Parameter*));pm->n_params=0;pm->constraints=(ParametricConstraint**)malloc(16*sizeof(ParametricConstraint*));pm->n_constraints=0;return pm;}
void mbse_param_model_free(ParametricModel* pm){if(!pm)return;free(pm->name);for(int i=0;i<pm->n_params;i++)mbse_param_free(pm->params[i]);free(pm->params);for(int i=0;i<pm->n_constraints;i++){free(pm->constraints[i]->name);free(pm->constraints[i]->expression);free(pm->constraints[i]);}free(pm->constraints);free(pm);}
void mbse_param_model_add_param(ParametricModel* pm, Parameter* p){if(pm)pm->params[pm->n_params++]=p;}
void mbse_param_model_add_constraint(ParametricModel* pm, ParametricConstraint* c){if(pm)pm->constraints[pm->n_constraints++]=c;}
double mbse_param_model_evaluate_moe(ParametricModel* pm){if(!pm||pm->n_constraints==0)return 0.0;double total=0.0;for(int i=0;i<pm->n_constraints;i++)if(pm->constraints[i]->n_inputs>0)total+=mbse_constraint_evaluate(pm->constraints[i],pm->constraints[i]->input_values);pm->overall_moe=total;return total;}
TradeStudy* mbse_trade_create(int n_alts, int n_criteria, const char** names){TradeStudy* ts=(TradeStudy*)calloc(1,sizeof(TradeStudy));ts->n_alternatives=n_alts;ts->n_criteria=n_criteria;ts->alt_names=(char**)malloc(n_alts*sizeof(char*));for(int i=0;i<n_alts;i++)ts->alt_names[i]=strdup(names[i]);ts->scores=(double**)malloc(n_alts*sizeof(double*));for(int i=0;i<n_alts;i++)ts->scores[i]=(double*)calloc(n_criteria,sizeof(double));ts->weights=(double*)calloc(n_criteria,sizeof(double));for(int i=0;i<n_criteria;i++)ts->weights[i]=1.0;ts->total_score=(double*)calloc(n_alts,sizeof(double));ts->ranking=(int*)malloc(n_alts*sizeof(int));return ts;}
void mbse_trade_free(TradeStudy* ts){if(!ts)return;for(int i=0;i<ts->n_alternatives;i++){free(ts->alt_names[i]);free(ts->scores[i]);}free(ts->alt_names);free(ts->scores);free(ts->weights);free(ts->total_score);free(ts->ranking);free(ts);}
void mbse_trade_set_weight(TradeStudy* ts, int crit, double w){if(ts&&crit>=0&&crit<ts->n_criteria)ts->weights[crit]=w;}
void mbse_trade_set_score(TradeStudy* ts, int alt, int crit, double s){if(ts&&alt>=0&&alt<ts->n_alternatives&&crit>=0&&crit<ts->n_criteria)ts->scores[alt][crit]=s;}
void mbse_trade_compute(TradeStudy* ts){if(!ts)return;double wsum=0.0;for(int j=0;j<ts->n_criteria;j++)wsum+=ts->weights[j];for(int i=0;i<ts->n_alternatives;i++){ts->total_score[i]=0.0;for(int j=0;j<ts->n_criteria;j++)ts->total_score[i]+=ts->scores[i][j]*ts->weights[j];ts->total_score[i]/=wsum;ts->ranking[i]=i;}for(int i=0;i<ts->n_alternatives;i++)for(int j=i+1;j<ts->n_alternatives;j++)if(ts->total_score[ts->ranking[j]]>ts->total_score[ts->ranking[i]]){int t=ts->ranking[i];ts->ranking[i]=ts->ranking[j];ts->ranking[j]=t;}}
int mbse_trade_best(TradeStudy* ts){return ts&&ts->n_alternatives>0?ts->ranking[0]:-1;}
void mbse_trade_sensitivity(TradeStudy* ts, int crit, double* out){if(!ts||!out||crit<0||crit>=ts->n_criteria)return;double orig=ts->weights[crit];for(int s=-5;s<=5;s++){ts->weights[crit]=orig*(1.0+s*0.1);mbse_trade_compute(ts);out[s+5]=ts->total_score[0];}ts->weights[crit]=orig;mbse_trade_compute(ts);}
void mbse_trade_print(TradeStudy* ts){if(!ts)return;printf("Trade Study (%d alts x %d criteria):\n",ts->n_alternatives,ts->n_criteria);for(int r=0;r<ts->n_alternatives;r++){int i=ts->ranking[r];printf("  #%d: %s score=%.3f\n",r+1,ts->alt_names[i],ts->total_score[i]);}}
void mbse_trade_export_csv(TradeStudy* ts, const char* filename){if(!ts||!filename)return;FILE* f=fopen(filename,"w");if(!f)return;fprintf(f,"Alternative,Score\n");for(int i=0;i<ts->n_alternatives;i++)fprintf(f,"%s,%.3f\n",ts->alt_names[i],ts->total_score[i]);fclose(f);}
bool mbse_param_in_bounds(Parameter* p){if(!p)return false;return p->value>=p->min&&p->value<=p->max;}
int mbse_param_model_count_params(ParametricModel* pm){return pm?pm->n_params:0;}
double mbse_param_model_sensitivity(ParametricModel* pm, int param_idx, double delta){if(!pm||param_idx<0||param_idx>=pm->n_params)return 0.0;double orig=pm->params[param_idx]->value;pm->params[param_idx]->value+=delta;double moe=mbse_param_model_evaluate_moe(pm);pm->params[param_idx]->value=orig;return (moe-pm->overall_moe)/delta;}

int mbse_trade_pareto_optimal(TradeStudy* ts, bool* is_pareto) {if(!ts||!is_pareto)return 0;int n=ts->n_alternatives,m=ts->n_criteria;for(int i=0;i<n;i++){is_pareto[i]=true;for(int j=0;j<n;j++){if(i==j)continue;bool dominated=true,strictly=false;for(int k=0;k<m;k++){if(ts->scores[j][k]<ts->scores[i][k]){dominated=false;break;}if(ts->scores[j][k]>ts->scores[i][k])strictly=true;}if(dominated&&strictly){is_pareto[i]=false;break;}}}int cnt=0;for(int i=0;i<n;i++)if(is_pareto[i])cnt++;return cnt;}
void mbse_trade_topsis(TradeStudy* ts, double* ideal, double* anti_ideal) {if(!ts)return;int n=ts->n_alternatives,m=ts->n_criteria;for(int j=0;j<m;j++){ideal[j]=-1e9;anti_ideal[j]=1e9;for(int i=0;i<n;i++){if(ts->scores[i][j]>ideal[j])ideal[j]=ts->scores[i][j];if(ts->scores[i][j]<anti_ideal[j])anti_ideal[j]=ts->scores[i][j];}}}

/* TOPSIS distance computation */
void mbse_trade_topsis_scores(TradeStudy* ts, double* ideal, double* anti_ideal, double* scores) {
    if(!ts||!ideal||!anti_ideal||!scores)return;int n=ts->n_alternatives,m=ts->n_criteria;
    for(int i=0;i<n;i++){double d_plus=0.0,d_minus=0.0;
        for(int j=0;j<m;j++){double diff_plus=ts->scores[i][j]-ideal[j];double diff_minus=ts->scores[i][j]-anti_ideal[j];
            d_plus+=diff_plus*diff_plus;d_minus+=diff_minus*diff_minus;}
        d_plus=sqrt(d_plus);d_minus=sqrt(d_minus);
        scores[i]=(d_plus+d_minus)>1e-12?d_minus/(d_plus+d_minus):0.5;}
}

/* Monte Carlo simulation for trade study robustness */
void mbse_trade_monte_carlo(TradeStudy* ts, int n_trials, double noise_std, double* robust_scores) {
    if(!ts||!robust_scores)return;int n=ts->n_alternatives,m=ts->n_criteria;
    for(int i=0;i<n;i++)robust_scores[i]=0.0;
    double** orig=(double**)malloc(n*sizeof(double*));for(int i=0;i<n;i++){orig[i]=(double*)malloc(m*sizeof(double));memcpy(orig[i],ts->scores[i],m*sizeof(double));}
    for(int trial=0;trial<n_trials;trial++){for(int i=0;i<n;i++)for(int j=0;j<m;j++)ts->scores[i][j]=orig[i][j]+((double)rand()/RAND_MAX-0.5)*2.0*noise_std;
        mbse_trade_compute(ts);for(int i=0;i<n;i++)robust_scores[i]+=ts->total_score[i];}
    for(int i=0;i<n;i++)robust_scores[i]/=n_trials;
    for(int i=0;i<n;i++){memcpy(ts->scores[i],orig[i],m*sizeof(double));free(orig[i]);}free(orig);mbse_trade_compute(ts);
}

/* Parametric design space exploration */
void mbse_param_design_space(ParametricModel* pm, double* param_values, int n_samples, double* moe_output) {
    if(!pm||!param_values||!moe_output)return;
    for(int s=0;s<n_samples;s++){for(int i=0;i<pm->n_params;i++)pm->params[i]->value=param_values[s*pm->n_params+i];moe_output[s]=mbse_param_model_evaluate_moe(pm);}
}

/* Constraint satisfaction check for all constraints */
bool mbse_param_all_satisfied(ParametricModel* pm, double* thresholds) {
    if(!pm||!thresholds)return false;for(int i=0;i<pm->n_constraints;i++){double val=mbse_constraint_evaluate(pm->constraints[i],pm->constraints[i]->input_values);if(val<thresholds[i])return false;}return true;
}

/* Trade study correlation matrix between criteria */
void mbse_trade_correlation(TradeStudy* ts, double** corr_matrix) {
    if(!ts||!corr_matrix)return;int n=ts->n_alternatives,m=ts->n_criteria;
    double* means=(double*)calloc(m,sizeof(double));double* stds=(double*)calloc(m,sizeof(double));
    for(int j=0;j<m;j++){for(int i=0;i<n;i++)means[j]+=ts->scores[i][j];means[j]/=n;}
    for(int j=0;j<m;j++){for(int i=0;i<n;i++){double d=ts->scores[i][j]-means[j];stds[j]+=d*d;}stds[j]=sqrt(stds[j]/n);}
    for(int j=0;j<m;j++)for(int k=0;k<m;k++){double cov=0.0;for(int i=0;i<n;i++)cov+=(ts->scores[i][j]-means[j])*(ts->scores[i][k]-means[k]);cov/=n;corr_matrix[j][k]=(stds[j]*stds[k]>1e-12)?cov/(stds[j]*stds[k]):0.0;}
    free(means);free(stds);
}

/* Weight elicitation via pairwise comparison (AHP) */
void mbse_param_ahp_pairwise(double** pairwise, int n, double* weights) {
    if(!pairwise||!weights||n<2)return;double* colsum=(double*)calloc(n,sizeof(double));
    for(int j=0;j<n;j++)for(int i=0;i<n;i++)colsum[j]+=pairwise[i][j];
    for(int i=0;i<n;i++){double rs=0.0;for(int j=0;j<n;j++)rs+=pairwise[i][j]/colsum[j];weights[i]=rs/n;}
    free(colsum);
}

/* ============================================================================
 * Parametric Solvers — Linear, Newton-Raphson, Fixed-Point
 * ============================================================================ */

/* Evaluate linear expression: result = sum(coeff[i]*vars[i]) + constant.
 * coeff[n_vars], vars[n_vars], coeff[n_vars] is the constant term. */
double mbse_eval_linear_constraint(double* coeff, double* vars, int n_vars) {
    if (!coeff || !vars || n_vars < 1) return 0.0;
    double result = coeff[n_vars];
    for (int i = 0; i < n_vars; i++)
        result += coeff[i] * vars[i];
    return result;
}

/* Solve Ax=b via Gaussian elimination with partial pivoting.
 * A[n*n] row-major, b[n], x[n] output. Returns 0 if OK, -1 if singular. */
int mbse_solve_linear_system(double* A, double* b, double* x, int n) {
    if (!A || !b || !x || n <= 0) return -1;
    double* aug = (double*)malloc((size_t)(n*(n+1)) * sizeof(double));
    if (!aug) return -1;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) aug[i*(n+1)+j] = A[i*n+j];
        aug[i*(n+1)+n] = b[i];
    }
    for (int col = 0; col < n; col++) {
        int mr = col; double mv = fabs(aug[col*(n+1)+col]);
        for (int r = col+1; r < n; r++) {
            double v = fabs(aug[r*(n+1)+col]);
            if (v > mv) { mv = v; mr = r; }
        }
        if (mv < 1e-12) { free(aug); return -1; }
        if (mr != col)
            for (int j = 0; j <= n; j++) {
                double t = aug[col*(n+1)+j];
                aug[col*(n+1)+j] = aug[mr*(n+1)+j];
                aug[mr*(n+1)+j] = t;
            }
        double pv = aug[col*(n+1)+col];
        for (int r = col+1; r < n; r++) {
            double f = aug[r*(n+1)+col] / pv;
            for (int j = col; j <= n; j++)
                aug[r*(n+1)+j] -= f * aug[col*(n+1)+j];
        }
    }
    for (int i = n-1; i >= 0; i--) {
        double s = aug[i*(n+1)+n];
        for (int j = i+1; j < n; j++)
            s -= aug[i*(n+1)+j] * x[j];
        x[i] = s / aug[i*(n+1)+i];
    }
    free(aug); return 0;
}

/* Newton-Raphson for F(x)=0. F(x,n,out) fills out=F(x).
 * J(x,n,jac) fills jac[n*n] row-major Jacobian.
 * x[n] in/out. Returns iterations or -1 if diverged. */
int mbse_newton_raphson(void (*F)(double*,int,double*),
                          void (*J)(double*,int,double*),
                          double* x, int n, int max_iter, double tol) {
    if (!F || !J || !x || n <= 0) return -1;
    double* Fx = (double*)malloc((size_t)n*sizeof(double));
    double* Jx = (double*)malloc((size_t)(n*n)*sizeof(double));
    double* dx = (double*)malloc((size_t)n*sizeof(double));
    if (!Fx || !Jx || !dx) { free(Fx);free(Jx);free(dx);return -1; }
    for (int iter = 0; iter < max_iter; iter++) {
        F(x, n, Fx); J(x, n, Jx);
        double* nF = (double*)malloc((size_t)n*sizeof(double));
        if (nF) {
            for (int i = 0; i < n; i++) nF[i] = -Fx[i];
            if (mbse_solve_linear_system(Jx, nF, dx, n) != 0) { free(nF);break; }
            free(nF);
        }
        double md = 0.0;
        for (int i = 0; i < n; i++) {
            x[i] += dx[i];
            if (fabs(dx[i]) > md) md = fabs(dx[i]);
        }
        if (md < tol) { free(Fx);free(Jx);free(dx);return iter+1; }
    }
    free(Fx);free(Jx);free(dx);return -1;
}

/* Fixed-point iteration: x_{k+1} = g(x_k). */
int mbse_fixed_point_iter(double (*g)(double*,int,int),
                            double* x, int n, int max_iter, double tol) {
    if (!g || !x || n <= 0) return -1;
    double* xn = (double*)malloc((size_t)n*sizeof(double));
    if (!xn) return -1;
    for (int iter = 0; iter < max_iter; iter++) {
        double mc = 0.0;
        for (int i = 0; i < n; i++) {
            xn[i] = g(x, n, i);
            double dc = fabs(xn[i] - x[i]);
            if (dc > mc) mc = dc;
        }
        memcpy(x, xn, (size_t)n*sizeof(double));
        if (mc < tol) { free(xn); return iter+1; }
    }
    free(xn); return -1;
}



/* Parametric constraint solver: forward evaluation of constraint network.
 * Constraints define relationships between system parameters.
 * Returns 0 if all constraints are satisfied, 1+ if violated. */
int mbse_eval_constraints(int n_constraints, const double* params,
                           const double* constraint_values, const double* tolerances) {
    if (!params || !constraint_values || !tolerances || n_constraints <= 0) return -1;
    int violated = 0;
    for (int i = 0; i < n_constraints; i++) {
        double error = constraint_values[i];
        if (error < -tolerances[i] || error > tolerances[i]) violated++;
    }
    return violated;
}

/* Parametric sensitivity: compute Jacobian of constraints w.r.t. parameters
 * using finite differences. */
double mbse_parametric_sensitivity(int n_params, const double* params, int param_idx,
    double delta, double (*eval)(const double*, int)) {
    if (!params || !eval || param_idx < 0 || param_idx >= n_params) return 0.0;
    double* params_pert = malloc(n_params * sizeof(double));
    memcpy(params_pert, params, n_params * sizeof(double));
    params_pert[param_idx] += delta;
    double y_hi = eval(params_pert, n_params);
    params_pert[param_idx] -= 2.0 * delta;
    double y_lo = eval(params_pert, n_params);
    free(params_pert);
    return (y_hi - y_lo) / (2.0 * delta + 1e-10);
}
