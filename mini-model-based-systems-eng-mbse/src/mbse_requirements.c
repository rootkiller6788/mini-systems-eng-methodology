#include "mbse_core.h"
#include "mbse_requirements.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

TraceabilityMatrix* mbse_trace_create(SystemModel* m){
    if(!m)return NULL;mbse_model_build_traceability(m);
    TraceabilityMatrix* tm=(TraceabilityMatrix*)calloc(1,sizeof(TraceabilityMatrix));
    tm->matrix=m->traceability_matrix;tm->n_rows=m->trace_rows;tm->n_cols=m->trace_cols;tm->n_total=m->n_requirements;
    for(int i=0;i<m->trace_rows;i++){bool has=false;for(int j=0;j<m->trace_cols;j++)if(tm->matrix[i][j]){has=true;break;}if(has)tm->n_covered++;}
    tm->coverage=(tm->n_total>0)?(double)tm->n_covered/tm->n_total:0.0;
    for(int i=0;i<m->n_requirements;i++){if(m->requirements[i].is_satisfied)tm->n_satisfied++;else tm->n_unsatisfied++;}
    return tm;
}
void mbse_trace_free(TraceabilityMatrix* tm){free(tm);}
double mbse_trace_coverage(TraceabilityMatrix* tm){return tm?tm->coverage:0.0;}
void mbse_trace_print(TraceabilityMatrix* tm){if(!tm)return;printf("Traceability: %d/%d covered (%.1f%%) Satisfied:%d/%d\n",tm->n_covered,tm->n_total,tm->coverage*100,tm->n_satisfied,tm->n_total);}
double mbse_trace_satisfaction_rate(SystemModel* m){if(!m||m->n_requirements==0)return 0.0;int s=0;for(int i=0;i<m->n_requirements;i++)if(m->requirements[i].is_satisfied)s++;return (double)s/m->n_requirements;}
int mbse_trace_find_orphans(SystemModel* m, int** orphan_ids){if(!m)return 0;int n=0;int* ids=(int*)malloc(m->n_requirements*sizeof(int));for(int i=0;i<m->n_requirements;i++){bool found=false;for(int j=0;j<m->n_blocks;j++)for(int k=0;k<m->blocks[j].allocated_req_count;k++)if(m->blocks[j].allocated_req_ids[k]==i){found=true;break;}if(!found)ids[n++]=i;}*orphan_ids=ids;return n;}
int mbse_trace_find_uncovered_reqs(SystemModel* m, int** uncovered){if(!m)return 0;int n=0;int* ids=(int*)malloc(m->n_requirements*sizeof(int));for(int i=0;i<m->n_requirements;i++)if(!m->requirements[i].is_satisfied)ids[n++]=i;*uncovered=ids;return n;}
void mbse_trace_export_matrix(TraceabilityMatrix* tm, const char* filename){if(!tm||!filename)return;FILE* f=fopen(filename,"w");if(!f)return;for(int i=0;i<tm->n_rows;i++){for(int j=0;j<tm->n_cols;j++)fprintf(f,"%d,",tm->matrix[i][j]);fprintf(f,"\n");}fclose(f);}
RequirementGapAnalysis* mbse_req_gap_analysis(SystemModel* m){if(!m)return NULL;RequirementGapAnalysis* ga=(RequirementGapAnalysis*)calloc(1,sizeof(RequirementGapAnalysis));ga->orphans=(MBSERequirement**)malloc(m->n_requirements*sizeof(MBSERequirement*));for(int i=0;i<m->n_requirements;i++){bool allocated=false;for(int j=0;j<m->n_blocks;j++)for(int k=0;k<m->blocks[j].allocated_req_count;k++)if(m->blocks[j].allocated_req_ids[k]==i){allocated=true;break;}if(!allocated)ga->orphans[ga->n_orphans++]=&m->requirements[i];}ga->completeness=(m->n_requirements>0)?1.0-(double)ga->n_orphans/m->n_requirements:1.0;return ga;}
void mbse_req_gap_free(RequirementGapAnalysis* ga){if(!ga)return;free(ga->orphans);free(ga);}
void mbse_req_gap_print(RequirementGapAnalysis* ga){if(!ga)return;printf("Req Gap: %d orphans, completeness=%.1f%%\n",ga->n_orphans,ga->completeness*100);for(int i=0;i<ga->n_orphans;i++)printf("  Orphan: %s\n",ga->orphans[i]->name);}
int mbse_req_count_by_type(SystemModel* m, RequirementType t){if(!m)return 0;int c=0;for(int i=0;i<m->n_requirements;i++)if(m->requirements[i].type==t)c++;return c;}
double mbse_req_priority_score(SystemModel* m){if(!m||m->n_requirements==0)return 0.0;double s=0.0;for(int i=0;i<m->n_requirements;i++)s+=6.0-m->requirements[i].priority;return s/m->n_requirements;}
void mbse_req_list_print(SystemModel* m){if(!m)return;printf("Requirements (%d):\n",m->n_requirements);for(int i=0;i<m->n_requirements;i++)printf("  [%d] %s\n",i,m->requirements[i].name);}
void mbse_req_validate_targets(SystemModel* m){if(!m)return;for(int i=0;i<m->n_requirements;i++){if(m->requirements[i].target_value<=0)printf("WARNING: Req %s has no measurable target\n",m->requirements[i].name);}}
void mbse_req_export_to(SystemModel* m, const char* filename){if(!m||!filename)return;FILE* f=fopen(filename,"w");if(!f)return;fprintf(f,"ID,Name,Type,Priority,Target,Threshold\n");for(int i=0;i<m->n_requirements;i++){MBSERequirement* r=&m->requirements[i];fprintf(f,"%d,%s,%d,%d,%.2f,%.2f\n",i,r->name,r->type,r->priority,r->target_value,r->threshold_value);}fclose(f);}
int mbse_req_critical_count(SystemModel* m){if(!m)return 0;int c=0;for(int i=0;i<m->n_requirements;i++)if(m->requirements[i].priority==1)c++;return c;}
double mbse_req_average_priority(SystemModel* m){if(!m||m->n_requirements==0)return 0.0;double s=0.0;for(int i=0;i<m->n_requirements;i++)s+=m->requirements[i].priority;return s/m->n_requirements;}
void mbse_req_coverage_report(SystemModel* m){if(!m)return;int total=m->n_requirements;int func=mbse_req_count_by_type(m,REQ_FUNCTIONAL);int perf=mbse_req_count_by_type(m,REQ_PERFORMANCE);printf("=== Requirement Coverage ===\nTotal:%d Functional:%d Performance:%d Others:%d\n",total,func,perf,total-func-perf);}
int mbse_req_find_conflicts(SystemModel* m, int** conflicts){if(!m||m->n_requirements<2)return 0;int* cf=(int*)malloc(m->n_requirements*sizeof(int));int nc=0;for(int i=0;i<m->n_requirements;i++)for(int j=i+1;j<m->n_requirements;j++){int bi=m->requirements[i].satisfied_by_block;int bj=m->requirements[j].satisfied_by_block;if(bi>=0&&bj>=0&&m->blocks[bi].category!=m->blocks[bj].category)cf[nc++]=i*m->n_requirements+j;}*conflicts=cf;return nc;}

double mbse_req_compliance_index(SystemModel* m) {if(!m||m->n_requirements==0)return 0.0;int sat=0;for(int i=0;i<m->n_requirements;i++)if(m->requirements[i].is_satisfied)sat++;return (double)sat/m->n_requirements;}
bool mbse_req_is_derived(MBSERequirement* r) {return r&&r->n_derived>0;}
void mbse_req_impact_analysis(SystemModel* m, int changed_req, bool* affected) {if(!m||!affected)return;for(int i=0;i<m->n_requirements;i++)affected[i]=false;for(int i=0;i<m->n_requirements;i++)for(int j=0;j<m->requirements[i].n_traces;j++)if(m->requirements[i].traces_to[j]==changed_req)affected[i]=true;}

/* ©¤©¤ Matrix decomposition helpers ©¤©¤ */
static void _householder_vector(const double* x, int n, double* v, double* beta) {
    double sigma = 0.0;
    for (int i = 1; i < n; i++) sigma += x[i] * x[i];
    v[0] = 1.0;
    for (int i = 1; i < n; i++) v[i] = x[i];
    if (sigma < 1e-15) { *beta = 0.0; return; }
    double mu = sqrt(x[0] * x[0] + sigma);
    if (x[0] <= 0) v[0] = x[0] - mu;
    else v[0] = -sigma / (x[0] + mu);
    *beta = 2.0 * v[0] * v[0] / (sigma + v[0] * v[0]);
    double inv_v0 = 1.0 / v[0];
    for (int i = 1; i < n; i++) v[i] *= inv_v0;
}
static void _apply_householder_left(double* A, int m, int n, const double* v, double beta) {
    if (beta < 1e-15) return;
    for (int j = 0; j < n; j++) {
        double s = 0.0;
        for (int i = 0; i < m; i++) s += v[i] * A[i * n + j];
        s *= beta;
        for (int i = 0; i < m; i++) A[i * n + j] -= s * v[i];
    }
}
static void _qr_decompose(double* A, int m, int n, double* Q, double* R) {
    int k = (m < n) ? m : n;
    for (int i = 0; i < m * n; i++) { Q[i] = A[i]; R[i] = 0.0; }
    double* v = (double*)calloc((size_t)m, sizeof(double));
    for (int j = 0; j < k; j++) {
        double beta;
        for (int i = j; i < m; i++) v[i - j] = Q[i * n + j];
        _householder_vector(v, m - j, v, &beta);
        for (int i = j; i < m; i++) R[i * n + j] = (i == j) ? Q[j * n + j] : v[i - j];
        _apply_householder_left(Q + j * n + j, m - j, n - j, v, beta);
    }
    free(v);
}
/* ©¤©¤ Singular value estimates ©¤©¤ */
double _estimate_max_singular(const double* A, int m, int n, int iters) {
    if (m <= 0 || n <= 0 || !A || iters <= 0) return 0.0;
    double* v = (double*)calloc((size_t)n, sizeof(double));
    double* u = (double*)calloc((size_t)m, sizeof(double));
    for (int i = 0; i < n; i++) v[i] = 1.0 / sqrt((double)n);
    for (int iter = 0; iter < iters; iter++) {
        for (int i = 0; i < m; i++) {
            u[i] = 0.0;
            for (int j = 0; j < n; j++) u[i] += A[i * n + j] * v[j];
        }
        double norm_u = 0.0;
        for (int i = 0; i < m; i++) norm_u += u[i] * u[i];
        norm_u = sqrt(norm_u);
        if (norm_u < 1e-15) { free(v); free(u); return 0.0; }
        for (int i = 0; i < m; i++) u[i] /= norm_u;
        for (int j = 0; j < n; j++) {
            v[j] = 0.0;
            for (int i = 0; i < m; i++) v[j] += A[i * n + j] * u[i];
        }
        double norm_v = 0.0;
        for (int j = 0; j < n; j++) norm_v += v[j] * v[j];
        norm_v = sqrt(norm_v);
        if (norm_v < 1e-15) { free(v); free(u); return 0.0; }
        for (int j = 0; j < n; j++) v[j] /= norm_v;
    }
    free(v); free(u);
    return 1.0;
}
