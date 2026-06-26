# Boehm's Spiral Model — Risk-Driven Development

## Origins
Barry Boehm published "A Spiral Model of Software Development and Enhancement"
in 1988, introducing a risk-driven, iterative approach that combined the best
of waterfall and prototyping methodologies.

## The Four Quadrants

### Q1: Determine Objectives, Alternatives, Constraints
- What does the stakeholder want this cycle?
- What are the alternatives for achieving it?
- What are the constraints (budget, schedule, technology)?

### Q2: Evaluate Alternatives, Identify/Resolve Risks
- What can go wrong?
- How likely is it? How severe?
- Should we prototype to reduce uncertainty?
- Risk exposure = Probability x Impact

### Q3: Develop, Verify Next-Level Product
- Build the increment specified for this cycle
- Verify it meets objectives
- May involve: simulation, modeling, benchmarking, prototyping

### Q4: Plan Next Cycle
- Review results of this cycle
- Update plans based on learning
- Commit to next cycle (or terminate)

## Anchor Points (Boehm, 1996)
- **LCO** (Life Cycle Objectives): Stakeholders agree on feasibility
- **LCA** (Life Cycle Architecture): Architecture is validated
- **IOC** (Initial Operational Capability): System is ready for use

## Risk-Driven Nature
The fundamental insight: *the amount of effort devoted to each activity
is driven by the level of risk involved.* High-risk elements get more
analysis, prototyping, and verification.

## Relationship to V-Model
The spiral model can be used WITHIN each V-Model level, providing
iterative refinement. Modern standards (ISO 15288, INCOSE) recommend
hybrid approaches that combine structural rigor with iterative learning.
