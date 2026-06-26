#ifndef MBSE_ARCHITECTURE_H
#define MBSE_ARCHITECTURE_H
#include "mbse_core.h"
/* ===================================================================
 * Architecture analysis provides:
 *   - Decomposition hierarchy metrics (depth, fan-out, cohesion)
 *   - Modularity quantification based on coupling/cohesion ratio
 *   - Functional allocation to physical architecture elements
 *   - Interface density: actual / maximum possible connections
 *   - Layer validation for strictly layered architectures
 *
 * The INCOSE SE Handbook defines architecture as the
 * fundamental organization of a system embodied in its
 * components, their relationships, and the principles
 * governing their design and evolution.
 *
 * Core metrics:
 *   Cohesion = internal_interfaces / total_interfaces
 *   Coupling  = total_interfaces / total_blocks
 *   Modularity = cohesion * (1 - coupling/n_blocks)
 *
 * References: ISO 42010, Maier & Rechtin (2009), Crawley (2016)
 * =================================================================== */
typedef struct { int depth; double fan_out_avg, fan_out_max, cohesion, coupling; bool is_strictly_layered; } HierarchyMetrics;
/*
 * Architecture analysis provides:
 *   - Hierarchy depth and fan-out metrics
 *   - Cohesion/coupling based modularity assessment
 *   - Functional allocation to physical components
 *   - Interface density and complexity analysis
 *
 * Key References:
 *   INCOSE SE Handbook Ch.7 - Architecture Definition
 *   ISO/IEC 42010 - Architecture Description
 *   Maier & Rechtin - The Art of Systems Architecting
 *   Crawley et al. - System Architecture: Strategy
 */
typedef struct { int block_id, n_functions; int* function_ids; double allocation_fitness; bool is_overloaded, is_underutilized; } AllocationResult;
typedef struct { int source, target, n_ifaces; double iface_complexity, coupling_strength; } BlockCoupling;
HierarchyMetrics* mbse_arch_hierarchy_metrics(SystemModel* m);
void mbse_arch_hierarchy_free(HierarchyMetrics* hm);
int mbse_arch_max_depth(SystemModel* m);
double mbse_arch_modularity(SystemModel* m);
AllocationResult* mbse_arch_allocate_function(SystemModel* m, int block_id, int n_funcs, int* func_ids);
void mbse_arch_allocation_free(AllocationResult* ar);
BlockCoupling* mbse_arch_coupling_analysis(SystemModel* m, int a, int b);
void mbse_arch_coupling_free(BlockCoupling* bc);
double mbse_arch_total_coupling(SystemModel* m);
void mbse_arch_print_breakdown(SystemModel* m);
#endif
