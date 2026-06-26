#ifndef SSM_CORE_H
#define SSM_CORE_H

#include "ssm_types.h"
#include "ssm_catwoe.h"
#include "ssm_rootdef.h"
#include "ssm_conceptual.h"
#include "ssm_comparison.h"
#include "ssm_streams.h"
#include "ssm_intervention.h"
#include "ssm_eval.h"

/* ── Mathematical Constants ────────────────────── */
#define SSM_PI          3.14159265358979323846
#define SSM_E           2.71828182845904523536
#define SSM_EPSILON     1e-12

/* ── SSM Process Constants ─────────────────────── */
#define SSM_MAX_ITERATIONS        10
#define SSM_DEFAULT_STAKEHOLDERS  20
#define SSM_DEFAULT_ACTIVITIES    50
#define SSM_MAX_CYCLE_DEPTH       5
#define SSM_MIN_ACCOMMODATION      0.60

/* ── CATWOE Constants ──────────────────────────── */
#define SSM_MAX_CUSTOMERS         15
#define SSM_MAX_ACTORS            10
#define SSM_MAX_OWNERS            8
#define SSM_MAX_CONSTRAINTS       20
#define SSM_DEFAULT_MEASURABILITY  0.5

/* ── Conceptual Model Constants ────────────────── */
#define SSM_MAX_ACTIVITY_LEVEL     5
#define SSM_MAX_DEPENDENCIES       10
#define SSM_DEFAULT_MONITORING      1.0
#define SSM_MIN_LOGICAL_COVERAGE    0.80

/* ── Comparison Constants ──────────────────────── */
#define SSM_GAP_SIGNIFICANT         0.40
#define SSM_GAP_CRITICAL            0.70
#define SSM_ALIGNMENT_ADEQUATE      0.75

/* ── 5E Criteria Defaults ──────────────────────── */
#define SSM_EFFICACY_MIN           0.50
#define SSM_EFFICIENCY_MIN         0.40
#define SSM_EFFECTIVENESS_MIN      0.60
#define SSM_ETHICALITY_MIN         0.70
#define SSM_ELEGANCE_MIN           0.30

/* ── Accommodation Heuristics ──────────────────── */
#define SSM_ACCOMMODATION_THRESHOLD  0.70
#define SSM_CONFLICT_TOLERABLE       0.30

#endif /* SSM_CORE_H */
