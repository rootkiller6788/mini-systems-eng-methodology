#include "cst_power.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

CSTPowerAnalysis* cst_power_create(CSTSystem* sys){
    CSTPowerAnalysis* pa=calloc(1,sizeof(CSTPowerAnalysis));pa->sys=sys;
    pa->power_scores=calloc(sys->n_stakeholders,sizeof(double));pa->n_scores=sys->n_stakeholders;
    for(int i=0;i<sys->n_stakeholders;i++)pa->power_scores[i]=sys->stakeholders[i].power;
    return pa;
}
void cst_power_free(CSTPowerAnalysis* pa){if(!pa)return;free(pa->power_scores);free(pa);}

void cst_power_analyze(CSTPowerAnalysis* pa){
    double total=0.0,sumsq=0.0,max_p=0.0;int n=pa->sys->n_stakeholders;
    for(int i=0;i<n;i++){double p=pa->sys->stakeholders[i].power;total+=p;sumsq+=p*p;if(p>max_p)max_p=p;}
    double mean=total/fmax(n,1);double var=sumsq/fmax(n,1)-mean*mean;
    pa->power_concentration=var*10.0;if(pa->power_concentration>1.0)pa->power_concentration=1.0;
    pa->power_distance=1.0-(mean/max_p>1.0?1.0:mean/fmax(max_p,0.01));
    pa->n_dominant_actors=0;for(int i=0;i<n;i++)if(pa->sys->stakeholders[i].power>0.6)pa->n_dominant_actors++;
}
double cst_power_concentration_index(const CSTPowerAnalysis* pa){return pa->power_concentration;}
double cst_power_distance_index(const CSTPowerAnalysis* pa){return pa->power_distance;}
int cst_power_dominant_count(const CSTPowerAnalysis* pa){return pa->n_dominant_actors;}
bool cst_power_is_democratic(const CSTPowerAnalysis* pa){return pa->power_concentration<0.3&&pa->power_distance<0.5&&pa->n_dominant_actors<=2;}

void cst_power_print(const CSTPowerAnalysis* pa){
    printf("=== Power Analysis ===\n");
    printf("Concentration=%.3f Distance=%.3f Dominant=%d Democratic=%s\n",pa->power_concentration,pa->power_distance,pa->n_dominant_actors,cst_power_is_democratic(pa)?"YES":"NO");
    for(int i=0;i<pa->sys->n_stakeholders;i++)printf("  %s power=%.2f\n",pa->sys->stakeholders[i].name,pa->sys->stakeholders[i].power);
}

/* Lukes three dimensions of power (1974) */
double cst_power_lukes_first_dimension(const CSTPowerAnalysis* pa){return pa->power_concentration;}
double cst_power_lukes_second_dimension(const CSTPowerAnalysis* pa){return pa->power_distance;}
double cst_power_lukes_third_dimension(const CSTPowerAnalysis* pa){return pa->sys->awareness.boundary_openness;}
double cst_power_lukes_aggregate(const CSTPowerAnalysis* pa){return(cst_power_lukes_first_dimension(pa)+cst_power_lukes_second_dimension(pa)+cst_power_lukes_third_dimension(pa))/3.0;}

/* Gaventa power cube (2006): levels, spaces, forms */
typedef enum{CST_POWER_LEVEL_LOCAL=0,CST_POWER_LEVEL_NATIONAL=1,CST_POWER_LEVEL_GLOBAL=2}CSTPowerLevel;
typedef enum{CST_POWER_SPACE_CLOSED=0,CST_POWER_SPACE_INVITED=1,CST_POWER_SPACE_CLAIMED=2}CSTPowerSpace;
typedef enum{CST_POWER_FORM_VISIBLE=0,CST_POWER_FORM_HIDDEN=1,CST_POWER_FORM_INVISIBLE=2}CSTPowerForm;
double cst_power_gaventa_index(const CSTPowerAnalysis* pa){
    double visible=pa->power_concentration,hidden=pa->power_distance,invisible=pa->sys->awareness.boundary_openness;
    return(visible+hidden+invisible)/3.0;
}

/* Power-interest grid for stakeholder mapping */
void cst_power_interest_classify(const CSTPowerAnalysis* pa, int* high_power_high_interest, int* high_power_low_interest, int* low_power_high_interest, int* low_power_low_interest){
    *high_power_high_interest=*high_power_low_interest=*low_power_high_interest=*low_power_low_interest=0;
    for(int i=0;i<pa->sys->n_stakeholders;i++){double p=pa->sys->stakeholders[i].power;double l=pa->sys->stakeholders[i].legitimacy;if(p>0.5&&l>0.5)(*high_power_high_interest)++;else if(p>0.5)(*high_power_low_interest)++;else if(l>0.5)(*low_power_high_interest)++;else(*low_power_low_interest)++;}
}

/* Empowerment assessment */
double cst_power_empowerment_potential(const CSTPowerAnalysis* pa){
    int low=0;for(int i=0;i<pa->sys->n_stakeholders;i++)if(pa->sys->stakeholders[i].power<0.3)low++;
    return(pa->sys->n_stakeholders>0)?(double)low/pa->sys->n_stakeholders:0.0;
}
bool cst_power_needs_empowerment(const CSTPowerAnalysis* pa){return cst_power_empowerment_potential(pa)>0.3;}

/* Foucault's power/knowledge analysis */
double cst_power_foucault_disciplinary(const CSTPowerAnalysis* pa){return pa->sys->systemicity_score*pa->power_concentration;}
double cst_power_foucault_normalizing(const CSTPowerAnalysis* pa){return pa->sys->awareness.boundary_openness;}
bool cst_power_is_panoptic(const CSTPowerAnalysis* pa){return cst_power_foucault_disciplinary(pa)>0.5&&pa->power_distance<0.3;}

/* Gramsci's hegemony analysis */
double cst_power_gramsci_hegemony(const CSTPowerAnalysis* pa){return pa->power_concentration*pa->sys->critical_depth;}
bool cst_power_hegemonic_consent(const CSTPowerAnalysis* pa){return cst_power_gramsci_hegemony(pa)>0.4&&pa->n_dominant_actors<=2;}

/* Arendt's power-as-collective-action */
double cst_power_arendt_collective(const CSTPowerAnalysis* pa){return cst_participation_index(pa->sys);}
bool cst_power_is_collective_action(const CSTPowerAnalysis* pa){return cst_power_arendt_collective(pa)>0.5;}

/* Power asymmetry analysis */
double cst_power_gini_coefficient(const CSTPowerAnalysis* pa){
    int n=pa->sys->n_stakeholders;if(n<2)return 0.0;
    double sum=0.0;for(int i=0;i<n;i++)for(int j=0;j<n;j++)sum+=fabs(pa->sys->stakeholders[i].power-pa->sys->stakeholders[j].power);
    double mean=0.0;for(int i=0;i<n;i++)mean+=pa->sys->stakeholders[i].power;mean/=n;
    return(mean>1e-10&&n>0)?sum/(2.0*n*n*mean):0.0;
}
int cst_power_most_powerful(const CSTPowerAnalysis* pa){int best=0;for(int i=1;i<pa->sys->n_stakeholders;i++)if(pa->sys->stakeholders[i].power>pa->sys->stakeholders[best].power)best=i;return best;}
int cst_power_least_powerful(const CSTPowerAnalysis* pa){int worst=0;for(int i=1;i<pa->sys->n_stakeholders;i++)if(pa->sys->stakeholders[i].power<pa->sys->stakeholders[worst].power)worst=i;return worst;}

/* Power transition analysis */
double cst_power_stability_index(const CSTPowerAnalysis* pa){return 1.0-pa->power_concentration;}
bool cst_power_is_stable(const CSTPowerAnalysis* pa){return cst_power_stability_index(pa)>0.5;}
double cst_power_transition_likelihood(const CSTPowerAnalysis* pa){return pa->power_distance*pa->power_concentration;}
bool cst_power_transition_imminent(const CSTPowerAnalysis* pa){return cst_power_transition_likelihood(pa)>0.4;}

/* Bourdieu's forms of capital: social, cultural, symbolic */
typedef enum{CST_CAPITAL_SOCIAL=0,CST_CAPITAL_CULTURAL=1,CST_CAPITAL_SYMBOLIC=2,CST_CAPITAL_ECONOMIC=3}CSTCapitalForm;
double cst_power_social_capital(const CSTPowerAnalysis* pa){return cst_deliberative_quality(pa->sys);}
double cst_power_cultural_capital(const CSTPowerAnalysis* pa){return pa->sys->critical_depth;}
double cst_power_symbolic_capital(const CSTPowerAnalysis* pa){return pa->sys->systemicity_score;}
double cst_power_economic_capital(const CSTPowerAnalysis* pa){return pa->power_concentration;}
double cst_power_bourdieu_aggregate(const CSTPowerAnalysis* pa){return(cst_power_social_capital(pa)+cst_power_cultural_capital(pa)+cst_power_symbolic_capital(pa)+cst_power_economic_capital(pa))/4.0;}

/* Bachrach & Baratz (1962): two faces of power */
double cst_power_first_face(const CSTPowerAnalysis* pa){return pa->power_concentration;}
double cst_power_second_face(const CSTPowerAnalysis* pa){return 1.0-cst_participation_index(pa->sys);}
bool cst_power_has_hidden_agenda(const CSTPowerAnalysis* pa){return cst_power_second_face(pa)>0.5;}

/* Clegg's circuits of power (1989) */
typedef enum{CST_CIRCUIT_EPISODIC=0,CST_CIRCUIT_DISPOSITIONAL=1,CST_CIRCUIT_FACILITATIVE=2}CSTPowerCircuit;
double cst_power_circuit_score(const CSTPowerAnalysis* pa, CSTPowerCircuit circuit){switch(circuit){case CST_CIRCUIT_EPISODIC:return pa->power_concentration;case CST_CIRCUIT_DISPOSITIONAL:return pa->power_distance;case CST_CIRCUIT_FACILITATIVE:return pa->sys->systemicity_score;default:return 0.0;}}

/* Power mapping: network centrality approximation */
double cst_power_network_centrality(const CSTPowerAnalysis* pa){double sum=0.0;for(int i=0;i<pa->sys->n_stakeholders;i++){double influence=0.0;for(int j=0;j<pa->sys->n_stakeholders;j++)if(i!=j)influence+=pa->sys->stakeholders[j].power*pa->sys->stakeholders[i].legitimacy;sum+=influence;}return(pa->sys->n_stakeholders>1)?sum/(pa->sys->n_stakeholders*(pa->sys->n_stakeholders-1)):0.0;}

/* Power report generator */
void cst_power_full_report(const CSTPowerAnalysis* pa){
    cst_power_print(pa);
    printf("\n=== Extended Power Analysis ===\n");
    printf("Lukes 3D: 1st=%.2f 2nd=%.2f 3rd=%.2f Aggregate=%.2f\n",cst_power_lukes_first_dimension(pa),cst_power_lukes_second_dimension(pa),cst_power_lukes_third_dimension(pa),cst_power_lukes_aggregate(pa));
    printf("Foucault Disciplinary: %.2f\n",cst_power_foucault_disciplinary(pa));
    printf("Gramsci Hegemony: %.2f\n",cst_power_gramsci_hegemony(pa));
    printf("Arendt Collective: %.2f\n",cst_power_arendt_collective(pa));
    printf("Bourdieu Capital: %.2f\n",cst_power_bourdieu_aggregate(pa));
    printf("Gini Coefficient: %.3f\n",cst_power_gini_coefficient(pa));
    printf("Network Centrality: %.3f\n",cst_power_network_centrality(pa));
}
