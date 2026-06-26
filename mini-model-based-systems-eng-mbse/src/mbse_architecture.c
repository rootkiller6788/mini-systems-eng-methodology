#include "mbse_core.h"
#include "mbse_architecture.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

HierarchyMetrics* mbse_arch_hierarchy_metrics(SystemModel* m){if(!m)return NULL;HierarchyMetrics* hm=(HierarchyMetrics*)calloc(1,sizeof(HierarchyMetrics));int* depth=(int*)calloc(m->n_blocks,sizeof(int));hm->depth=0;int tc=0;for(int i=0;i<m->n_blocks;i++){if(m->blocks[i].parent_id>=0){int p=m->blocks[i].parent_id;depth[i]=depth[p]+1;if(depth[i]>hm->depth)hm->depth=depth[i];}tc+=m->blocks[i].n_children;}free(depth);int nwc=0;for(int i=0;i<m->n_blocks;i++)if(m->blocks[i].n_children>0)nwc++;hm->fan_out_avg=nwc>0?(double)tc/nwc:0.0;hm->fan_out_max=0.0;for(int i=0;i<m->n_blocks;i++)if(m->blocks[i].n_children>hm->fan_out_max)hm->fan_out_max=m->blocks[i].n_children;int internal=0,external=m->n_interfaces;for(int i=0;i<m->n_interfaces;i++){int s=m->interfaces[i].source_block,t=m->interfaces[i].target_block;if(s>=0&&t>=0&&s<m->n_blocks&&t<m->n_blocks){bool sp=(m->blocks[s].parent_id>=0&&m->blocks[s].parent_id==m->blocks[t].parent_id);if(sp)internal++;}}hm->cohesion=external>0?(double)internal/external:0.0;hm->coupling=m->n_blocks>0?(double)m->n_interfaces/m->n_blocks:0.0;hm->is_strictly_layered=(hm->fan_out_avg<3.0&&hm->cohesion>0.3);return hm;}
void mbse_arch_hierarchy_free(HierarchyMetrics* hm){free(hm);}
int mbse_arch_max_depth(SystemModel* m){HierarchyMetrics* hm=mbse_arch_hierarchy_metrics(m);int d=hm?hm->depth:0;mbse_arch_hierarchy_free(hm);return d;}
double mbse_arch_modularity(SystemModel* m){if(!m||m->n_blocks<2)return 0.0;HierarchyMetrics* hm=mbse_arch_hierarchy_metrics(m);double mod=hm->cohesion*(1.0-hm->coupling/m->n_blocks);mbse_arch_hierarchy_free(hm);return mod;}
AllocationResult* mbse_arch_allocate_function(SystemModel* m, int block_id, int n_funcs, int* func_ids){if(!m||block_id<0||block_id>=m->n_blocks)return NULL;AllocationResult* ar=(AllocationResult*)calloc(1,sizeof(AllocationResult));ar->block_id=block_id;ar->n_functions=n_funcs;ar->function_ids=(int*)malloc(n_funcs*sizeof(int));memcpy(ar->function_ids,func_ids,n_funcs*sizeof(int));ar->allocation_fitness=m->blocks[block_id].allocated_req_count>0?1.0/(1.0+n_funcs):0.5;ar->is_overloaded=(n_funcs>10);ar->is_underutilized=(n_funcs<2&&m->blocks[block_id].n_children==0);return ar;}
void mbse_arch_allocation_free(AllocationResult* ar){if(!ar)return;free(ar->function_ids);free(ar);}
double mbse_arch_allocation_coverage(SystemModel* m){if(!m||m->n_requirements==0)return 0.0;int cov=0;for(int i=0;i<m->n_requirements;i++)if(m->requirements[i].satisfied_by_block>=0)cov++;return (double)cov/m->n_requirements;}
BlockCoupling* mbse_arch_coupling_analysis(SystemModel* m, int a, int b){if(!m||a<0||b<0||a>=m->n_blocks||b>=m->n_blocks)return NULL;BlockCoupling* bc=(BlockCoupling*)calloc(1,sizeof(BlockCoupling));bc->source=a;bc->target=b;for(int i=0;i<m->n_interfaces;i++)if(mbse_iface_connects(&m->interfaces[i],a,b))bc->n_ifaces++;bc->iface_complexity=bc->n_ifaces;bc->coupling_strength=bc->n_ifaces/(1.0+m->n_blocks);return bc;}
void mbse_arch_coupling_free(BlockCoupling* bc){free(bc);}
double mbse_arch_total_coupling(SystemModel* m){if(!m)return 0.0;double s=0.0;for(int i=0;i<m->n_interfaces;i++)s+=m->interfaces[i].data_rate+m->interfaces[i].latency*0.1;return s;}
void mbse_arch_print_breakdown(SystemModel* m){if(!m)return;printf("Architecture Breakdown:\n");HierarchyMetrics* hm=mbse_arch_hierarchy_metrics(m);printf("  Depth:%d Fan-out avg:%.1f max:%.1f Cohesion:%.2f Coupling:%.2f\n",hm->depth,hm->fan_out_avg,hm->fan_out_max,hm->cohesion,hm->coupling);mbse_arch_hierarchy_free(hm);}
int mbse_arch_count_leaf_blocks(SystemModel* m){if(!m)return 0;int c=0;for(int i=0;i<m->n_blocks;i++)if(m->blocks[i].n_children==0)c++;return c;}
int* mbse_arch_interfaces_per_block(SystemModel* m, int* out_n){if(!m){*out_n=0;return NULL;}int* cnt=(int*)calloc(m->n_blocks,sizeof(int));for(int i=0;i<m->n_interfaces;i++){cnt[m->interfaces[i].source_block]++;if(m->interfaces[i].is_bidirectional)cnt[m->interfaces[i].target_block]++;}*out_n=m->n_blocks;return cnt;}
void mbse_arch_validate_hierarchy(SystemModel* m){if(!m)return;for(int i=0;i<m->n_blocks;i++)if(m->blocks[i].parent_id<0&&i>0)printf("Block %d (%s) has no parent\n",i,m->blocks[i].name);}

double mbse_arch_interface_density(SystemModel* m) { if(!m||m->n_blocks<2)return 0.0; int max_if=m->n_blocks*(m->n_blocks-1); return max_if>0?(double)m->n_interfaces/max_if:0.0; }
int mbse_arch_decomposition_levels(SystemModel* m) { HierarchyMetrics* hm=mbse_arch_hierarchy_metrics(m); int d=hm?hm->depth:0; mbse_arch_hierarchy_free(hm); return d+1; }
bool mbse_arch_is_balanced(SystemModel* m) { if(!m)return false; int tc=0; for(int i=0;i<m->n_blocks;i++)tc+=m->blocks[i].n_children; return tc<=m->n_blocks*3; }
double mbse_arch_average_component_size(SystemModel* m) { if(!m||m->n_blocks==0)return 0.0; double t=0.0; for(int i=0;i<m->n_blocks;i++)t+=m->blocks[i].mass; return t/m->n_blocks; }

double mbse_arch_interface_complexity_index(SystemModel* m) {if(!m||m->n_interfaces==0)return 0.0;double idx=0.0;for(int i=0;i<m->n_interfaces;i++){idx+=m->interfaces[i].data_rate+m->interfaces[i].latency*10.0+(!m->interfaces[i].is_bidirectional?0.0:5.0);}return idx/m->n_interfaces;}
int mbse_arch_count_dangling_interfaces(SystemModel* m) {if(!m)return 0;int c=0;for(int i=0;i<m->n_interfaces;i++){if(m->interfaces[i].source_block>=m->n_blocks||m->interfaces[i].target_block>=m->n_blocks)c++;}return c;}
double mbse_arch_cyclomatic_complexity(SystemModel* m) {if(!m)return 0.0;return m->n_interfaces-m->n_blocks+2.0;}

/* Architecture view generation: component tree */
void mbse_arch_component_tree(SystemModel* m, int root_id, int depth, int* visited) {
    if(!m||root_id<0||root_id>=m->n_blocks)return;if(visited[root_id])return;visited[root_id]=1;
    for(int i=0;i<depth;i++)printf("  ");printf("%s (cost=%.0f)\n",m->blocks[root_id].name,m->blocks[root_id].cost);
    for(int i=0;i<m->blocks[root_id].n_children;i++)mbse_arch_component_tree(m,m->blocks[root_id].child_ids[i],depth+1,visited);
}

/* Critical block analysis: blocks with most dependencies */
int mbse_arch_critical_blocks(SystemModel* m, double threshold, int* critical_ids) {
    if(!m||!critical_ids)return 0;int count=0;double avg_if=mbse_arch_interface_density(m);
    for(int i=0;i<m->n_blocks;i++){int* iface_count;int n_ignored;iface_count=mbse_arch_interfaces_per_block(m,&n_ignored);
        double score=iface_count?iface_count[i]:0.0;free(iface_count);if(score>threshold*avg_if*m->n_blocks)critical_ids[count++]=i;}
    return count;
}

/* Interface compatibility matrix for architecture */
void mbse_arch_compatibility_matrix(SystemModel* m, int** compat) {
    if(!m||!compat)return;
    for(int i=0;i<m->n_blocks;i++)for(int j=0;j<m->n_blocks;j++)compat[i][j]=mbse_model_count_interfaces_between(m,i,j);
}

/* Modularity optimization: suggest block merges */
int mbse_arch_suggest_merges(SystemModel* m, double coupling_threshold, int* merge_pairs) {
    if(!m||!merge_pairs)return 0;int count=0;
    for(int i=0;i<m->n_blocks;i++)for(int j=i+1;j<m->n_blocks;j++){
        BlockCoupling* bc=mbse_arch_coupling_analysis(m,i,j);
        if(bc&&bc->coupling_strength>coupling_threshold){merge_pairs[count*2]=i;merge_pairs[count*2+1]=j;count++;}
        mbse_arch_coupling_free(bc);}
    return count;
}

/* Architecture pattern detection: layered */
bool mbse_arch_is_layered(SystemModel* m) {
    if(!m)return false;HierarchyMetrics* hm=mbse_arch_hierarchy_metrics(m);bool l=hm->is_strictly_layered;mbse_arch_hierarchy_free(hm);return l;
}

/* Architecture pattern detection: hub-spoke */
bool mbse_arch_is_hub_spoke(SystemModel* m) {
    if(!m||m->n_blocks<3)return false;int max_if=0,hub_id=-1;
    for(int i=0;i<m->n_blocks;i++){int ifaces=0;for(int j=0;j<m->n_interfaces;j++)if(m->interfaces[j].source_block==i||m->interfaces[j].target_block==i)ifaces++;if(ifaces>max_if){max_if=ifaces;hub_id=i;}}
    return max_if>m->n_blocks/2;
}

/* Architecture pattern detection: bus */
bool mbse_arch_is_bus(SystemModel* m) {
    if(!m||m->n_blocks<3)return false;
    for(int i=0;i<m->n_interfaces;i++)if(m->interfaces[i].type==IFACE_DATA&&m->interfaces[i].is_bidirectional)return true;
    return false;
}

/* Cost per interface analysis */
double mbse_arch_cost_per_interface(SystemModel* m) {
    if(!m||m->n_interfaces==0)return 0.0;return m->total_cost/m->n_interfaces;
}

/* Interface criticality: single point of failure risk */
int mbse_arch_spof_interfaces(SystemModel* m, int* spof_ids) {
    if(!m||!spof_ids)return 0;int count=0;
    for(int i=0;i<m->n_interfaces;i++){
        int src=m->interfaces[i].source_block,tgt=m->interfaces[i].target_block;
        int src_if=0,tgt_if=0;
        for(int j=0;j<m->n_interfaces;j++){if(m->interfaces[j].source_block==src||m->interfaces[j].target_block==src)src_if++;if(m->interfaces[j].source_block==tgt||m->interfaces[j].target_block==tgt)tgt_if++;}
        if(src_if==1||tgt_if==1)spof_ids[count++]=i;}
    return count;
}

/* Architecture complexity index: n_components * avg_interfaces / n_components.
 * Higher values indicate more complex architectures needing more V&V effort. */
double mbse_complexity_index(int n_components, const int* interface_counts) {
    if (!interface_counts || n_components <= 0) return 0.0;
    double total_ifaces = 0.0;
    for (int i = 0; i < n_components; i++)
        total_ifaces += (double)interface_counts[i];
    return total_ifaces / (double)n_components;
}

/* Emergent behavior detection: if system-level behavior differs from
 * component-level sum, emergence is present (positive or negative).
 * Returns emergence magnitude as fraction of system behavior. */
double mbse_emergence_detect(double system_output, const double* component_outputs,
                              int n_components) {
    if (!component_outputs || n_components <= 0) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < n_components; i++) sum += component_outputs[i];
    return (system_output + sum > 1e-10)
        ? (system_output - sum) / (system_output + sum) : 0.0;
}
