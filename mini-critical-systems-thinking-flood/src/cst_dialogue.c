#include "cst_dialogue.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

CSTDialogue* cst_dialogue_create(CSTSystem* sys, CSTDialogueType type){
    CSTDialogue* d=calloc(1,sizeof(CSTDialogue));
    if(!d) return NULL;
    d->sys=sys; d->type=type; d->n_participants=0; d->n_rounds=0;
    d->inclusivity=0.5; d->depth=0.3; d->consensus_level=0.0;
    d->is_facilitated=false;
    return d;
}

/* ============================================================================
 * Dialogue Quality & Boundary Critique Integration
 *
 * Ulrich's (1983) boundary critique emphasizes that dialogue quality
 * depends on who is included (boundary judgements) and how deeply
 * assumptions are questioned. These functions assess dialogue health.
 * ============================================================================ */

/* Assess dialogue depth: how many levels of "why" have been explored.
 * Based on the "Five Whys" technique and Argyris's double-loop learning.
 * depth_0 = surface symptoms, depth_1 = action strategies,
 * depth_2 = governing values, depth_3 = underlying assumptions.
 * Returns depth score in [0, 1]. */
double cst_dialogue_depth_assessment(CSTDialogue* d) {
    if (!d) return 0.0;
    double depth = (double)d->n_rounds / 10.0;
    if (depth > 1.0) depth = 1.0;
    /* Augment by facilitation quality */
    if (d->is_facilitated) depth *= 1.3;
    if (depth > 1.0) depth = 1.0;
    return depth;
}

/* Habermasian ideal speech assessment: how close the dialogue
 * approaches Habermas's conditions for communicative rationality.
 * Scored 0-1 for: comprehensibility, truth, sincerity, legitimacy.
 * Returns average across the four validity claims. */
double cst_habermas_ideal_speech_score(CSTDialogue* d) {
    if (!d) return 0.0;
    /* Comprehensibility: depends on shared language/terminology */
    double comp = d->inclusivity;
    /* Truth: depth of factual exploration */
    double truth = d->depth;
    /* Sincerity: inversely related to power asymmetry */
    double sine = (d->sys) ? (1.0 - d->sys->power_asymmetry) : 0.5;
    if (sine < 0.0) sine = 0.0;
    /* Legitimacy: consensus level achieved */
    double legit = d->consensus_level;
    return 0.25 * (comp + truth + sine + legit);
}

/* Participants' epistemic diversity: measures how different
 * the knowledge backgrounds are among participants.
 * Uses Simpson's diversity index on knowledge domain frequencies.
 * Returns [0, 1] where 1 = maximally diverse. */
double cst_epistemic_diversity(CSTDialogue* d) {
    if (!d || d->n_participants < 2) return 0.0;
    /* Count domains (simplified: 5 knowledge domains) */
    int domains[5] = {0, 0, 0, 0, 0};
    for (int i = 0; i < d->n_participants && i < 100; i++) {
        int dm = i % 5;  /* Simplified assignment */
        if (dm >= 0 && dm < 5) domains[dm]++;
    }
    /* Simpson index: 1 - sum(p_i^2) */
    double n = (double)d->n_participants;
    double simpson = 0.0;
    for (int i = 0; i < 5; i++) {
        double p = (double)domains[i] / n;
        simpson += p * p;
    }
    return 1.0 - simpson;
}

/* Generate dialogue round summary. Prints round number and
 * key metrics to stdout. */
void cst_dialogue_round_summary(CSTDialogue* d, int round) {
    if (!d) return;
    printf("--- Dialogue Round %d ---\n", round);
    printf("Participants: %d  Inclusivity: %.2f  Depth: %.2f\n",
           d->n_participants, d->inclusivity, d->depth);
    printf("Consensus: %.2f  Facilitated: %s\n",
           d->consensus_level, d->is_facilitated ? "yes" : "no");
}

void cst_dialogue_free(CSTDialogue* d){free(d);}

void cst_dialogue_run_round(CSTDialogue* d){
    d->n_rounds++;d->n_participants=d->sys->n_stakeholders;
    d->inclusivity=cst_stakeholder_inclusiveness(d->sys);
    d->depth=fmin(1.0,d->depth+0.1*d->sys->critical_depth);
    int marg=0;for(int i=0;i<d->sys->n_stakeholders;i++)if(d->sys->stakeholders[i].is_marginalized)marg++;
    d->consensus_level=1.0-(double)marg/fmax(d->sys->n_stakeholders,1)*0.5;
    d->sys->critical_depth=fmin(1.0,d->sys->critical_depth+0.05);
    d->sys->iteration++;
}
void cst_dialogue_run_n_rounds(CSTDialogue* d, int n){for(int i=0;i<n;i++)cst_dialogue_run_round(d);}

double cst_dialogue_quality(const CSTDialogue* d){
    return d->inclusivity*0.3+d->depth*0.3+d->consensus_level*0.2+(d->n_rounds>3?0.2:0.1);
}
double cst_dialogue_bohm_score(const CSTDialogue* d){
    /* Bohm dialogue: suspending assumptions, observing thought, proprioception of thinking */
    return d->depth*0.4+(1.0-d->sys->awareness.boundary_openness)*0.3+d->inclusivity*0.3;
}
bool cst_dialogue_has_emerged(const CSTDialogue* d){return d->n_rounds>=3&&d->consensus_level>0.5&&d->depth>0.5;}

void cst_dialogue_print(const CSTDialogue* d){
    printf("=== Dialogue ===\nType=%d Rounds=%d Participants=%d\n",(int)d->type,d->n_rounds,d->n_participants);
    printf("Inclusivity=%.3f Depth=%.3f Consensus=%.3f Quality=%.3f\n",d->inclusivity,d->depth,d->consensus_level,cst_dialogue_quality(d));
    printf("BohmScore=%.3f Emerged=%s Facilitated=%s\n",cst_dialogue_bohm_score(d),cst_dialogue_has_emerged(d)?"YES":"NO",d->is_facilitated?"YES":"NO");
}

/* Socratic dialogue: elenchus method */
double cst_dialogue_socratic_depth(const CSTDialogue* d){return d->depth*d->sys->critical_depth;}
bool cst_dialogue_socratic_aporia(const CSTDialogue* d){return d->n_rounds>=5&&d->depth>0.6&&d->consensus_level<0.4;}

/* Deliberative democracy (Habermas ideal speech situation) */
double cst_dialogue_habermas_score(const CSTDialogue* d){
    double inclusiveness=d->inclusivity, no_coercion=1.0-d->sys->awareness.boundary_openness;
    double equal_participation=cst_participation_index(d->sys), reasoned_argument=d->depth;
    return(inclusiveness*0.25+no_coercion*0.25+equal_participation*0.25+reasoned_argument*0.25);
}
bool cst_dialogue_is_ideal_speech(const CSTDialogue* d){return cst_dialogue_habermas_score(d)>0.7;}

/* Appreciative inquiry (Cooperrider) 4-D cycle */
typedef enum{CST_AI_DISCOVER=0,CST_AI_DREAM=1,CST_AI_DESIGN=2,CST_AI_DESTINY=3}CSTAIPhase;
double cst_dialogue_appreciative_score(const CSTDialogue* d){return d->consensus_level*0.5+d->inclusivity*0.5;}
bool cst_dialogue_is_appreciative(const CSTDialogue* d){return d->type==CST_DIALOG_APPRECIATIVE&&cst_dialogue_appreciative_score(d)>0.5;}

/* Dialogue to decision continuum */
double cst_dialogue_decision_readiness(const CSTDialogue* d){return d->consensus_level*d->inclusivity*d->depth;}
bool cst_dialogue_ready_for_decision(const CSTDialogue* d){return cst_dialogue_decision_readiness(d)>0.3;}

/* Freire's dialogical pedagogy: praxis = reflection + action */
double cst_dialogue_freire_praxis(const CSTDialogue* d){return d->depth*0.5+d->sys->emancipatory_index*0.5;}
bool cst_dialogue_is_liberatory(const CSTDialogue* d){return cst_dialogue_freire_praxis(d)>0.6&&d->n_rounds>=3;}

/* Gadamer's fusion of horizons */
double cst_dialogue_horizon_fusion(const CSTDialogue* d){return d->consensus_level*d->inclusivity;}
bool cst_dialogue_horizons_merged(const CSTDialogue* d){return cst_dialogue_horizon_fusion(d)>0.4;}

/* Buber's I-Thou vs I-It dialogue */
double cst_dialogue_ithou_quality(const CSTDialogue* d){return(1.0-d->sys->awareness.boundary_openness)*0.5+d->inclusivity*0.5;}
bool cst_dialogue_is_ithou(const CSTDialogue* d){return cst_dialogue_ithou_quality(d)>0.6;}

/* World Cafe method (Brown & Isaacs) */
double cst_dialogue_world_cafe_score(const CSTDialogue* d){return d->n_participants>5?d->inclusivity*0.7+d->consensus_level*0.3:d->inclusivity*0.5+d->consensus_level*0.5;}

/* Open Space Technology (Harrison Owen) */
double cst_dialogue_open_space_score(const CSTDialogue* d){return d->inclusivity*0.4+d->depth*0.3+((double)d->n_rounds/10.0>0.3?0.3:((double)d->n_rounds/10.0));}

/* Dynamic facilitation (Rough) */
double cst_dialogue_dynamic_facilitation_score(const CSTDialogue* d){return d->depth*0.5+(d->is_facilitated?0.3:0.1)+d->consensus_level*0.2;}

/* Future Search (Weisbord & Janoff) */
double cst_dialogue_future_search_score(const CSTDialogue* d){return d->consensus_level*0.4+d->inclusivity*0.3+(d->n_rounds>2?0.3:0.1);}
bool cst_dialogue_future_search_ready(const CSTDialogue* d){return d->n_rounds>=3&&d->consensus_level>0.5;}

/* Participatory Rural Appraisal (Chambers) */
double cst_dialogue_pra_score(const CSTDialogue* d){return d->inclusivity*0.5+d->depth*0.3+(1.0-d->sys->awareness.boundary_openness)*0.2;}
bool cst_dialogue_is_participatory_enough(const CSTDialogue* d){return cst_dialogue_pra_score(d)>0.6;}

/* Dialogue facilitation quality */
typedef enum{CST_FACILITATION_NONE=0,CST_FACILITATION_BASIC=1,CST_FACILITATION_SKILLED=2,CST_FACILITATION_MASTER=3}CSTFacilitationLevel;
CSTFacilitationLevel cst_dialogue_facilitation_level(const CSTDialogue* d){if(!d->is_facilitated)return CST_FACILITATION_NONE;if(d->n_rounds<2)return CST_FACILITATION_BASIC;if(d->depth<0.5)return CST_FACILITATION_SKILLED;return CST_FACILITATION_MASTER;}
const char* cst_dialogue_facilitation_label(CSTFacilitationLevel f){switch(f){case CST_FACILITATION_NONE:return"None";case CST_FACILITATION_BASIC:return"Basic";case CST_FACILITATION_SKILLED:return"Skilled";case CST_FACILITATION_MASTER:return"Master";default:return"Unknown";}}

/* Conflict transformation in dialogue (Lederach) */
double cst_dialogue_conflict_transformation(const CSTDialogue* d){return d->consensus_level*d->depth;}
bool cst_dialogue_conflict_resolved(const CSTDialogue* d){return cst_dialogue_conflict_transformation(d)>0.4&&d->n_rounds>=3;}

/* Polarity management (Johnson, 1992): both/and thinking */
double cst_dialogue_polarity_score(const CSTDialogue* d){return d->inclusivity*d->depth;}
bool cst_dialogue_is_polarity_aware(const CSTDialogue* d){return cst_dialogue_polarity_score(d)>0.3;}
