#include "../include/cst_core.h"
#include <stdlib.h>
#include <string.h>

/* Power Analysis in CST — informed by Foucault and Ulrich
 *
 * Critical systems thinking must account for power asymmetries
 * that determine whose boundaries and values prevail. */

typedef struct { char* group; double power_index; int is_marginalized; double inclusion_score; } PowerGroup;
typedef struct { PowerGroup* groups; int n; double gini_coeff; int n_marginalized; } PowerAnalysis;

PowerAnalysis* pa_create(void) {
    PowerAnalysis* pa = calloc(1, sizeof(PowerAnalysis));
    if (!pa) return NULL;
    pa->groups = calloc(32, sizeof(PowerGroup));
    return pa;
}

void pa_free(PowerAnalysis* pa) {
    if (!pa) return;
    for (int i = 0; i < pa->n; i++) free(pa->groups[i].group);
    free(pa->groups); free(pa);
}

int pa_add_group(PowerAnalysis* pa, const char* name, double power, int marginalized) {
    if (!pa || !name) return -1;
    pa->groups[pa->n].group = strdup(name);
    pa->groups[pa->n].power_index = power;
    pa->groups[pa->n].is_marginalized = marginalized;
    if (marginalized) pa->n_marginalized++;
    return pa->n++;
}

double pa_compute_gini(PowerAnalysis* pa) {
    if (!pa || pa->n < 2) return 0.0;
    double sum_abs_diff = 0.0, sum_power = 0.0;
    for (int i = 0; i < pa->n; i++) sum_power += pa->groups[i].power_index;
    if (sum_power < 1e-10) return 0.0;
    for (int i = 0; i < pa->n; i++)
        for (int j = 0; j < pa->n; j++)
            sum_abs_diff += (pa->groups[i].power_index > pa->groups[j].power_index)
                ? (pa->groups[i].power_index - pa->groups[j].power_index) : 0.0;
    pa->gini_coeff = sum_abs_diff / (2.0 * pa->n * sum_power);
    return pa->gini_coeff;
}
