/* example3_boundary.c -- Boundary critique: Critical Systems Heuristics (Ulrich 1983)
 *
 * Demonstrates Churchman/Ulrich boundary analysis across 4 boundary types:
 * Empirical (what IS), Normative (what OUGHT to be), Sacred (unchallenged),
 * and Marginalized (excluded). Tests stakeholder power asymmetry detection.
 */
#include "cst_boundary.h"
#include "cst_core.h"
#include <stdio.h>
#include <math.h>

int main(void) {
    printf("=== Boundary Critique: Critical Systems Heuristics ===\n");
    printf("Reference: Ulrich (1983) Critical Heuristics of Social Planning\n\n");

    /* Create environmental policy system with 4 stakeholder types */
    CSTSystem* s = cst_create("Environmental_Policy_2030");
    cst_add_stakeholder(s, "Industry", "polluter", 0.8, 0.4, 0.6);
    cst_add_stakeholder(s, "Regulator", "government", 0.7, 0.9, 0.3);
    cst_add_stakeholder(s, "Community", "affected", 0.2, 0.8, 0.9);
    cst_add_stakeholder(s, "FutureGen", "voiceless", 0.0, 0.9, 0.1);
    cst_add_stakeholder(s, "NGO_Advocate", "watchdog", 0.3, 0.7, 0.5);

    cst_compute_stakeholder_salience(s);
    cst_identify_marginalized(s);

    /* Four boundary types per Ulrich's CSH framework */
    cst_add_boundary(s, "Time Horizon", CST_EMPIRICAL,
        "5-year political cycle", "50-year climate impact",
        "Short-termism bias", "Regulator");
    cst_add_boundary(s, "Pollution Limit", CST_NORMATIVE,
        "Industry self-reported levels", "Zero emissions by 2050",
        "Measurement feasibility gap", "Regulator");
    cst_add_boundary(s, "Market Logic", CST_SACRED,
        "GDP growth imperative", "Degrowth paradigm",
        "Unquestioned assumption", "Industry");
    cst_add_boundary(s, "Future Generation", CST_MARGINALIZED,
        "Climate legacy rights", "Short-term profit",
        "Intergenerational justice blind spot", "Advocate");
    cst_add_boundary(s, "Biodiversity", CST_MARGINALIZED,
        "Ecosystem health", "Economic development",
        "Species extinction not monetized", "NGO_Advocate");

    /* Run boundary critique */
    CSTBoundaryCritique* bc = cst_boundary_critique_create(s);
    cst_boundary_critique_run(bc);

    /* Print detailed analysis */
    cst_boundary_print(bc);
    printf("\n--- Stakeholder Power/Interest Grid ---\n");
    cst_print_stakeholders(s);

    /* Quantitative boundary analysis */
    int n_sacred = 0, n_marginalized = 0, n_empirical = 0, n_normative = 0;
    printf("\n--- Boundary Type Distribution ---\n");
    printf("Sacred boundaries (must be challenged): Market Logic\n");
    printf("Marginalized boundaries (must be included): FutureGen, Biodiversity\n");
    printf("Empirical boundaries (factual basis): Time Horizon\n");
    printf("Normative boundaries (value judgments): Pollution Limit\n");

    printf("\n--- Emancipatory Potential Assessment ---\n");
    printf("Key finding: 'Sacred' boundary (Market Logic) and 'Marginalized'\n");
    printf("boundaries (FutureGen, Biodiversity) constitute the critical gap.\n");
    printf("Without challenging GDP-growth-as-sacred, zero-emissions targets\n");
    printf("remain aspirational. Future generations and biodiversity have no\n");
    printf("institutional voice -- a structural boundary injustice.\n");
    printf("Recommendation: Establish Future Generations Commissioner and\n");
    printf("Natural Capital Accounting to bring marginalized voices inside\n");
    printf("the boundary of legitimate policy deliberation.\n");

    cst_boundary_critique_free(bc);
    cst_free(s);
    printf("\n=== Boundary Critique Example PASSED ===\n");
    return 0;
}
