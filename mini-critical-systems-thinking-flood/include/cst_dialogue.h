#ifndef CST_DIALOGUE_H
#define CST_DIALOGUE_H
#include "cst_core.h"

typedef enum { CST_DIALOG_BOHM=0, CST_DIALOG_SOCRATIC=1, CST_DIALOG_DELIBERATIVE=2, CST_DIALOG_APPRECIATIVE=3 } CSTDialogueType;
typedef struct { CSTSystem* sys; CSTDialogueType type; int n_participants; int n_rounds; double inclusivity; double depth; double consensus_level; bool is_facilitated; } CSTDialogue;

CSTDialogue* cst_dialogue_create(CSTSystem* sys, CSTDialogueType type);
void cst_dialogue_free(CSTDialogue* d);
void cst_dialogue_run_round(CSTDialogue* d);
void cst_dialogue_run_n_rounds(CSTDialogue* d, int n);
double cst_dialogue_quality(const CSTDialogue* d);
double cst_dialogue_bohm_score(const CSTDialogue* d);
bool cst_dialogue_has_emerged(const CSTDialogue* d);
void cst_dialogue_print(const CSTDialogue* d);
#endif
