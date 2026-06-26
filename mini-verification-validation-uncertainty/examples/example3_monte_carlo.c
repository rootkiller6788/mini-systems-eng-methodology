#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "vvu_core.h"
#include "vvu_monte_carlo.h"

static double eval_func(const double* inputs, int n, void* ctx) {
    (void)ctx;
    /* Simple model: output = input^2 + noise */
    double sum = 0.0;
    for (int i = 0; i < n; i++)
        sum += inputs[i] * inputs[i];
    return sum;
}

int main(void) {
    printf("=== Example 3: Uncertainty Propagation (Monte Carlo) ===\n\n");
    int n_samples = 5000;
    VVU_MCConfig* cfg = vvu_mc_config_create(n_samples, 2);
    vvu_mc_config_set_seed(cfg, 12345);
    /* Define two uncertain inputs */
    VVU_Uncertainty uncerts[2];
    memset(&uncerts[0], 0, sizeof(VVU_Uncertainty));
    uncerts[0].name = "length"; uncerts[0].nominal = 5.0;
    uncerts[0].distribution = VVU_DIST_NORMAL;
    uncerts[0].param1 = 5.0; uncerts[0].param2 = 0.5;
    uncerts[0].lower_bound = 3.5; uncerts[0].upper_bound = 6.5;
    memset(&uncerts[1], 0, sizeof(VVU_Uncertainty));
    uncerts[1].name = "width"; uncerts[1].nominal = 3.0;
    uncerts[1].distribution = VVU_DIST_UNIFORM;
    uncerts[1].param1 = 2.5; uncerts[1].param2 = 3.5;
    uncerts[1].lower_bound = 2.5; uncerts[1].upper_bound = 3.5;
    printf("Uncertainty inputs:\n");
    printf("  %s: Normal(%.1f, %.1f) in [%.1f, %.1f]\n",
           uncerts[0].name, uncerts[0].mean, uncerts[0].std_dev,
           uncerts[0].lower_bound, uncerts[0].upper_bound);
    printf("  %s: Uniform(%.1f, %.1f)\n",
           uncerts[1].name, uncerts[1].lower_bound, uncerts[1].upper_bound);
    printf("\nRunning %d Monte Carlo samples...\n", cfg->n_samples);
    VVU_MCStatistics* stats = vvu_mc_run(uncerts, 2, eval_func, NULL, cfg);
    vvu_mc_statistics_print(stats);
    double ci = vvu_mc_confidence_interval_width(&stats->mean[0], 1, 0.95);
    printf("95%% CI width: %.4f\n", ci);
    int required = vvu_mc_required_samples(0.1, stats->variance[0]);
    printf("Required samples for 0.1 precision: %d\n", required);
    vvu_mc_statistics_free(stats);
    vvu_mc_config_free(cfg);
    printf("\nKey insight: Monte Carlo propagation reveals the output distribution,\n");
    printf("enabling risk-informed decisions under uncertainty.\n");
    return 0;
}
