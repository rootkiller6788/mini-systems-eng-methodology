#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "lifecycle_core.h"
#include "lifecycle_gates.h"

/* Stage Gates & Technical Reviews */

static const char* review_names[] = {"SRR","SDR","PDR","CDR","TRR","PRR","SAR","ORR","PIR","DRR"};
static const char* finding_names[] = {"None","Observation","Minor","Major","Critical","Showstopper"};

GateReviewSystem* gates_create(const char* name) {
    GateReviewSystem* grs = calloc(1, sizeof(GateReviewSystem));
    if (!grs) return NULL;
    { LifecycleSystem* _t = lc_create(name, LC_MODEL_V); grs->base = *_t; free(_t); }
    return grs;
}

void gates_free(GateReviewSystem* grs) {
    if (!grs) return;
    for (int i = 0; i < REVIEW_NUM; i++) {
        if (grs->review_configured[i]) {
            TechnicalReview* rev = &grs->reviews[i];
            for (int j = 0; j < rev->n_entry_criteria; j++) free(rev->entry_criteria[j]);
            free(rev->entry_criteria);
            for (int j = 0; j < rev->n_actions; j++) {
                free(rev->action_items[j].description);
                free(rev->action_items[j].assigned_to);
                free(rev->action_items[j].resolution);
            }
            free(rev->action_items);
            free(rev->name);
            free(rev->decision_rationale);
        }
    }
    for (int i = 0; i < STAGE_NUM; i++) {
        if (grs->gate_configured[i]) {
            for (int j = 0; j < grs->gates[i].n_deliverables_required; j++)
                free(grs->gates[i].deliverables_required[j]);
            free(grs->gates[i].deliverables_required);
        }
    }
    lc_cleanup(&grs->base);
    free(grs);
}

TechnicalReview* gates_configure_review(GateReviewSystem* grs, ReviewType type, LifecycleStage stage) {
    if (!grs || type < 0 || type >= REVIEW_NUM) return NULL;
    TechnicalReview* rev = &grs->reviews[type];
    rev->type = type;
    rev->name = strdup(review_names[type]);
    rev->entry_stage = stage;
    rev->entry_criteria = NULL;
    rev->n_entry_criteria = 0;
    rev->action_capacity = 16;
    rev->action_items = calloc(16, sizeof(ActionItem));
    rev->n_actions = 0;
    rev->n_attendees = 0;
    rev->is_complete = false;
    rev->is_passed = false;
    rev->decision = GATE_NO_GO;
    rev->decision_rationale = strdup("");
    rev->review_date = 0.0;
    grs->review_configured[type] = true;
    return rev;
}

void gates_add_entry_criterion(TechnicalReview* rev, const char* criterion) {
    if (!rev || !criterion) return;
    int n = rev->n_entry_criteria;
    rev->entry_criteria = realloc(rev->entry_criteria, (size_t)(n + 1) * sizeof(char*));
    rev->entry_criteria[n] = strdup(criterion);
    rev->n_entry_criteria = n + 1;
}

void gates_add_action_item(TechnicalReview* rev, const char* desc,
    ReviewFinding severity, const char* assignee, double due_days) {
    if (!rev || !desc) return;
    if (rev->n_actions >= rev->action_capacity) {
        rev->action_capacity *= 2;
        rev->action_items = realloc(rev->action_items,
            (size_t)rev->action_capacity * sizeof(ActionItem));
    }
    ActionItem* ai = &rev->action_items[rev->n_actions];
    ai->description = strdup(desc);
    ai->severity = severity;
    ai->assigned_to = strdup(assignee ? assignee : "");
    ai->is_resolved = false;
    ai->resolution = strdup("");
    ai->due_days = due_days;
    rev->n_actions++;
}

void gates_resolve_action_item(TechnicalReview* rev, int idx, const char* resolution) {
    if (!rev || idx < 0 || idx >= rev->n_actions) return;
    rev->action_items[idx].is_resolved = true;
    free(rev->action_items[idx].resolution);
    rev->action_items[idx].resolution = strdup(resolution ? resolution : "");
}

void gates_conduct_review(GateReviewSystem* grs, ReviewType type,
    GateDecision decision, const char* rationale) {
    if (!grs || type < 0 || type >= REVIEW_NUM || !grs->review_configured[type]) return;
    TechnicalReview* rev = &grs->reviews[type];
    rev->decision = decision;
    free(rev->decision_rationale);
    rev->decision_rationale = strdup(rationale ? rationale : "");
    rev->is_complete = true;
    rev->is_passed = (decision == GATE_GO || decision == GATE_CONDITIONAL);
    rev->review_date = grs->base.elapsed_days;
    grs->n_reviews_completed++;
    if (rev->is_passed) grs->n_reviews_passed++;
}

bool gates_review_passed(GateReviewSystem* grs, ReviewType type) {
    return grs && type >= 0 && type < REVIEW_NUM && grs->review_configured[type]
           && grs->reviews[type].is_passed;
}

void gates_configure_gate(GateReviewSystem* grs, LifecycleStage from, LifecycleStage to) {
    if (!grs || from < 0 || from >= STAGE_NUM) return;
    StageGate* sg = &grs->gates[from];
    sg->from_stage = from;
    sg->to_stage = to;
    sg->is_passed = false;
    sg->decision = GATE_NO_GO;
    sg->deliverables_required = NULL;
    sg->n_deliverables_required = 0;
    sg->n_deliverables_submitted = 0;
    sg->gate_date = 30.0;
    sg->actual_date = 0.0;
    grs->gate_configured[from] = true;
}

void gates_pass_gate(GateReviewSystem* grs, LifecycleStage from) {
    if (!grs || from < 0 || from >= STAGE_NUM || !grs->gate_configured[from]) return;
    grs->gates[from].is_passed = true;
    grs->gates[from].decision = GATE_GO;
    grs->gates[from].actual_date = grs->base.elapsed_days;
    grs->n_gates_passed++;
    grs->base.current_stage = grs->gates[from].to_stage;
}

GateDecision gates_get_decision(GateReviewSystem* grs, LifecycleStage from_stage) {
    if (!grs || from_stage < 0 || from_stage >= STAGE_NUM || !grs->gate_configured[from_stage])
        return GATE_NO_GO;
    return grs->gates[from_stage].decision;
}

int gates_open_actions(GateReviewSystem* grs) {
    if (!grs) return 0;
    int count = 0;
    for (int i = 0; i < REVIEW_NUM; i++) {
        if (!grs->review_configured[i]) continue;
        for (int j = 0; j < grs->reviews[i].n_actions; j++)
            if (!grs->reviews[i].action_items[j].is_resolved) count++;
    }
    return count;
}

int gates_critical_actions(GateReviewSystem* grs) {
    if (!grs) return 0;
    int count = 0;
    for (int i = 0; i < REVIEW_NUM; i++) {
        if (!grs->review_configured[i]) continue;
        for (int j = 0; j < grs->reviews[i].n_actions; j++)
            if (!grs->reviews[i].action_items[j].is_resolved &&
                grs->reviews[i].action_items[j].severity >= FINDING_CRITICAL)
                count++;
    }
    return count;
}

bool gates_can_proceed(GateReviewSystem* grs, LifecycleStage to_stage) {
    (void)to_stage;
    return grs ? gates_critical_actions(grs) == 0 : false;
}

double gates_review_completion(GateReviewSystem* grs) {
    if (!grs) return 0.0;
    int configured = 0, passed = 0;
    for (int i = 0; i < REVIEW_NUM; i++) {
        if (grs->review_configured[i]) { configured++; if (grs->reviews[i].is_passed) passed++; }
    }
    return configured > 0 ? (double)passed / configured : 0.0;
}

const char* gates_review_name(ReviewType type) {
    return (type >= 0 && type < REVIEW_NUM) ? review_names[type] : "Unknown";
}

const char* gates_finding_name(ReviewFinding finding) {
    return (finding >= 0 && finding <= 5) ? finding_names[finding] : "Unknown";
}

void gates_print(GateReviewSystem* grs) {
    if (!grs) { printf("GateReviewSystem: NULL\n"); return; }
    printf("=== Gate & Review System: %s ===\n", grs->base.name);
    printf("  Reviews: %d completed, %d passed (%.0f%%)\n",
           grs->n_reviews_completed, grs->n_reviews_passed,
           100.0 * gates_review_completion(grs));
    printf("  Gates: %d passed, %d failed\n",
           grs->n_gates_passed, grs->n_gates_failed);
    printf("  Open Actions: %d (%d critical/showstopper)\n",
           gates_open_actions(grs), gates_critical_actions(grs));
    printf("  Current Stage: %s  Can Proceed: %s\n",
           lc_stage_name(grs->base.current_stage),
           gates_can_proceed(grs, grs->base.current_stage) ? "YES" : "NO");
    for (int i = 0; i < REVIEW_NUM; i++) {
        if (!grs->review_configured[i]) continue;
        printf("  %s [%s]: %s\n", review_names[i],
               lc_stage_name(grs->reviews[i].entry_stage),
               grs->reviews[i].is_passed ? "PASSED" : "PENDING");
    }
}

void gates_print_review(TechnicalReview* rev) {
    if (!rev) { printf("TechnicalReview: NULL\n"); return; }
    printf("=== Review: %s ===\n", rev->name);
    printf("  Stage: %s  Attendees: %d\n",
           lc_stage_name(rev->entry_stage), rev->n_attendees);
    printf("  Decision: %d  Passed: %s\n", rev->decision,
           rev->is_passed ? "YES" : "NO");
    printf("  Entry Criteria (%d):\n", rev->n_entry_criteria);
    for (int i = 0; i < rev->n_entry_criteria; i++)
        printf("    - %s\n", rev->entry_criteria[i]);
    printf("  Action Items (%d):\n", rev->n_actions);
    for (int i = 0; i < rev->n_actions && i < 10; i++)
        printf("    %d. [%s] %s -> %s %s\n", i + 1,
               gates_finding_name(rev->action_items[i].severity),
               rev->action_items[i].description,
               rev->action_items[i].assigned_to,
               rev->action_items[i].is_resolved ? "?" : "?");
}

/* Extended gate and review functions */
int gates_total_action_items(GateReviewSystem* grs) {
    if (!grs) return 0;
    int total = 0;
    for (int i = 0; i < REVIEW_NUM; i++)
        if (grs->review_configured[i]) total += grs->reviews[i].n_actions;
    return total;
}

int gates_resolved_action_items(GateReviewSystem* grs) {
    if (!grs) return 0;
    int total = 0;
    for (int i = 0; i < REVIEW_NUM; i++) {
        if (!grs->review_configured[i]) continue;
        for (int j = 0; j < grs->reviews[i].n_actions; j++)
            if (grs->reviews[i].action_items[j].is_resolved) total++;
    }
    return total;
}

double gates_action_resolution_rate(GateReviewSystem* grs) {
    int total = gates_total_action_items(grs);
    if (total == 0) return 1.0;
    return (double)gates_resolved_action_items(grs) / total;
}

double gates_average_review_score(GateReviewSystem* grs) {
    if (!grs) return 0.0;
    int passed = 0, total = 0;
    for (int i = 0; i < REVIEW_NUM; i++) {
        if (grs->review_configured[i]) { total++; if (grs->reviews[i].is_passed) passed++; }
    }
    return total > 0 ? (double)passed / total : 0.0;
}

int gates_next_review_index(GateReviewSystem* grs) {
    if (!grs) return -1;
    for (int i = 0; i < REVIEW_NUM; i++)
        if (grs->review_configured[i] && !grs->reviews[i].is_complete) return i;
    return -1;
}

bool gates_is_review_overdue(GateReviewSystem* grs, int review_idx, double current_day) {
    if (!grs || review_idx < 0 || review_idx >= REVIEW_NUM || !grs->review_configured[review_idx]) return false;
    return current_day > grs->reviews[review_idx].review_date && !grs->reviews[review_idx].is_complete;
}

double gates_gate_latency(GateReviewSystem* grs, LifecycleStage stage) {
    if (!grs || stage < 0 || stage >= STAGE_NUM || !grs->gate_configured[stage]) return 0.0;
    if (grs->gates[stage].actual_date < 1e-10) return 0.0;
    return grs->gates[stage].actual_date - grs->gates[stage].gate_date;
}

