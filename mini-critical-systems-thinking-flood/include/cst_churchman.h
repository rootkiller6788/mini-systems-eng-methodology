#ifndef CST_CHURCHMAN_H
#define CST_CHURCHMAN_H
#include "cst_core.h"

/* Churchman's Enemies of the Systems Approach (1979)
 * C. West Churchman identified 4 main "enemies" that prevent
 * systems thinking from being truly holistic:
 * 1. Politics — power dynamics exclude certain perspectives
 * 2. Morality — ethical judgments are suppressed in "objective" analysis
 * 3. Religion — ultimate concerns are dismissed as irrational
 * 4. Aesthetics — beauty, harmony, and quality are ignored
 *
 * Plus 5 secondary enemies:
 * 5. Efficiency — optimizing parts destroys the whole
 * 6. Rationality — bounded rationality limits understanding
 * 7. Expertise — expert knowledge crowds out local wisdom
 * 8. Planning — plans become more real than the reality they describe
 * 9. Models — models replace the system they model */

typedef enum {
    CST_ENEMY_POLITICS = 0, CST_ENEMY_MORALITY = 1,
    CST_ENEMY_RELIGION = 2, CST_ENEMY_AESTHETICS = 3,
    CST_ENEMY_EFFICIENCY = 4, CST_ENEMY_RATIONALITY = 5,
    CST_ENEMY_EXPERTISE = 6, CST_ENEMY_PLANNING = 7,
    CST_ENEMY_MODELS = 8
} CSTEnemyType;

typedef struct {
    CSTEnemyType type;
    char* name; char* description; char* warning_sign;
    double  threat_level;    /* 0-1: how severely this enemy threatens the project */
    bool    is_active;       /* Whether this enemy is currently manifesting */
    char**  countermeasures;
    int     n_countermeasures;
} CSTEnemy;

typedef struct {
    CSTSystem* sys;
    CSTEnemy*  enemies;
    int        n_enemies;
    double     overall_threat;     /* Average threat across all enemies */
    double     defense_readiness;  /* System's preparedness against enemies */
    int        n_active_threats;
    bool       is_vulnerable;      /* True if any threat > 0.7 */
} CSTChurchmanAnalysis;

CSTChurchmanAnalysis* cst_churchman_create(CSTSystem* sys);
void cst_churchman_free(CSTChurchmanAnalysis* ca);
void cst_churchman_identify_all(CSTChurchmanAnalysis* ca);
void cst_churchman_assess_threats(CSTChurchmanAnalysis* ca);
void cst_churchman_recommend_defenses(CSTChurchmanAnalysis* ca);
const char* cst_enemy_name(CSTEnemyType e);
const char* cst_enemy_description(CSTEnemyType e);
const char* cst_enemy_warning(CSTEnemyType e);
double cst_churchman_vulnerability_index(const CSTChurchmanAnalysis* ca);
bool cst_churchman_needs_defense(const CSTChurchmanAnalysis* ca, CSTEnemyType e);
void cst_churchman_print(const CSTChurchmanAnalysis* ca);
double cst_churchman_defense_score(const CSTChurchmanAnalysis* ca);
bool cst_churchman_is_well_defended(const CSTChurchmanAnalysis* ca);
int cst_churchman_count_active_enemies(const CSTChurchmanAnalysis* ca);
const char* cst_churchman_most_dangerous(const CSTChurchmanAnalysis* ca);
void cst_churchman_report_full(const CSTChurchmanAnalysis* ca);
void cst_churchman_dialectical_sweep(CSTChurchmanAnalysis* ca);
double cst_churchman_sweep_impact(const CSTChurchmanAnalysis* ca);
bool cst_churchman_boundary_shifted(const CSTChurchmanAnalysis* ca);
void cst_churchman_genealogy_of_boundaries(const CSTChurchmanAnalysis* ca);
void cst_churchman_deploy_defense(CSTChurchmanAnalysis* ca, CSTEnemyType e);
void cst_churchman_deploy_all_defenses(CSTChurchmanAnalysis* ca);
double cst_churchman_systemic_health(const CSTChurchmanAnalysis* ca);
bool cst_churchman_crisis_mode(const CSTChurchmanAnalysis* ca);
int cst_churchman_priority_enemies(const CSTChurchmanAnalysis* ca, CSTEnemyType* out, int max_n);
void cst_churchman_strategic_review(const CSTChurchmanAnalysis* ca);

#endif /* CST_CHURCHMAN_H */