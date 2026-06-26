#include "cst_ethics.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

CSTEthicsReport* cst_ethics_create(CSTSystem* sys){
    CSTEthicsReport* er=calloc(1,sizeof(CSTEthicsReport));er->sys=sys;
    er->assessments=calloc(5,sizeof(CSTEthicalAssessment));er->n_assessments=5;
    CSTEthicalPrinciple ps[]={CST_ETHIC_BENEFICENCE,CST_ETHIC_NONMALEFICENCE,CST_ETHIC_JUSTICE,CST_ETHIC_AUTONOMY,CST_ETHIC_TRANSPARENCY};
    for(int i=0;i<5;i++){er->assessments[i].principle=ps[i];er->assessments[i].name=strdup(cst_principle_name(ps[i]));er->assessments[i].score=0.5;}
    return er;
}
void cst_ethics_free(CSTEthicsReport* er){if(!er)return;for(int i=0;i<er->n_assessments;i++){free(er->assessments[i].name);for(int j=0;j<er->assessments[i].n_concerns;j++)free(er->assessments[i].concerns[j]);free(er->assessments[i].concerns);}free(er->assessments);for(int i=0;i<er->n_recommendations;i++)free(er->recommendations[i]);free(er->recommendations);free(er);}
void cst_ethics_assess_all(CSTEthicsReport* er){
    er->beneficence=cst_stakeholder_inclusiveness(er->sys);
    er->nonmaleficence=cst_is_coercive_context(er->sys)?0.3:0.8;
    er->justice=1.0-er->sys->awareness.boundary_openness;
    er->autonomy=er->sys->pluralism_index>0.3?0.7:0.3;
    er->transparency=er->sys->critical_depth;
    cst_ethics_assess_principle(er,CST_ETHIC_BENEFICENCE,er->beneficence);
    cst_ethics_assess_principle(er,CST_ETHIC_NONMALEFICENCE,er->nonmaleficence);
    cst_ethics_assess_principle(er,CST_ETHIC_JUSTICE,er->justice);
    cst_ethics_assess_principle(er,CST_ETHIC_AUTONOMY,er->autonomy);
    cst_ethics_assess_principle(er,CST_ETHIC_TRANSPARENCY,er->transparency);
    er->overall_ethics_score=cst_ethics_aggregate_score(er);
    cst_ethics_generate_recommendations(er);
}
void cst_ethics_assess_principle(CSTEthicsReport* er,CSTEthicalPrinciple p,double score){
    for(int i=0;i<er->n_assessments;i++)if(er->assessments[i].principle==p){
        er->assessments[i].score=score;er->assessments[i].is_violated=(score<0.3);
        if(er->assessments[i].is_violated)er->n_violations++;break;
    }
    er->has_ethical_violations=(er->n_violations>0);
}
double cst_ethics_aggregate_score(const CSTEthicsReport* er){double s=0.0;for(int i=0;i<er->n_assessments;i++)s+=er->assessments[i].score;return s/er->n_assessments;}
bool cst_ethics_is_acceptable(const CSTEthicsReport* er){return er->overall_ethics_score>0.6&&er->n_violations==0;}
int cst_ethics_count_violations(const CSTEthicsReport* er){return er->n_violations;}
void cst_ethics_generate_recommendations(CSTEthicsReport* er){
    free(er->recommendations);er->recommendations=NULL;er->n_recommendations=0;
    for(int i=0;i<er->n_assessments;i++)if(er->assessments[i].is_violated){
        er->recommendations=realloc(er->recommendations,(size_t)(er->n_recommendations+1)*sizeof(char*));
        char buf[128];snprintf(buf,sizeof(buf),"Address %s violation (score=%.2f)",er->assessments[i].name,er->assessments[i].score);
        er->recommendations[er->n_recommendations++]=strdup(buf);
    }
}
const char* cst_principle_name(CSTEthicalPrinciple p){switch(p){case CST_ETHIC_BENEFICENCE:return"Beneficence";case CST_ETHIC_NONMALEFICENCE:return"Non-maleficence";case CST_ETHIC_JUSTICE:return"Justice";case CST_ETHIC_AUTONOMY:return"Autonomy";case CST_ETHIC_TRANSPARENCY:return"Transparency";default:return"Unknown";}}
double cst_ethics_violation_severity(const CSTEthicsReport* er){double s=0.0;for(int i=0;i<er->n_assessments;i++)if(er->assessments[i].is_violated)s+=1.0-er->assessments[i].score;return s;}
bool cst_ethics_needs_urgent_review(const CSTEthicsReport* er){return er->n_violations>=3||er->overall_ethics_score<0.3;}
int cst_ethics_principle_rank(const CSTEthicsReport* er,CSTEthicalPrinciple p){for(int i=0;i<er->n_assessments;i++)if(er->assessments[i].principle==p)return i;return -1;}
double cst_ethics_compliance_index(const CSTEthicsReport* er){return er->overall_ethics_score;}
double cst_ethics_rawlsian_difference(const CSTEthicsReport* er){double w=1.0;for(int i=0;i<er->sys->n_stakeholders;i++)if(er->sys->stakeholders[i].salience<w)w=er->sys->stakeholders[i].salience;return w;}
bool cst_ethics_satisfies_rawls(const CSTEthicsReport* er){return cst_ethics_rawlsian_difference(er)>0.4;}
double cst_ethics_capability_approach(const CSTEthicsReport* er){int n=0;for(int i=0;i<er->sys->n_stakeholders;i++)if(er->sys->stakeholders[i].power>0.3&&er->sys->stakeholders[i].legitimacy>0.3)n++;return(er->sys->n_stakeholders>0)?(double)n/er->sys->n_stakeholders:0.0;}
double cst_ethics_distributive_justice(const CSTEthicsReport* er){double mn=1.0,mx=0.0;for(int i=0;i<er->sys->n_stakeholders;i++){double s=er->sys->stakeholders[i].salience;if(s<mn)mn=s;if(s>mx)mx=s;}return(mx>1e-10)?mn/mx:0.0;}
double cst_ethics_procedural_justice(const CSTEthicsReport* er){return er->transparency*0.5+er->autonomy*0.5;}
void cst_ethics_audit_stakeholders(const CSTEthicsReport* er){printf("Ethics Audit:\n");for(int i=0;i<er->sys->n_stakeholders;i++)printf("  %s P=%.2f L=%.2f Sal=%.2f%s\n",er->sys->stakeholders[i].name,er->sys->stakeholders[i].power,er->sys->stakeholders[i].legitimacy,er->sys->stakeholders[i].salience,er->sys->stakeholders[i].is_marginalized?" [MARG]":"");}
void cst_ethics_care_perspective(const CSTEthicsReport* er){printf("Ethics of Care: viol=%s\n",er->has_ethical_violations?"NEEDS":"OK");for(int i=0;i<er->sys->n_stakeholders;i++)if(er->sys->stakeholders[i].urgency>0.7)printf("  URGENT: %s\n",er->sys->stakeholders[i].name);}
void cst_ethics_full_report(const CSTEthicsReport* er){cst_ethics_print(er);printf("RawlsianDiff=%.2f DistribJustice=%.2f ProcedJustice=%.2f\n",cst_ethics_rawlsian_difference(er),cst_ethics_distributive_justice(er),cst_ethics_procedural_justice(er));}
void cst_ethics_print(const CSTEthicsReport* er){printf("=== Ethics Report ===\nOverall=%.3f Acceptable=%s Violations=%d\n",er->overall_ethics_score,cst_ethics_is_acceptable(er)?"YES":"NO",er->n_violations);for(int i=0;i<er->n_assessments;i++)printf("  %-20s %.2f%s\n",er->assessments[i].name,er->assessments[i].score,er->assessments[i].is_violated?" *":"");
if(er->n_recommendations>0){printf("Recommendations:\n");for(int i=0;i<er->n_recommendations;i++)printf("  -> %s\n",er->recommendations[i]);}}
void cst_ethics_report_card(const CSTEthicsReport* er){cst_ethics_print(er);}

/* Multiple ethical frameworks */
double cst_ethics_utilitarian_score(const CSTEthicsReport* er){
    double total_benefit=0.0;int n=0;
    for(int i=0;i<er->sys->n_stakeholders;i++){total_benefit+=er->sys->stakeholders[i].salience;n++;}
    return(n>0)?total_benefit/n:0.0;
}
double cst_ethics_deontological_score(const CSTEthicsReport* er){
    return(er->nonmaleficence>0.6&&er->autonomy>0.5&&er->justice>0.4)?0.8:0.3;
}
double cst_ethics_virtue_score(const CSTEthicsReport* er){
    return(er->beneficence+er->transparency)/2.0;
}
double cst_ethics_care_score(const CSTEthicsReport* er){
    double vulnerable=0.0;int nv=0;
    for(int i=0;i<er->sys->n_stakeholders;i++)if(er->sys->stakeholders[i].is_marginalized){vulnerable+=er->sys->stakeholders[i].salience;nv++;}
    return(nv>0)?1.0-vulnerable/nv:1.0;
}
double cst_ethics_multi_framework_assessment(const CSTEthicsReport* er){
    return cst_ethics_utilitarian_score(er)*0.25+cst_ethics_deontological_score(er)*0.25+cst_ethics_virtue_score(er)*0.2+cst_ethics_care_score(er)*0.3;
}

/* Ethics maturity model */
typedef enum{CST_ETHICS_CHAOTIC=0,CST_ETHICS_REACTIVE=1,CST_ETHICS_COMPLIANT=2,CST_ETHICS_PROACTIVE=3,CST_ETHICS_TRANSFORMATIVE=4}CSTEthicsMaturity;
CSTEthicsMaturity cst_ethics_maturity_level(const CSTEthicsReport* er){
    double s=er->overall_ethics_score;
    if(s<0.2)return CST_ETHICS_CHAOTIC;if(s<0.4)return CST_ETHICS_REACTIVE;
    if(s<0.6)return CST_ETHICS_COMPLIANT;if(s<0.8)return CST_ETHICS_PROACTIVE;
    return CST_ETHICS_TRANSFORMATIVE;
}
const char* cst_ethics_maturity_label(CSTEthicsMaturity m){switch(m){case CST_ETHICS_CHAOTIC:return"Chaotic";case CST_ETHICS_REACTIVE:return"Reactive";case CST_ETHICS_COMPLIANT:return"Compliant";case CST_ETHICS_PROACTIVE:return"Proactive";case CST_ETHICS_TRANSFORMATIVE:return"Transformative";default:return"Unknown";}}
void cst_ethics_maturity_report(const CSTEthicsReport* er){CSTEthicsMaturity m=cst_ethics_maturity_level(er);printf("Ethics Maturity: %s (%.2f)\n",cst_ethics_maturity_label(m),er->overall_ethics_score);printf("Utilitarian=%.2f Deont=%.2f Virtue=%.2f Care=%.2f Multi=%.2f\n",cst_ethics_utilitarian_score(er),cst_ethics_deontological_score(er),cst_ethics_virtue_score(er),cst_ethics_care_score(er),cst_ethics_multi_framework_assessment(er));}

/* Intergenerational justice */
double cst_ethics_future_generations_weight(const CSTEthicsReport* er){
    int n_future=0;for(int i=0;i<er->sys->n_stakeholders;i++)if(er->sys->stakeholders[i].power<0.1)n_future++;
    return(er->sys->n_stakeholders>0)?(double)n_future/er->sys->n_stakeholders:0.0;
}
double cst_ethics_sustainability_ethics(const CSTEthicsReport* er){return er->justice*0.4+cst_ethics_future_generations_weight(er)*0.6;}

/* Ethics improvement plan */
int cst_ethics_improvement_priorities(const CSTEthicsReport* er, CSTEthicalPrinciple* order, int max_n){
    double scores[5];CSTEthicalPrinciple ps[5];
    for(int i=0;i<5;i++){scores[i]=1.0-er->assessments[i].score;ps[i]=er->assessments[i].principle;}
    for(int i=0;i<4;i++)for(int j=i+1;j<5;j++)if(scores[j]>scores[i]){double ts=scores[i];scores[i]=scores[j];scores[j]=ts;CSTEthicalPrinciple tp=ps[i];ps[i]=ps[j];ps[j]=tp;}
    int n=0;for(int i=0;i<5&&n<max_n;i++)if(scores[i]>0.3)order[n++]=ps[i];
    return n;
}

/* Ethics of recognition (Honneth, 1995; Fraser, 2003) */
double cst_ethics_recognition_score(const CSTEthicsReport* er){
    int recognized=0;for(int i=0;i<er->sys->n_stakeholders;i++)if(!er->sys->stakeholders[i].is_marginalized)recognized++;
    return(er->sys->n_stakeholders>0)?(double)recognized/er->sys->n_stakeholders:0.0;
}
bool cst_ethics_all_recognized(const CSTEthicsReport* er){return cst_ethics_recognition_score(er)>0.9;}

/* Ethics of the other (Levinas): responsibility to the vulnerable */
double cst_ethics_levinas_responsibility(const CSTEthicsReport* er){
    double most_vulnerable=1.0;for(int i=0;i<er->sys->n_stakeholders;i++){double v=1.0-er->sys->stakeholders[i].salience;if(v<most_vulnerable)most_vulnerable=v;}
    return most_vulnerable;
}
bool cst_ethics_responsibility_fulfilled(const CSTEthicsReport* er){return cst_ethics_levinas_responsibility(er)>0.5;}

/* Discourse ethics (Habermas, Apel): inclusive dialogue */
double cst_ethics_discourse_quality(const CSTEthicsReport* er){return cst_deliberative_quality(er->sys);}
bool cst_ethics_discourse_adequate(const CSTEthicsReport* er){return cst_ethics_discourse_quality(er)>0.6;}

/* Ethics audit trail */
void cst_ethics_audit_trail(const CSTEthicsReport* er){
    printf("=== Ethics Audit Trail ===\n");
    printf("System: %s\n",er->sys->name);printf("Iterations: %d\n",er->sys->iteration);
    printf("Stakeholders: %d (marginalized: ",er->sys->n_stakeholders);
    int m=0;for(int i=0;i<er->sys->n_stakeholders;i++)if(er->sys->stakeholders[i].is_marginalized)m++;
    printf("%d)\n",m);
    printf("Boundaries: %d Openness=%.3f\n",er->sys->awareness.n_boundaries,er->sys->awareness.boundary_openness);
    printf("Emancipatory Index: %.3f\n",er->sys->emancipatory_index);
}

/* Boundary Critique per Ulrich (1983) — CSH 12 boundary questions.
 * Maps the four boundary dimensions to their sources. */
typedef struct {
    char* sources_of_motivation;   /* Who benefits? */
    char* sources_of_control;      /* Who decides? */
    char* sources_of_knowledge;    /* Who provides expertise? */
    char* sources_of_legitimacy;   /* Who guarantees legitimacy? */
} BoundaryCritique;

/* Emancipatory ethics check per Ulrich (1983): verify all affected parties
 * are represented in the boundary critique. */
int cst_emancipation_check(const BoundaryCritique* bc) {
    if (!bc) return 0;
    int score = 0;
    if (bc->sources_of_motivation  && bc->sources_of_motivation[0]  != '\0') score++;
    if (bc->sources_of_control     && bc->sources_of_control[0]     != '\0') score++;
    if (bc->sources_of_knowledge   && bc->sources_of_knowledge[0]   != '\0') score++;
    if (bc->sources_of_legitimacy  && bc->sources_of_legitimacy[0]  != '\0') score++;
    return score;
}
