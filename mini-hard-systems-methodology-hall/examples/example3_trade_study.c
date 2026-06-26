#include "hsm_trade.h"
#include "hsm_logic.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("============================================\n");
    printf("  Trade Study: Multi-Criteria Decision\n");
    printf("============================================\n\n");

    /* Trade study for UAV design */
    TradeStudy* ts = hsm_trade_create();
    hsm_trade_add_criterion(ts, "Range (km)", 0.3, true, 100, 500);
    hsm_trade_add_criterion(ts, "Endurance (hr)", 0.25, true, 4, 24);
    hsm_trade_add_criterion(ts, "Cost ($K)", 0.25, false, 0, 500);
    hsm_trade_add_criterion(ts, "Weight (kg)", 0.2, false, 0, 50);

    hsm_trade_add_alternative(ts, "Fixed-Wing", "Long range, high endurance", 200, 6, 0.15);
    hsm_trade_add_alternative(ts, "Quadrotor", "VTOL, low endurance", 150, 3, 0.20);
    hsm_trade_add_alternative(ts, "Hybrid VTOL", "Best of both worlds", 350, 8, 0.30);

    hsm_trade_set_scores(ts, 0, (double[]){0.9, 0.8, 0.5, 0.6});
    hsm_trade_set_scores(ts, 1, (double[]){0.3, 0.4, 0.8, 0.7});
    hsm_trade_set_scores(ts, 2, (double[]){0.7, 0.9, 0.3, 0.4});

    hsm_trade_compute(ts);
    printf("--- Full Trade Study ---\n");
    hsm_trade_print(ts);
    printf("\n--- Ranking ---\n");
    hsm_trade_print_ranking(ts);

    /* Decision matrix with AHP weights */
    printf("\n--- Decision Matrix (AHP) ---\n");
    double w[] = {0.35, 0.25, 0.25, 0.15};
    HSMDecisionMatrix* dm = hsm_dm_create(3, 4, w);
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 4; j++)
            hsm_dm_set_score(dm, i, j, ts->alternatives[i].scores[j]);
    hsm_dm_compute_ranking(dm);
    hsm_dm_print(dm);
    printf("Best: Alternative %d\n", hsm_dm_best(dm));

    /* Cost-benefit */
    printf("\n--- Cost-Benefit Ratios ---\n");
    for (int i = 0; i < 3; i++)
        printf("  %s: C/B = %.4f\n", ts->alternatives[i].name, hsm_cost_benefit_ratio(ts, i));

    /* Pareto frontier */
    int n_pareto;
    int* pareto = hsm_trade_pareto_frontier(ts, 2, 0, &n_pareto);
    printf("\n--- Pareto Frontier (Cost vs Range) ---\n");
    printf("  %d non-dominated solutions:\n", n_pareto);
    for (int i = 0; i < n_pareto; i++)
        printf("    %s\n", ts->alternatives[pareto[i]].name);
    free(pareto);

    /* ROI and NPV */
    double cf[] = {-500, 100, 150, 200, 250};
    printf("\n--- Financial Analysis ---\n");
    printf("  NPV (10%%): %.2f\n", hsm_net_present_value(cf, 5, 0.1));
    printf("  ROI: %.2f%%\n", hsm_return_on_investment(700, 500) * 100);

    hsm_dm_free(dm);
    hsm_trade_free(ts);
    return 0;
}
