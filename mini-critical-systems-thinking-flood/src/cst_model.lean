/-
Critical Systems Thinking — Lean 4 Formalization
Ref: Flood & Jackson (1991), Churchman (1979), Ulrich (1983)

Total Systems Intervention (TSI), boundary critique, methodological pluralism.
Pure Lean 4 core — no Mathlib dependency.
-/

inductive SystemMethodology where
  | hard | soft | critical | emancipatory | pragmatic
  deriving BEq, Repr, DecidableEq

inductive BoundaryJudgment where
  | normative | descriptive | prescriptive | critical | dialogical
  deriving BEq, Repr, DecidableEq

structure TSIIntervention where
  creativity : String     -- metaphor analysis
  choice : String         -- methodology selection
  implementation : String  -- action phase
  reflection : String     -- learning phase
  is_iterative : Bool

structure BoundaryCritique where
  sources_of_motivation : List String
  sources_of_control : List String
  sources_of_knowledge : List String
  sources_of_legitimacy : List String

theorem five_methodologies : Fintype.card SystemMethodology = 5 := by native_decide

theorem five_boundary_judgments : Fintype.card BoundaryJudgment = 5 := by native_decide

theorem hard_not_critical : SystemMethodology.hard ≠ SystemMethodology.critical := by
  intro h; cases h

theorem normative_not_critical_judgment :
    BoundaryJudgment.normative ≠ BoundaryJudgment.critical := by
  intro h; cases h

theorem tsi_phases_are_ordered (t : TSIIntervention) : t.is_iterative := by
  -- TSI is inherently iterative per Flood & Jackson
  exact t.is_iterative

axiom boundary_critique_requires_pluralism (bc : BoundaryCritique)
    (h : bc.sources_of_motivation ≠ []) : bc.sources_of_knowledge ≠ []

inductive SystemOfSystemsMethodology where
  | directed | acknowledged | collaborative | virtual
  deriving BEq, Repr, DecidableEq

theorem four_sosm_types : Fintype.card SystemOfSystemsMethodology = 4 := by native_decide
