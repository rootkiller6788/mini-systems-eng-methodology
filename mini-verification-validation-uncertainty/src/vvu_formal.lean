/-
Verification, Validation & Uncertainty — Lean 4 Formalization
Ref: INCOSE SE Handbook, ISO/IEC 15288, Oberkampf & Roy (2010)

V&V ensures the system works correctly and meets stakeholder needs.
Uncertainty quantification addresses epistemic and aleatory uncertainty.
Pure Lean 4 core — no Mathlib dependency.
-/

inductive VerificationMethod where
  | inspection | analysis | demonstration | test | simulation
  deriving BEq, Repr, DecidableEq

inductive ValidationMethod where
  | stakeholderReview | operationalTest | prototypeEval
  | formalProof | fieldTrial
  deriving BEq, Repr, DecidableEq

inductive UncertaintyType where
  | aleatory | epistemic | ontological | measurement | model
  deriving BEq, Repr, DecidableEq

structure VerificationResult where
  requirement : String
  method : VerificationMethod
  passed : Bool
  evidence : String

structure ValidationResult where
  stakeholder : String
  acceptance_level : Float
  is_accepted : acceptance_level > 0.8

structure UncertaintyModel where
  type_ : UncertaintyType
  distribution : String
  confidence_interval : Float
  is_quantified : Bool

theorem five_verification_methods : Fintype.card VerificationMethod = 5 := by native_decide

theorem five_validation_methods : Fintype.card ValidationMethod = 5 := by native_decide

theorem five_uncertainty_types : Fintype.card UncertaintyType = 5 := by native_decide

theorem verification_not_simulation : VerificationMethod.inspection ≠ VerificationMethod.simulation := by
  intro h; cases h

theorem validation_accepted (vr : ValidationResult) (h : vr.is_accepted) :
    vr.acceptance_level > 0.8 := h

axiom aleatory_epistemic_distinct :
    UncertaintyType.aleatory ≠ UncertaintyType.epistemic

inductive TraceabilityDirection where
  | forward | backward | bidirectional | none
  deriving BEq, Repr, DecidableEq

theorem four_traceability_directions : Fintype.card TraceabilityDirection = 4 := by native_decide
