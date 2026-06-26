#include "sos_types.h"
#include "sos_architecture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* SoS Performance Metrics */

typedef struct { double value; double target; double variance; char* name; } SoSMetric;
typedef struct { SoSMetric* metrics; int n; int cap; double overall_score; } SoSDashboard;

SoSMetric* sos_metric_create(const char* name, double target) {
    SoSMetric* m = calloc(1, sizeof(SoSMetric));
    if (!m) return NULL;
    m->name = name ? malloc(strlen(name) + 1) : NULL;
    if (name && m->name) strcpy(m->name, name);
    m->target = target; m->value = 0; m->variance = 0;
    return m;
}
void sos_metric_free(SoSMetric* m) { if(m){free(m->name);free(m);} }

SoSDashboard* sos_dashboard_create(void) { return calloc(1, sizeof(SoSDashboard)); }
void sos_dashboard_free(SoSDashboard* db) {
    if(!db) return;
    for(int i=0;i<db->n;i++) sos_metric_free(&db->metrics[i]);
    free(db->metrics); free(db);
}
int sos_dashboard_add(SoSDashboard* db, SoSMetric* m) {
    if(!db||!m) return -1;
    if(db->n>=db->cap){int nc=(db->cap==0)?8:db->cap*2; SoSMetric*nm=realloc(db->metrics,(size_t)nc*sizeof(SoSMetric)); if(!nm)return -1; db->metrics=nm;db->cap=nc;}
    db->metrics[db->n]=*m; return db->n++;
}
void sos_dashboard_update(SoSDashboard* db, int idx, double value) {
    if(!db||idx<0||idx>=db->n) return;
    db->metrics[idx].value=value;
    if(db->metrics[idx].target>1e-12) db->metrics[idx].variance=fabs(value-db->metrics[idx].target)/db->metrics[idx].target;
}
double sos_dashboard_compute_score(SoSDashboard* db) {
    if(!db||db->n==0) return 0;
    double s=0; for(int i=0;i<db->n;i++){double t=db->metrics[i].target; s+=(t>1e-12)?fmin(db->metrics[i].value/t,2.0):0;} db->overall_score=s/(double)db->n; return db->overall_score;
}
void sos_dashboard_print(const SoSDashboard* db) {
    if(!db) return;
    printf("=== SoS Dashboard (score=%.2f) ===\n", db->overall_score);
    for(int i=0;i<db->n;i++) printf("  %s: %.2f / %.2f (%.0f%%)\n",db->metrics[i].name,db->metrics[i].value,db->metrics[i].target,db->metrics[i].value/(db->metrics[i].target+1e-12)*100);
}

/* SoS Trade-off Analysis */
typedef struct { double* values; int n_obj; char** obj_names; } ParetoPoint;
typedef struct { ParetoPoint* points; int n; int cap; int* pareto_front; int n_pareto; } ParetoSet;

ParetoPoint* sos_pareto_point_create(int n_obj) { ParetoPoint* p=calloc(1,sizeof(ParetoPoint)); if(p){p->values=calloc((size_t)n_obj,sizeof(double)); p->obj_names=calloc((size_t)n_obj,sizeof(char*)); p->n_obj=n_obj;} return p; }
void sos_pareto_point_free(ParetoPoint* p) { if(!p)return; free(p->values); for(int i=0;i<p->n_obj;i++)free(p->obj_names[i]); free(p->obj_names); free(p); }
ParetoSet* sos_pareto_set_create(void) { return calloc(1,sizeof(ParetoSet)); }
void sos_pareto_set_free(ParetoSet* ps) {
    if(!ps)return; for(int i=0;i<ps->n;i++)sos_pareto_point_free(&ps->points[i]); free(ps->points); free(ps->pareto_front); free(ps);
}
int sos_pareto_add(ParetoSet* ps, ParetoPoint* p) {
    if(!ps||!p)return -1;
    if(ps->n>=ps->cap){int nc=(ps->cap==0)?16:ps->cap*2; ParetoPoint*np=realloc(ps->points,(size_t)nc*sizeof(ParetoPoint)); if(!np)return -1; ps->points=np;ps->cap=nc;}
    ps->points[ps->n]=*p; return ps->n++;
}
void sos_pareto_find_front(ParetoSet* ps) {
    if(!ps||ps->n<2) return;
    free(ps->pareto_front); ps->pareto_front=malloc((size_t)ps->n*sizeof(int)); ps->n_pareto=0;
    for(int i=0;i<ps->n;i++){bool dominated=false;
        for(int j=0;j<ps->n&&!dominated;j++){
            if(i==j)continue; bool better=true,any_strict=false;
            for(int k=0;k<ps->points[i].n_obj;k++){
                if(ps->points[j].values[k]<ps->points[i].values[k]){better=false;break;}
                if(ps->points[j].values[k]>ps->points[i].values[k]) any_strict=true;
            }
            if(better&&any_strict) dominated=true;
        }
        if(!dominated) ps->pareto_front[ps->n_pareto++]=i;
    }
}
void sos_pareto_print(const ParetoSet* ps) {
    if(!ps)return;
    printf("=== Pareto Front: %d / %d points ===\n",ps->n_pareto,ps->n);
    for(int i=0;i<ps->n_pareto&&i<10;i++){int idx=ps->pareto_front[i]; printf("  [%d]",idx); for(int k=0;k<ps->points[idx].n_obj;k++)printf(" %.3f",ps->points[idx].values[k]); printf("\n");}
}
/* Extended Pareto analysis and SoS optimization */
void sos_pareto_set_objective_name(ParetoSet* ps, int obj_idx, const char* name) {
    if(!ps||obj_idx<0||!name) return;
    for(int i=0;i<ps->n;i++){free(ps->points[i].obj_names[obj_idx]); ps->points[i].obj_names[obj_idx]=malloc(strlen(name)+1); if(ps->points[i].obj_names[obj_idx])strcpy(ps->points[i].obj_names[obj_idx],name);}
}

int sos_pareto_count_dominated(const ParetoSet* ps, int idx) {
    if(!ps||idx<0||idx>=ps->n) return 0;
    int count=0;
    for(int j=0;j<ps->n;j++){if(j==idx)continue; bool better=true,any_strict=false; for(int k=0;k<ps->points[idx].n_obj&&better;k++){if(ps->points[j].values[k]<ps->points[idx].values[k]){better=false;break;}if(ps->points[j].values[k]>ps->points[idx].values[k])any_strict=true;} if(better&&any_strict)count++;}
    return count;
}

double sos_pareto_hypervolume(const ParetoSet* ps) {
    if(!ps||ps->n_pareto==0) return 0.0;
    double vol=0; for(int i=0;i<ps->n_pareto;i++){int idx=ps->pareto_front[i]; double prod=1.0; for(int k=0;k<ps->points[idx].n_obj;k++) prod*=fmax(ps->points[idx].values[k],1e-9); vol+=prod;}
    return vol/(double)ps->n_pareto;
}

typedef struct { double* lower; double* upper; int n_dims; } ConstraintBox;
ConstraintBox* sos_constraint_box_create(int n_dims) {
    ConstraintBox* cb=calloc(1,sizeof(ConstraintBox)); if(!cb)return NULL;
    cb->lower=calloc((size_t)n_dims,sizeof(double)); cb->upper=calloc((size_t)n_dims,sizeof(double)); cb->n_dims=n_dims; return cb;
}
void sos_constraint_box_free(ConstraintBox* cb){if(cb){free(cb->lower);free(cb->upper);free(cb);}}
void sos_constraint_box_set(ConstraintBox* cb, int dim, double lo, double hi){if(cb&&dim>=0&&dim<cb->n_dims){cb->lower[dim]=lo;cb->upper[dim]=hi;}}
bool sos_constraint_box_contains(const ConstraintBox* cb, const double* point){if(!cb||!point)return false; for(int i=0;i<cb->n_dims;i++) if(point[i]<cb->lower[i]||point[i]>cb->upper[i])return false; return true;}
