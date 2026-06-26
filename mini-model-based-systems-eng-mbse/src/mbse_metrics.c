#include "mbse_core.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

static double mbse_compute_modularity(SystemModel* m);

/* Architecture quality & pattern detection metrics */

double mbse_arch_quality_score(SystemModel* m) {
    if(!m||m->n_blocks<=0)return 0.0;
    double mod=mbse_compute_modularity(m);
    double coupling=(m->n_interfaces>0&&m->n_blocks>0)?
        (double)m->n_interfaces/m->n_blocks:1.0;
    return mod*0.6+(1.0-fmin(coupling,1.0))*0.4;
}

double mbse_compute_modularity(SystemModel* m){
    if(!m||m->n_blocks<2)return 1.0;
    int internal=0,external=0;
    for(int i=0;i<m->n_interfaces;i++){
        MBSEInterface* iface=&m->interfaces[i];
        if(iface->source_block>=0&&iface->target_block>=0) internal++;
    }
    for(int i=0;i<m->n_blocks;i++)
        for(int j=i+1;j<m->n_blocks;j++) external++;
    return(external>0)?(double)internal/(double)(internal+external):1.0;
}

double mbse_compute_cohesion(SystemModel* m){
    if(!m||m->n_blocks<2)return 1.0;
    double total=0.0;int count=0;
    for(int i=0;i<m->n_interfaces;i++){
        MBSEInterface* iface=&m->interfaces[i];
        if(iface->source_block>=0&&iface->target_block>=0){
            total+=iface->data_rate+iface->reliability;count++;
        }
    }
    return(count>0)?total/(double)count:0.0;
}

int mbse_arch_pattern_detect(SystemModel* m){
    if(!m||m->n_blocks<3)return 0;
    int hub_count=0;int max_if=0,hub_id=-1;
    for(int i=0;i<m->n_blocks;i++){
        int if_count=0;
        for(int j=0;j<m->n_interfaces;j++)
            if(m->interfaces[j].source_block==i||m->interfaces[j].target_block==i)
                if_count++;
        if(if_count>max_if){max_if=if_count;hub_id=i;}
    }
    for(int i=0;i<m->n_blocks;i++)if(i!=hub_id)hub_count++;
    if(max_if>hub_count*0.6)return 1;
    int chain_len=1,current=0;
    for(int step=0;step<m->n_blocks;step++){
        int next=-1;
        for(int j=0;j<m->n_interfaces;j++)
            if(m->interfaces[j].source_block==current){next=m->interfaces[j].target_block;break;}
        if(next>=0&&next!=current){chain_len++;current=next;}else break;
    }
    if(chain_len>m->n_blocks*0.7)return 2;
    if(m->n_interfaces>m->n_blocks*1.5)return 3;
    return 0;
}

int mbse_count_components(SystemModel* m){
    if(!m||m->n_blocks<1)return 0;
    bool* visited=calloc(m->n_blocks,sizeof(bool));
    int* stack=calloc(m->n_blocks,sizeof(int));
    int comps=0;
    for(int i=0;i<m->n_blocks;i++){
        if(visited[i])continue;
        comps++;int top=0;stack[top++]=i;visited[i]=true;
        while(top>0){
            int v=stack[--top];
            for(int j=0;j<m->n_interfaces;j++){
                int nb=-1;
                if(m->interfaces[j].source_block==v)nb=m->interfaces[j].target_block;
                else if(m->interfaces[j].target_block==v)nb=m->interfaces[j].source_block;
                if(nb>=0&&!visited[nb]){visited[nb]=true;stack[top++]=nb;}
            }
        }
    }
    free(visited);free(stack);return comps;
}

/* ── MBSE Architecture Decision Scorecard ───────────────────────── */
double mbse_arch_decision_score(SystemModel* m, const double* weights,
                                  const double* scores, int n_criteria) {
    if (!m || !weights || !scores || n_criteria <= 0) return 0.0;
    double total = 0.0, w_sum = 0.0;
    for (int i = 0; i < n_criteria; i++) {
        total += weights[i] * scores[i]; w_sum += weights[i];
    }
    return (w_sum > 1e-12) ? total / w_sum : 0.0;
}

/* ── MBSE Technical Performance Measure variance ────────────────── */
double mbse_tpm_variance(double current, double planned, double threshold) {
    if (fabs(planned) < 1e-12) return 0.0;
    double variance = (current - planned) / planned;
    return (fabs(variance) > threshold) ? variance : 0.0;
}

/* ── MBSE Schedule Risk Index (NAFCOM knowledge base) ───────────── */
double mbse_schedule_risk_index(SystemModel* m, double tech_novelty) {
    if (!m) return 0.0;
    double mod = mbse_compute_modularity(m);
    double quality = mbse_arch_quality_score(m);
    return 0.4 * (1.0 - mod) + 0.4 * tech_novelty + 0.2 * (1.0 - quality);
}
