#include "mbse_core.h"
#include "mbse_requirements.h"
#include "mbse_verification.h"
#include <stdio.h>

int main(void){
    printf("=== Requirements Engineering ===
");
    SystemModel* m=mbse_model_create("Satellite","Earth obs");
    MBSEBlock* p=mbse_block_create("Payload",BLOCK_HARDWARE);
    MBSEBlock* b=mbse_block_create("Bus",BLOCK_HARDWARE);
    p->cost=5e6;b->cost=3e6;mbse_model_add_block(m,p);mbse_model_add_block(m,b);
    mbse_block_free(p);mbse_block_free(b);
    MBSERequirement* r1=mbse_req_create("SYS-001",REQ_PERFORMANCE,"Life 5yr",1);
    MBSERequirement* r2=mbse_req_create("PAY-001",REQ_PERFORMANCE,"Res 0.5m",2);
    MBSERequirement* r3=mbse_req_create("BUS-001",REQ_SAFETY,"Debris avoid",1);
    MBSERequirement* r4=mbse_req_create("IF-001",REQ_INTERFACE,"DL 1Gbps",3);
    mbse_model_add_requirement(m,r1);mbse_model_add_requirement(m,r2);
    mbse_model_add_requirement(m,r3);mbse_model_add_requirement(m,r4);
    mbse_req_free(r1);mbse_req_free(r2);mbse_req_free(r3);mbse_req_free(r4);
    mbse_block_allocate_req(&m->blocks[0],0);mbse_block_allocate_req(&m->blocks[0],1);
    mbse_block_allocate_req(&m->blocks[1],0);mbse_block_allocate_req(&m->blocks[1],2);
    mbse_block_allocate_req(&m->blocks[1],3);
    mbse_model_build_traceability(m);mbse_model_print_traceability(m);
    TraceabilityMatrix* tm=mbse_trace_create(m);mbse_trace_print(tm);
    RequirementGapAnalysis* ga=mbse_req_gap_analysis(m);mbse_req_gap_print(ga);
    VerificationPlan* vp=mbse_vv_plan_create("SatVVP");
    for(int i=0;i<m->n_requirements;i++){
        mbse_vv_add_item(vp,i,mbse_vv_recommend_method(m->requirements[i].type),1.0);
    }
    mbse_vv_record_result(vp,0,true,0.8);mbse_vv_record_result(vp,1,true,0.5);
    mbse_vv_record_result(vp,2,true,1.0);mbse_vv_generate_procedures(vp);
    mbse_vv_plan_print(vp);
    VerificationReport* vr=mbse_vv_generate_report(vp,m);mbse_vv_report_print(vr);
    mbse_vv_report_free(vr);mbse_vv_plan_free(vp);
    mbse_req_gap_free(ga);mbse_trace_free(tm);mbse_model_free(m);
    printf("
=== Requirements Demo Complete ===
");
    return 0;
}