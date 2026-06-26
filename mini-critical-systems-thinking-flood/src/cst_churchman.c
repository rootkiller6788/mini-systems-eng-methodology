#include "cst_churchman.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

CSTChurchmanAnalysis* cst_churchman_create(CSTSystem* sys) {
    CSTChurchmanAnalysis* ca = calloc(1, sizeof(CSTChurchmanAnalysis));
    ca->sys = sys;
    ca->enemies = calloc(9, sizeof(CSTEnemy));
    ca->n_enemies = 9;
    CSTEnemyType types[] = {CST_ENEMY_POLITICS,CST_ENEMY_MORALITY,CST_ENEMY_RELIGION,CST_ENEMY_AESTHETICS,CST_ENEMY_EFFICIENCY,CST_ENEMY_RATIONALITY,CST_ENEMY_EXPERTISE,CST_ENEMY_PLANNING,CST_ENEMY_MODELS};
    for (int i=0;i<9;i++) {ca->enemies[i].type=types[i]; ca->enemies[i].name=strdup(cst_enemy_name(types[i])); ca->enemies[i].description=strdup(cst_enemy_description(types[i])); ca->enemies[i].warning_sign=strdup(cst_enemy_warning(types[i])); ca->enemies[i].threat_level=0.3;}
    return ca;
}
void cst_churchman_free(CSTChurchmanAnalysis* ca) { if(!ca)return; for(int i=0;i<ca->n_enemies;i++){free(ca->enemies[i].name);free(ca->enemies[i].description);free(ca->enemies[i].warning_sign);for(int j=0;j<ca->enemies[i].n_countermeasures;j++)free(ca->enemies[i].countermeasures[j]);free(ca->enemies[i].countermeasures);} free(ca->enemies); free(ca); }

void cst_churchman_identify_all(CSTChurchmanAnalysis* ca) {
    ca->enemies[CST_ENEMY_POLITICS].threat_level = (ca->sys->awareness.boundary_openness < 0.4) ? 0.8 : 0.3;
    ca->enemies[CST_ENEMY_MORALITY].threat_level = fmax(0.1, 0.5 - ca->sys->emancipatory_index);
    ca->enemies[CST_ENEMY_AESTHETICS].threat_level = (ca->sys->systemicity_score < 0.4) ? 0.7 : 0.3;
    ca->enemies[CST_ENEMY_EFFICIENCY].threat_level = 0.4;
    ca->enemies[CST_ENEMY_RATIONALITY].threat_level = (ca->sys->critical_depth < 0.5) ? 0.6 : 0.3;
    ca->enemies[CST_ENEMY_EXPERTISE].threat_level = (ca->sys->n_stakeholders < 5) ? 0.5 : 0.2;
    ca->enemies[CST_ENEMY_PLANNING].threat_level = 0.3;
    ca->enemies[CST_ENEMY_MODELS].threat_level = (cst_boundary_openness(ca->sys) < 0.3) ? 0.7 : 0.3;
    int sacred_found = 0; for(int i=0;i<ca->sys->awareness.n_boundaries;i++) if(ca->sys->awareness.boundaries[i].type==CST_SACRED) sacred_found=1;
    ca->enemies[CST_ENEMY_RELIGION].threat_level = sacred_found ? 0.6 : 0.3;
    for(int i=0;i<9;i++) if(ca->enemies[i].threat_level>0.5) ca->enemies[i].is_active=true;
}
void cst_churchman_assess_threats(CSTChurchmanAnalysis* ca) {
    double sum=0.0; ca->n_active_threats=0;
    for(int i=0;i<ca->n_enemies;i++){sum+=ca->enemies[i].threat_level; if(ca->enemies[i].is_active)ca->n_active_threats++;}
    ca->overall_threat=(ca->n_enemies>0)?sum/ca->n_enemies:0.0;
    ca->is_vulnerable=ca->overall_threat>0.5; ca->defense_readiness=1.0-ca->overall_threat;
}
void cst_churchman_recommend_defenses(CSTChurchmanAnalysis* ca) {
    for(int i=0;i<ca->n_enemies;i++){
        if(!ca->enemies[i].is_active)continue;
        free(ca->enemies[i].countermeasures); ca->enemies[i].countermeasures=NULL; ca->enemies[i].n_countermeasures=0;
        switch(ca->enemies[i].type){
            case CST_ENEMY_POLITICS: ca->enemies[i].countermeasures=malloc(2*sizeof(char*)); ca->enemies[i].countermeasures[0]=strdup("Include marginalized stakeholders"); ca->enemies[i].countermeasures[1]=strdup("Make power dynamics explicit"); ca->enemies[i].n_countermeasures=2; break;
            case CST_ENEMY_MORALITY: ca->enemies[i].countermeasures=malloc(1*sizeof(char*)); ca->enemies[i].countermeasures[0]=strdup("Conduct ethical boundary critique"); ca->enemies[i].n_countermeasures=1; break;
            case CST_ENEMY_MODELS: ca->enemies[i].countermeasures=malloc(2*sizeof(char*)); ca->enemies[i].countermeasures[0]=strdup("Validate model with stakeholders"); ca->enemies[i].countermeasures[1]=strdup("Use multiple diverse models"); ca->enemies[i].n_countermeasures=2; break;
            default: ca->enemies[i].countermeasures=malloc(1*sizeof(char*)); ca->enemies[i].countermeasures[0]=strdup("Apply critical reflection"); ca->enemies[i].n_countermeasures=1; break;
        }
    }
}
const char* cst_enemy_name(CSTEnemyType e) { switch(e){case CST_ENEMY_POLITICS:return"Politics";case CST_ENEMY_MORALITY:return"Morality";case CST_ENEMY_RELIGION:return"Religion";case CST_ENEMY_AESTHETICS:return"Aesthetics";case CST_ENEMY_EFFICIENCY:return"Efficiency";case CST_ENEMY_RATIONALITY:return"Rationality";case CST_ENEMY_EXPERTISE:return"Expertise";case CST_ENEMY_PLANNING:return"Planning";case CST_ENEMY_MODELS:return"Models";default:return"Unknown";} }
const char* cst_enemy_description(CSTEnemyType e) { switch(e){case CST_ENEMY_POLITICS:return"Power dynamics exclude certain perspectives";case CST_ENEMY_MORALITY:return"Ethical judgments suppressed in objective analysis";case CST_ENEMY_RELIGION:return"Ultimate concerns dismissed as irrational";case CST_ENEMY_AESTHETICS:return"Beauty and quality ignored";case CST_ENEMY_EFFICIENCY:return"Optimizing parts destroys the whole";case CST_ENEMY_RATIONALITY:return"Bounded rationality limits comprehension";case CST_ENEMY_EXPERTISE:return"Expert knowledge crowds out local wisdom";case CST_ENEMY_PLANNING:return"Plans become more real than reality";case CST_ENEMY_MODELS:return"Models replace the system they model";default:return"Unknown";} }
const char* cst_enemy_warning(CSTEnemyType e) { switch(e){case CST_ENEMY_POLITICS:return"Who is excluded?";case CST_ENEMY_MORALITY:return"What values are suppressed?";case CST_ENEMY_RELIGION:return"What sacred commitments are ignored?";case CST_ENEMY_AESTHETICS:return"Is the solution beautiful?";case CST_ENEMY_EFFICIENCY:return"Optimizing parts or whole?";case CST_ENEMY_RATIONALITY:return"What lies beyond comprehension?";case CST_ENEMY_EXPERTISE:return"Whose knowledge is privileged?";case CST_ENEMY_PLANNING:return"Plan more important than purpose?";case CST_ENEMY_MODELS:return"Managing model or system?";default:return"Unknown";} }
double cst_churchman_vulnerability_index(const CSTChurchmanAnalysis* ca){return ca->overall_threat;}
bool cst_churchman_needs_defense(const CSTChurchmanAnalysis* ca, CSTEnemyType e){return (e>=0&&(int)e<ca->n_enemies)?ca->enemies[(int)e].is_active:false;}
double cst_churchman_defense_score(const CSTChurchmanAnalysis* ca){return ca->defense_readiness;}
bool cst_churchman_is_well_defended(const CSTChurchmanAnalysis* ca){return ca->defense_readiness>0.7;}
int cst_churchman_count_active_enemies(const CSTChurchmanAnalysis* ca){return ca->n_active_threats;}
const char* cst_churchman_most_dangerous(const CSTChurchmanAnalysis* ca){int w=-1;double mt=0.0;for(int i=0;i<ca->n_enemies;i++)if(ca->enemies[i].threat_level>mt){mt=ca->enemies[i].threat_level;w=i;}return(w>=0)?ca->enemies[w].name:"None";}
void cst_churchman_deploy_defense(CSTChurchmanAnalysis* ca, CSTEnemyType e){if(e<0||(int)e>=ca->n_enemies)return;ca->enemies[(int)e].threat_level*=0.5;ca->enemies[(int)e].is_active=false;}
void cst_churchman_deploy_all_defenses(CSTChurchmanAnalysis* ca){cst_churchman_recommend_defenses(ca);for(int i=0;i<ca->n_enemies;i++)if(ca->enemies[i].is_active)cst_churchman_deploy_defense(ca,ca->enemies[i].type);}
double cst_churchman_systemic_health(const CSTChurchmanAnalysis* ca){return(ca->defense_readiness+1.0-ca->overall_threat)/2.0;}
bool cst_churchman_crisis_mode(const CSTChurchmanAnalysis* ca){return ca->n_active_threats>=4||ca->overall_threat>0.7;}
int cst_churchman_priority_enemies(const CSTChurchmanAnalysis* ca, CSTEnemyType* out, int max_n){int n=0;for(int i=0;i<ca->n_enemies&&n<max_n;i++)if(ca->enemies[i].is_active)out[n++]=ca->enemies[i].type;return n;}
void cst_churchman_dialectical_sweep(CSTChurchmanAnalysis* ca){for(int i=0;i<ca->sys->awareness.n_boundaries;i++){CSTBoundary* b=&ca->sys->awareness.boundaries[i];if(b->type==CST_MARGINALIZED)b->ethical_weight=fmin(1.0,b->ethical_weight+0.2);if(b->type==CST_SACRED)b->ethical_weight=fmin(1.0,b->ethical_weight-0.1);}ca->sys->emancipatory_index=fmin(1.0,ca->sys->emancipatory_index+0.15);}
double cst_churchman_sweep_impact(const CSTChurchmanAnalysis* ca){double impact=0.0;int n=0;for(int i=0;i<ca->sys->awareness.n_boundaries;i++){impact+=ca->sys->awareness.boundaries[i].ethical_weight;n++;}return(n>0)?impact/n:0.0;}
bool cst_churchman_boundary_shifted(const CSTChurchmanAnalysis* ca){return ca->sys->awareness.critical_awareness_index>0.5||ca->sys->emancipatory_index>0.4;}
void cst_churchman_genealogy_of_boundaries(const CSTChurchmanAnalysis* ca){
    printf("=== Boundary Genealogy ===\n");
    for(int i=0;i<ca->sys->awareness.n_boundaries;i++){CSTBoundary* b=&ca->sys->awareness.boundaries[i];printf("  [%s] %s\n  In:%s\n  Out:%s\n  Why:%s\n",b->source?b->source:"unknown",b->name,b->what_is_included,b->what_is_excluded,b->justification);}
}
void cst_churchman_report_full(const CSTChurchmanAnalysis* ca){cst_churchman_print(ca);printf("Most dangerous: %s\nWell-defended: %s\n",cst_churchman_most_dangerous(ca),cst_churchman_is_well_defended(ca)?"YES":"NO");}
void cst_churchman_strategic_review(const CSTChurchmanAnalysis* ca){cst_churchman_report_full(ca);printf("Systemic health: %.2f\nCrisis mode: %s\n",cst_churchman_systemic_health(ca),cst_churchman_crisis_mode(ca)?"YES":"NO");}
void cst_churchman_print(const CSTChurchmanAnalysis* ca){
    printf("=== Churchman Enemies ===\nThreat=%.3f Defense=%.3f Vulnerable=%s Active=%d\n",ca->overall_threat,ca->defense_readiness,ca->is_vulnerable?"YES":"NO",ca->n_active_threats);
    for(int i=0;i<ca->n_enemies;i++)if(ca->enemies[i].is_active)printf("  [ACTIVE] %-15s threat=%.2f -- %s\n",ca->enemies[i].name,ca->enemies[i].threat_level,ca->enemies[i].warning_sign);
}

/* Churchman dialectical inquiry: thesis-antithesis-synthesis */
void cst_churchman_dialectical_pair(CSTChurchmanAnalysis* ca, CSTEnemyType e1, CSTEnemyType e2){
    double t1=ca->enemies[(int)e1].threat_level,t2=ca->enemies[(int)e2].threat_level;
    double synthesis=(t1*(1.0-t2)+t2*(1.0-t1))/2.0;
    printf("Dialectic %s vs %s: t1=%.2f t2=%.2f synthesis=%.2f\n",cst_enemy_name(e1),cst_enemy_name(e2),t1,t2,synthesis);
    ca->enemies[(int)e1].threat_level=fmax(0.1,t1*0.7+synthesis*0.3);
    ca->enemies[(int)e2].threat_level=fmax(0.1,t2*0.7+synthesis*0.3);
}
/* Enemy interaction: some enemies amplify each other */
double cst_churchman_enemy_interaction(const CSTChurchmanAnalysis* ca, CSTEnemyType e1, CSTEnemyType e2){
    static const double interaction[9][9]={
        {1.0,0.5,0.2,0.3,0.6,0.4,0.7,0.5,0.4},{0.5,1.0,0.6,0.4,0.3,0.3,0.2,0.3,0.3},
        {0.2,0.6,1.0,0.5,0.2,0.2,0.1,0.2,0.2},{0.3,0.4,0.5,1.0,0.3,0.4,0.2,0.3,0.3},
        {0.6,0.3,0.2,0.3,1.0,0.5,0.6,0.4,0.7},{0.4,0.3,0.2,0.4,0.5,1.0,0.5,0.3,0.5},
        {0.7,0.2,0.1,0.2,0.6,0.5,1.0,0.4,0.6},{0.5,0.3,0.2,0.3,0.4,0.3,0.4,1.0,0.5},
        {0.4,0.3,0.2,0.3,0.7,0.5,0.6,0.5,1.0}
    };
    double t1=ca->enemies[(int)e1].threat_level,t2=ca->enemies[(int)e2].threat_level;
    return (t1+t2)/2.0*interaction[(int)e1][(int)e2];
}
/* Historical configuration presets */
void cst_churchman_configure_urban(CSTChurchmanAnalysis* ca){
    ca->enemies[CST_ENEMY_POLITICS].threat_level=0.8;ca->enemies[CST_ENEMY_POLITICS].is_active=true;
    ca->enemies[CST_ENEMY_EFFICIENCY].threat_level=0.7;ca->enemies[CST_ENEMY_EFFICIENCY].is_active=true;
    ca->enemies[CST_ENEMY_MODELS].threat_level=0.6;ca->enemies[CST_ENEMY_MODELS].is_active=true;
    ca->enemies[CST_ENEMY_MORALITY].threat_level=0.5;
    cst_churchman_assess_threats(ca);
}
void cst_churchman_configure_healthcare(CSTChurchmanAnalysis* ca){
    ca->enemies[CST_ENEMY_EXPERTISE].threat_level=0.8;ca->enemies[CST_ENEMY_EXPERTISE].is_active=true;
    ca->enemies[CST_ENEMY_MORALITY].threat_level=0.7;ca->enemies[CST_ENEMY_MORALITY].is_active=true;
    ca->enemies[CST_ENEMY_PLANNING].threat_level=0.6;ca->enemies[CST_ENEMY_PLANNING].is_active=true;
    cst_churchman_assess_threats(ca);
}
void cst_churchman_configure_environmental(CSTChurchmanAnalysis* ca){
    ca->enemies[CST_ENEMY_RELIGION].threat_level=0.8;ca->enemies[CST_ENEMY_RELIGION].is_active=true;
    ca->enemies[CST_ENEMY_AESTHETICS].threat_level=0.7;ca->enemies[CST_ENEMY_AESTHETICS].is_active=true;
    ca->enemies[CST_ENEMY_MODELS].threat_level=0.9;ca->enemies[CST_ENEMY_MODELS].is_active=true;
    ca->enemies[CST_ENEMY_RATIONALITY].threat_level=0.6;ca->enemies[CST_ENEMY_RATIONALITY].is_active=true;
    cst_churchman_assess_threats(ca);
}
/* Defense strategy ensemble: try multiple defenses, pick best */
int cst_churchman_optimal_defense_order(const CSTChurchmanAnalysis* ca, CSTEnemyType* order, int max_n){
    double priority[9];CSTEnemyType types[9];
    for(int i=0;i<9;i++){priority[i]=ca->enemies[i].is_active?ca->enemies[i].threat_level:0.0;types[i]=(CSTEnemyType)i;}
    for(int i=0;i<8;i++)for(int j=i+1;j<9;j++)if(priority[j]>priority[i]){double tp=priority[i];priority[i]=priority[j];priority[j]=tp;CSTEnemyType tt=types[i];types[i]=types[j];types[j]=tt;}
    int n=0;for(int i=0;i<9&&n<max_n;i++)if(priority[i]>0){order[n++]=types[i];}
    return n;
}

/* Churchman's "The Design of Inquiring Systems" (1971) */
typedef enum{CST_INQUIRY_LEIBNIZ=0,CST_INQUIRY_LOCKE=1,CST_INQUIRY_KANT=2,CST_INQUIRY_HEGEL=3,CST_INQUIRY_SINGER=4}CSTInquiringSystem;
double cst_churchman_inquiring_score(const CSTChurchmanAnalysis* ca, CSTInquiringSystem is){
    switch(is){
        case CST_INQUIRY_LEIBNIZ:return ca->sys->systemicity_score;
        case CST_INQUIRY_LOCKE:return cst_stakeholder_inclusiveness(ca->sys);
        case CST_INQUIRY_KANT:return ca->sys->critical_depth;
        case CST_INQUIRY_HEGEL:return 1.0-ca->sys->awareness.boundary_openness;
        case CST_INQUIRY_SINGER:return ca->sys->emancipatory_index;
        default:return 0.0;
    }
}
const char* cst_churchman_inquiring_label(CSTInquiringSystem is){switch(is){case CST_INQUIRY_LEIBNIZ:return"Leibnizian (formal models)";case CST_INQUIRY_LOCKE:return"Lockean (empirical consensus)";case CST_INQUIRY_KANT:return"Kantian (multiple perspectives)";case CST_INQUIRY_HEGEL:return"Hegelian (dialectical synthesis)";case CST_INQUIRY_SINGER:return"Singerian (pragmatic progress)";default:return"Unknown";}}

/* Churchman's moral responsibility of systems designers */
double cst_churchman_moral_responsibility(const CSTChurchmanAnalysis* ca){
    return ca->enemies[CST_ENEMY_MORALITY].threat_level>0.5?1.0-ca->enemies[CST_ENEMY_MORALITY].threat_level:0.7;
}
bool cst_churchman_is_ethically_responsible(const CSTChurchmanAnalysis* ca){return cst_churchman_moral_responsibility(ca)>0.5;}

/* Sweeping-in process: expanding system boundaries */
void cst_churchman_sweep_in_process(CSTChurchmanAnalysis* ca){
    for(int i=0;i<ca->sys->awareness.n_boundaries;i++){
        if(ca->sys->awareness.boundaries[i].type!=CST_EMPIRICAL)ca->sys->awareness.boundaries[i].ethical_weight=fmin(1.0,ca->sys->awareness.boundaries[i].ethical_weight+0.15);
    }
    ca->sys->critical_depth=fmin(1.0,ca->sys->critical_depth+0.05);
    cst_churchman_identify_all(ca);
}
int cst_churchman_count_sweepable(const CSTChurchmanAnalysis* ca){
    int c=0;for(int i=0;i<ca->sys->awareness.n_boundaries;i++)if(ca->sys->awareness.boundaries[i].ethical_weight<0.7)c++;return c;
}
