#include "mbse_core.h"
#include "mbse_requirements.h"
#include "mbse_architecture.h"
#include "mbse_behavior.h"
#include "mbse_parametric.h"
#include "mbse_verification.h"
#include <stdio.h>
#include <stdlib.h>

int main(void){
    printf("=== MBSE Demo ===
");
    SystemModel* uav=mbse_model_create("UAV","Tactical UAV");
    MBSEBlock* b1=mbse_block_create("Airframe",BLOCK_HARDWARE);
    b1->cost=150000;b1->mass=350;b1->reliability=0.995;mbse_model_add_block(uav,b1);mbse_block_free(b1);
    MBSEBlock* b2=mbse_block_create("Engine",BLOCK_HARDWARE);
    b2->cost=80000;b2->mass=120;b2->reliability=0.99;mbse_model_add_block(uav,b2);mbse_block_free(b2);
    MBSEBlock* b3=mbse_block_create("Avionics",BLOCK_HARDWARE);
    b3->cost=120000;b3->mass=25;b3->reliability=0.998;mbse_model_add_block(uav,b3);mbse_block_free(b3);
    MBSEBlock* b4=mbse_block_create("FlightSW",BLOCK_SOFTWARE);
    b4->cost=50000;b4->reliability=0.999;mbse_model_add_block(uav,b4);mbse_block_free(b4);
    MBSERequirement* r1=mbse_req_create("R1",REQ_PERFORMANCE,"Range 1000km",1);
    mbse_model_add_requirement(uav,r1);
    MBSERequirement* r2=mbse_req_create("R2",REQ_SAFETY,"MTBF>500h",1);
    mbse_model_add_requirement(uav,r2);
    MBSERequirement* r3=mbse_req_create("R3",REQ_INTERFACE,"Std gimbal",3);
    mbse_model_add_requirement(uav,r3);
    MBSEInterface* i1=mbse_iface_create("EngineMount",IFACE_MECHANICAL,0,1);
    MBSEInterface* i2=mbse_iface_create("DataBus",IFACE_DATA,2,3);
    mbse_model_add_interface(uav,i1);mbse_model_add_interface(uav,i2);
    
    mbse_block_allocate_req(&uav->blocks[0],0);
    mbse_block_allocate_req(&uav->blocks[0],1);
    mbse_block_allocate_req(&uav->blocks[1],0);
    mbse_model_build_traceability(uav);
    mbse_model_print_traceability(uav);
    mbse_model_compute_cost(uav);mbse_model_compute_reliability(uav);
    mbse_model_print(uav);
    TraceabilityMatrix* tm=mbse_trace_create(uav);mbse_trace_print(tm);mbse_trace_free(tm);
    mbse_model_free(uav);
    printf("
=== MBSE Demo Complete ===
");
    return 0;
}