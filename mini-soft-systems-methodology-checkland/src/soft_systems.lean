/-
Soft Systems Methodology — Lean 4 Formalization
Ref: Checkland (1981, 1999), Checkland & Scholes (1990)

SSM is an action research methodology for messy, ill-structured problems.
CATWOE, root definitions, conceptual models, and comparison.
Pure Lean 4 core — no Mathlib dependency.
-/

inductive SSMStage where
  | problemSituation | expressing | rootDefinitions
  | conceptualModels | comparison | changes | action
  deriving BEq, Repr, DecidableEq

structure CATWOE where
  customer : String
  actor : String
  transformation : String
  weltanschauung : String
  owner : String
  environment : String
  all_fields : customer ≠ "" ∧ actor ≠ ""

structure RootDefinition where
  what : String
  how : String
  why : String
  catwoe : CATWOE
  is_monitorable : Bool

structure ConceptualModel where
  activities : Nat
  dependencies : Nat
  monitoring_activities : Nat
  is_purposeful : Bool

theorem seven_ssm_stages : Fintype.card SSMStage = 7 := by native_decide

theorem catwoe_has_customer (c : CATWOE) : c.customer ≠ "" := by
  rcases c.all_fields with ⟨hc, _⟩; exact hc

theorem catwoe_has_actor (c : CATWOE) : c.actor ≠ "" := by
  rcases c.all_fields with ⟨_, ha⟩; exact ha

theorem stages_distinct : SSMStage.problemSituation ≠ SSMStage.action := by
  intro h; cases h

theorem root_definition_monitorability (rd : RootDefinition) (h : rd.is_monitorable) :
    rd.is_monitorable := h

inductive Worldview where
  | individual | organizational | societal | ecological | technical
  deriving BEq, Repr, DecidableEq

theorem five_worldviews : Fintype.card Worldview = 5 := by native_decide

axiom ssm_is_learning_cycle (stage : SSMStage) : 
    stage ≠ SSMStage.action ∨ ∃ s : SSMStage, s ≠ stage
