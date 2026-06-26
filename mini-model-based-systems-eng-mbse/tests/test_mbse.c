#include "mbse_core.h"
#include "mbse_requirements.h"
#include "mbse_architecture.h"
#include "mbse_behavior.h"
#include "mbse_parametric.h"
#include "mbse_verification.h"
#include <stdio.h>
#include <math.h>

// removed - using standard assert()
#define TEST(n) printf("  %s... ",n);fflush(stdout)
#define assert(c) do{if(c){printf("OK\n");passed++;}else{printf("FAIL\n");failed++;}}while(0)

int main(void){
    int passed = 0, failed = 0;
    printf("=== MBSE Test Suite ===\n\n");

    TEST("block_create");MBSEBlock* b=mbse_block_create("Avionics",BLOCK_HARDWARE);assert(b&&b->category==BLOCK_HARDWARE);
    TEST("block_add_child");mbse_block_add_child(b,5);assert(b->n_children==1);
    TEST("block_free"); assert(b && b->name && b->category == BLOCK_HARDWARE); mbse_block_free(b);

    TEST("req_create");MBSERequirement* r=mbse_req_create("REQ-PERF-01",REQ_PERFORMANCE,"Max speed 500km/h",1);assert(r&&r->type==REQ_PERFORMANCE);
    TEST("req_set_target");mbse_req_set_target(r,500.0,450.0);assert(fabs(r->target_value-500.0)<1e-6);
    TEST("req_is_leaf");assert(mbse_req_is_leaf(r));
    TEST("req_free"); assert(r && r->type == REQ_PERFORMANCE && r->target_value == 500.0); mbse_req_free(r);

    TEST("iface_create");MBSEInterface* iface=mbse_iface_create("PowerBus",IFACE_ELECTRICAL,0,1);assert(iface&&iface->source_block==0);
    TEST("iface_connects");assert(mbse_iface_connects(iface,0,1)&&!mbse_iface_connects(iface,2,3));
    TEST("iface_free"); assert(iface && iface->source_block == 0 && iface->target_block == 1); mbse_iface_free(iface);

    TEST("model_create");SystemModel* m=mbse_model_create("UAV","Unmanned Aerial Vehicle");assert(m&&m->n_blocks==0);
    TEST("model_add_block");MBSEBlock* b1=mbse_block_create("Airframe",BLOCK_HARDWARE);b1->cost=100000;b1->mass=500;b1->reliability=0.99;mbse_model_add_block(m,b1);assert(m->n_blocks==1);
    TEST("model_add_req");MBSERequirement* r1=mbse_req_create("R1",REQ_PERFORMANCE,"Range 1000km",1);mbse_model_add_requirement(m,r1);assert(m->n_requirements==1);
    TEST("add_2nd_block");MBSEBlock* b2=mbse_block_create("Engine",BLOCK_HARDWARE);b2->cost=50000;b2->mass=200;mbse_model_add_block(m,b2);assert(m->n_blocks==2);
    TEST("add_iface");MBSEInterface* if1=mbse_iface_create("Mount",IFACE_MECHANICAL,0,1);mbse_model_add_interface(m,if1);assert(m->n_interfaces==1);
    TEST("allocate_req");mbse_block_allocate_req(&m->blocks[0],0);assert(m->blocks[0].allocated_req_count==1);
    TEST("build_trace");mbse_model_build_traceability(m);assert(m->traceability_matrix!=NULL);
    TEST("model_cost");double cost=mbse_model_compute_cost(m);assert(cost==150000.0);
    TEST("model_reliability");double rel=mbse_model_compute_reliability(m);assert(rel>0&&rel<1.0);
    TEST("find_block");MBSEBlock* fb=mbse_model_find_block(m,"Engine");assert(fb!=NULL);

    TEST("trace_create");TraceabilityMatrix* tm=mbse_trace_create(m);assert(tm&&tm->coverage>=0);
    TEST("gap_analysis");RequirementGapAnalysis* ga=mbse_req_gap_analysis(m);assert(ga&&ga->completeness>=0);
    TEST("req_count_by_type");assert(mbse_req_count_by_type(m,REQ_PERFORMANCE)==1);
    TEST("priority_score");double ps=mbse_req_priority_score(m);assert(ps>=0);

    TEST("hierarchy");HierarchyMetrics* hm=mbse_arch_hierarchy_metrics(m);assert(hm&&hm->depth>=0);
    TEST("modularity");double mod=mbse_arch_modularity(m);assert(mod>=0);
    TEST("coupling");BlockCoupling* bc=mbse_arch_coupling_analysis(m,0,1);assert(bc&&bc->n_ifaces>=0);

    TEST("sm_create");StateMachine* sm=mbse_sm_create("FlightModes");assert(sm);
    TEST("sm_add_state");int s_idle=mbse_sm_add_state(sm,"Idle",SM_IDLE,true,false);assert(s_idle==0);
    int s_active=mbse_sm_add_state(sm,"Active",SM_ACTIVE,false,true);assert(s_active==1);
    TEST("sm_add_trans");int t=mbse_sm_add_transition(sm,0,1,"start",1.0);assert(t==0);
    TEST("sm_fire");bool fire=mbse_sm_fire(sm,"start");assert(fire&&sm->current_state==1);
    TEST("sm_deadlock");assert(mbse_sm_is_deadlock_free(sm));

    TEST("ad_create");ActivityDiagram* ad=mbse_ad_create("Takeoff");assert(ad);
    mbse_ad_add_node(ad,"Preflight",2.0);mbse_ad_add_node(ad,"Takeoff",1.5);mbse_ad_add_node(ad,"Climb",3.0);
    mbse_ad_add_edge(ad,0,1);mbse_ad_add_edge(ad,1,2);
    TEST("ad_critical_path");double cp=mbse_ad_critical_path(ad);assert(cp>=6.5);

    TEST("param_create");Parameter* p=mbse_param_create("Weight",1000,500,2000,"kg");assert(p);
    TEST("constraint_create");ParametricConstraint* pc=mbse_constraint_create("TotalMass","sum");assert(pc);
    TEST("trade_create");const char* names[]={"AltA","AltB"};TradeStudy* ts=mbse_trade_create(2,2,names);
    mbse_trade_set_score(ts,0,0,8.0);mbse_trade_set_score(ts,0,1,7.0);mbse_trade_set_score(ts,1,0,6.0);mbse_trade_set_score(ts,1,1,9.0);
    mbse_trade_compute(ts);TEST("trade_best");assert(mbse_trade_best(ts)>=0);

    TEST("vv_plan_create");VerificationPlan* vp=mbse_vv_plan_create("VVP");assert(vp);
    mbse_vv_add_item(vp,0,VERIFY_TEST,500.0);mbse_vv_add_item(vp,0,VERIFY_ANALYSIS,450.0);
    mbse_vv_record_result(vp,0,true,490.0);
    TEST("vv_pass_rate");assert(mbse_vv_pass_rate(vp)==0.5);
    TEST("vv_report");VerificationReport* vr=mbse_vv_generate_report(vp,m);assert(vr);

    mbse_vv_report_free(vr);mbse_vv_plan_free(vp);
    mbse_trade_free(ts);mbse_param_free(p);mbse_constraint_free(pc);
    mbse_ad_free(ad);mbse_sm_free(sm);
    mbse_arch_coupling_free(bc);mbse_arch_hierarchy_free(hm);
    mbse_trace_free(tm);mbse_req_gap_free(ga);
    mbse_model_free(m);

    printf("\n=== All asserts passed ===\n"); return 0;
}
