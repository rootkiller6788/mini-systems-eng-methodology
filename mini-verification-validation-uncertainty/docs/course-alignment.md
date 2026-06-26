# Course Alignment ? Verification, Validation & Uncertainty

## MIT 16.842: Fundamentals of Systems Engineering
- **Verification planning**: Test matrices, coverage analysis
- **Validation strategy**: Stakeholder engagement, operational scenarios
- **V-model**: Verification at each decomposition level
- Module connection: `vvu_verif_add` links tests to requirements
- Module connection: `vvu_traceability_matrix` shows req-test mapping

## MIT 6.241J: Dynamic Systems and Control
- **Model validation**: Does the model match reality?
- **Residual analysis**: Statistical tests for model adequacy
- **Uncertainty in control**: Robust control margins
- Module connection: `vvu_mc_run` propagates uncertainty through dynamics

## Stanford AA203: Optimal and Learning-based Control
- **Robustness verification**: Worst-case analysis
- **Statistical validation**: Cross-validation, bootstrapping
- **Uncertainty in learning**: Bayesian approaches
- Module connection: `vvu_mc_statistics_print` gives output distributions

## Stanford EE363: Convex Optimization
- **Constraint verification**: Feasibility checking against specs
- **Sensitivity analysis**: How do solutions change with parameters?
- Module connection: `vvu_compute_confidence` composite metric

## Berkeley EE221A: Linear Systems Theory
- **Model verification**: Eigenvalue checks, stability margins
- **Observability**: Can we verify internal states?
- **Controllability**: Can validation tests exercise all states?

## Caltech CDS110: Introduction to Control Theory
- **Performance verification**: Step response, frequency response
- **Robustness margins**: Gain/phase margin verification
- Module connection: `vvu_verif_run` with pass_threshold and tolerance

## ETH 227-0216: System Identification
- **Cross-validation**: Train/test split for model validation
- **Uncertainty quantification**: Parameter confidence intervals
- **Information criteria**: AIC/BIC for model selection
- Module connection: `vvu_binomial_confidence_lower/upper`

## Key Formulas

### Verification Pass Rate
```
pass_rate = N_pass / N_total
```

### Binomial Confidence Interval (Wald)
```
CI = p_hat +/- z * sqrt(p_hat * (1 - p_hat) / n)
```

### Monte Carlo Mean and Variance
```
mu = (1/N) * sum_i f(X_i)
sigma^2 = (1/(N-1)) * sum_i (f(X_i) - mu)^2
```

### Required MC Samples
```
N = (z * sigma / precision)^2
```

### Composite Confidence
```
C = 0.4 * C_verif + 0.3 * C_valid + 0.3 * C_uncert
```

## Teaching Notes

This module is suitable for:
1. **Week 1-2**: V&V fundamentals, terminology, standards
2. **Week 3-4**: Requirements traceability, test planning
3. **Week 5-6**: Validation methods, stakeholder engagement
4. **Week 7-8**: Uncertainty types, Monte Carlo methods
5. **Week 9-10**: Metrics, confidence, gap analysis

### Exercises
1. Build a verification test matrix for a drone autopilot
2. Propagate sensor uncertainty through a Kalman filter
3. Compute confidence intervals for pass rates
4. Gap analysis for an incomplete V&V plan
