#include "cst_core.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

CSTSystem* cst_create(const char* name) {
    CSTSystem* s = calloc(1, sizeof(CSTSystem));
    if (!s) return NULL;
    s->name = strdup(name); s->time = 0.0; s->iteration = 0;
    s->current_phase = CST_CREATIVITY; s->phase_progress = 0.0;
    s->systemicity_score = 0.5; s->pluralism_index = 0.0;
    s->critical_depth = 0.3; s->emancipatory_index = 0.0;
    s->stakeholders = calloc(CST_MAX_STAKEHOLDERS, sizeof(CSTStakeholder));
    s->metaphors = calloc(CST_MAX_METAPHORS, sizeof(CSTMetaphorLens));
    s->methodologies = calloc(CST_MAX_METHODOLOGIES, sizeof(CSTMethodology));
    return s;
}

void cst_free(CSTSystem* s) {
    if (!s) return;
    free(s->name);
    for (int i = 0; i < s->n_stakeholders; i++) { free(s->stakeholders[i].name); free(s->stakeholders[i].role); }
    free(s->stakeholders);
    for (int i = 0; i < s->n_metaphors; i++) {
        free(s->metaphors[i].name);
        if (s->metaphors[i].key_insights) {
            for (int j = 0; j < s->metaphors[i].n_insights; j++) free(s->metaphors[i].key_insights[j]);
            free(s->metaphors[i].key_insights);
        }
    }
    free(s->metaphors);
    for (int i = 0; i < s->n_methodologies; i++) { free(s->methodologies[i].name); free(s->methodologies[i].strengths); free(s->methodologies[i].weaknesses); }
    free(s->methodologies);
    free(s->context.description);
    if (s->awareness.boundaries) {
        for (int i = 0; i < s->awareness.n_boundaries; i++) {
            free(s->awareness.boundaries[i].name); free(s->awareness.boundaries[i].what_is_included);
            free(s->awareness.boundaries[i].what_is_excluded); free(s->awareness.boundaries[i].justification);
            free(s->awareness.boundaries[i].source);
        }
        free(s->awareness.boundaries);
    }
    if (s->awareness.blind_spots) { for (int i = 0; i < s->awareness.n_blind_spots; i++) free(s->awareness.blind_spots[i]); free(s->awareness.blind_spots); }
    free(s);
}

int cst_add_stakeholder(CSTSystem* s, const char* name, const char* role, double power, double legitimacy, double urgency) {
    int i = s->n_stakeholders++; CSTStakeholder* sh = &s->stakeholders[i]; memset(sh, 0, sizeof(CSTStakeholder));
    sh->name = strdup(name); sh->role = strdup(role); sh->power = power; sh->legitimacy = legitimacy; sh->urgency = urgency; return i;
}
void cst_compute_stakeholder_salience(CSTSystem* s) { for (int i = 0; i < s->n_stakeholders; i++) { s->stakeholders[i].salience = (s->stakeholders[i].power + s->stakeholders[i].legitimacy + s->stakeholders[i].urgency) / 3.0; } }
void cst_identify_marginalized(CSTSystem* s) { for (int i = 0; i < s->n_stakeholders; i++) { CSTStakeholder* sh = &s->stakeholders[i]; sh->is_marginalized = (sh->power < 0.3 && sh->legitimacy < 0.5) || (sh->salience < 0.2); } }
double cst_stakeholder_inclusiveness(const CSTSystem* s) { if (s->n_stakeholders == 0) return 0.0; int inc = 0; for (int i=0;i<s->n_stakeholders;i++) if(!s->stakeholders[i].is_marginalized)inc++; return (double)inc/s->n_stakeholders; }
int cst_add_metaphor(CSTSystem* s, const char* name, CSTMetaphor type) { int i = s->n_metaphors++; CSTMetaphorLens* ml = &s->metaphors[i]; memset(ml, 0, sizeof(CSTMetaphorLens)); ml->name = strdup(name); ml->type = type; return i; }
void cst_assess_metaphor(CSTSystem* s, int idx, double rel, double ins) { if (idx>=0&&idx<s->n_metaphors) { s->metaphors[idx].relevance=rel; s->metaphors[idx].insight_score=ins; } }
void cst_select_dominant_metaphor(CSTSystem* s) { double best=-1.0; int bi=-1; for(int i=0;i<s->n_metaphors;i++){double sc=s->metaphors[i].relevance*0.4+s->metaphors[i].insight_score*0.6; if(sc>best){best=sc;bi=i;}} for(int i=0;i<s->n_metaphors;i++)s->metaphors[i].dominant=(i==bi); }
const char* cst_metaphor_description(CSTMetaphor m) { switch(m){case CST_MACHINE:return"Machine: rules, hierarchy, control";case CST_ORGANISM:return"Organism: adaptation, environment, needs";case CST_BRAIN:return"Brain: learning, information processing";case CST_CULTURE:return"Culture: values, norms, shared meaning";case CST_POLITICAL:return"Political: power, conflict, interests";case CST_PSYCHIC_PRISON:return"Psychic Prison: trapped thinking";case CST_FLUX:return"Flux: change, emergence, self-organization";case CST_INSTRUMENT:return"Instrument: exploitation, domination";default:return"Unknown";} }
const char* cst_metaphor_question(CSTMetaphor m) { switch(m){case CST_MACHINE:return"Is this system functioning efficiently?";case CST_ORGANISM:return"What does this system need to survive?";case CST_BRAIN:return"How does this system learn?";case CST_CULTURE:return"What values shape this system?";case CST_POLITICAL:return"Who has power?";case CST_PSYCHIC_PRISON:return"What assumptions trap this system?";case CST_FLUX:return"What patterns are transforming this system?";case CST_INSTRUMENT:return"Who is being exploited?";default:return"What is this system?";} }
int cst_add_methodology(CSTSystem* s, const char* name, CSTMethodFamily family) { int i=s->n_methodologies++; CSTMethodology* m=&s->methodologies[i]; memset(m,0,sizeof(CSTMethodology)); m->name=strdup(name); m->family=family; return i; }
void cst_assess_methodology(CSTSystem* s, int idx, double suit, double feas, double accept) { if(idx<0||idx>=s->n_methodologies)return; CSTMethodology* m=&s->methodologies[idx]; m->suitability=suit; m->feasibility=feas; m->acceptability=accept; m->overall_score=suit*0.4+feas*0.3+accept*0.3; }
void cst_select_methodologies(CSTSystem* s, int max_count) { for(int i=0;i<s->n_methodologies-1;i++)for(int j=i+1;j<s->n_methodologies;j++)if(s->methodologies[j].overall_score>s->methodologies[i].overall_score){CSTMethodology t=s->methodologies[i];s->methodologies[i]=s->methodologies[j];s->methodologies[j]=t;} int cnt=0; for(int i=0;i<s->n_methodologies;i++){s->methodologies[i].selected=(cnt<max_count);if(s->methodologies[i].selected)cnt++;} }
void cst_classify_context(CSTSystem* s, CSTComplexity comp, CSTRelationship rel, const char* desc) { s->context.complexity=comp; s->context.relationship=rel; free(s->context.description); s->context.description=strdup(desc); s->context.is_coercive_context=(rel==CST_COERCIVE); s->context.requires_emancipatory=(rel==CST_COERCIVE&&comp==CST_COMPLEX); }
const char* cst_sosm_cell_label(const CSTSystem* s) { static const char* lbl[3][3]={{"Simple-Unitary (Hard SE)","Simple-Pluralist (Soft OR)","Simple-Coercive (Critical)"},{"Complex-Unitary (SD)","Complex-Pluralist (SSM)","Complex-Coercive (CST/TSI)"},{"Chaotic-Unitary (Crisis)","Chaotic-Pluralist (Postmodern)","Chaotic-Coercive (Emancipatory)"}}; int r=(int)s->context.complexity,c=(int)s->context.relationship; return(r>=0&&r<3&&c>=0&&c<3)?lbl[r][c]:"Unknown"; }
const char* cst_recommended_methodology(const CSTSystem* s) { static const char* rec[3][3]={{"Hard SE/OR","SAST/SODA","CSH"},{"SD/VSM","Soft Systems Methodology","TSI/Boundary Critique"},{"Crisis Management","Postmodern Deconstruction","Emancipatory Action Research"}}; int r=(int)s->context.complexity,c=(int)s->context.relationship; return(r>=0&&r<3&&c>=0&&c<3)?rec[r][c]:"More analysis needed"; }
double cst_metaphor_distance(CSTMetaphor a, CSTMetaphor b) { static const double d[8][8]={{0.0,0.4,0.3,0.6,0.5,0.8,0.7,0.3},{0.4,0.0,0.2,0.3,0.4,0.6,0.5,0.7},{0.3,0.2,0.0,0.4,0.5,0.5,0.3,0.6},{0.6,0.3,0.4,0.0,0.2,0.3,0.5,0.5},{0.5,0.4,0.5,0.2,0.0,0.3,0.4,0.1},{0.8,0.6,0.5,0.3,0.3,0.0,0.6,0.4},{0.7,0.5,0.3,0.5,0.4,0.6,0.0,0.7},{0.3,0.7,0.6,0.5,0.1,0.4,0.7,0.0}}; return d[(int)a][(int)b]; }
double cst_metaphor_pluralism_score(const CSTSystem* s) { if(s->n_metaphors<2)return 0.0; double sum=0.0;int p=0; for(int i=0;i<s->n_metaphors;i++)for(int j=i+1;j<s->n_metaphors;j++){sum+=cst_metaphor_distance(s->metaphors[i].type,s->metaphors[j].type);p++;} return(p>0)?sum/p:0.0; }
int cst_add_boundary(CSTSystem* s, const char* name, CSTBoundaryType type, const char* inc, const char* exc, const char* just, const char* src) { s->awareness.boundaries=realloc(s->awareness.boundaries,(size_t)(s->awareness.n_boundaries+1)*sizeof(CSTBoundary)); int i=s->awareness.n_boundaries++; CSTBoundary* b=&s->awareness.boundaries[i]; memset(b,0,sizeof(CSTBoundary)); b->name=strdup(name); b->type=type; b->what_is_included=strdup(inc); b->what_is_excluded=strdup(exc); b->justification=strdup(just); b->source=strdup(src); b->ethical_weight=0.5; return i; }
void cst_analyze_boundaries(CSTSystem* s) { int contested=0,marg=0; double sum=0.0; for(int i=0;i<s->awareness.n_boundaries;i++){CSTBoundary* b=&s->awareness.boundaries[i]; if(b->type==CST_MARGINALIZED)marg++; if(b->is_contested)contested++; double el=(double)strlen(b->what_is_excluded),il=(double)strlen(b->what_is_included); sum+=(el>0)?il/(il+el):1.0; } s->awareness.n_marginalized_voices=marg; s->awareness.boundary_openness=(s->awareness.n_boundaries>0)?sum/s->awareness.n_boundaries:0.5; s->awareness.critical_awareness_index=contested*0.3+(1.0-s->awareness.boundary_openness)*0.7; if(s->awareness.critical_awareness_index>1.0)s->awareness.critical_awareness_index=1.0; }
void cst_identify_blind_spots(CSTSystem* s) { if(s->awareness.blind_spots){for(int i=0;i<s->awareness.n_blind_spots;i++)free(s->awareness.blind_spots[i]);free(s->awareness.blind_spots);} s->awareness.blind_spots=NULL; s->awareness.n_blind_spots=0; int cnt[4]={0}; for(int i=0;i<s->awareness.n_boundaries;i++)cnt[(int)s->awareness.boundaries[i].type]++; const char* names[4]={"Empirical not examined","Normative not examined","Sacred not surfaced","Marginalized excluded"}; for(int t=0;t<4;t++)if(cnt[t]==0){s->awareness.blind_spots=realloc(s->awareness.blind_spots,(size_t)(s->awareness.n_blind_spots+1)*sizeof(char*));s->awareness.blind_spots[s->awareness.n_blind_spots++]=strdup(names[t]);} }
double cst_boundary_openness(const CSTSystem* s){return s->awareness.boundary_openness;}
void cst_tsi_creativity_phase(CSTSystem* s){s->current_phase=CST_CREATIVITY;cst_select_dominant_metaphor(s);cst_analyze_boundaries(s);cst_identify_blind_spots(s);s->creativity_rounds++;s->phase_progress=1.0;s->critical_depth=fmin(1.0,s->critical_depth+0.1);}
void cst_tsi_choice_phase(CSTSystem* s){s->current_phase=CST_CHOICE;cst_classify_context(s,s->context.complexity,s->context.relationship,s->context.description);cst_select_methodologies(s,3);s->phase_progress=1.0;}
void cst_tsi_implementation_phase(CSTSystem* s){s->current_phase=CST_IMPLEMENTATION;s->implementation_rounds++;int sel=0;for(int i=0;i<s->n_methodologies;i++)if(s->methodologies[i].selected)sel++;s->pluralism_index=(sel>1)?(double)(sel-1)/sel:0.0;s->emancipatory_index=fmin(1.0,s->awareness.n_marginalized_voices*0.25);s->phase_progress=1.0;}
void cst_tsi_reflection_phase(CSTSystem* s){s->current_phase=CST_REFLECTION;cst_analyze_boundaries(s);s->systemicity_score=s->critical_depth*0.5+s->pluralism_index*0.5;s->phase_progress=1.0;s->iteration++;}
void cst_tsi_full_cycle(CSTSystem* s){cst_tsi_creativity_phase(s);cst_tsi_choice_phase(s);cst_tsi_implementation_phase(s);cst_tsi_reflection_phase(s);}
double cst_critical_awareness_index(CSTSystem* s){cst_analyze_boundaries(s);return s->awareness.critical_awareness_index;}
double cst_emancipatory_index(CSTSystem* s){return s->emancipatory_index;}
double cst_pluralism_index(CSTSystem* s){return s->pluralism_index;}
double cst_systemicity_score(CSTSystem* s){return s->systemicity_score;}
double cst_critical_depth(CSTSystem* s){return s->critical_depth;}
bool cst_is_coercive_context(const CSTSystem* s){return s->context.is_coercive_context;}
bool cst_is_emancipatory_needed(const CSTSystem* s){return s->context.requires_emancipatory;}
int cst_count_blind_spots(const CSTSystem* s){return s->awareness.n_blind_spots;}
bool cst_has_marginalized_stakeholders(const CSTSystem* s){for(int i=0;i<s->n_stakeholders;i++)if(s->stakeholders[i].is_marginalized)return true;return false;}
bool cst_has_contested_boundaries(const CSTSystem* s){for(int i=0;i<s->awareness.n_boundaries;i++)if(s->awareness.boundaries[i].is_contested)return true;return false;}
bool cst_has_sacred_boundaries(const CSTSystem* s){for(int i=0;i<s->awareness.n_boundaries;i++)if(s->awareness.boundaries[i].type==CST_SACRED)return true;return false;}
double cst_methodological_diversity_index(const CSTSystem* s){if(s->n_methodologies<2)return 0.0;int fam[5]={0};for(int i=0;i<s->n_methodologies;i++)fam[(int)s->methodologies[i].family]++;int act=0;for(int i=0;i<5;i++)if(fam[i]>0)act++;return(double)act/5.0;}
bool cst_is_methodologically_pluralist(const CSTSystem* s){return cst_methodological_diversity_index(s)>0.4;}
double cst_participation_index(const CSTSystem* s){if(s->n_stakeholders<2)return 0.0;int emp=0;for(int i=0;i<s->n_stakeholders;i++)if(s->stakeholders[i].power>0.4&&!s->stakeholders[i].is_marginalized)emp++;return(double)emp/s->n_stakeholders;}
double cst_deliberative_quality(const CSTSystem* s){return(cst_participation_index(s)+(1.0-s->awareness.boundary_openness))/2.0;}
bool cst_is_participatory_enough(const CSTSystem* s){return cst_participation_index(s)>0.5&&cst_stakeholder_inclusiveness(s)>0.7;}
double cst_learning_rate(const CSTSystem* s){return(s->iteration<1)?0.0:s->critical_depth/fmax(s->iteration,1);}
double cst_adaptation_capacity(const CSTSystem* s){return(cst_metaphor_pluralism_score(s)+cst_methodological_diversity_index(s))/2.0;}
bool cst_is_learning_system(const CSTSystem* s){return s->iteration>=2&&s->critical_depth>0.4&&cst_adaptation_capacity(s)>0.3;}
double cst_systemic_coherence(const CSTSystem* s){return((1.0-s->awareness.boundary_openness)+cst_stakeholder_inclusiveness(s)+cst_methodological_diversity_index(s))/3.0;}
double cst_theoretical_depth(const CSTSystem* s){return s->critical_depth*0.4+s->pluralism_index*0.3+cst_metaphor_pluralism_score(s)*0.3;}
int cst_count_active_metaphors(const CSTSystem* s){int c=0;for(int i=0;i<s->n_metaphors;i++)if(s->metaphors[i].relevance>0.5)c++;return c;}
int cst_count_selected_methodologies(const CSTSystem* s){int c=0;for(int i=0;i<s->n_methodologies;i++)if(s->methodologies[i].selected)c++;return c;}
int cst_intervention_mix_count(const CSTSystem* s){bool u[5]={false};int c=0;for(int i=0;i<s->n_methodologies;i++)if(s->methodologies[i].selected)u[(int)s->methodologies[i].family]=true;for(int i=0;i<5;i++)if(u[i])c++;return c;}
CSTHolisticScore cst_holistic_assess(const CSTSystem* s){CSTHolisticScore hs;memset(&hs,0,sizeof(hs));hs.systemic=cst_systemic_coherence(s);hs.critical=s->critical_depth;hs.participatory=cst_participation_index(s);hs.ethical=1.0-s->awareness.boundary_openness;hs.pluralist=cst_methodological_diversity_index(s);hs.emancipatory=s->emancipatory_index;hs.overall=hs.systemic*0.2+hs.critical*0.2+hs.participatory*0.15+hs.ethical*0.15+hs.pluralist*0.15+hs.emancipatory*0.15;return hs;}
int cst_improvement_potential(const CSTSystem* s){CSTHolisticScore h=cst_holistic_assess(s);int g=0;if(h.systemic<0.5)g++;if(h.critical<0.5)g++;if(h.participatory<0.5)g++;if(h.ethical<0.5)g++;if(h.pluralist<0.5)g++;if(h.emancipatory<0.5)g++;return g;}
void cst_metaphor_shift_dominant(CSTSystem* s, CSTMetaphor nd){for(int i=0;i<s->n_metaphors;i++)s->metaphors[i].dominant=(s->metaphors[i].type==nd);}
CSTMetaphor cst_metaphor_most_novel(const CSTSystem* s){CSTMetaphor best=CST_MACHINE;double bn=-1.0;for(int i=0;i<s->n_metaphors;i++)if(s->metaphors[i].insight_score>bn){bn=s->metaphors[i].insight_score;best=s->metaphors[i].type;}return best;}
int cst_metaphor_rank_by_insight(const CSTSystem* s, CSTMetaphor* r, int mx){double sc[8];CSTMetaphor tp[8];int n=s->n_metaphors;for(int i=0;i<n;i++){sc[i]=s->metaphors[i].insight_score;tp[i]=s->metaphors[i].type;}for(int i=0;i<n-1;i++)for(int j=i+1;j<n;j++)if(sc[j]>sc[i]){double ts=sc[i];sc[i]=sc[j];sc[j]=ts;CSTMetaphor tt=tp[i];tp[i]=tp[j];tp[j]=tt;}int k=(n<mx)?n:mx;for(int i=0;i<k;i++)r[i]=tp[i];return k;}
bool cst_detect_paradigm_shift(const CSTSystem* a, const CSTSystem* b){return fabs(a->systemicity_score-b->systemicity_score)+fabs(a->pluralism_index-b->pluralism_index)+fabs(a->critical_depth-b->critical_depth)+fabs(a->emancipatory_index-b->emancipatory_index)>0.3;}
double cst_paradigm_distance(const CSTSystem* a, const CSTSystem* b){return(fabs(a->systemicity_score-b->systemicity_score)+fabs(a->pluralism_index-b->pluralism_index)+fabs(a->critical_depth-b->critical_depth)+fabs(a->emancipatory_index-b->emancipatory_index)+fabs(a->awareness.boundary_openness-b->awareness.boundary_openness))/5.0;}
void cst_print_state(const CSTSystem* s){printf("CST: %s iter=%d CAI=%.3f Plur=%.3f Eman=%.3f\n",s->name,s->iteration,cst_critical_awareness_index((CSTSystem*)s),s->pluralism_index,s->emancipatory_index);}
void cst_print_stakeholders(const CSTSystem* s){printf("Stakeholders(%d):\n",s->n_stakeholders);for(int i=0;i<s->n_stakeholders;i++)printf("  %s P=%.2f L=%.2f U=%.2f Sal=%.2f%s\n",s->stakeholders[i].name,s->stakeholders[i].power,s->stakeholders[i].legitimacy,s->stakeholders[i].urgency,s->stakeholders[i].salience,s->stakeholders[i].is_marginalized?" [MARG]":"");}
void cst_print_boundaries(const CSTSystem* s){printf("Boundaries(%d):\n",s->awareness.n_boundaries);const char* tn[]={"Empirical","Normative","Sacred","Marginalized"};for(int i=0;i<s->awareness.n_boundaries;i++)printf("  [%s] %s\n",tn[(int)s->awareness.boundaries[i].type],s->awareness.boundaries[i].name);}
void cst_print_methodologies(const CSTSystem* s){printf("Methods(%d):\n",s->n_methodologies);const char* fn[]={"Hard","Soft","Critical","Emancipatory","Postmodern"};for(int i=0;i<s->n_methodologies;i++)printf("  %s [%s] Score=%.2f%s\n",s->methodologies[i].name,fn[(int)s->methodologies[i].family],s->methodologies[i].overall_score,s->methodologies[i].selected?" SEL":"");}
void cst_print_metaphors(const CSTSystem* s){printf("Metaphors(%d):\n",s->n_metaphors);for(int i=0;i<s->n_metaphors;i++)printf("  %s Rel=%.2f Ins=%.2f%s\n",s->metaphors[i].name,s->metaphors[i].relevance,s->metaphors[i].insight_score,s->metaphors[i].dominant?" DOM":"");}
void cst_print_holistic(const CSTSystem* s){CSTHolisticScore h=cst_holistic_assess(s);printf("Holistic: Sys=%.2f Crit=%.2f Part=%.2f Eth=%.2f Plur=%.2f Eman=%.2f Overall=%.2f\n",h.systemic,h.critical,h.participatory,h.ethical,h.pluralist,h.emancipatory,h.overall);}
void cst_print_evolution_summary(const CSTSystem* s){printf("Evol: Iter=%d Depth=%.3f Eman=%.3f Plur=%.3f\n",s->iteration,s->critical_depth,s->emancipatory_index,s->pluralism_index);}

/* ===== Multiple Salience Computation Models ===== */
double cst_salience_mitchell(double power, double legitimacy, double urgency){return(power+legitimacy+urgency)/3.0;}
double cst_salience_power_interest(double power, double interest){return power*0.6+interest*0.4;}
double cst_salience_influence_impact(double influence, double impact){return(influence>impact)?influence:impact;}
void cst_stakeholder_power_interest_grid(const CSTSystem* s, double* grid){
    double hi_power=0, hi_interest=0, lo_power=0, lo_interest=0; int nhi=0, nli=0;
    for(int i=0;i<s->n_stakeholders;i++){double p=s->stakeholders[i].power;double leg=s->stakeholders[i].legitimacy;if(p>0.5){hi_power+=p;if(leg>0.5)hi_interest+=leg;nhi++;}else{lo_power+=p;if(leg>0.5)lo_interest+=leg;nli++;}}
    grid[0]=(nhi>0)?hi_power/nhi:0;grid[1]=(nhi>0)?hi_interest/nhi:0;grid[2]=(nli>0)?lo_power/nli:0;grid[3]=(nli>0)?lo_interest/nli:0;
}
int cst_stakeholder_conflict_index(const CSTSystem* s, int a, int b){
    if(a<0||a>=s->n_stakeholders||b<0||b>=s->n_stakeholders)return 0;
    double pa=s->stakeholders[a].power, pb=s->stakeholders[b].power;
    double ua=s->stakeholders[a].urgency, ub=s->stakeholders[b].urgency;
    return(int)(fabs(pa-pb)*10+fabs(ua-ub)*10);
}

/* ===== Multiple Methodology Selection Strategies ===== */
void cst_select_methodologies_weighted(CSTSystem* s, int max_count, double ws, double wf, double wa){
    for(int i=0;i<s->n_methodologies;i++)s->methodologies[i].overall_score=s->methodologies[i].suitability*ws+s->methodologies[i].feasibility*wf+s->methodologies[i].acceptability*wa;
    cst_select_methodologies(s,max_count);
}
void cst_select_methodologies_pareto(CSTSystem* s){
    for(int i=0;i<s->n_methodologies;i++){if(cst_methodology_is_dominated(s,i))s->methodologies[i].selected=false;else s->methodologies[i].selected=true;}
}
int cst_methodology_dominance_count(const CSTSystem* s, int idx){
    if(idx<0||idx>=s->n_methodologies)return 0;int c=0;
    for(int j=0;j<s->n_methodologies;j++)if(j!=idx){if(s->methodologies[j].suitability>=s->methodologies[idx].suitability&&s->methodologies[j].feasibility>=s->methodologies[idx].feasibility&&s->methodologies[j].acceptability>=s->methodologies[idx].acceptability&&(s->methodologies[j].suitability>s->methodologies[idx].suitability||s->methodologies[j].feasibility>s->methodologies[idx].feasibility||s->methodologies[j].acceptability>s->methodologies[idx].acceptability))c++;}
    return c;
}
bool cst_methodology_is_dominated(const CSTSystem* s, int idx){return cst_methodology_dominance_count(s,idx)>0;}

/* ===== System Archetype Detection (Senge, 1990) ===== */
CSTSystemArchetype cst_detect_system_archetype(const CSTSystem* s){
    if(s->emancipatory_index<0.2&&cst_is_coercive_context(s))return CST_ARCH_TRAGEDY_COMMONS;
    if(cst_has_marginalized_stakeholders(s)&&s->emancipatory_index<0.3)return CST_ARCH_SUCCESS_TO_SUCCESSFUL;
    if(s->awareness.critical_awareness_index<0.3&&s->iteration>2)return CST_ARCH_ERODING_GOALS;
    if(s->awareness.boundary_openness<0.3&&!cst_is_coercive_context(s))return CST_ARCH_SHIFTING_BURDEN;
    if(cst_boundary_openness(s)>0.7&&s->critical_depth<0.3)return CST_ARCH_FIXES_FAIL;
    if(s->pluralism_index<0.2&&cst_is_methodologically_pluralist(s))return CST_ARCH_ESCALATION;
    if(s->systemicity_score<0.3&&s->iteration>3)return CST_ARCH_LIMITS_GROWTH;
    return CST_ARCH_NONE;
}
const char* cst_archetype_label(CSTSystemArchetype a){switch(a){case CST_ARCH_FIXES_FAIL:return"Fixes That Fail";case CST_ARCH_SHIFTING_BURDEN:return"Shifting the Burden";case CST_ARCH_LIMITS_GROWTH:return"Limits to Growth";case CST_ARCH_TRAGEDY_COMMONS:return"Tragedy of the Commons";case CST_ARCH_ERODING_GOALS:return"Eroding Goals";case CST_ARCH_ESCALATION:return"Escalation";case CST_ARCH_SUCCESS_TO_SUCCESSFUL:return"Success to the Successful";default:return"No Archetype Detected";}}
const char* cst_archetype_remedy(CSTSystemArchetype a){switch(a){case CST_ARCH_FIXES_FAIL:return"Address root causes, not symptoms";case CST_ARCH_SHIFTING_BURDEN:return"Strengthen fundamental solutions over symptomatic fixes";case CST_ARCH_LIMITS_GROWTH:return"Identify and remove limiting factors before growth stalls";case CST_ARCH_TRAGEDY_COMMONS:return"Establish shared governance and resource management";case CST_ARCH_ERODING_GOALS:return"Reset goals to original vision, resist drift";case CST_ARCH_ESCALATION:return"Break the cycle with cooperative rather than competitive dynamics";case CST_ARCH_SUCCESS_TO_SUCCESSFUL:return"Redistribute resources to prevent winner-take-all dynamics";default:return"No remedy needed";}}
double cst_archetype_confidence(const CSTSystem* s, CSTSystemArchetype a){CSTSystemArchetype d=cst_detect_system_archetype(s);if(d==CST_ARCH_NONE)return 0.0;return(d==a)?0.7:0.1;}

/* ===== Jackson Creative Holism Paradigm Analysis ===== */
CSTJacksonParadigm cst_jackson_classify(const CSTSystem* s){
    double scores[4]={s->systemicity_score,1.0-s->awareness.boundary_openness,s->emancipatory_index,s->pluralism_index};
    int best=0;for(int i=1;i<4;i++)if(scores[i]>scores[best])best=i;return(CSTJacksonParadigm)best;
}
const char* cst_jackson_paradigm_label(CSTJacksonParadigm p){switch(p){case CST_PARADIGM_FUNCTIONALIST:return"Functionalist (Hard Systems)";case CST_PARADIGM_INTERPRETIVE:return"Interpretive (Soft Systems)";case CST_PARADIGM_EMANCIPATORY:return"Emancipatory (Critical Systems)";case CST_PARADIGM_POSTMODERN:return"Postmodern (Pluralist Systems)";default:return"Unknown";}}
double cst_jackson_paradigm_fit_score(const CSTSystem* s, CSTJacksonParadigm p){switch(p){case CST_PARADIGM_FUNCTIONALIST:return s->systemicity_score;case CST_PARADIGM_INTERPRETIVE:return 1.0-s->awareness.boundary_openness;case CST_PARADIGM_EMANCIPATORY:return s->emancipatory_index;case CST_PARADIGM_POSTMODERN:return s->pluralism_index;default:return 0.0;}}
bool cst_is_creatively_holistic(const CSTSystem* s){return cst_creative_holism_index(s)>0.6&&cst_intervention_mix_count(s)>=2;}
double cst_creative_holism_index(const CSTSystem* s){CSTJacksonParadigm best=cst_jackson_classify(s);double fit=cst_jackson_paradigm_fit_score(s,best);return fit*0.35+cst_methodological_diversity_index(s)*0.25+(1.0-s->awareness.boundary_openness)*0.25+s->emancipatory_index*0.15;}

/* ===== Viable System Model (Beer, 1972) alignment ===== */
typedef enum{CST_VSM_SYSTEM1=0,CST_VSM_SYSTEM2=1,CST_VSM_SYSTEM3=2,CST_VSM_SYSTEM4=3,CST_VSM_SYSTEM5=4}CSTVSMLevel;
double cst_vsm_alignment(const CSTSystem* s, CSTVSMLevel level){
    switch(level){
        case CST_VSM_SYSTEM1:return s->systemicity_score;
        case CST_VSM_SYSTEM2:return 1.0-s->awareness.boundary_openness;
        case CST_VSM_SYSTEM3:return s->critical_depth;
        case CST_VSM_SYSTEM4:return s->pluralism_index;
        case CST_VSM_SYSTEM5:return s->emancipatory_index;
        default:return 0.0;
    }
}
double cst_vsm_viability_index(const CSTSystem* s){
    double sum=0.0;for(int i=0;i<5;i++)sum+=cst_vsm_alignment(s,(CSTVSMLevel)i);
    return sum/5.0;
}
bool cst_vsm_is_viable(const CSTSystem* s){return cst_vsm_viability_index(s)>0.5;}

/* ===== Soft Systems Methodology (Checkland) richness analysis ===== */
double cst_ssm_rich_picture_score(const CSTSystem* s){
    return cst_metaphor_pluralism_score(s)*0.4+cst_participation_index(s)*0.3+cst_deliberative_quality(s)*0.3;
}
double cst_ssm_root_definition_quality(const CSTSystem* s){
    /* CATWOE: Customers, Actors, Transformation, Weltanschauung, Owner, Environment */
    double C=cst_stakeholder_inclusiveness(s),A=cst_participation_index(s);
    double T=s->systemicity_score,W=cst_metaphor_pluralism_score(s);
    double O=1.0-s->awareness.boundary_openness,E=1.0-cst_boundary_openness(s);
    return(C+A+T+W+O+E)/6.0;
}
bool cst_ssm_is_catwoe_complete(const CSTSystem* s){return cst_ssm_root_definition_quality(s)>0.5;}

/* ===== System dynamics archetype detection extension ===== */
double cst_archetype_fixes_that_fail_score(const CSTSystem* s){
    return(1.0-s->awareness.critical_awareness_index)*0.5+(s->critical_depth<0.4?0.5:0.0);
}
double cst_archetype_shifting_burden_score(const CSTSystem* s){
    return s->awareness.boundary_openness*0.4+(cst_is_coercive_context(s)?0.3:0.0);
}
double cst_archetype_limits_to_growth_score(const CSTSystem* s){
    return(1.0-s->systemicity_score)*0.5+(s->iteration>2?0.2:0.0);
}

/* ===== Complexity-based assessment ===== */
double cst_cynefin_complexity(const CSTSystem* s){
    /* Cynefin framework (Snowden): simple, complicated, complex, chaotic, disorder */
    if(s->context.complexity==CST_SIMPLE&&s->context.relationship==CST_UNITARY)return 0.1;
    if(s->context.complexity==CST_COMPLEX&&s->context.relationship==CST_PLURALIST)return 0.5;
    if(s->context.complexity==CST_CHAOTIC)return 0.9;
    return 0.3;
}
const char* cst_cynefin_domain(const CSTSystem* s){
    double c=cst_cynefin_complexity(s);
    if(c<0.2)return"Simple (Best practice)";if(c<0.4)return"Complicated (Expert analysis)";
    if(c<0.6)return"Complex (Probe-Sense-Respond)";if(c<0.8)return"Chaotic (Act-Sense-Respond)";
    return"Disorder";
}

/* ===== Stakeholder Salience Dynamics (Mitchell, Agle & Wood, 1997) ===== */
typedef enum{CST_STAKEHOLDER_LATENT=0,CST_STAKEHOLDER_EXPECTANT=1,CST_STAKEHOLDER_DEFINITIVE=2,CST_STAKEHOLDER_DORMANT=3}CSTStakeholderClass;
CSTStakeholderClass cst_classify_stakeholder(const CSTSystem* s, int idx){
    if(idx<0||idx>=s->n_stakeholders)return CST_STAKEHOLDER_LATENT;
    CSTStakeholder* sh=&s->stakeholders[idx];int cnt=0;
    if(sh->power>0.5)cnt++;if(sh->legitimacy>0.5)cnt++;if(sh->urgency>0.5)cnt++;
    if(cnt>=3)return CST_STAKEHOLDER_DEFINITIVE;if(cnt==2)return CST_STAKEHOLDER_EXPECTANT;return CST_STAKEHOLDER_LATENT;
}
const char* cst_stakeholder_class_label(CSTStakeholderClass c){switch(c){case CST_STAKEHOLDER_LATENT:return"Latent";case CST_STAKEHOLDER_EXPECTANT:return"Expectant";case CST_STAKEHOLDER_DEFINITIVE:return"Definitive";case CST_STAKEHOLDER_DORMANT:return"Dormant";default:return"Unknown";}}
int cst_stakeholder_count_class(const CSTSystem* s, CSTStakeholderClass target){int c=0;for(int i=0;i<s->n_stakeholders;i++)if(cst_classify_stakeholder(s,i)==target)c++;return c;}

/* ===== System Dynamics Loop Dominance Analysis ===== */
typedef enum{CST_LOOP_REINFORCING=0,CST_LOOP_BALANCING=1,CST_LOOP_NEUTRAL=2}CSTLoopPolarity;
CSTLoopPolarity cst_detect_dominant_loop(const CSTSystem* s){
    if(s->systemicity_score>0.6&&s->emancipatory_index<0.3)return CST_LOOP_REINFORCING;
    if(s->systemicity_score<0.4&&s->emancipatory_index>0.5)return CST_LOOP_BALANCING;
    return CST_LOOP_NEUTRAL;
}
const char* cst_loop_polarity_label(CSTLoopPolarity p){switch(p){case CST_LOOP_REINFORCING:return"Reinforcing (R)";case CST_LOOP_BALANCING:return"Balancing (B)";case CST_LOOP_NEUTRAL:return"Neutral";default:return"Unknown";}}

/* ===== Critical Systems Thinking Maturity Model ===== */
typedef enum{CST_MATURITY_AD_HOC=0,CST_MATURITY_AWARE=1,CST_MATURITY_CAPABLE=2,CST_MATURITY_INTEGRATED=3,CST_MATURITY_TRANSFORMATIVE=4}CSTPracticeMaturity;
CSTPracticeMaturity cst_assess_maturity(CSTSystem* s){
    double score=cst_systemic_coherence(s)*0.3+cst_critical_awareness_index(s)*0.25+
                 cst_participation_index(s)*0.2+cst_methodological_diversity_index(s)*0.15+
                 s->emancipatory_index*0.1;
    if(score<0.2)return CST_MATURITY_AD_HOC;if(score<0.4)return CST_MATURITY_AWARE;
    if(score<0.6)return CST_MATURITY_CAPABLE;if(score<0.8)return CST_MATURITY_INTEGRATED;
    return CST_MATURITY_TRANSFORMATIVE;
}
const char* cst_maturity_label(CSTPracticeMaturity m){switch(m){case CST_MATURITY_AD_HOC:return"Ad-hoc";case CST_MATURITY_AWARE:return"Aware";case CST_MATURITY_CAPABLE:return"Capable";case CST_MATURITY_INTEGRATED:return"Integrated";case CST_MATURITY_TRANSFORMATIVE:return"Transformative";default:return"Unknown";}}

/* ===== Pluralism Spectrum Analysis ===== */
double cst_pluralism_spectrum_score(const CSTSystem* s){
    double method_pluralism=cst_methodological_diversity_index(s);
    double metaphor_pluralism=cst_metaphor_pluralism_score(s);
    double stakeholder_pluralism=cst_stakeholder_inclusiveness(s);
    return method_pluralism*0.35+metaphor_pluralism*0.3+stakeholder_pluralism*0.35;
}
bool cst_is_pluralist_practice(const CSTSystem* s){return cst_pluralism_spectrum_score(s)>0.5;}

/* ===== Reflexivity assessment (Bourdieu, 1992; Alvesson & Skoldberg, 2000) ===== */
double cst_reflexivity_index(const CSTSystem* s){return s->critical_depth*0.5+s->iteration*0.1;}
bool cst_is_reflexive(const CSTSystem* s){return cst_reflexivity_index(s)>0.4;}

/* ===== Ultra-stability (Ashby, 1956): requisite variety ===== */
double cst_ashby_variety_ratio(const CSTSystem* s){return cst_methodological_diversity_index(s)/fmax(cst_boundary_openness(s),0.01);}
bool cst_has_requisite_variety(const CSTSystem* s){return cst_ashby_variety_ratio(s)>1.0;}

/* ===== Second-order cybernetics: observing the observer ===== */
double cst_second_order_observation(const CSTSystem* s){
    double observer_awareness=s->critical_depth;
    double system_awareness=s->systemicity_score;
    return(observer_awareness+system_awareness)/2.0;
}
bool cst_is_second_order(const CSTSystem* s){return cst_second_order_observation(s)>0.5;}

/* ===== Luhmann's social systems autopoiesis ===== */
double cst_luhmann_autopoiesis(const CSTSystem* s){
    double communication=cst_deliberative_quality(s);
    double self_reproduction=s->systemicity_score;
    double structural_coupling=1.0-s->awareness.boundary_openness;
    return(communication+self_reproduction+structural_coupling)/3.0;
}
bool cst_is_autopoietic(const CSTSystem* s){return cst_luhmann_autopoiesis(s)>0.5;}

/* ===== Bateson's levels of learning ===== */
typedef enum{CST_BATESON_ZERO=0,CST_BATESON_I=1,CST_BATESON_II=2,CST_BATESON_III=3}CSTBatesonLevel;
CSTBatesonLevel cst_bateson_learning_level(const CSTSystem* s){
    if(s->critical_depth<0.2)return CST_BATESON_ZERO;
    if(s->critical_depth<0.4)return CST_BATESON_I;
    if(s->critical_depth<0.7)return CST_BATESON_II;
    return CST_BATESON_III;
}
const char* cst_bateson_level_label(CSTBatesonLevel b){switch(b){case CST_BATESON_ZERO:return"Zero (stimulus-response)";case CST_BATESON_I:return"I (adaptive)";case CST_BATESON_II:return"II (learning to learn)";case CST_BATESON_III:return"III (transformative)";default:return"Unknown";}}

/* ===== Ackoff's idealized design ===== */
double cst_ackoff_idealization_gap(const CSTSystem* s){return 1.0-s->systemicity_score;}
bool cst_ackoff_is_idealized(const CSTSystem* s){return cst_ackoff_idealization_gap(s)<0.3;}

/* ===== Systemic health composite ===== */
double cst_systemic_health_index(const CSTSystem* s){
    double viability=cst_vsm_viability_index(s);
    double reflexivity=cst_reflexivity_index(s);
    double variety=cst_ashby_variety_ratio(s)>1.0?1.0:cst_ashby_variety_ratio(s);
    double autopoiesis=cst_luhmann_autopoiesis(s);
    return(viability*0.25+reflexivity*0.25+variety*0.25+autopoiesis*0.25);
}

/* ===== System diagnosis report generator ===== */
void cst_print_full_diagnosis(CSTSystem* s){
    cst_print_state(s);
    printf("\n");cst_print_stakeholders(s);
    printf("\n");cst_print_boundaries(s);
    printf("\n");cst_print_methodologies(s);
    printf("\n");cst_print_metaphors(s);
    printf("\n");cst_print_holistic(s);
    printf("\n");cst_print_evolution_summary(s);
    CSTSystemArchetype arch=cst_detect_system_archetype(s);
    printf("\n=== System Archetype: %s ===\n",cst_archetype_label(arch));
    printf("Remedy: %s\n",cst_archetype_remedy(arch));
    CSTJacksonParadigm jp=cst_jackson_classify(s);
    printf("Jackson Paradigm: %s (fit=%.3f)\n",cst_jackson_paradigm_label(jp),cst_jackson_paradigm_fit_score(s,jp));
    printf("Creative Holism: %.3f (%s)\n",cst_creative_holism_index(s),cst_is_creatively_holistic(s)?"Holistic":"Partial");
    CSTPracticeMaturity pm=cst_assess_maturity(s);
    printf("Practice Maturity: %s\n",cst_maturity_label(pm));
    printf("Systemic Health: %.3f\n",cst_systemic_health_index(s));
    printf("Improvement Potential: %d/6 dimensions\n",cst_improvement_potential(s));
}

/* Boundary sweep: enumerate all possible boundary judgments for a system.
 * Flood & Jackson (1991): critical systems thinking requires examining
 * every boundary choice, not accepting the default.
 * Returns number of boundary configurations generated. */
int cst_enumerate_boundaries(CSTSystem* sys, int max_configs, BoundaryConfig* out) {
    if (!sys || !out || max_configs < 1) return -1;
    int count = 0;
    for (int m = 0; m < 3 && count < max_configs; m++) {
        for (int c = 0; c < 3 && count < max_configs; c++) {
            for (int k = 0; k < 3 && count < max_configs; k++) {
                for (int l = 0; l < 3 && count < max_configs; l++) {
                    out[count].motivation_sweep = m;
                    out[count].control_sweep = c;
                    out[count].knowledge_sweep = k;
                    out[count].legitimacy_sweep = l;
                    out[count].is_normative = (m==0 && c==0);
                    count++;
                }
            }
        }
    }
    return count;
}

/* Compute openness score for a boundary configuration.
 * Ulrich (1983): boundary openness is inversely proportional to
 * the degree to which boundary dimensions are "given" vs "chosen".
 * Fully normative (all given) -> openness = 0.0
 * Fully explorative (all chosen) -> openness = 1.0 */
double cst_boundary_config_openness(const BoundaryConfig* cfg) {
    if (!cfg) return 0.0;
    double sum = (double)(cfg->motivation_sweep + cfg->control_sweep +
                          cfg->knowledge_sweep + cfg->legitimacy_sweep);
    return sum / 8.0;  /* max sum = 8 (4 dims x max value 2) */
}
