# Model-Based Systems Engineering (MBSE)

## Overview

MBSE is the formalized application of modeling to support system
requirements, design, analysis, verification, and validation activities
beginning in the conceptual design phase and continuing throughout
development and later lifecycle phases. (INCOSE SE Vision 2025)

## Core Concepts

### The System Model
- Single source of truth for system information
- Integrates requirements, structure, behavior, and parametrics
- Enables automated analysis and consistency checking

### Four Pillars of SysML

| Pillar | Diagram Types | Our Implementation |
|---|---|---|
| Structure | BDD, IBD, Package | mbse_block, mbse_arch |
| Requirements | Requirement Diagram | mbse_req, mbse_trace |
| Behavior | State Machine, Activity, Sequence | mbse_sm, mbse_ad |
| Parametrics | Parametric Diagram | mbse_param, mbse_trade |

### Traceability
- Every requirement traced to at least one design element
- Every design element traced to at least one requirement
- Coverage metrics: #covered / #total

## Key References
- INCOSE (2023). Systems Engineering Handbook, 5th Ed.
- ISO/IEC/IEEE 15288:2023. System Life Cycle Processes
- OMG (2023). SysML v2 Specification
- NASA (2016). Systems Engineering Handbook
- Friedenthal, Moore, Steiner (2014). A Practical Guide to SysML
- Dori (2002). Object-Process Methodology
- Walden et al. (2015). INCOSE SE Handbook
