#ifndef MBSE_CORE_H
#define MBSE_CORE_H

#include <stdbool.h>
#include <stddef.h>

/* ============================================================================
 * Model-Based Systems Engineering (MBSE) Core Types
 *
 * Based on INCOSE SE Handbook (2023), ISO/IEC/IEEE 15288:2023,
 * OMG SysML v2, NASA SE Handbook, Dori OPM (2002)
 * ============================================================================ */

/* --- Block Categories (SysML Block stereotype mapping) --- */
typedef enum {
    BLOCK_SYSTEM = 0, BLOCK_SUBSYSTEM = 1, BLOCK_COMPONENT = 2,
    BLOCK_SOFTWARE = 3, BLOCK_HARDWARE = 4, BLOCK_HUMAN = 5,
    BLOCK_ENVIRONMENT = 6, BLOCK_EXTERNAL_SYSTEM = 7, BLOCK_ABSTRACT = 8
} BlockCategory;

/* --- Requirement Types (INCOSE requirement taxonomy) --- */
typedef enum {
    REQ_FUNCTIONAL = 0, REQ_PERFORMANCE = 1, REQ_INTERFACE = 2,
    REQ_DESIGN_CONSTRAINT = 3, REQ_SAFETY = 4, REQ_SECURITY = 5,
    REQ_RELIABILITY = 6, REQ_VERIFICATION = 7, REQ_NON_FUNCTIONAL = 8
} RequirementType;

/* --- Interface Types (SysML proxy port stereotypes) --- */
typedef enum {
    IFACE_MECHANICAL = 0, IFACE_ELECTRICAL = 1, IFACE_DATA = 2,
    IFACE_SOFTWARE_API = 3, IFACE_THERMAL = 4, IFACE_HUMAN_MACHINE = 5,
    IFACE_FLUID = 6, IFACE_OPTICAL = 7
} InterfaceType;

/* --- Verification Methods (ISO 15288 verification approaches) --- */
typedef enum {
    VERIFY_TEST = 0, VERIFY_ANALYSIS = 1, VERIFY_DEMONSTRATION = 2,
    VERIFY_INSPECTION = 3, VERIFY_SIMULATION = 4, VERIFY_CERTIFICATION = 5
} VerificationMethod;

/* ===== Core Data Structures ===== */

/* SysML Block: a modular unit of system description */
typedef struct {
    int id; char* name; BlockCategory category; char* description;
    int parent_id; int* child_ids; int n_children, child_capacity;
    double mass, power, cost, reliability;
    int allocated_req_count; int* allocated_req_ids; int req_capacity;
} MBSEBlock;

/* System Requirement with traceability links */
typedef struct {
    int id; char* name; RequirementType type; char* text;
    double target_value, threshold_value; int priority;
    int satisfied_by_block; int verified_by_method;
    bool is_satisfied, is_verified;
    int* derived_from; int n_derived, derived_capacity;
    int* traces_to; int n_traces, traces_capacity;
} MBSERequirement;

/* Interface between two blocks */
typedef struct {
    int id; char* name; InterfaceType type;
    int source_block, target_block; char* protocol_spec;
    double data_rate, latency; bool is_bidirectional; double reliability;
} MBSEInterface;

/* Top-level System Model container */
typedef struct {
    char* name; char* description;
    MBSEBlock* blocks; int n_blocks, block_capacity;
    MBSERequirement* requirements; int n_requirements, req_capacity;
    MBSEInterface* interfaces; int n_interfaces, iface_capacity;
    int** traceability_matrix; int trace_rows, trace_cols;
    double** interface_matrix;   /* [n_blocks][n_blocks] adjacency for DSM analysis */
    double total_cost, total_mass, total_power, overall_reliability;
} SystemModel;

/* === Block API === */
MBSEBlock*  mbse_block_create(const char* name, BlockCategory cat);
void        mbse_block_free(MBSEBlock* b);
void        mbse_block_add_child(MBSEBlock* parent, int child_id);
void        mbse_block_allocate_req(MBSEBlock* b, int req_id);
void        mbse_block_print(MBSEBlock* b);

/* === Requirement API === */
MBSERequirement* mbse_req_create(const char* name, RequirementType type, const char* text, int priority);
void             mbse_req_free(MBSERequirement* r);
void             mbse_req_set_target(MBSERequirement* r, double target, double threshold);
void             mbse_req_trace_to(MBSERequirement* r, int req_id);
bool             mbse_req_is_leaf(MBSERequirement* r);
void             mbse_req_print(MBSERequirement* r);

/* === Interface API === */
MBSEInterface* mbse_iface_create(const char* name, InterfaceType type, int source, int target);
void           mbse_iface_free(MBSEInterface* iface);
bool           mbse_iface_connects(MBSEInterface* iface, int a, int b);
void           mbse_iface_print(MBSEInterface* iface);

/* === System Model API === */
SystemModel*     mbse_model_create(const char* name, const char* desc);
void             mbse_model_free(SystemModel* m);
int              mbse_model_add_block(SystemModel* m, MBSEBlock* b);
int              mbse_model_add_requirement(SystemModel* m, MBSERequirement* r);
int              mbse_model_add_interface(SystemModel* m, MBSEInterface* iface);
void             mbse_model_build_traceability(SystemModel* m);
double           mbse_model_compute_cost(SystemModel* m);
double           mbse_model_compute_reliability(SystemModel* m);
double           mbse_model_compute_mass_margin(SystemModel* m, double target);
MBSEBlock*       mbse_model_find_block(SystemModel* m, const char* name);
MBSERequirement* mbse_model_find_req(SystemModel* m, const char* name);
int              mbse_model_count_unallocated_reqs(SystemModel* m);
int              mbse_model_count_interfaces_between(SystemModel* m, int a, int b);
int              mbse_model_count_blocks_by_category(SystemModel* m, BlockCategory cat);
void             mbse_model_print(SystemModel* m);
void             mbse_model_print_traceability(SystemModel* m);
void             mbse_model_export_csv(SystemModel* m, const char* filename);

#endif
