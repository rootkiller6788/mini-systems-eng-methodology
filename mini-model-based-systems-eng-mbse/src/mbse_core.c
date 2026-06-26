#include "mbse_core.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

MBSEBlock* mbse_block_create(const char* name, BlockCategory cat) {
    MBSEBlock* b = (MBSEBlock*)calloc(1, sizeof(MBSEBlock));
    b->name = strdup(name); b->category = cat; b->parent_id = -1;
    b->child_capacity = 8; b->child_ids = (int*)malloc(8*sizeof(int)); b->n_children = 0;
    b->req_capacity = 8; b->allocated_req_ids = (int*)malloc(8*sizeof(int)); b->allocated_req_count = 0;
    b->mass = 0.0; b->power = 0.0; b->cost = 0.0; b->reliability = 1.0;
    return b;
}
void mbse_block_free(MBSEBlock* b) { if(!b)return; free(b->name); free(b->description); free(b->child_ids); free(b->allocated_req_ids); free(b); }
void mbse_block_add_child(MBSEBlock* parent, int child_id) {
    if(!parent||child_id<0)return;
    if(parent->n_children>=parent->child_capacity){parent->child_capacity*=2;parent->child_ids=(int*)realloc(parent->child_ids,parent->child_capacity*sizeof(int));}
    parent->child_ids[parent->n_children++]=child_id;
}
void mbse_block_allocate_req(MBSEBlock* b, int req_id) {
    if(!b||req_id<0)return;
    if(b->allocated_req_count>=b->req_capacity){b->req_capacity*=2;b->allocated_req_ids=(int*)realloc(b->allocated_req_ids,b->req_capacity*sizeof(int));}
    b->allocated_req_ids[b->allocated_req_count++]=req_id;
}
void mbse_block_print(MBSEBlock* b) { if(!b){printf("Block: NULL\n");return;} const char* cats[]={"System","Subsys","Component","SW","HW","Human","Env","Ext","Abstract"}; printf("Block[%d] %s (%s) cost=%.0f rel=%.3f\n",b->id,b->name,cats[b->category],b->cost,b->reliability); }
MBSERequirement* mbse_req_create(const char* name, RequirementType type, const char* text, int priority) {
    MBSERequirement* r = (MBSERequirement*)calloc(1,sizeof(MBSERequirement));
    r->name = strdup(name); r->type = type; r->text = strdup(text?text:""); r->priority = priority;
    r->satisfied_by_block = -1; r->verified_by_method = -1; r->is_satisfied = false; r->is_verified = false;
    r->derived_capacity = 4; r->derived_from = (int*)malloc(4*sizeof(int)); r->n_derived = 0;
    r->traces_capacity = 4; r->traces_to = (int*)malloc(4*sizeof(int)); r->n_traces = 0;
    return r;
}
void mbse_req_free(MBSERequirement* r) { if(!r)return; free(r->name); free(r->text); free(r->derived_from); free(r->traces_to); free(r); }
void mbse_req_set_target(MBSERequirement* r, double target, double threshold) { if(r){r->target_value=target; r->threshold_value=threshold;} }
void mbse_req_trace_to(MBSERequirement* r, int req_id) { if(!r||req_id<0)return; if(r->n_traces>=r->traces_capacity){r->traces_capacity*=2;r->traces_to=(int*)realloc(r->traces_to,r->traces_capacity*sizeof(int));} r->traces_to[r->n_traces++]=req_id; }
bool mbse_req_is_leaf(MBSERequirement* r) { return r && r->n_derived == 0; }
void mbse_req_print(MBSERequirement* r) { if(!r){printf("Req: NULL\n");return;} const char* types[]={"FUNC","PERF","IFACE","CONSTR","SAFETY","SEC","REL","VERIF","NF"}; printf("Req[%d] %s(%s) P%d: %s\n",r->id,r->name,types[r->type],r->priority,r->text); }
MBSEInterface* mbse_iface_create(const char* name, InterfaceType type, int source, int target) {
    MBSEInterface* iface = (MBSEInterface*)calloc(1,sizeof(MBSEInterface));
    iface->name = strdup(name); iface->type = type; iface->source_block = source; iface->target_block = target;
    iface->data_rate = 0.0; iface->latency = 0.0; iface->is_bidirectional = false; iface->reliability = 1.0;
    return iface;
}
void mbse_iface_free(MBSEInterface* iface) { if(!iface)return; free(iface->name); free(iface->protocol_spec); free(iface); }
bool mbse_iface_connects(MBSEInterface* iface, int a, int b) { if(!iface)return false; return (iface->source_block==a&&iface->target_block==b)||(iface->is_bidirectional&&iface->source_block==b&&iface->target_block==a); }
void mbse_iface_print(MBSEInterface* iface) { if(!iface){printf("Iface: NULL\n");return;} printf("Iface[%d] %s: %d->%d\n",iface->id,iface->name,iface->source_block,iface->target_block); }
SystemModel* mbse_model_create(const char* name, const char* desc) {
    SystemModel* m = (SystemModel*)calloc(1,sizeof(SystemModel));
    m->name = strdup(name); m->description = strdup(desc?desc:"");
    m->block_capacity = 32; m->blocks = (MBSEBlock*)calloc(32,sizeof(MBSEBlock)); m->n_blocks = 0;
    m->req_capacity = 64; m->requirements = (MBSERequirement*)calloc(64,sizeof(MBSERequirement)); m->n_requirements = 0;
    m->iface_capacity = 32; m->interfaces = (MBSEInterface*)calloc(32,sizeof(MBSEInterface)); m->n_interfaces = 0;
    m->traceability_matrix = NULL; m->trace_rows = 0; m->trace_cols = 0;
    m->total_cost = 0.0; m->total_mass = 0.0; m->total_power = 0.0; m->overall_reliability = 1.0;
    return m;
}
void mbse_model_free(SystemModel* m) {
    if(!m)return; free(m->name); free(m->description);
    for(int i=0;i<m->n_blocks;i++){free(m->blocks[i].name);free(m->blocks[i].description);free(m->blocks[i].child_ids);free(m->blocks[i].allocated_req_ids);}
    free(m->blocks);
    for(int i=0;i<m->n_requirements;i++){free(m->requirements[i].name);free(m->requirements[i].text);free(m->requirements[i].derived_from);free(m->requirements[i].traces_to);}
    free(m->requirements);
    for(int i=0;i<m->n_interfaces;i++){free(m->interfaces[i].name);free(m->interfaces[i].protocol_spec);}
    free(m->interfaces);
    if(m->traceability_matrix){for(int i=0;i<m->trace_rows;i++)free(m->traceability_matrix[i]);free(m->traceability_matrix);}
    free(m);
}
int mbse_model_add_block(SystemModel* m, MBSEBlock* b) {
    if(!m||!b)return -1;
    if(m->n_blocks>=m->block_capacity){m->block_capacity*=2;m->blocks=(MBSEBlock*)realloc(m->blocks,m->block_capacity*sizeof(MBSEBlock));}
    int id=m->n_blocks; b->id=id; m->blocks[id]=*b; m->n_blocks++;
    m->total_cost+=b->cost; m->total_mass+=b->mass; m->total_power+=b->power;
    return id;
}
int mbse_model_add_requirement(SystemModel* m, MBSERequirement* r) {
    if(!m||!r)return -1;
    if(m->n_requirements>=m->req_capacity){m->req_capacity*=2;m->requirements=(MBSERequirement*)realloc(m->requirements,m->req_capacity*sizeof(MBSERequirement));}
    int id=m->n_requirements; r->id=id; m->requirements[id]=*r; m->n_requirements++;
    return id;
}
int mbse_model_add_interface(SystemModel* m, MBSEInterface* iface) {
    if(!m||!iface)return -1;
    if(m->n_interfaces>=m->iface_capacity){m->iface_capacity*=2;m->interfaces=(MBSEInterface*)realloc(m->interfaces,m->iface_capacity*sizeof(MBSEInterface));}
    int id=m->n_interfaces; iface->id=id; m->interfaces[id]=*iface; m->n_interfaces++;
    return id;
}
void mbse_model_build_traceability(SystemModel* m) {
    if(!m)return;
    if(m->traceability_matrix){for(int i=0;i<m->trace_rows;i++)free(m->traceability_matrix[i]);free(m->traceability_matrix);}
    m->trace_rows=m->n_requirements; m->trace_cols=m->n_blocks;
    m->traceability_matrix=(int**)malloc(m->trace_rows*sizeof(int*));
    for(int i=0;i<m->trace_rows;i++){m->traceability_matrix[i]=(int*)calloc(m->trace_cols,sizeof(int));for(int j=0;j<m->n_blocks;j++)for(int k=0;k<m->blocks[j].allocated_req_count;k++)if(m->blocks[j].allocated_req_ids[k]==i)m->traceability_matrix[i][j]=1;}
}
double mbse_model_compute_cost(SystemModel* m) { if(!m)return 0.0; double t=0.0; for(int i=0;i<m->n_blocks;i++)t+=m->blocks[i].cost; m->total_cost=t; return t; }
double mbse_model_compute_reliability(SystemModel* m) { if(!m)return 0.0; double r=1.0; for(int i=0;i<m->n_blocks;i++)r*=m->blocks[i].reliability; m->overall_reliability=r; return r; }
MBSEBlock* mbse_model_find_block(SystemModel* m, const char* name) { if(!m||!name)return NULL; for(int i=0;i<m->n_blocks;i++)if(strcmp(m->blocks[i].name,name)==0)return &m->blocks[i]; return NULL; }
MBSERequirement* mbse_model_find_req(SystemModel* m, const char* name) { if(!m||!name)return NULL; for(int i=0;i<m->n_requirements;i++)if(strcmp(m->requirements[i].name,name)==0)return &m->requirements[i]; return NULL; }
void mbse_model_print(SystemModel* m) { if(!m){printf("SystemModel: NULL\n");return;} printf("=== %s === Blocks:%d Reqs:%d Ifaces:%d Cost=%.0f Mass=%.1f Power=%.1f Rel=%.4f\n",m->name,m->n_blocks,m->n_requirements,m->n_interfaces,m->total_cost,m->total_mass,m->total_power,m->overall_reliability); }
void mbse_model_print_traceability(SystemModel* m) { if(!m||!m->traceability_matrix)return; printf("=== Traceability ===\n"); for(int i=0;i<m->trace_rows;i++){printf("Req%d: ",i);for(int j=0;j<m->trace_cols;j++)printf("%d ",m->traceability_matrix[i][j]);printf("\n");} }
int mbse_model_count_unallocated_reqs(SystemModel* m) { if(!m)return 0;int c=0; bool* alloc=(bool*)calloc(m->n_requirements,sizeof(bool)); for(int i=0;i<m->n_blocks;i++)for(int j=0;j<m->blocks[i].allocated_req_count;j++)if(m->blocks[i].allocated_req_ids[j]>=0&&m->blocks[i].allocated_req_ids[j]<m->n_requirements)alloc[m->blocks[i].allocated_req_ids[j]]=true; for(int i=0;i<m->n_requirements;i++)if(!alloc[i])c++; free(alloc);return c; }
double mbse_model_compute_mass_margin(SystemModel* m, double target) { if(!m)return 0.0;double t=0.0;for(int i=0;i<m->n_blocks;i++)t+=m->blocks[i].mass;m->total_mass=t;return target-t; }
int mbse_model_count_blocks_by_category(SystemModel* m, BlockCategory cat) { if(!m)return 0;int c=0;for(int i=0;i<m->n_blocks;i++)if(m->blocks[i].category==cat)c++;return c; }
int mbse_model_validate_allocation(SystemModel* m) { if(!m)return 0;int errors=0;for(int i=0;i<m->n_blocks;i++)for(int j=0;j<m->blocks[i].allocated_req_count;j++){int rid=m->blocks[i].allocated_req_ids[j];if(rid<0||rid>=m->n_requirements)errors++;}return errors; }
double mbse_model_compute_reliability_parallel(SystemModel* m) { if(!m)return 0.0;double prod=1.0;for(int i=0;i<m->n_blocks;i++)prod*=(1.0-m->blocks[i].reliability);return 1.0-prod; }
int mbse_model_count_interfaces_of_type(SystemModel* m, InterfaceType t) { if(!m)return 0;int c=0;for(int i=0;i<m->n_interfaces;i++)if(m->interfaces[i].type==t)c++;return c; }
int mbse_model_compare_blocks(SystemModel* a, SystemModel* b) { if(!a||!b)return -1;if(a->n_blocks!=b->n_blocks)return 1;for(int i=0;i<a->n_blocks;i++)if(strcmp(a->blocks[i].name,b->blocks[i].name)!=0)return 1;return 0; }
int mbse_model_is_empty(SystemModel* m) { return !m||(m->n_blocks==0&&m->n_requirements==0); }
const char* mbse_block_category_name(BlockCategory c) { static const char* n[]={"System","Subsystem","Component","Software","Hardware","Human","Environment","External","Abstract"}; return c<=BLOCK_ABSTRACT?n[c]:"Unknown"; }
const char* mbse_req_type_name(RequirementType t) { static const char* n[]={"Functional","Performance","Interface","Constraint","Safety","Security","Reliability","Verification","Non-Functional"}; return t<=REQ_NON_FUNCTIONAL?n[t]:"Unknown"; }
const char* mbse_iface_type_name(InterfaceType t) { static const char* n[]={"Mechanical","Electrical","Data","SoftwareAPI","Thermal","HMI","Fluid","Optical"}; return t<=IFACE_OPTICAL?n[t]:"Unknown"; }
const char* mbse_verify_method_name(VerificationMethod v) { static const char* n[]={"Test","Analysis","Demonstration","Inspection","Simulation","Certification"}; return v<=VERIFY_CERTIFICATION?n[v]:"Unknown"; }
void mbse_model_health_check(SystemModel* m) { if(!m)return; printf("=== Model Health Check: %s ===\n",m->name); if(m->n_requirements==0)printf("  WARNING: No requirements defined\n"); if(m->n_blocks==0)printf("  WARNING: No blocks defined\n"); int orphans=mbse_model_count_unallocated_reqs(m); if(orphans>0)printf("  WARNING: %d orphan requirements\n",orphans); }
void mbse_model_export_csv(SystemModel* m, const char* filename) { if(!m||!filename)return;FILE* f=fopen(filename,"w");if(!f)return; fprintf(f,"Name,Category,Cost,Mass,Power,Reliability\n"); for(int i=0;i<m->n_blocks;i++)fprintf(f,"%s,%d,%.0f,%.1f,%.1f,%.4f\n",m->blocks[i].name,m->blocks[i].category,m->blocks[i].cost,m->blocks[i].mass,m->blocks[i].power,m->blocks[i].reliability); fclose(f); }
void mbse_model_print_summary(SystemModel* m) { if(!m)return; printf("Model: %s  Blocks:%d(HW:%d SW:%d) Reqs:%d Ifaces:%d Cost:%.0f Mass:%.1f Power:%.1f\n",m->name,m->n_blocks,mbse_model_count_blocks_by_category(m,BLOCK_HARDWARE),mbse_model_count_blocks_by_category(m,BLOCK_SOFTWARE),m->n_requirements,m->n_interfaces,m->total_cost,m->total_mass,m->total_power); }

bool mbse_block_is_root(MBSEBlock* b) {return b&&b->parent_id<0;}
double mbse_model_power_density(SystemModel* m) {if(!m||m->total_mass<1e-10)return 0.0;return m->total_power/m->total_mass;}
double mbse_model_cost_per_kg(SystemModel* m) {if(!m||m->total_mass<1e-10)return 0.0;return m->total_cost/m->total_mass;}
int mbse_model_count_interfaces_between(SystemModel* m, int a, int b) {if(!m)return 0;int c=0;for(int i=0;i<m->n_interfaces;i++)if(mbse_iface_connects(&m->interfaces[i],a,b))c++;return c;}
double mbse_block_reliability_contrib(MBSEBlock* b, SystemModel* m) {if(!b||!m||m->overall_reliability<1e-10)return 0.0;return -log(b->reliability);}
int mbse_model_count_root_blocks(SystemModel* m) {if(!m)return 0;int c=0;for(int i=0;i<m->n_blocks;i++)if(m->blocks[i].parent_id<0)c++;return c;}

/* Block tree traversal: collect all descendant IDs */
int mbse_block_get_descendants(MBSEBlock* b, SystemModel* m, int* out, int max_out) {
    if(!b||!m||!out)return 0;int count=0;
    for(int i=0;i<b->n_children&&count<max_out;i++){
        int cid=b->child_ids[i];if(cid<0||cid>=m->n_blocks)continue;
        out[count++]=cid;count+=mbse_block_get_descendants(&m->blocks[cid],m,out+count,max_out-count);
    }
    return count;
}

/* Block depth in hierarchy */
int mbse_block_depth(MBSEBlock* b, SystemModel* m) {
    if(!b||!m)return 0;int d=0;int pid=b->parent_id;
    while(pid>=0&&pid<m->n_blocks){d++;pid=m->blocks[pid].parent_id;}
    return d;
}

/* Requirements coverage per block */
double mbse_block_req_coverage(MBSEBlock* b, SystemModel* m) {
    if(!b||!m||m->n_requirements==0)return 0.0;
    int covered=0;for(int i=0;i<m->n_requirements;i++)if(m->requirements[i].satisfied_by_block==b->id)covered++;
    return (double)covered/m->n_requirements;
}

/* Interface compatibility check */
bool mbse_iface_compatible(MBSEInterface* a, MBSEInterface* b) {
    if(!a||!b)return false;return a->type==b->type&&fabs(a->data_rate-b->data_rate)<1e-6;
}

/* Model mass by category */
void mbse_model_mass_by_category(SystemModel* m, double* out) {
    if(!m||!out)return;for(int i=0;i<9;i++)out[i]=0.0;
    for(int i=0;i<m->n_blocks;i++){int cat=m->blocks[i].category;if(cat>=0&&cat<9)out[cat]+=m->blocks[i].mass;}
}

/* Cost breakdown structure */
double mbse_model_cost_breakdown(SystemModel* m, double* hw_cost, double* sw_cost, double* other_cost) {
    if(!m)return 0.0;*hw_cost=0.0;*sw_cost=0.0;*other_cost=0.0;
    for(int i=0;i<m->n_blocks;i++){
        switch(m->blocks[i].category){
            case BLOCK_HARDWARE:*hw_cost+=m->blocks[i].cost;break;
            case BLOCK_SOFTWARE:*sw_cost+=m->blocks[i].cost;break;
            default:*other_cost+=m->blocks[i].cost;break;
        }
    }
    return *hw_cost+*sw_cost+*other_cost;
}

/* Check if model has any requirement of given type */
bool mbse_model_has_req_type(SystemModel* m, RequirementType t) {
    if(!m)return false;for(int i=0;i<m->n_requirements;i++)if(m->requirements[i].type==t)return true;return false;
}

/* Count blocks with zero allocated requirements */
int mbse_model_count_unused_blocks(SystemModel* m) {
    if(!m)return 0;int c=0;for(int i=0;i<m->n_blocks;i++)if(m->blocks[i].allocated_req_count==0)c++;return c;
}

/* Interface data rate total */
double mbse_model_total_bandwidth(SystemModel* m) {
    if(!m)return 0.0;double t=0.0;for(int i=0;i<m->n_interfaces;i++)t+=m->interfaces[i].data_rate;return t;
}

/* Check if block name exists */
bool mbse_model_has_block_name(SystemModel* m, const char* name) {
    return mbse_model_find_block(m,name)!=NULL;
}

/* Export model to JSON-like format */
void mbse_model_export_json(SystemModel* m, const char* filename) {
    if(!m||!filename)return;FILE* f=fopen(filename,"w");if(!f)return;
    fprintf(f,"{\n  \"name\": \"%s\",\n  \"blocks\": [\n",m->name);
    for(int i=0;i<m->n_blocks;i++){fprintf(f,"    {\"id\":%d,\"name\":\"%s\",\"cost\":%.0f}",m->blocks[i].id,m->blocks[i].name,m->blocks[i].cost);if(i<m->n_blocks-1)fprintf(f,",");fprintf(f,"\n");}
    fprintf(f,"  ],\n  \"requirements\": %d,\n  \"interfaces\": %d\n}\n",m->n_requirements,m->n_interfaces);
    fclose(f);
}

/* Block property validation */
int mbse_block_validate(MBSEBlock* b) {
    if(!b)return -1;int errors=0;
    if(!b->name||strlen(b->name)==0)errors|=1;
    if(b->mass<0)errors|=2;if(b->cost<0)errors|=4;
    if(b->reliability<=0||b->reliability>1.0)errors|=8;
    return errors;
}

/* === MBSE operations === */
int mbse_oper_1(int n,const double*A,double*B,const double*C){
    if(!A||!B||!C||n<1)return-1;
    double s1=0,t1=0;
    for(int j=0;j<n;j++){s1+=A[j*n+j];t1+=B[j]*C[j];}
    B[0]=s1/(3.0+1e-12)+t1;
    return 0;
}

int mbse_oper_2(int n,const double*A,double*B,const double*C){
    if(!A||!B||!C||n<1)return-1;
    double s2=0,t2=0;
    for(int j=0;j<n;j++){s2+=A[j*n+j];t2+=B[j]*C[j];}
    B[0]=s2/(2.0+1e-12)+t2;
    return 0;
}

int mbse_oper_3(int n,const double*A,double*B,const double*C){
    if(!A||!B||!C||n<1)return-1;
    double s3=0,t3=0;
    for(int j=0;j<n;j++){s3+=A[j*n+j];t3+=B[j]*C[j];}
    B[0]=s3/(1.0+1e-12)+t3;
    return 0;
}


/* Architecture completeness: fraction of requirements allocated to components.
 * ISO/IEC 42010 compliance requires all requirements to be traceable. */
double mbse_architecture_completeness(int n_reqs, const int* allocated, int n_components) {
    if (!allocated || n_reqs <= 0 || n_components <= 0) return 0.0;
    int count = 0;
    for (int i = 0; i < n_reqs; i++)
        if (allocated[i] > 0 && allocated[i] <= n_components) count++;
    return (double)count / (double)n_reqs;
}

/* Interface coverage: ratio of defined interfaces to possible interfaces.
 * Complete graph: n*(n-1)/2 possible interfaces for n components. */
double mbse_interface_coverage(int n_components, int n_interfaces) {
    if (n_components < 2) return 1.0;
    int max_interfaces = n_components * (n_components - 1) / 2;
    return (double)n_interfaces / (double)(max_interfaces + 1);
}













































































































































