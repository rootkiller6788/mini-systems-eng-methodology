#ifndef CST_CORE_H
#define CST_CORE_H
#include <stdbool.h>
#include <stddef.h>

/* ==============================================================
 * Critical Systems Thinking — Core Types & Framework
 *
 * Based on: Flood & Jackson (1991), Ulrich (1983), Midgley (2000)
 *
 * CST integrates three commitments:
 *   1. Critical Awareness — examine assumptions & boundary judgments
 *   2. Social Awareness — recognize power structures & societal context
 *   3. Human Emancipation — systems thinking for human well-being
 *
 * Companion source: cst_core.c
 * ============================================================== */

/* Constants */
#define CST_EPSILON      1e-12
#define CST_MAX_STAKEHOLDERS 64
#define CST_MAX_METAPHORS    16
#define CST_MAX_METHODOLOGIES 32
#define CST_MAX_CRITERIA     16
#define CST_MAX_BOUNDARIES   32

/* Problem context complexity */
typedef enum { CST_SIMPLE = 0, CST_COMPLEX = 1, CST_CHAOTIC = 2 } CSTComplexity;

/* Relationship type (SOSM — System of Systems Methodologies) */
typedef enum { CST_UNITARY = 0, CST_PLURALIST = 1, CST_COERCIVE = 2 } CSTRelationship;

/* Organizational metaphor (Morgan, 1986 / Flood & Jackson, 1991) */
typedef enum {
    CST_MACHINE = 0, CST_ORGANISM = 1, CST_BRAIN = 2,
    CST_CULTURE = 3, CST_POLITICAL = 4, CST_PSYCHIC_PRISON = 5,
    CST_FLUX = 6, CST_INSTRUMENT = 7
} CSTMetaphor;

/* TSI (Total Systems Intervention) phase */
typedef enum { CST_CREATIVITY = 0, CST_CHOICE = 1,
               CST_IMPLEMENTATION = 2, CST_REFLECTION = 3 } CSTTSIPhase;

/* Methodology family */
typedef enum {
    CST_HARD_SYSTEMS = 0, CST_SOFT_SYSTEMS = 1,
    CST_CRITICAL_SYS = 2, CST_EMANCIPATORY = 3, CST_POSTMODERN = 4
} CSTMethodFamily;

/* Boundary judgment type (Ulrich, 1983) */
typedef enum {
    CST_EMPIRICAL = 0, CST_NORMATIVE = 1,
    CST_SACRED = 2, CST_MARGINALIZED = 3
} CSTBoundaryType;

/* Stakeholder */
typedef struct {
    char* name; char* role;
    double power, legitimacy, urgency, salience;
    bool is_marginalized;
    int boundary_claims[4];
} CSTStakeholder;

/* Metaphor lens */
typedef struct {
    char* name; CSTMetaphor type;
    double relevance, insight_score;
    char** key_insights; int n_insights; bool dominant;
} CSTMetaphorLens;

/* Methodology */
typedef struct {
    char* name; CSTMethodFamily family;
    double suitability, feasibility, acceptability, overall_score;
    char* strengths; char* weaknesses; bool selected;
} CSTMethodology;

/* Problem context */
typedef struct {
    CSTComplexity complexity; CSTRelationship relationship;
    char* description; bool is_coercive_context, requires_emancipatory;
} CSTProblemContext;

/* Boundary judgment */
typedef struct {
    char* name; CSTBoundaryType type;
    char* what_is_included; char* what_is_excluded;
    char* justification; char* source;
    double ethical_weight; bool is_contested;
} CSTBoundary;

/* Critical awareness assessment */
typedef struct {
    CSTBoundary* boundaries; int n_boundaries;
    double critical_awareness_index, boundary_openness;
    double reflexivity_score, emancipation_potential;
    int n_marginalized_voices;
    char** blind_spots; int n_blind_spots;
} CSTCriticalAwareness;

/* CSTSystem */
typedef struct {
    char* name; double time; int iteration;
    CSTStakeholder* stakeholders; int n_stakeholders;
    CSTMetaphorLens* metaphors; int n_metaphors;
    CSTMethodology* methodologies; int n_methodologies;
    CSTProblemContext context;
    CSTCriticalAwareness awareness;
    CSTTSIPhase current_phase; double phase_progress;
    int creativity_rounds, implementation_rounds;
    double systemicity_score, pluralism_index;
    double critical_depth, emancipatory_index;
    double power_asymmetry;  /* Gini-like power concentration [0-1] */
} CSTSystem;

/* API */
CSTSystem* cst_create(const char* name);
void cst_free(CSTSystem* s);
int  cst_add_stakeholder(CSTSystem* s, const char* name, const char* role,
                         double power, double legitimacy, double urgency);
void cst_compute_stakeholder_salience(CSTSystem* s);
void cst_identify_marginalized(CSTSystem* s);
int  cst_add_metaphor(CSTSystem* s, const char* name, CSTMetaphor type);
void cst_assess_metaphor(CSTSystem* s, int idx, double relevance, double insight);
void cst_select_dominant_metaphor(CSTSystem* s);
const char* cst_metaphor_description(CSTMetaphor m);
const char* cst_metaphor_question(CSTMetaphor m);
double cst_metaphor_distance(CSTMetaphor a, CSTMetaphor b);
double cst_metaphor_pluralism_score(const CSTSystem* s);
int  cst_add_methodology(CSTSystem* s, const char* name, CSTMethodFamily family);
void cst_assess_methodology(CSTSystem* s, int idx, double suit, double feas, double accept);
void cst_select_methodologies(CSTSystem* s, int max_count);
void cst_classify_context(CSTSystem* s, CSTComplexity comp, CSTRelationship rel, const char* desc);
const char* cst_sosm_cell_label(const CSTSystem* s);
const char* cst_recommended_methodology(const CSTSystem* s);
int  cst_add_boundary(CSTSystem* s, const char* name, CSTBoundaryType type,
                      const char* included, const char* excluded,
                      const char* justification, const char* source);
void cst_analyze_boundaries(CSTSystem* s);
void cst_identify_blind_spots(CSTSystem* s);
double cst_boundary_openness(const CSTSystem* s);
void cst_tsi_creativity_phase(CSTSystem* s);
void cst_tsi_choice_phase(CSTSystem* s);
void cst_tsi_implementation_phase(CSTSystem* s);
void cst_tsi_reflection_phase(CSTSystem* s);
void cst_tsi_full_cycle(CSTSystem* s);
double cst_critical_awareness_index(CSTSystem* s);
double cst_emancipatory_index(CSTSystem* s);
double cst_pluralism_index(CSTSystem* s);
double cst_systemicity_score(CSTSystem* s);
double cst_critical_depth(CSTSystem* s);
bool cst_is_coercive_context(const CSTSystem* s);
bool cst_is_emancipatory_needed(const CSTSystem* s);
int  cst_count_blind_spots(const CSTSystem* s);
double cst_stakeholder_inclusiveness(const CSTSystem* s);
void cst_print_state(const CSTSystem* s);
void cst_print_stakeholders(const CSTSystem* s);
void cst_print_boundaries(const CSTSystem* s);
void cst_print_methodologies(const CSTSystem* s);
void cst_print_metaphors(const CSTSystem* s);

/* Multiple salience computation models */
double cst_salience_mitchell(double power, double legitimacy, double urgency);
double cst_salience_power_interest(double power, double interest);
double cst_salience_influence_impact(double influence, double impact);
void cst_stakeholder_power_interest_grid(const CSTSystem* s, double* grid);  /* 4-quadrant scores */
int  cst_stakeholder_conflict_index(const CSTSystem* s, int a, int b);

/* Multiple methodology selection strategies */
void cst_select_methodologies_weighted(CSTSystem* s, int max_count, double w_suit, double w_feas, double w_accept);
void cst_select_methodologies_pareto(CSTSystem* s);
int  cst_methodology_dominance_count(const CSTSystem* s, int idx);
bool cst_methodology_is_dominated(const CSTSystem* s, int idx);

/* System archetype detection (Senge, 1990) */
typedef enum { CST_ARCH_NONE=0, CST_ARCH_FIXES_FAIL=1, CST_ARCH_SHIFTING_BURDEN=2, CST_ARCH_LIMITS_GROWTH=3, CST_ARCH_TRAGEDY_COMMONS=4, CST_ARCH_ERODING_GOALS=5, CST_ARCH_ESCALATION=6, CST_ARCH_SUCCESS_TO_SUCCESSFUL=7 } CSTSystemArchetype;
CSTSystemArchetype cst_detect_system_archetype(const CSTSystem* s);
const char* cst_archetype_label(CSTSystemArchetype a);
const char* cst_archetype_remedy(CSTSystemArchetype a);
double cst_archetype_confidence(const CSTSystem* s, CSTSystemArchetype a);

/* Jackson creative holism paradigm analysis */
typedef enum { CST_PARADIGM_FUNCTIONALIST=0, CST_PARADIGM_INTERPRETIVE=1, CST_PARADIGM_EMANCIPATORY=2, CST_PARADIGM_POSTMODERN=3 } CSTJacksonParadigm;
CSTJacksonParadigm cst_jackson_classify(const CSTSystem* s);
const char* cst_jackson_paradigm_label(CSTJacksonParadigm p);
double cst_jackson_paradigm_fit_score(const CSTSystem* s, CSTJacksonParadigm p);
bool cst_is_creatively_holistic(const CSTSystem* s);
double cst_creative_holism_index(const CSTSystem* s);

/* Pluralism & diversity */
double cst_methodological_diversity_index(const CSTSystem* s);
bool cst_is_methodologically_pluralist(const CSTSystem* s);
double cst_theoretical_depth(const CSTSystem* s);
double cst_systemic_coherence(const CSTSystem* s);
int cst_count_active_metaphors(const CSTSystem* s);
int cst_count_selected_methodologies(const CSTSystem* s);

/* Participation */
double cst_participation_index(const CSTSystem* s);
double cst_deliberative_quality(const CSTSystem* s);
bool cst_is_participatory_enough(const CSTSystem* s);

/* Learning & adaptation */
double cst_learning_rate(const CSTSystem* s);
double cst_adaptation_capacity(const CSTSystem* s);
bool cst_is_learning_system(const CSTSystem* s);

/* Holistic assessment */
typedef struct { double systemic,critical,participatory,ethical,pluralist,emancipatory,overall; } CSTHolisticScore;
CSTHolisticScore cst_holistic_assess(const CSTSystem* s);
void cst_print_holistic(const CSTSystem* s);

/* Diagnostics */
bool cst_has_marginalized_stakeholders(const CSTSystem* s);
bool cst_has_contested_boundaries(const CSTSystem* s);
bool cst_has_sacred_boundaries(const CSTSystem* s);
int cst_improvement_potential(const CSTSystem* s);
const char* cst_metaphor_description(CSTMetaphor m);
const char* cst_metaphor_question(CSTMetaphor m);
double cst_metaphor_distance(CSTMetaphor a, CSTMetaphor b);
double cst_metaphor_pluralism_score(const CSTSystem* s);
double cst_reflexivity_assessment(CSTSystem* s);
double cst_emancipation_potential(CSTSystem* s);
void cst_metaphor_shift_dominant(CSTSystem* s, CSTMetaphor new_d);
CSTMetaphor cst_metaphor_most_novel(const CSTSystem* s);
int cst_metaphor_rank_by_insight(const CSTSystem* s, CSTMetaphor* ranking, int max_n);
bool cst_detect_paradigm_shift(const CSTSystem* s, const CSTSystem* prev);
double cst_paradigm_distance(const CSTSystem* a, const CSTSystem* b);
void cst_print_evolution_summary(const CSTSystem* s);

/* Boundary sweep configuration (Flood & Jackson, 1991)
 * Each config represents a specific boundary judgment combination
 * across motivation, control, knowledge, and legitimacy dimensions. */
typedef struct {
    int motivation_sweep;      /* 0=given, 1=partially-chosen, 2=chosen */
    int control_sweep;         /* 0=given, 1=partially-chosen, 2=chosen */
    int knowledge_sweep;       /* 0=given, 1=partially-chosen, 2=chosen */
    int legitimacy_sweep;      /* 0=given, 1=partially-chosen, 2=chosen */
    bool is_normative;         /* true if normative boundary judgment (Ulrich) */
    double boundary_openness;  /* computed openness score for this config [0-1] */
} BoundaryConfig;
int cst_enumerate_boundaries(CSTSystem* sys, int max_configs, BoundaryConfig* out);
double cst_boundary_config_openness(const BoundaryConfig* cfg);

#endif /* CST_CORE_H */