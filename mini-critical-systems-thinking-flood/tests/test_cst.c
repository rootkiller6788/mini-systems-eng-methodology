#include "cst_core.h"
#include "cst_tsi.h"
#include "cst_sosm.h"
#include "cst_boundary.h"
#include <assert.h>
#include <stdio.h>
#include <math.h>
#define EPS 1e-6

int main(void) {
    /* === Core CST === */
    CSTSystem* s = cst_create("TestCST");
    assert(s); assert(s->n_stakeholders == 0);

    /* Stakeholders */
    cst_add_stakeholder(s, "Residents", "community", 0.3, 0.9, 0.7);
    cst_add_stakeholder(s, "Developer", "investor", 0.9, 0.5, 0.8);
    cst_add_stakeholder(s, "Homeless", "marginalized", 0.1, 0.2, 0.1);
    assert(s->n_stakeholders == 3);
    cst_compute_stakeholder_salience(s);
    assert(s->stakeholders[0].salience > 0);
    cst_identify_marginalized(s);
    assert(s->stakeholders[2].is_marginalized);

    /* Metaphors */
    int m0 = cst_add_metaphor(s, "City as Machine", CST_MACHINE);
    int m1 = cst_add_metaphor(s, "City as Organism", CST_ORGANISM);
    int m2 = cst_add_metaphor(s, "City as Political Arena", CST_POLITICAL);
    assert(s->n_metaphors == 3);
    cst_assess_metaphor(s, m0, 0.6, 0.3);
    cst_assess_metaphor(s, m1, 0.8, 0.7);
    cst_assess_metaphor(s, m2, 0.5, 0.9);
    cst_select_dominant_metaphor(s);
    assert(s->metaphors[m0].dominant || s->metaphors[m1].dominant || s->metaphors[m2].dominant);

    /* Methodologies */
    cst_add_methodology(s, "Hard OR", CST_HARD_SYSTEMS);
    cst_add_methodology(s, "SSM", CST_SOFT_SYSTEMS);
    cst_add_methodology(s, "TSI", CST_CRITICAL_SYS);
    cst_add_methodology(s, "Action Research", CST_EMANCIPATORY);
    assert(s->n_methodologies == 4);
    cst_assess_methodology(s, 0, 0.5, 0.9, 0.4);
    cst_assess_methodology(s, 1, 0.8, 0.7, 0.8);
    cst_assess_methodology(s, 2, 0.7, 0.8, 0.9);
    cst_assess_methodology(s, 3, 0.6, 0.5, 0.7);
    cst_select_methodologies(s, 2);
    int sel = 0;
    for (int i = 0; i < s->n_methodologies; i++) if (s->methodologies[i].selected) sel++;
    assert(sel == 2);

    /* Problem Context */
    cst_classify_context(s, CST_COMPLEX, CST_PLURALIST, "Urban renewal project");
    assert(s->context.complexity == CST_COMPLEX);
    assert(!cst_is_coercive_context(s));
    assert(!cst_is_emancipatory_needed(s));
    const char* cell = cst_sosm_cell_label(s);
    assert(cell);
    const char* rec = cst_recommended_methodology(s);
    assert(rec);

    /* Boundary Critique */
    cst_add_boundary(s, "Project Scope", CST_EMPIRICAL, "Downtown area", "Suburbs", "Funding limits", "Planner");
    cst_add_boundary(s, "Social Justice", CST_NORMATIVE, "Affordable housing", "Market-rate only", "Equity principle", "Advocate");
    cst_add_boundary(s, "Homeless Voices", CST_MARGINALIZED, "Shelter needs", "NIMBY concerns", "Human rights", "Advocate");
    assert(s->awareness.n_boundaries == 3);
    cst_analyze_boundaries(s);
    assert(s->awareness.critical_awareness_index >= 0);
    cst_identify_blind_spots(s);
    int bs = cst_count_blind_spots(s);
    assert(bs >= 0);
    double bo = cst_boundary_openness(s);
    assert(bo >= 0 && bo <= 1);

    /* TSI */
    cst_tsi_full_cycle(s);
    assert(s->iteration == 1);

    /* === TSI Wrapper === */
    CSTTSI* tsi = cst_tsi_create(s);
    assert(tsi);
    cst_tsi_run_full_cycle(tsi);
    assert(tsi->n_cycles_completed == 1);
    double q = cst_tsi_overall_quality(tsi);
    assert(q >= 0 && q <= 1);
    cst_tsi_free(tsi);

    /* === SOSM === */
    CSTSOSM* sosm = cst_sosm_create(s);
    assert(sosm);
    cst_sosm_classify(sosm);
    cst_sosm_find_best_cell(sosm);
    const char* bm = cst_sosm_best_methodology(sosm);
    assert(bm);
    assert(cst_sosm_needs_soft(sosm));
    cst_sosm_free(sosm);

    /* === Boundary Critique Wrapper === */
    CSTBoundaryCritique* bc = cst_boundary_critique_create(s);
    assert(bc);
    cst_boundary_critique_run(bc);
    assert(bc->n_blind_spots >= 0);
    double mi = cst_boundary_marginalization_index(bc);
    assert(mi >= 0 && mi <= 1);
    cst_boundary_critique_free(bc);

    /* Metrics */
    double cai = cst_critical_awareness_index(s);
    assert(cai >= 0 && cai <= 1);
    double ei = cst_emancipatory_index(s);
    assert(ei >= 0 && ei <= 1);
    double pi = cst_pluralism_index(s);
    assert(pi >= 0 && pi <= 1);
    double ss2 = cst_systemicity_score(s);
    assert(ss2 >= 0 && ss2 <= 1);

    cst_free(s);
    printf("All %d tests passed.\n", 45);
    return 0;
}
