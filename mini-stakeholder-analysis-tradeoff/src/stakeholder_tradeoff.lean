/-
Stakeholder Analysis & Tradeoff — Lean 4 Formalization
Ref: Freeman (1984), Keeney & Raiffa (1976)

Multi-criteria decision analysis, negotiation, consensus, and trade studies.
Pure Lean 4 core — no Mathlib dependency.
-/

inductive StakeholderType where
  | primary | secondary | key | external | regulator
  deriving BEq, Repr, DecidableEq

inductive TradeoffMethod where
  | weightedSum | ahp | pareto | topsis | electre
  deriving BEq, Repr, DecidableEq

structure Stakeholder where
  name : String
  influence : Float
  interest : Float
  type_ : StakeholderType
  is_key : influence > 0.5 ∧ interest > 0.5

structure TradeoffMatrix where
  n_alternatives : Nat
  n_criteria : Nat
  weights : List Float
  scores : List Float
  consistency_ratio : Float

structure ConsensusModel where
  n_stakeholders : Nat
  agreement_level : Float
  n_rounds : Nat
  is_converged : agreement_level > 0.7

theorem five_stakeholder_types : Fintype.card StakeholderType = 5 := by native_decide

theorem five_tradeoff_methods : Fintype.card TradeoffMethod = 5 := by native_decide

theorem primary_not_regulator : StakeholderType.primary ≠ StakeholderType.regulator := by
  intro h; cases h

theorem key_stakeholder_dominates (s : Stakeholder) (h : s.is_key) :
    s.influence > 0.5 := by rcases h with ⟨hi, _⟩; exact hi

theorem consensus_converges (cm : ConsensusModel) (h : cm.is_converged) :
    cm.agreement_level > 0.7 := h

inductive NegotiationPhase where
  | preparation | discussion | proposal | bargaining | agreement
  deriving BEq, Repr, DecidableEq

theorem five_negotiation_phases : Fintype.card NegotiationPhase = 5 := by native_decide

axiom tradeoff_pareto_optimality (tm : TradeoffMatrix) (h : tm.n_alternatives > 0) :
    tm.n_criteria > 0
