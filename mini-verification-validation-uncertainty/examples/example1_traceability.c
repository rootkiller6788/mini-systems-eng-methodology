/* V&V: Requirements traceability matrix for Mars Climate Orbiter (NASA 1999). */
#include "vvu_core.h"
#include <stdio.h>
int main(void){printf("=== V&V: Mars Climate Orbiter Failure Analysis ===

");
 printf("NASA Mars Climate Orbiter (1999): $327M mission lost
");
 printf("due to unit conversion error (lbf-s vs N-s).

");
 printf("Root Cause: Verification Gap
");
 printf("  Thruster software used pound-force seconds (Imperial)
");
 printf("  Navigation team assumed Newton-seconds (Metric)
");
 printf("  No V&V check caught the 4.45x mismatch.

");
 printf("Requirements Traceability Matrix (RTM):
");
 printf("  REQ-001: Thruster impulse in N-s        -> Test T-001: Unit check
");
 printf("  REQ-002: Navigation in metric units     -> Test T-002: Interface check
");
 printf("  REQ-003: Software-spec consistency      -> Test T-003: Cross-check

");
 printf("V&V Lessons Learned:
");
 printf("  1. Verify ALL interface units between subsystems
");
 printf("  2. End-to-end simulation with real units
");
 printf("  3. Independent cross-check by separate team
");
 printf("
Example 1 PASSED
");return 0;}
