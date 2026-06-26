/* example3_conceptual.c -- SSM Conceptual Model: Public Library System
 *
 * Full demonstration of Checkland's SSM Stage 4: building a conceptual
 * model from a root definition. Tests activity modeling, dependency
 * analysis, topological sorting, and coverage metrics.
 */
#include "ssm_core.h"
#include <stdio.h>
#include <stdlib.h>

int main(void){
    printf("=== SSM Conceptual Model: Public Library System ===\n");
    printf("Root Definition: A system owned by the local authority,\n");
    printf("operated by librarians, to provide knowledge access to\n");
    printf("community members by lending materials, in order to\n");
    printf("support lifelong learning and cultural enrichment.\n\n");

    ConceptualModel* cm = ssm_conceptual_create(
        "A system to provide library services by lending books "
        "in order to support learning");

    /* Stage 4a: Define activities (7-10 verbs) */
    int a0 = ssm_conceptual_add_activity(cm, "Acquire materials",
        "Select, order, and receive books/media from suppliers");
    int a1 = ssm_conceptual_add_activity(cm, "Catalog items",
        "Assign Dewey/ISBN metadata and create searchable records");
    int a2 = ssm_conceptual_add_activity(cm, "Register borrowers",
        "Issue library cards to community members");
    int a3 = ssm_conceptual_add_activity(cm, "Lend materials",
        "Check out items to registered borrowers");
    int a4 = ssm_conceptual_add_activity(cm, "Monitor returns",
        "Track due dates, send reminders, manage overdue items");
    int a5 = ssm_conceptual_add_activity(cm, "Manage reservations",
        "Handle hold requests and notify when items become available");
    int a6 = ssm_conceptual_add_activity(cm, "Maintain collection",
        "Repair, replace, and weed items to maintain quality");

    /* Stage 4b: Define logical dependencies */
    ssm_conceptual_add_dependency(cm, a0, a1);
    ssm_conceptual_add_dependency(cm, a1, a3);
    ssm_conceptual_add_dependency(cm, a2, a3);
    ssm_conceptual_add_dependency(cm, a3, a4);
    ssm_conceptual_add_dependency(cm, a4, a5);
    ssm_conceptual_add_dependency(cm, a4, a6);
    ssm_conceptual_add_dependency(cm, a5, a3);

    /* Set activity levels (0=operational, 1=tactical, 2=strategic) */
    ssm_activity_set_level(cm->activities[a0], 2);  /* Acquisitions = strategic */
    ssm_activity_set_level(cm->activities[a3], 0);  /* Lending = operational */
    ssm_activity_set_level(cm->activities[a6], 1);  /* Maintenance = tactical */

    /* Set performance indicators */
    ssm_activity_add_indicator(cm->activities[a3], "Books lent/month", 500.0);
    ssm_activity_add_indicator(cm->activities[a4], "Return rate (%)", 95.0);
    ssm_activity_add_indicator(cm->activities[a6], "Collection condition", 0.90);

    /* Print model */
    ssm_conceptual_print(cm);

    /* Analyze model properties */
    printf("\n=== Model Analysis ===\n");
    int n_levels = ssm_conceptual_count_levels(cm);
    double coverage = ssm_conceptual_coverage(cm);
    double complexity = ssm_conceptual_complexity(cm);
    int connected = ssm_conceptual_is_connected(cm);

    printf("Activity levels (strategic/tactical/operational): %d\n", n_levels);
    printf("Root definition coverage: %.2f\n", coverage);
    printf("Model complexity (dependencies/activity): %.2f\n", complexity);
    printf("Fully connected: %s\n", connected ? "YES" : "NO (warning)");

    /* Topological order validation */
    int n;
    int* order = ssm_conceptual_topological_sort(cm, &n);
    printf("\nTopological execution order (%d activities):\n", n);
    for (int i = 0; i < n; i++) {
        printf("  Step %d: %s (level %d)\n", i + 1,
               cm->activities[order[i]]->name,
               cm->activities[order[i]]->level);
    }

    /* Dependency cycle check */
    printf("\nDependency cycles: ");
    if (connected && n > 0) {
        printf("NONE -- model is a valid DAG\n");
    } else {
        printf("DETECTED -- revise activity dependencies\n");
    }

    free(order);
    ssm_conceptual_free(cm);
    printf("\n=== Conceptual Model Example PASSED ===\n");
    return 0;
}

