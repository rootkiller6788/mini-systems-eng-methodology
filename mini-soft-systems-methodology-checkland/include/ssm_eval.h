#ifndef SSM_EVAL_H
#define SSM_EVAL_H
#include "ssm_types.h"

/* Systemic Evaluation — measuring SSM effectiveness */

typedef struct { char* name; double before_score; double after_score; double improvement; char* qualitative_evidence; } SSMMetric;
typedef struct { char* intervention_name; SSMMetric** metrics; int n_metrics; double overall_improvement; bool sustainable; char** lessons_learned; int n_lessons; } SSMEvaluation;

typedef struct { char* stakeholder; double satisfaction; double perceived_fairness; double willingness_to_continue; char** concerns; int n_concerns; } StakeholderFeedback;
typedef struct { char* situation_name; StakeholderFeedback** feedback; int n_feedback; double overall_satisfaction; double process_legitimacy; } SSMFeedbackSummary;

typedef struct { int cycle_number; char** key_insights; int n_insights; double accommodation_achieved; bool new_worldviews_emerged; int conflicts_resolved; int conflicts_remaining; } LearningHistory;

SSMEvaluation* ssm_evaluation_create(const char* name);
void ssm_evaluation_free(SSMEvaluation* ev);
int ssm_eval_add_metric(SSMEvaluation* ev, const char* name, double before, double after, const char* evidence);
double ssm_eval_compute_improvement(SSMEvaluation* ev);
void ssm_eval_add_lesson(SSMEvaluation* ev, const char* lesson);
void ssm_eval_print(const SSMEvaluation* ev);

SSMFeedbackSummary* ssm_feedback_create(const char* name);
void ssm_feedback_free(SSMFeedbackSummary* fs);
int ssm_feedback_add_stakeholder(SSMFeedbackSummary* fs, const char* name, double satisfaction, double fairness);
void ssm_feedback_add_concern(SSMFeedbackSummary* fs, int stk_id, const char* concern);
double ssm_feedback_compute_satisfaction(SSMFeedbackSummary* fs);
double ssm_feedback_compute_legitimacy(SSMFeedbackSummary* fs);
void ssm_feedback_print(const SSMFeedbackSummary* fs);

LearningHistory* ssm_history_create(int cycle);
void ssm_history_free(LearningHistory* lh);
void ssm_history_add_insight(LearningHistory* lh, const char* insight);
void ssm_history_record_accommodation(LearningHistory* lh, double accommodation);
bool ssm_history_is_improving(const LearningHistory* lh);
double ssm_history_learning_rate(const LearningHistory* lh, const LearningHistory* prev);
void ssm_history_print(const LearningHistory* lh);

#endif
