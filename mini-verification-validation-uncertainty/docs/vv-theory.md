# Verification, Validation & Uncertainty ? Theory

## The V&V Trinity

Every engineered system must answer three fundamental questions:

1. **Verification**: "Did we build the system right?"
   - Conformance to specifications
   - Requirements traceability
   - Test coverage and pass/fail rates

2. **Validation**: "Did we build the right system?"
   - Stakeholder needs alignment
   - Operational context fitness
   - Face validity and expert review

3. **Uncertainty**: "How well do we know what we know?"
   - Aleatory (inherent randomness, irreducible)
   - Epistemic (knowledge gaps, reducible with data)
   - Propagation through system models

## Verification Methods

| Method | Description | Automation |
|--------|-------------|------------|
| Inspection | Manual review of artifacts | Low |
| Analysis | Mathematical/analytical evaluation | Medium |
| Demonstration | Observe system behavior | Medium |
| Test | Controlled execution with pass/fail | High |
| Formal Proof | Mathematical proof of properties | High |

## Validation Methods

| Method | Description | Stakeholder Engagement |
|--------|-------------|----------------------|
| User Study | Observe real users | High |
| Expert Review | Domain expert assessment | Medium |
| Operational Test | Deploy in real environment | High |
| Face Validity | Does it look right? | Low |
| A/B Testing | Compare alternatives | Medium |

## Uncertainty Types

### Aleatory (Irreducible)
- Natural variability in physical processes
- Manufacturing tolerances
- Environmental randomness

### Epistemic (Reducible)
- Lack of knowledge about system parameters
- Model form uncertainty
- Measurement errors

### Distribution Types
- **Uniform**: Equal probability across range
- **Normal**: Gaussian distribution (CLT)
- **Lognormal**: Positive-valued, multiplicative processes
- **Triangular**: Expert judgment with mode
- **Beta**: Bounded [0,1], flexible shape

## Monte Carlo Propagation

The workhorse of uncertainty quantification:
```
mu_Y = (1/N) * sum_i f(X_i, theta)
sigma_Y^2 = (1/(N-1)) * sum_i (f(X_i, theta) - mu_Y)^2
```

Convergence rate: O(1/sqrt(N))
Confidence interval width decreases as 1/sqrt(N)

## Latin Hypercube Sampling (LHS)

Divides each input distribution into N equiprobable strata,
then randomly permutes across dimensions:
- Better coverage than simple random sampling
- Reduces variance for additive models
- Required samples ? 10 * n_inputs

## Key Standards

- IEEE 1012-2016: System/Software V&V
- ISO 9001: Quality Management Systems
- NASA-STD-7009: Models & Simulations Standard
- DoD VV&A Recommended Practices Guide
- ASME V&V 10: Computational Solid Mechanics
- ASME V&V 20: Computational Fluid Dynamics
