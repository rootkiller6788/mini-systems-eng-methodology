#ifndef CST_BOUNDARY_H
#define CST_BOUNDARY_H
#include "cst_core.h"

/* Boundary Critique (Ulrich, 1983; Midgley, 2000)
 * Systematic examination of what is included/excluded in system boundaries */
typedef struct {
    CSTSystem* sys;
    double boundary_openness;
    double ethical_adequacy;
    double stakeholder_coverage;
    double marginalized_inclusion;
    int n_contested_boundaries;
    int n_blind_spots;
} CSTBoundaryCritique;

CSTBoundaryCritique* cst_boundary_critique_create(CSTSystem* sys);
void cst_boundary_critique_free(CSTBoundaryCritique* bc);
void cst_boundary_critique_run(CSTBoundaryCritique* bc);
void cst_boundary_sweep_in(CSTBoundaryCritique* bc, int boundary_idx);
void cst_boundary_sweep_out(CSTBoundaryCritique* bc, int boundary_idx);
double cst_boundary_marginalization_index(const CSTBoundaryCritique* bc);
double cst_boundary_contestation_index(const CSTBoundaryCritique* bc);
int cst_boundary_count_marginalized(const CSTBoundaryCritique* bc);
void cst_boundary_print(const CSTBoundaryCritique* bc);
double cst_boundary_power_analysis(const CSTBoundaryCritique* bc);
double cst_boundary_ethical_gap(const CSTBoundaryCritique* bc);
bool cst_boundary_needs_restructuring(const CSTBoundaryCritique* bc);
int cst_boundary_recommend_sweep_in(const CSTBoundaryCritique* bc);
const char* cst_boundary_type_name(CSTBoundaryType t);
bool cst_boundary_is_ethical(const CSTBoundaryCritique* bc);
void cst_boundary_ulrich_checklist(const CSTBoundaryCritique* bc);
double cst_boundary_ulrich_coverage(const CSTBoundaryCritique* bc);
bool cst_boundary_has_shifted(const CSTBoundaryCritique* bc, int idx, double thresh);
int cst_boundary_shift_count(const CSTBoundaryCritique* bc);
double cst_boundary_stakeholder_alignment(const CSTBoundaryCritique* bc, int si);
int cst_boundary_most_aligned_stakeholder(const CSTBoundaryCritique* bc);
int cst_boundary_least_aligned_stakeholder(const CSTBoundaryCritique* bc);
void cst_boundary_record_state(CSTBoundaryCritique* bc);
bool cst_boundary_resolution_possible(const CSTBoundaryCritique* bc);

/* Boundary evolution states */
typedef enum { CST_BOUNDARY_STABLE=0, CST_BOUNDARY_EVOLVING=1, CST_BOUNDARY_TURBULENT=2 } CSTBoundaryState;
CSTBoundaryState cst_boundary_classify_state(const CSTBoundaryCritique* bc);
const char* cst_boundary_state_label(CSTBoundaryState s);

/* Midgley systemic intervention boundary methods */
double cst_boundary_midgley_coefficient(const CSTBoundaryCritique* bc);
int  cst_boundary_identify_systemic_boundaries(const CSTBoundaryCritique* bc, int* indices, int max_n);

/* Multiple boundary negotiation strategies */
typedef enum { CST_NEGOTIATE_AVERAGE=0, CST_NEGOTIATE_MAX=1, CST_NEGOTIATE_MIN=2, CST_NEGOTIATE_WEIGHTED=3 } CSTNegotiationStrategy;
void cst_boundary_negotiate_strategy(CSTBoundaryCritique* bc, int a, int b, CSTNegotiationStrategy strategy);
double cst_boundary_evolution_rate(const CSTBoundaryCritique* bc);
bool cst_boundary_is_stable(const CSTBoundaryCritique* bc);
bool cst_boundary_is_evolving(const CSTBoundaryCritique* bc);
bool cst_boundary_is_turbulent(const CSTBoundaryCritique* bc);
int cst_boundary_sweep_all_marginalized(CSTBoundaryCritique* bc);
int cst_boundary_sweep_all_sacred(CSTBoundaryCritique* bc);

#endif /* CST_BOUNDARY_H */