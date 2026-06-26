/* stakeholder_applications.c -- L7: Supplier selection AHP case (Nydick & Hill 1992).
 * Classic supplier selection using AHP across quality, price, delivery, service.
 * Reference: Nydick & Hill (1992) JPSM 28(1):31-39
 */
#include "stakeholder_model.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    char *name;
    double quality, price, delivery, service, overall;
} Supplier;

Supplier* supp_new(const char *name, double q, double p, double d, double s) {
    Supplier *sp = calloc(1, sizeof(Supplier));
    sp->name = strdup(name);
    sp->quality = q; sp->price = p; sp->delivery = d; sp->service = s;
    return sp;
}
void supp_free(Supplier *sp) { if (sp) { free(sp->name); free(sp); } }

int supp_evaluate(Supplier *sp, double w_q, double w_p, double w_d, double w_s) {
    if (!sp) return -1;
    double sum = w_q + w_p + w_d + w_s;
    sp->overall = (w_q * sp->quality + w_p * sp->price +
                   w_d * sp->delivery + w_s * sp->service) / sum;
    return 0;
}

int supp_rank(Supplier **suppliers, int n, int *ranking) {
    if (!suppliers || !ranking || n < 1) return -1;
    for (int i = 0; i < n; i++) ranking[i] = i;
    for (int i = 0; i < n - 1; i++)
        for (int j = i + 1; j < n; j++)
            if (suppliers[ranking[i]]->overall < suppliers[ranking[j]]->overall) {
                int t = ranking[i]; ranking[i] = ranking[j]; ranking[j] = t;
            }
    return 0;
}

int supp_sensitivity(Supplier **suppliers, int n, int criterion,
                      int *rank_change_count) {
    if (!suppliers || !rank_change_count) return -1;
    *rank_change_count = 0;
    int base_rank[10];
    supp_rank(suppliers, n, base_rank);
    double weights[4] = {0.3, 0.3, 0.2, 0.2};
    for (int step = -5; step <= 5; step++) {
        weights[criterion] = 0.3 + (double)step * 0.03;
        if (weights[criterion] < 0.05) continue;
        for (int i = 0; i < n; i++)
            supp_evaluate(suppliers[i], weights[0], weights[1], weights[2], weights[3]);
        int new_rank[10];
        supp_rank(suppliers, n, new_rank);
        for (int i = 0; i < n; i++)
            if (base_rank[i] != new_rank[i]) { (*rank_change_count)++; break; }
    }
    return 0;
}

void supp_print_report(Supplier **suppliers, int n) {
    printf("=== Supplier Selection AHP Report (Nydick & Hill) ===\n");
    int ranking[10];
    supp_rank(suppliers, n, ranking);
    for (int i = 0; i < n; i++)
        printf("  #%d: %s (Q=%.2f P=%.2f D=%.2f S=%.2f) Overall=%.3f\n",
               i+1, suppliers[ranking[i]]->name,
               suppliers[ranking[i]]->quality, suppliers[ranking[i]]->price,
               suppliers[ranking[i]]->delivery, suppliers[ranking[i]]->service,
               suppliers[ranking[i]]->overall);
}
