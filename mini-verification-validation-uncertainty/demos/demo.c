/* V&V Demo -- Verification and Validation with Uncertainty */
#include <stdio.h>
#include <math.h>
#include "vvu_core.h"
int main(void) {
    printf("=== V&V with Uncertainty Demo ===
");
    printf("Verification: Did we build the system right?
");
    printf("Validation:  Did we build the right system?
");
    printf("
Uncertainty Quantification (UQ) methods:
");
    printf("  Monte Carlo simulation for propagation analysis
");
    printf("  Sensitivity analysis (Sobol indices, Morris method)
");
    printf("  Bayesian model calibration for parameter updating
");
    printf("
Famous V&V failures: Mars Climate Orbiter (units),
");
    printf("  Fukushima (beyond-design-basis), 737 MAX (single sensor)
");
    printf("
=== V&V Demo Complete ===
");
    return 0;
}
