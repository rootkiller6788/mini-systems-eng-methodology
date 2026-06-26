#include "ssm_core.h"
#include <stdio.h>
int main(void){
    printf("=== Root Definition: PQR + 5E Analysis ===\n\n");
    RootDefinition* rd=ssm_rootdef_create("University Admissions",SSM_RD_ISSUE_BASED);
    ssm_rootdef_set_pqr(rd,"select qualified applicants","holistic admissions process",
        "build a diverse and excellent student body");
    printf("Root Definition:\n");
    ssm_rootdef_print(rd);

    ssm_rootdef_set_efficacy(rd,0.9); ssm_rootdef_set_efficiency(rd,0.6);
    ssm_rootdef_set_effectiveness(rd,0.8); ssm_rootdef_set_ethicality(rd,0.7);
    ssm_rootdef_set_elegance(rd,0.5);
    printf("\n5E Composite: %.2f  Viable: %s  Monitorable: %s\n",
        ssm_rootdef_5e_composite(rd),ssm_rootdef_is_viable(rd)?"YES":"NO",
        ssm_rootdef_monitorable(rd)?"YES":"NO");

    /* Demonstrate multi-RD comparison */
    RootDefinition* rd2=ssm_rootdef_create("Exam-only Admissions",SSM_RD_PRIMARY_TASK);
    ssm_rootdef_set_pqr(rd2,"select highest scorers","standardized exam ranking",
        "maximize academic merit");
    ssm_rootdef_set_efficacy(rd2,0.95);ssm_rootdef_set_efficiency(rd2,0.9);
    ssm_rootdef_set_effectiveness(rd2,0.5);ssm_rootdef_set_ethicality(rd2,0.3);
    ssm_rootdef_set_elegance(rd2,0.9);

    RootDefinition* rds[]={rd,rd2};
    ssm_rootdef_rank_by_relevance(rds,2);
    printf("\nRanked Root Definitions:\n");
    for(int i=0;i<2;i++)printf("  %d: %s (rank %d)\n",i+1,rds[i]->name,rds[i]->relevance_rank);

    printf("\nPair conflict: %.2f\n",ssm_rootdef_pair_conflict(rd,rd2));
    ssm_rootdef_free(rd);ssm_rootdef_free(rd2);
    return 0;
}

