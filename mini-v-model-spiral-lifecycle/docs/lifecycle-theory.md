# Systems Engineering Lifecycle Models

## The V-Model (Forsberg andamp; Mooz, 1991)

The V-Model represents the systems engineering lifecycle as a V-shape:
- **Left side**: Decomposition (Concept -> Requirements -> Architecture -> Design)
- **Bottom**: Implementation (Build/Code/Fabricate)
- **Right side**: Integration (Unit Test -> Integration -> System -> Acceptance)

### Key Principles
1. Each decomposition level has a corresponding verification level
2. Verification planning begins during decomposition
3. Traceability flows from requirements through design to tests
4. Reviews (SRR, PDR, CDR, TRR, PRR) occur at key milestones

### V-Symmetry Index
Measures how balanced the V is:
`symmetry = 1 - |left_progress - right_progress|`
1.0 = perfectly balanced (verification keeps pace with design)

## The Spiral Model (Boehm, 1988)

Each cycle progresses through four phases:
1. **Determine Objectives** — What to achieve this cycle
2. **Identify and Resolve Risks** — Risk analysis, prototyping if needed
3. **Development and Test** — Build and verify increment
4. **Plan Next Cycle** — Review and commit to next iteration

### Key Principles
- Risk drives the process — high-risk elements are addressed early
- Each cycle produces an increment of the system
- The spiral radius represents cumulative cost
- Anchor points (LCO, LCA, IOC) mark key milestones

## Hybrid Approaches
Modern systems engineering often combines:
- V-Model structure (for traceability and rigor)
- Spiral iterations (for risk management and learning)
- Agile methods (for rapid feedback)
- Stage gates (for governance and decision-making)

## ISO/IEC 15288 Life Cycle Stages
1. Concept 2. Development 3. Production 4. Utilization 5. Support 6. Retirement
