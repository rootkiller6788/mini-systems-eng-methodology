#include "mbse_core.h"
#include "mbse_verification.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

VerificationPlan* mbse_vv_plan_create(const char* name){VerificationPlan* vp=(VerificationPlan*)calloc(1,sizeof(VerificationPlan));vp->name=strdup(name);vp->item_capacity=64;vp->items=(VerificationItem*)calloc(64,sizeof(VerificationItem));vp->n_items=0;vp->pass_rate=0.0;vp->n_passed=0;vp->n_failed=0;vp->n_pending=0;return vp;}
void mbse_vv_plan_free(VerificationPlan* vp){if(!vp)return;free(vp->name);for(int i=0;i<vp->n_items;i++){free(vp->items[i].procedure);free(vp->items[i].date);}free(vp->items);free(vp);}
int mbse_vv_add_item(VerificationPlan* vp, int req_id, int method, double criteria){if(!vp)return -1;if(vp->n_items>=vp->item_capacity){vp->item_capacity*=2;vp->items=(VerificationItem*)realloc(vp->items,vp->item_capacity*sizeof(VerificationItem));}int id=vp->n_items;vp->items[id].req_id=req_id;vp->items[id].method=method;vp->items[id].pass_criteria=criteria;vp->items[id].passed=false;vp->items[id].actual_value=0.0;vp->n_pending++;vp->n_items++;return id;}
void mbse_vv_record_result(VerificationPlan* vp, int idx, bool passed, double actual){if(!vp||idx<0||idx>=vp->n_items)return;if(!vp->items[idx].passed)vp->n_pending--;vp->items[idx].passed=passed;vp->items[idx].actual_value=actual;if(passed)vp->n_passed++;else vp->n_failed++;vp->pass_rate=vp->n_items>0?(double)vp->n_passed/vp->n_items:0.0;}
double mbse_vv_pass_rate(VerificationPlan* vp){return vp?vp->pass_rate:0.0;}
void mbse_vv_plan_print(VerificationPlan* vp){if(!vp)return;printf("Verification Plan: %s (%d items, %.1f%% passed)\n",vp->name,vp->n_items,vp->pass_rate*100);for(int i=0;i<vp->n_items;i++)printf("  Item[%d]: req=%d method=%d passed=%s actual=%.2f\n",i,vp->items[i].req_id,vp->items[i].method,vp->items[i].passed?"Y":"N",vp->items[i].actual_value);}
VerificationReport* mbse_vv_generate_report(VerificationPlan* vp, SystemModel* m){if(!vp)return NULL;VerificationReport* vr=(VerificationReport*)calloc(1,sizeof(VerificationReport));vr->n_reqs=m?m->n_requirements:0;vr->n_verified=vp->n_passed;vr->verification_coverage=vr->n_reqs>0?(double)vp->n_items/vr->n_reqs:0.0;for(int i=0;i<vp->n_items;i++)if(vp->items[i].method>=0&&vp->items[i].method<6)vr->n_methods[vp->items[i].method]++;vr->risk_score=mbse_vv_risk_score(vp);vr->is_acceptable=(vr->verification_coverage>=0.8&&vp->pass_rate>=0.9);return vr;}
void mbse_vv_report_free(VerificationReport* vr){free(vr);}
int mbse_vv_recommend_method(RequirementType t){switch(t){case REQ_FUNCTIONAL:return VERIFY_TEST;case REQ_PERFORMANCE:return VERIFY_TEST;case REQ_INTERFACE:return VERIFY_DEMONSTRATION;case REQ_SAFETY:return VERIFY_ANALYSIS;case REQ_SECURITY:return VERIFY_INSPECTION;case REQ_RELIABILITY:return VERIFY_SIMULATION;default:return VERIFY_INSPECTION;}}
void mbse_vv_report_print(VerificationReport* vr){if(!vr)return;printf("Verification Report: %d/%d verified (%.1f%%)\n",vr->n_verified,vr->n_reqs,vr->verification_coverage*100);printf("Methods: Test=%d Analysis=%d Demo=%d Inspect=%d Sim=%d Cert=%d\n",vr->n_methods[0],vr->n_methods[1],vr->n_methods[2],vr->n_methods[3],vr->n_methods[4],vr->n_methods[5]);printf("Risk: %.2f, Acceptable: %s\n",vr->risk_score,vr->is_acceptable?"YES":"NO");}
double mbse_vv_risk_score(VerificationPlan* vp){if(!vp||vp->n_items==0)return 0.0;return (double)(vp->n_failed+vp->n_pending)/(double)vp->n_items;}
int mbse_vv_critical_items(VerificationPlan* vp){if(!vp)return 0;int c=0;for(int i=0;i<vp->n_items;i++)if(!vp->items[i].passed)c++;return c;}
bool mbse_vv_is_verifiable(MBSERequirement* r){if(!r)return false;return r->target_value>0.0||r->threshold_value>0.0||r->text!=NULL;}
void mbse_vv_generate_procedures(VerificationPlan* vp){if(!vp)return;const char* methods[]={"Test Procedure","Analysis Report","Demonstration Checklist","Inspection Checklist","Simulation Run","Certification Review"};for(int i=0;i<vp->n_items;i++){free(vp->items[i].procedure);int m=vp->items[i].method;if(m<0)m=0;if(m>5)m=5;char buf[128];snprintf(buf,sizeof(buf),"%s for req %d",methods[m],vp->items[i].req_id);vp->items[i].procedure=strdup(buf);}}
void mbse_vv_export_report(VerificationReport* vr, const char* filename){if(!vr||!filename)return;FILE* f=fopen(filename,"w");if(!f)return;fprintf(f,"Verified,Total,Coverage,Risk,Acceptable\n%d,%d,%.2f,%.2f,%s\n",vr->n_verified,vr->n_reqs,vr->verification_coverage,vr->risk_score,vr->is_acceptable?"YES":"NO");fclose(f);}
void mbse_vv_status_summary(VerificationPlan* vp){if(!vp)return;printf("=== V&V Status ===\nPassed: %d Failed: %d Pending: %d\nPass rate: %.1f%%\n",vp->n_passed,vp->n_failed,vp->n_pending,vp->pass_rate*100);}
double mbse_vv_coverage_gap(VerificationPlan* vp, SystemModel* m){if(!vp||!m||m->n_requirements==0)return 1.0;return 1.0-(double)vp->n_items/m->n_requirements;}
int mbse_vv_total_methods_used(VerificationPlan* vp){if(!vp)return 0;bool used[6]={false};int c=0;for(int i=0;i<vp->n_items;i++){int m=vp->items[i].method;if(m>=0&&m<6&&!used[m]){used[m]=true;c++;}}return c;}
void mbse_vv_completion_estimate(VerificationPlan* vp){if(!vp)return;int remaining=vp->n_pending;printf("Estimated completion: %d items remaining, pass rate: %.1f%%\n",remaining,vp->pass_rate*100);}
void mbse_vv_print_procedures(VerificationPlan* vp){if(!vp)return;for(int i=0;i<vp->n_items;i++)printf("  Item %d: %s\n",i,vp->items[i].procedure?vp->items[i].procedure:"N/A");}

void mbse_vv_schedule_gantt(VerificationPlan* vp, int* start_days) {if(!vp||!start_days)return;int day=0;for(int i=0;i<vp->n_items;i++){start_days[i]=day;day+=vp->items[i].method==VERIFY_TEST?5:vp->items[i].method==VERIFY_ANALYSIS?3:2;}}
double mbse_vv_schedule_duration(VerificationPlan* vp) {if(!vp)return 0.0;int* starts=(int*)malloc(vp->n_items*sizeof(int));mbse_vv_schedule_gantt(vp,starts);int last_end=0;for(int i=0;i<vp->n_items;i++){int dur=vp->items[i].method==VERIFY_TEST?5:vp->items[i].method==VERIFY_ANALYSIS?3:2;if(starts[i]+dur>last_end)last_end=starts[i]+dur;}free(starts);return(double)last_end;}
int mbse_vv_risk_level(VerificationPlan* vp) {if(!vp)return 0;double r=mbse_vv_risk_score(vp);if(r<0.1)return 0;if(r<0.3)return 1;if(r<0.5)return 2;return 3;}

/* Verification traceability matrix completeness */
double mbse_vv_trace_coverage(VerificationPlan* vp, SystemModel* m) {
    if(!vp||!m||m->n_requirements==0)return 0.0;bool* covered=(bool*)calloc(m->n_requirements,sizeof(bool));
    for(int i=0;i<vp->n_items;i++){int rid=vp->items[i].req_id;if(rid>=0&&rid<m->n_requirements)covered[rid]=true;}
    int count=0;for(int i=0;i<m->n_requirements;i++)if(covered[i])count++;free(covered);return (double)count/m->n_requirements;
}

/* Identify untested requirements */
int mbse_vv_untested_reqs(VerificationPlan* vp, SystemModel* m, int* out_ids) {
    if(!vp||!m||!out_ids)return 0;bool* covered=(bool*)calloc(m->n_requirements,sizeof(bool));int n=0;
    for(int i=0;i<vp->n_items;i++){int rid=vp->items[i].req_id;if(rid>=0&&rid<m->n_requirements)covered[rid]=true;}
    for(int i=0;i<m->n_requirements;i++)if(!covered[i])out_ids[n++]=i;free(covered);return n;
}

/* Verification completion percentage */
double mbse_vv_completion_pct(VerificationPlan* vp) {
    if(!vp||vp->n_items==0)return 0.0;return (double)vp->n_passed/vp->n_items*100.0;
}

/* Method distribution analysis */
void mbse_vv_method_distribution(VerificationPlan* vp, double* distribution) {
    if(!vp||!distribution)return;for(int i=0;i<6;i++)distribution[i]=0.0;
    for(int i=0;i<vp->n_items;i++){int m=vp->items[i].method;if(m>=0&&m<6)distribution[m]++;}
    for(int i=0;i<6;i++)distribution[i]/=vp->n_items;
}

/* Cost estimate per verification method type */
double mbse_vv_method_cost_estimate(int method, int complexity) {
    double base[]={5000,3000,2000,1000,4000,8000};
    double factors[]={1.0,1.5,2.0,3.0,5.0};
    if(method<0||method>5||complexity<0||complexity>4)return 0.0;
    return base[method]*factors[complexity];
}

/* Total verification budget estimate */
double mbse_vv_total_budget(VerificationPlan* vp, int* complexities) {
    if(!vp||!complexities)return 0.0;double total=0.0;
    for(int i=0;i<vp->n_items;i++){int m=vp->items[i].method;int c=complexities[i];total+=mbse_vv_method_cost_estimate(m,c);}return total;
}

/* Verification schedule: find critical path items */
int mbse_vv_critical_path_items(VerificationPlan* vp, int* critical_ids) {
    if(!vp||!critical_ids)return 0;int count=0;
    for(int i=0;i<vp->n_items;i++)if(vp->items[i].method==VERIFY_TEST||vp->items[i].method==VERIFY_SIMULATION)critical_ids[count++]=i;
    return count;
}

/* Check if plan meets minimum coverage requirements */
bool mbse_vv_meets_minimum(VerificationPlan* vp, SystemModel* m, double min_coverage) {
    if(!vp||!m)return false;double cov=mbse_vv_trace_coverage(vp,m);return cov>=min_coverage;
}

/* Verification evidence status */
void mbse_vv_evidence_status(VerificationPlan* vp, bool* has_evidence, int* evidence_count) {
    if(!vp){*evidence_count=0;return;}*evidence_count=0;
    for(int i=0;i<vp->n_items;i++){has_evidence[i]=vp->items[i].passed;if(has_evidence[i])(*evidence_count)++;}
}

/* Identify verification bottlenecks */
int mbse_vv_bottleneck_items(VerificationPlan* vp, int* bottleneck_ids) {
    if(!vp||!bottleneck_ids)return 0;int count=0;
    for(int i=0;i<vp->n_items;i++)if(!vp->items[i].passed&&vp->items[i].pass_criteria>0.8)count++;
    return count;
}





























