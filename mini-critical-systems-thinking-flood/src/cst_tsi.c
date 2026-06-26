#include <math.h>
#include "cst_tsi.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

CSTTSI* cst_tsi_create(CSTSystem* sys) {
    CSTTSI* tsi = calloc(1, sizeof(CSTTSI));
    if (!tsi) return NULL;
    tsi->sys = sys;
    return tsi;
}
void cst_tsi_free(CSTTSI* tsi) { free(tsi); }

void cst_tsi_run_creativity(CSTTSI* tsi) {
    cst_tsi_creativity_phase(tsi->sys);
    tsi->creativity_rounds_done++;
    tsi->creativity_score = 0.0;
    for (int i = 0; i < tsi->sys->n_metaphors; i++)
        tsi->creativity_score += tsi->sys->metaphors[i].insight_score;
    if (tsi->sys->n_metaphors > 0) tsi->creativity_score /= tsi->sys->n_metaphors;
}

void cst_tsi_run_choice(CSTTSI* tsi) {
    cst_tsi_choice_phase(tsi->sys);
    int sel = 0;
    for (int i = 0; i < tsi->sys->n_methodologies; i++)
        if (tsi->sys->methodologies[i].selected) {
            sel++;
            tsi->choice_score += tsi->sys->methodologies[i].overall_score;
        }
    if (sel > 0) tsi->choice_score /= sel;
}

void cst_tsi_run_implementation(CSTTSI* tsi) {
    cst_tsi_implementation_phase(tsi->sys);
    tsi->implementation_rounds_done++;
    tsi->n_methods_applied = 0;
    for (int i = 0; i < tsi->sys->n_methodologies; i++)
        if (tsi->sys->methodologies[i].selected) tsi->n_methods_applied++;
    tsi->implementation_score = tsi->sys->emancipatory_index * 0.5 + tsi->sys->pluralism_index * 0.5;
}

void cst_tsi_run_reflection(CSTTSI* tsi) {
    cst_tsi_reflection_phase(tsi->sys);
    tsi->reflection_score = tsi->sys->critical_depth;
    tsi->n_cycles_completed++;
}

void cst_tsi_run_full_cycle(CSTTSI* tsi) {
    cst_tsi_full_cycle(tsi->sys);
    tsi->creativity_rounds_done++;
    tsi->implementation_rounds_done++;
    tsi->n_cycles_completed++;
    tsi->reflection_score = tsi->sys->critical_depth;
}

void cst_tsi_run_n_cycles(CSTTSI* tsi, int n) {
    for (int i = 0; i < n; i++) cst_tsi_run_full_cycle(tsi);
}

double cst_tsi_overall_quality(const CSTTSI* tsi) {
    return (tsi->creativity_score * 0.25 + tsi->choice_score * 0.25 +
            tsi->implementation_score * 0.25 + tsi->reflection_score * 0.25);
}

bool cst_tsi_is_converged(const CSTTSI* tsi) {
    return tsi->n_cycles_completed >= 3 && cst_tsi_overall_quality(tsi) > 0.7;
}

void cst_tsi_print(const CSTTSI* tsi) {
    printf("=== TSI: %s (cycles=%d) ===\n", tsi->sys->name, tsi->n_cycles_completed);
    printf("Creativity=%.3f Choice=%.3f Impl=%.3f Refl=%.3f Overall=%.3f\n",
           tsi->creativity_score, tsi->choice_score, tsi->implementation_score,
           tsi->reflection_score, cst_tsi_overall_quality(tsi));
    printf("Converged=%s MethodsApplied=%d\n",
           cst_tsi_is_converged(tsi) ? "YES" : "NO", tsi->n_methods_applied);
}

double cst_tsi_creativity_depth(const CSTTSI* tsi) { return tsi->creativity_score; }
double cst_tsi_choice_appropriateness(const CSTTSI* tsi) { return tsi->choice_score; }
double cst_tsi_implementation_fidelity(const CSTTSI* tsi) { return tsi->implementation_score; }
double cst_tsi_reflection_thoroughness(const CSTTSI* tsi) { return tsi->reflection_score; }
bool cst_tsi_is_balanced(const CSTTSI* tsi) {
    double min_s=fmin(fmin(tsi->creativity_score,tsi->choice_score),fmin(tsi->implementation_score,tsi->reflection_score));
    double max_s=fmax(fmax(tsi->creativity_score,tsi->choice_score),fmax(tsi->implementation_score,tsi->reflection_score));
    return (max_s>1e-10)&&(min_s/max_s>0.5);
}
void cst_tsi_phase_report(const CSTTSI* tsi) {
    printf("=== TSI Phase Report ===\n");
    printf("Creativity:%.3f Choice:%.3f Impl:%.3f Refl:%.3f Balanced:%s\n",
           tsi->creativity_score,tsi->choice_score,tsi->implementation_score,tsi->reflection_score,
           cst_tsi_is_balanced(tsi)?"YES":"NO");
}
int cst_tsi_weakest_phase(const CSTTSI* tsi) {
    double s[]={tsi->creativity_score,tsi->choice_score,tsi->implementation_score,tsi->reflection_score};
    int w=0; for(int i=1;i<4;i++) if(s[i]<s[w]) w=i; return w;
}

/* TSI cycle efficiency analysis */
double cst_tsi_cycle_efficiency(const CSTTSI* tsi){return tsi->n_cycles_completed>0?(double)tsi->n_methods_applied/tsi->n_cycles_completed:0.0;}
double cst_tsi_time_estimate(const CSTTSI* tsi){return tsi->n_cycles_completed*4.0;}
int cst_tsi_recommended_cycles(const CSTTSI* tsi){double q=cst_tsi_overall_quality(tsi);if(q<0.3)return 5;if(q<0.5)return 4;if(q<0.7)return 3;return 2;}

/* TSI phase transition analysis */
bool cst_tsi_phase_transition_ready(const CSTTSI* tsi, CSTTSIPhase from_phase){
    switch(from_phase){
        case CST_CREATIVITY:return tsi->creativity_score>0.3;
        case CST_CHOICE:return tsi->choice_score>0.3;
        case CST_IMPLEMENTATION:return tsi->implementation_score>0.3;
        case CST_REFLECTION:return tsi->reflection_score>0.3;
        default:return false;
    }
}
const char* cst_tsi_phase_name(CSTTSIPhase p){switch(p){case CST_CREATIVITY:return"Creativity";case CST_CHOICE:return"Choice";case CST_IMPLEMENTATION:return"Implementation";case CST_REFLECTION:return"Reflection";default:return"Unknown";}}

/* TSI maturity assessment */
typedef enum{CST_TSI_NOVICE=0,CST_TSI_PRACTITIONER=1,CST_TSI_EXPERT=2,CST_TSI_MASTER=3}CSTTSIMaturity;
CSTTSIMaturity cst_tsi_maturity(const CSTTSI* tsi){
    double q=cst_tsi_overall_quality(tsi);int c=tsi->n_cycles_completed;
    if(q<0.3||c<2)return CST_TSI_NOVICE;if(q<0.5||c<5)return CST_TSI_PRACTITIONER;if(q<0.7||c<10)return CST_TSI_EXPERT;return CST_TSI_MASTER;
}
const char* cst_tsi_maturity_label(CSTTSIMaturity m){switch(m){case CST_TSI_NOVICE:return"Novice";case CST_TSI_PRACTITIONER:return"Practitioner";case CST_TSI_EXPERT:return"Expert";case CST_TSI_MASTER:return"Master";default:return"Unknown";}}
