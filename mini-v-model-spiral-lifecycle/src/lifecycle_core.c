#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "lifecycle_core.h"

/* ============================================================================
 * Lifecycle Core ? Shared types, requirement & deliverable management
 * ============================================================================ */

/* --- Name tables --- */

static const char* stage_names[] = {
    "Pre-Concept", "Concept", "Preliminary Design", "Detailed Design",
    "Production", "Operations", "Disposal"
};

static const char* vlevel_names[] = {
    "Concept", "Requirements", "Architecture", "Design", "Implementation"
};

static const char* vverify_names[] = {
    "Unit Test", "Integration Test", "System Verification",
    "Acceptance Test", "Operational Test"
};

static const char* spiral_phase_names[] = {
    "Objectives", "Risk Analysis", "Development", "Planning"
};

static const char* model_names[] = {
    "V-Model", "Spiral", "Hybrid", "Agile", "Waterfall", "Incremental"
};

/* --- LifecycleSystem --- */

LifecycleSystem* lc_create(const char* name, LifecycleModelType model) {
    LifecycleSystem* lc = calloc(1, sizeof(LifecycleSystem));
    if (!lc) return NULL;
    lc->name = strdup(name ? name : "unnamed");
    lc->model_type = model;
    lc->current_stage = STAGE_PRE_CONCEPT;
    lc->overall_progress = 0.0;
    lc->total_budget = 0.0;
    lc->spent_budget = 0.0;
    lc->total_schedule_days = 365.0;
    lc->elapsed_days = 0.0;
    lc->requirements = NULL;
    lc->n_requirements = 0;
    lc->req_capacity = 0;
    lc->risks = NULL;
    lc->n_risks = 0;
    lc->risk_capacity = 0;
    lc->deliverables = NULL;
    lc->n_deliverables = 0;
    lc->deliv_capacity = 0;
    lc->total_risk_exposure = 0.0;
    lc->residual_risk_exposure = 0.0;
    lc->overall_risk_level = RISK_NEGLIGIBLE;
    lc->verification_coverage = 0.0;
    lc->validation_coverage = 0.0;
    lc->n_defects_found = 0;
    lc->n_defects_resolved = 0;
    return lc;
}

void lc_cleanup(LifecycleSystem* lc) {
    if (!lc) return;
    free(lc->name); lc->name = NULL;
    if (lc->requirements) {
        for (int i = 0; i < lc->n_requirements; i++) {
            free(lc->requirements[i].id);
            free(lc->requirements[i].description);
            free(lc->requirements[i].source);
            free(lc->requirements[i].verification_method);
            for (int j = 0; j < lc->requirements[i].n_traces_to; j++)
                free(lc->requirements[i].traces_to[j]);
            free(lc->requirements[i].traces_to);
            for (int j = 0; j < lc->requirements[i].n_traces_from; j++)
                free(lc->requirements[i].traces_from[j]);
            free(lc->requirements[i].traces_from);
        }
        free(lc->requirements); lc->requirements = NULL;
    }
    if (lc->risks) {
        for (int i = 0; i < lc->n_risks; i++) {
            free(lc->risks[i].id);
            free(lc->risks[i].description);
            free(lc->risks[i].mitigation);
            free(lc->risks[i].trigger);
        }
        free(lc->risks); lc->risks = NULL;
    }
    if (lc->deliverables) {
        for (int i = 0; i < lc->n_deliverables; i++) {
            free(lc->deliverables[i].name);
            free(lc->deliverables[i].description);
            free(lc->deliverables[i].format);
            for (int j = 0; j < lc->deliverables[i].n_reviewers; j++)
                free(lc->deliverables[i].reviewed_by[j]);
            free(lc->deliverables[i].reviewed_by);
        }
        free(lc->deliverables); lc->deliverables = NULL;
    }
    lc->n_requirements = 0; lc->req_capacity = 0;
    lc->n_risks = 0; lc->risk_capacity = 0;
    lc->n_deliverables = 0; lc->deliv_capacity = 0;
}

void lc_free(LifecycleSystem* lc) {
    if (!lc) return;
    lc_cleanup(lc);
    free(lc);
}

/* --- Requirement management --- */

TraceableRequirement* lc_add_requirement(LifecycleSystem* lc, const char* id,
    const char* desc, const char* source, VModelLevel level,
    VModelVerifyLevel verify_at, double priority) {
    if (!lc || !id) return NULL;
    if (lc->n_requirements >= lc->req_capacity) {
        lc->req_capacity = lc->req_capacity ? lc->req_capacity * 2 : 16;
        lc->requirements = realloc(lc->requirements,
            (size_t)lc->req_capacity * sizeof(TraceableRequirement));
    }
    TraceableRequirement* r = &lc->requirements[lc->n_requirements];
    memset(r, 0, sizeof(TraceableRequirement));
    r->id = strdup(id);
    r->description = strdup(desc ? desc : "");
    r->source = strdup(source ? source : "");
    r->allocated_to = level;
    r->verify_at = verify_at;
    r->priority = priority;
    r->is_verified = false;
    r->is_validated = false;
    r->traces_to = NULL;
    r->n_traces_to = 0;
    r->traces_from = NULL;
    r->n_traces_from = 0;
    r->verification_method = strdup("TBD");
    lc->n_requirements++;
    return r;
}

void lc_add_trace(LifecycleSystem* lc, const char* from_id, const char* to_id) {
    if (!lc || !from_id || !to_id) return;
    TraceableRequirement *from = NULL, *to = NULL;
    for (int i = 0; i < lc->n_requirements; i++) {
        if (strcmp(lc->requirements[i].id, from_id) == 0) from = &lc->requirements[i];
        if (strcmp(lc->requirements[i].id, to_id) == 0) to = &lc->requirements[i];
    }
    if (!from || !to) return;
    /* Forward trace from -> to */
    int n = from->n_traces_to;
    from->traces_to = realloc(from->traces_to, (size_t)(n + 1) * sizeof(char*));
    from->traces_to[n] = strdup(to_id);
    from->n_traces_to = n + 1;
    /* Backward trace to -> from */
    n = to->n_traces_from;
    to->traces_from = realloc(to->traces_from, (size_t)(n + 1) * sizeof(char*));
    to->traces_from[n] = strdup(from_id);
    to->n_traces_from = n + 1;
}

void lc_verify_requirement(LifecycleSystem* lc, const char* id, bool status) {
    if (!lc || !id) return;
    for (int i = 0; i < lc->n_requirements; i++) {
        if (strcmp(lc->requirements[i].id, id) == 0) {
            lc->requirements[i].is_verified = status;
            return;
        }
    }
}

void lc_validate_requirement(LifecycleSystem* lc, const char* id, bool status) {
    if (!lc || !id) return;
    for (int i = 0; i < lc->n_requirements; i++) {
        if (strcmp(lc->requirements[i].id, id) == 0) {
            lc->requirements[i].is_validated = status;
            return;
        }
    }
}

double lc_requirement_verification_coverage(LifecycleSystem* lc) {
    if (!lc || lc->n_requirements == 0) return 1.0;
    int verified = 0;
    for (int i = 0; i < lc->n_requirements; i++)
        if (lc->requirements[i].is_verified) verified++;
    lc->verification_coverage = (double)verified / lc->n_requirements;
    return lc->verification_coverage;
}

/* --- Deliverable management --- */

Deliverable* lc_add_deliverable(LifecycleSystem* lc, const char* name,
    const char* desc, LifecycleStage stage, const char* format) {
    if (!lc || !name) return NULL;
    if (lc->n_deliverables >= lc->deliv_capacity) {
        lc->deliv_capacity = lc->deliv_capacity ? lc->deliv_capacity * 2 : 16;
        lc->deliverables = realloc(lc->deliverables,
            (size_t)lc->deliv_capacity * sizeof(Deliverable));
    }
    Deliverable* d = &lc->deliverables[lc->n_deliverables];
    memset(d, 0, sizeof(Deliverable));
    d->name = strdup(name);
    d->description = strdup(desc ? desc : "");
    d->produced_in = stage;
    d->format = strdup(format ? format : "Document");
    d->is_delivered = false;
    d->is_approved = false;
    d->completion_pct = 0.0;
    d->reviewed_by = NULL;
    d->n_reviewers = 0;
    lc->n_deliverables++;
    return d;
}

void lc_approve_deliverable(LifecycleSystem* lc, const char* name) {
    if (!lc || !name) return;
    for (int i = 0; i < lc->n_deliverables; i++) {
        if (strcmp(lc->deliverables[i].name, name) == 0) {
            lc->deliverables[i].is_approved = true;
            lc->deliverables[i].is_delivered = true;
            lc->deliverables[i].completion_pct = 1.0;
            return;
        }
    }
}

double lc_deliverable_completion(LifecycleSystem* lc, LifecycleStage stage) {
    if (!lc) return 0.0;
    int total = 0, complete = 0;
    for (int i = 0; i < lc->n_deliverables; i++) {
        if (lc->deliverables[i].produced_in == stage) {
            total++;
            if (lc->deliverables[i].is_approved) complete++;
        }
    }
    return total > 0 ? (double)complete / total : 0.0;
}

/* --- Status / Names --- */

const char* lc_stage_name(LifecycleStage stage) {
    if (stage < 0 || stage >= STAGE_NUM) return "Unknown";
    return stage_names[stage];
}

const char* lc_vlevel_name(VModelLevel level) {
    if (level < 0 || level >= V_NUM_LEVELS) return "Unknown";
    return vlevel_names[level];
}

const char* lc_vverify_name(VModelVerifyLevel level) {
    if (level < 0 || level >= V_NUM_VERIFY) return "Unknown";
    return vverify_names[level];
}

const char* lc_spiral_phase_name(SpiralPhase phase) {
    if (phase < 0 || phase >= SPIRAL_NUM_PHASES) return "Unknown";
    return spiral_phase_names[phase];
}

const char* lc_model_name(LifecycleModelType model) {
    if (model < 0 || model > 5) return "Unknown";
    return model_names[model];
}

double lc_earned_value(LifecycleSystem* lc, double planned_value) {
    if (!lc) return 0.0;
    return planned_value * lc->overall_progress;
}

void lc_print_status(LifecycleSystem* lc) {
    if (!lc) { printf("LifecycleSystem: NULL\n"); return; }
    printf("=== Lifecycle: %s ===\n", lc->name);
    printf("  Model: %s  Stage: %s\n",
           lc_model_name(lc->model_type), lc_stage_name(lc->current_stage));
    printf("  Progress: %.1f%% (%s)\n",
           100.0 * lc->overall_progress,
           (lc->overall_progress > 0.8) ? "On Track" :
           (lc->overall_progress > 0.5) ? "Caution" : "At Risk");
    printf("  Budget: %.0f / %.0f (%.1f%% spent)\n",
           lc->spent_budget, lc->total_budget,
           lc->total_budget > 0 ? 100.0 * lc->spent_budget / lc->total_budget : 0.0);
    printf("  Schedule: %.0f / %.0f days\n",
           lc->elapsed_days, lc->total_schedule_days);
    printf("  Requirements: %d  Verified: %.1f%%  Validated: %.1f%%\n",
           lc->n_requirements,
           100.0 * lc->verification_coverage,
           100.0 * lc->validation_coverage);
    printf("  Risks: %d  Exposure: %.2f\n",
           lc->n_risks, lc->total_risk_exposure);
    printf("  Deliverables: %d  Defects: %d found, %d resolved\n",
           lc->n_deliverables, lc->n_defects_found, lc->n_defects_resolved);
}

/* ====================================================================
 * Extended Lifecycle Management Functions
 * ==================================================================== */

/* Calculate the critical path length based on deliverables */
double lc_critical_path_days(LifecycleSystem* lc) {
    if (!lc || lc->n_deliverables == 0) return 0.0;
    double max_days = 0.0;
    for (int i = 0; i < lc->n_deliverables; i++) {
        if (lc->deliverables[i].completion_pct < 1.0) {
            double remaining = (1.0 - lc->deliverables[i].completion_pct) * 30.0;
            if (remaining > max_days) max_days = remaining;
        }
    }
    return max_days;
}

/* Calculate budget burn rate (spent per day) */
double lc_burn_rate(LifecycleSystem* lc) {
    if (!lc || lc->elapsed_days < 1e-10) return 0.0;
    return lc->spent_budget / lc->elapsed_days;
}

/* Estimate completion date based on current burn rate */
double lc_estimate_completion_days(LifecycleSystem* lc) {
    if (!lc || lc->total_budget < 1e-10) return lc->total_schedule_days;
    double burn_rate = lc_burn_rate(lc);
    if (burn_rate < 1e-10) return lc->total_schedule_days * 2;
    double remaining_budget = lc->total_budget - lc->spent_budget;
    return lc->elapsed_days + remaining_budget / burn_rate;
}

/* Requirement volatility index: how many requirements changed */
double lc_requirement_volatility(LifecycleSystem* lc, int n_changes) {
    if (!lc || lc->n_requirements == 0) return 0.0;
    return (double)n_changes / lc->n_requirements;
}

/* Deliverable approval rate */
double lc_deliverable_approval_rate(LifecycleSystem* lc) {
    if (!lc || lc->n_deliverables == 0) return 0.0;
    int approved = 0;
    for (int i = 0; i < lc->n_deliverables; i++)
        if (lc->deliverables[i].is_approved) approved++;
    return (double)approved / lc->n_deliverables;
}

/* Overall lifecycle health: composite metric (0-1) */
double lc_health_index(LifecycleSystem* lc) {
    if (!lc) return 0.0;
    double score = 1.0;
    if (lc->elapsed_days > 0 && lc->total_schedule_days > 0) {
        double schedule_ratio = lc->elapsed_days / lc->total_schedule_days;
        double progress_ratio = lc->overall_progress;
        if (schedule_ratio > progress_ratio + 0.1) score -= 0.3;
    }
    if (lc->total_budget > 0 && lc->spent_budget > lc->total_budget * 0.9 && lc->overall_progress < 0.8) score -= 0.3;
    if (lc->total_risk_exposure > 0.5) score -= 0.2;
    if (lc->verification_coverage < 0.5) score -= 0.2;
    if (score < 0.0) score = 0.0;
    return score;
}

/* Count deliverables by stage */
int lc_deliverables_in_stage(LifecycleSystem* lc, LifecycleStage stage) {
    if (!lc) return 0;
    int count = 0;
    for (int i = 0; i < lc->n_deliverables; i++)
        if (lc->deliverables[i].produced_in == stage) count++;
    return count;
}

/* Find next incomplete stage */
LifecycleStage lc_next_incomplete_stage(LifecycleSystem* lc) {
    if (!lc) return STAGE_PRE_CONCEPT;
    for (int s = STAGE_PRE_CONCEPT; s < STAGE_NUM; s++) {
        if (lc_deliverable_completion(lc, (LifecycleStage)s) < 1.0) return (LifecycleStage)s;
    }
    return STAGE_DISPOSAL;
}

/* Estimate total lifecycle cost using stage percentages */
double lc_estimate_total_cost(LifecycleSystem* lc, double stage_costs[STAGE_NUM]) {
    if (!lc || !stage_costs) return 0.0;
    double total = 0.0;
    for (int s = 0; s < STAGE_NUM; s++) total += stage_costs[s];
    return total;
}

double lc_estimate_cost_to_complete(LifecycleSystem* lc) {
    if(!lc||lc->overall_progress<0.01)return lc->total_budget;
    return lc->total_budget*(1.0-lc->overall_progress);
}
double lc_schedule_performance_index(LifecycleSystem* lc){
    if(!lc||lc->total_schedule_days<1e-10)return 1.0;
    double planned=lc->elapsed_days/lc->total_schedule_days;
    return(planned>1e-10)?lc->overall_progress/planned:1.0;
}
double lc_cost_performance_index(LifecycleSystem* lc){
    if(!lc||lc->spent_budget<1e-10)return 1.0;
    double planned_spend=lc->overall_progress*lc->total_budget;
    return(planned_spend>1e-10)?planned_spend/lc->spent_budget:1.0;
}
int lc_requirements_at_level(LifecycleSystem* lc,VModelLevel level){
    if(!lc)return 0;int c=0;
    for(int i=0;i<lc->n_requirements;i++)if(lc->requirements[i].allocated_to==level)c++;
    return c;
}
int lc_requirements_verified_count(LifecycleSystem* lc){
    if(!lc)return 0;int c=0;
    for(int i=0;i<lc->n_requirements;i++)if(lc->requirements[i].is_verified)c++;
    return c;
}
double lc_technical_debt_days(LifecycleSystem* lc,double days_per_defect){
    if(!lc)return 0;int open=lc->n_defects_found-lc->n_defects_resolved;
    if(open<0)open=0;return open*days_per_defect;
}
double lc_risk_item_density(LifecycleSystem* lc){return(lc&&lc->n_requirements>0)?(double)lc->n_risks/lc->n_requirements:0;}
bool lc_is_late(LifecycleSystem* lc){return lc&&lc->elapsed_days>lc->total_schedule_days*lc->overall_progress*1.1;}
bool lc_is_over_budget(LifecycleSystem* lc){return lc&&lc->spent_budget>lc->total_budget*lc->overall_progress*1.1;}
int lc_total_action_items_open(LifecycleSystem* lc){(void)lc;return 0;}
double lc_risk_burndown_pct(LifecycleSystem* lc){return(lc&&lc->n_risks>0)?(double)lc->n_risks/lc->n_risks:1.0;}


































































































/* ── Lifecycle Cost Estimation: COCOMO II adaptation ──────────────
 * Boehm et al. (2000): Effort = A * (Size)^B * ∏(EM_i).
 * Simplified for systems engineering lifecycle cost. */
double lc_cocomo_effort(double size_kloc, double scale_factor, double* effort_multipliers,
                        int n_em) {
    if (size_kloc <= 0 || !effort_multipliers || n_em <= 0) return 0.0;
    double product = 1.0;
    for (int i = 0; i < n_em; i++) product *= effort_multipliers[i];
    return 2.94 * pow(size_kloc, scale_factor) * product;
}

/* ── Lifecycle Earned Value Management (EVM) ──────────────────────
 * ANSI/EIA-748: EVM indices for project health.
 * SPI = EV/PV, CPI = EV/AC.
 * SPI < 1 → behind schedule; CPI < 1 → over budget. */
void lc_evm_metrics(double planned_value, double earned_value, double actual_cost,
                    double* spi, double* cpi) {
    if (!spi || !cpi) return;
    *spi = (planned_value > 1e-12) ? earned_value / planned_value : 1.0;
    *cpi = (actual_cost > 1e-12) ? earned_value / actual_cost : 1.0;
}

/* ── Lifecycle Defect Removal Efficiency (DRE) ────────────────────
 * Jones (2000): DRE = defects_removed / (defects_removed + defects_escaped).
 * Measures effectiveness of V&V activities. */
double lc_defect_removal_efficiency(int defects_found, int defects_escaped) {
    int total = defects_found + defects_escaped;
    return (total > 0) ? (double)defects_found / (double)total : 0.0;
}

/* ── Lifecycle Risk Exposure (Boehm, 1991) ────────────────────────
 * RE = probability * loss_magnitude.
 * Top N risks by exposure drive spiral cycles. */
typedef struct { char* description; double probability; double loss_magnitude; double exposure; } LCRiskItem;
double lc_risk_exposure(double probability, double loss_magnitude) {
    return probability * loss_magnitude;
}
