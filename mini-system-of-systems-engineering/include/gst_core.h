#ifndef GST_CORE_H
#define GST_CORE_H
#include <stdbool.h>
#include <stddef.h>

/* Core types for System of Systems Engineering */

typedef enum {
    GST_FRAMEWORKS = 1, GST_CLOCKWORKS = 2, GST_CYBERNETIC = 3,
    GST_OPEN_SYSTEM = 4, GST_GENETIC = 5, GST_ANIMAL = 6,
    GST_HUMAN = 7, GST_SOCIAL = 8, GST_TRANSCENDENTAL = 9
} BouldingLevel;

typedef enum {
    GST_ISOLATED = 0, GST_CLOSED = 1, GST_OPEN = 2, GST_DISSIPATIVE = 3
} SystemOpenness;

typedef struct {
    char* name;
    BouldingLevel level;
    SystemOpenness openness;
    double complexity;
    double reliability;
    double autonomy_level;
    int n_interfaces;
    char** interface_names;
    int* interface_types;
    double* state; int n_states;
    double* capabilities; int n_capabilities;
    bool is_operational;
} ConstituentSystem;

typedef struct {
    ConstituentSystem* systems;
    int n_systems;
    int sys_cap;
} SystemSet;

ConstituentSystem* gst_system_create(const char* name, BouldingLevel level);
void gst_system_free(ConstituentSystem* cs);
void gst_system_set_state(ConstituentSystem* cs, int idx, double val);
double gst_system_get_state(ConstituentSystem* cs, int idx);
int gst_system_add_capability(ConstituentSystem* cs, double cap);
int gst_system_add_interface(ConstituentSystem* cs, const char* name, int type);
double gst_system_operational_readiness(ConstituentSystem* cs);
void gst_system_print(ConstituentSystem* cs);
void gst_system_free_contents(ConstituentSystem* cs);

SystemSet* gst_set_create(void);
void gst_set_free(SystemSet* set);
int gst_set_add(SystemSet* set, ConstituentSystem* cs);
ConstituentSystem* gst_set_get(SystemSet* set, int idx);
double gst_set_average_complexity(SystemSet* set);
double gst_set_average_autonomy(SystemSet* set);

double gst_sigmoid(double x, double k, double x0);
double gst_entropy(double* probs, int n);
double gst_normalize(double x, double min, double max);

#endif
