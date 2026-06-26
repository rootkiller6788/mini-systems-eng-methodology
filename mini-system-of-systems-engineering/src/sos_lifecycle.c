#include "sos_lifecycle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

SoSLifecycle* sos_lifecycle_create(SystemOfSystems* sos, EvolutionPattern pattern) {
    if (!sos) return NULL;
    SoSLifecycle* lc = calloc(1, sizeof(SoSLifecycle));
    if (!lc) return NULL;
    lc->sos = sos; lc->pattern = pattern; lc->lifetime = 20.0;
    lc->growth_rate = 0.1; lc->decay_rate = 0.05;
    lc->is_maturing = true;
    return lc;
}

void sos_lifecycle_free(SoSLifecycle* lc) { if (lc) { free(lc->snapshots); free(lc); } }

void sos_lifecycle_snapshot(SoSLifecycle* lc) {
    if (!lc || !lc->sos) return;
    if (lc->n_snapshots >= lc->cap) {
        int nc = (lc->cap == 0) ? 64 : lc->cap * 2;
        LifecycleSnapshot* ns = realloc(lc->snapshots, (size_t)nc * sizeof(LifecycleSnapshot));
        if (!ns) return;
        lc->snapshots = ns; lc->cap = nc;
    }
    LifecycleSnapshot* s = &lc->snapshots[lc->n_snapshots];
    s->time = lc->current_time;
    s->n_systems = sos_n_constituents(lc->sos);
    s->readiness = sos_readiness(lc->sos);
    s->integration = lc->sos->integration_level;
    s->emergence = lc->sos->chars.emergence;
    s->phase = lc->sos->phase;
    lc->n_snapshots++;
}

void sos_lifecycle_advance(SoSLifecycle* lc, double dt) {
    if (!lc) return;
    lc->current_time += dt;
    if (lc->current_time > lc->lifetime * 0.3) {
        if (lc->sos->phase == SOS_PHASE_FORMING) lc->sos->phase = SOS_PHASE_DEVELOPING;
        else if (lc->sos->phase == SOS_PHASE_DEVELOPING && lc->current_time > lc->lifetime * 0.5)
            lc->sos->phase = SOS_PHASE_OPERATING;
    }
    lc->sos->integration_level = fmin(lc->sos->integration_level + dt * lc->growth_rate, 1.0);
    if (lc->sos->integration_level > 0.8) lc->is_maturing = true;
    sos_lifecycle_snapshot(lc);
}

void sos_lifecycle_add_system(SoSLifecycle* lc, ConstituentSystem* cs) {
    if (!lc || !cs) return;
    sos_add_constituent(lc->sos, cs); lc->n_systems_added++;
    sos_lifecycle_snapshot(lc);
}

int sos_lifecycle_remove_system(SoSLifecycle* lc, int idx) {
    if (!lc) return -1;
    int r = sos_remove_constituent(lc->sos, idx);
    if (r == 0) lc->n_systems_removed++;
    sos_lifecycle_snapshot(lc); return r;
}

void sos_lifecycle_change_architecture(SoSLifecycle* lc, SoSArchitectureType new_type) {
    if (!lc) return; lc->n_architecture_changes++; (void)new_type;
}

double sos_lifecycle_time_to_maturity(SoSLifecycle* lc) {
    if (!lc || lc->growth_rate <= 0) return lc->lifetime;
    return (1.0 - lc->sos->integration_level) / lc->growth_rate;
}

bool sos_lifecycle_is_sustainable(SoSLifecycle* lc) { return lc && lc->growth_rate > lc->decay_rate; }
double sos_lifecycle_growth_rate(const SoSLifecycle* lc) { return lc ? lc->growth_rate : 0.0; }

double sos_lifecycle_churn_rate(const SoSLifecycle* lc) {
    if (!lc || lc->current_time <= 0) return 0.0;
    return (double)(lc->n_systems_added + lc->n_systems_removed) / lc->current_time;
}

int sos_lifecycle_predict_size(const SoSLifecycle* lc, double future_time) {
    if (!lc) return 0;
    int n = sos_n_constituents(lc->sos);
    return (int)((double)n + (lc->growth_rate - lc->decay_rate) * future_time);
}

void sos_lifecycle_print(const SoSLifecycle* lc) {
    if (!lc) return;
    printf("=== SoS Lifecycle ===\n");
    printf("Time: %.1f Systems: %d Added: %d Removed: %d\n",
           lc->current_time, sos_n_constituents(lc->sos),
           lc->n_systems_added, lc->n_systems_removed);
}
/* Extended lifecycle simulation */
void sos_lifecycle_simulate(SoSLifecycle* lc, double duration, double dt) {
    if (!lc || duration <= 0 || dt <= 0) return;
    int steps = (int)(duration / dt);
    for (int i = 0; i < steps; i++) {
        sos_lifecycle_advance(lc, dt);
        if (lc->current_time > lc->lifetime) break;
    }
}

void sos_lifecycle_set_growth_rate(SoSLifecycle* lc, double rate) {
    if (lc && rate >= 0) lc->growth_rate = rate;
}

void sos_lifecycle_set_decay_rate(SoSLifecycle* lc, double rate) {
    if (lc && rate >= 0) lc->decay_rate = rate;
}

double sos_lifecycle_system_turnover(const SoSLifecycle* lc) {
    if (!lc || lc->lifetime <= 0) return 0.0;
    return (double)(lc->n_systems_added + lc->n_systems_removed) / lc->lifetime;
}

int sos_lifecycle_count_active_snapshots(const SoSLifecycle* lc) {
    if (!lc) return 0;
    int n = 0;
    for (int i = 0; i < lc->n_snapshots; i++)
        if (lc->snapshots[i].phase >= SOS_PHASE_OPERATING) n++;
    return n;
}

const LifecycleSnapshot* sos_lifecycle_get_snapshot(const SoSLifecycle* lc, int idx) {
    if (!lc || idx < 0 || idx >= lc->n_snapshots) return NULL;
    return &lc->snapshots[idx];
}

bool sos_lifecycle_has_reached_steady_state(const SoSLifecycle* lc, double tol) {
    if (!lc || lc->n_snapshots < 10) return false;
    int n = lc->n_snapshots;
    double recent_avg = 0, older_avg = 0;
    for (int i = n/2; i < n; i++) recent_avg += lc->snapshots[i].readiness;
    for (int i = 0; i < n/2; i++) older_avg += lc->snapshots[i].readiness;
    recent_avg /= (double)(n - n/2); older_avg /= (double)(n/2);
    return fabs(recent_avg - older_avg) < tol;
}

double sos_lifecycle_peak_readiness(const SoSLifecycle* lc) {
    if (!lc || lc->n_snapshots == 0) return 0.0;
    double peak = 0;
    for (int i = 0; i < lc->n_snapshots; i++)
        if (lc->snapshots[i].readiness > peak) peak = lc->snapshots[i].readiness;
    return peak;
}

double sos_lifecycle_average_integration(const SoSLifecycle* lc) {
    if (!lc || lc->n_snapshots == 0) return 0.0;
    double sum = 0;
    for (int i = 0; i < lc->n_snapshots; i++)
        sum += lc->snapshots[i].integration;
    return sum / (double)lc->n_snapshots;
}