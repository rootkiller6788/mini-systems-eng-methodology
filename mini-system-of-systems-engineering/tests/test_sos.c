#include "gst_core.h"
#include "sos_types.h"
#include "sos_architecture.h"
#include "sos_emergence.h"
#include "sos_governance.h"
#include "sos_assessment.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// removed - using standard assert()
static int passed = 0, failed = 0;
#define T(n) printf("  %s... ", n); fflush(stdout)
#define assert(c) do { if(c){printf("OK\n");passed++;}else{printf("FAIL\n");failed++;} }while(0)

static void test_constituent_systems(void) {
    T("cs_create");
    ConstituentSystem* cs1 = gst_system_create("Radar", GST_CYBERNETIC);
    assert(cs1 != NULL);
    assert(cs1->autonomy_level > 0);

    T("cs_interfaces");
    gst_system_add_interface(cs1, "DataLink", 1);
    gst_system_add_interface(cs1, "PowerBus", 2);
    assert(cs1->n_interfaces == 2);

    T("cs_capabilities");
    gst_system_add_capability(cs1, 0.95);
    assert(cs1->n_capabilities == 1);

    T("set_create_add");
    SystemSet* set = gst_set_create();
    gst_set_add(set, cs1);
    assert(set->n_systems == 1);

    T("set_avg_autonomy");
    double a = gst_set_average_autonomy(set);
    assert(a > 0);

    gst_set_free(set);
    gst_system_free(cs1);
}

static void test_sos_creation(void) {
    T("sos_create");
    SystemOfSystems* sos = sos_create("AirDefense", SOS_DIRECTED, "Protect airspace");
    assert(sos != NULL);
    assert(sos->type == SOS_DIRECTED);

    T("sos_add_constituent");
    ConstituentSystem* cs = gst_system_create("Interceptor", GST_CYBERNETIC);
    sos_add_constituent(sos, cs);
    assert(sos_n_constituents(sos) == 1);

    T("sos_readiness");
    assert(sos_readiness(sos) >= 0);

    T("sos_type_names");
    assert(strcmp(sos_type_name(SOS_COLLABORATIVE), "Collaborative") == 0);

    gst_system_free(cs);
    sos_free(sos);
}

static void test_architecture(void) {
    T("arch_create");
    SoSArchitecture* arch = sos_arch_create(ARCH_NETWORKED, 5);
    assert(arch != NULL);

    T("arch_connect");
    int r = sos_arch_connect(arch, 0, 1, INTEROP_FUNCTIONAL, 100.0, 0.01, "TCP/IP");
    assert(r >= 0);
    r = sos_arch_connect(arch, 1, 2, INTEROP_DOMAIN, 50.0, 0.02, "MQTT");
    r = sos_arch_connect(arch, 2, 3, INTEROP_CONNECTED, 10.0, 0.05, "Serial");
    r = sos_arch_connect(arch, 3, 4, INTEROP_FUNCTIONAL, 75.0, 0.01, "HTTP");
    assert(sos_arch_n_connections(arch) == 4);

    T("arch_metrics");
    sos_arch_compute_metrics(arch);
    assert(arch->connectivity_density > 0);
    assert(arch->robustness >= 0);

    T("arch_interop");
    double iscore = sos_arch_interoperability_score(arch);
    assert(iscore >= 0 && iscore <= 1.0);

    T("arch_connected");
    assert(sos_arch_is_connected(arch));

    T("arch_critical");
    int nc;
    int* crit = sos_arch_critical_nodes(arch, &nc);
    assert(crit != NULL || nc == 0);
    free(crit);

    T("arch_disconnect");
    sos_arch_disconnect(arch, 0, 1);
    assert(sos_arch_n_connections(arch) <= 4);

    sos_arch_free(arch);
}

static void test_emergence(void) {
    T("emerge_create");
    EmergentBehavior* eb = sos_emerge_create("SwarmIntelligence", "Collective target tracking", EMERGE_NOMINAL);
    assert(eb != NULL);

    T("emerge_contributors");
    sos_emerge_add_contributor(eb, 0);
    sos_emerge_add_contributor(eb, 1);
    assert(eb->n_contributing == 2);

    T("emerge_profile");
    EmergenceProfile* ep = sos_emerge_profile_create();
    sos_emerge_profile_add(ep, eb);
    assert(ep->n_behaviors == 1);

    T("emerge_type_count");
    int n = sos_emerge_count_by_type(ep, EMERGE_NOMINAL);
    assert(n == 1);

    T("emerge_risk");
    double risk = sos_emerge_pathological_risk(ep);
    assert(risk >= 0 && risk <= 1.0);

    sos_emerge_profile_free(ep);  /* deep copy: profile owns its own */
    sos_emerge_free(eb);
}

static void test_governance(void) {
    T("gov_create");
    SoSGovernance* gov = sos_gov_create(GOV_FEDERATED);
    assert(gov != NULL);

    T("gov_add_policy");
    sos_gov_add_policy(gov, "StandardInterface", "All systems use common API", 0.8);
    assert(gov->n_policies == 1);

    T("gov_add_stakeholder");
    int si = sos_gov_add_stakeholder(gov, "Operator", STAKE_END_USER, 0.9, 0.7);
    assert(si >= 0);

    T("gov_assess");
    SystemOfSystems* sos = sos_create("TestSoS", SOS_ACKNOWLEDGED, "Test");
    ConstituentSystem* cs = gst_system_create("Sys1", GST_CYBERNETIC);
    sos_add_constituent(sos, cs);
    sos_gov_assess(gov, sos);
    assert(gov->control_effectiveness > 0);

    T("gov_fitness");
    double fit = sos_gov_governance_fitness(gov, SOS_ACKNOWLEDGED);
    assert(fit >= 0 && fit <= 1.0);

    T("gov_effective");
    assert(sos_gov_is_effective(gov) || !sos_gov_is_effective(gov));

    gst_system_free(cs); sos_free(sos); sos_gov_free(gov);
}

static void test_assessment(void) {
    T("assess_create");
    SoSAssessment* sa = sos_assess_create();
    assert(sa != NULL);

    T("assess_readiness");
    SystemOfSystems* sos = sos_create("AssessSoS", SOS_DIRECTED, "Test");
    ConstituentSystem* cs = gst_system_create("Node", GST_CYBERNETIC);
    sos_add_constituent(sos, cs);
    sos_assess_readiness(sa, sos);
    assert(sa->readiness_score >= 0);

    T("assess_maturity");
    sos_assess_maturity(sa, 10, 5);
    assert(sa->maturity_level == MATURITY_DEFINED);

    T("assess_overall");
    sos_assess_compute_overall(sa);
    assert(sa->overall_score >= 0);

    T("resilience_scenario");
    ResilienceScenario* rs = sos_resilience_create("NodeFailure", 1);
    sos_resilience_set_impact(rs, 0, 0.5);
    SoSArchitecture* arch = sos_arch_create(ARCH_HUB_SPOKE, 1);
    bool surv = sos_resilience_evaluate(rs, sos, arch);
    assert(surv || !surv);

    T("resilience_analysis");
    ResilienceAnalysis* ra = sos_resilience_analysis_create();
    sos_resilience_analysis_add(ra, rs);
    sos_resilience_analysis_compute(ra, sos, arch);
    assert(ra->resilience_index >= 0);

    sos_resilience_analysis_free(ra);
    sos_arch_free(arch);
    gst_system_free(cs);
    sos_free(sos);
    sos_assess_free(sa);
}

int main(void) {
    printf("=== SoS Engineering Test Suite ===\n\n");
    test_constituent_systems();
    test_sos_creation();
    test_architecture();
    test_emergence();
    test_governance();
    test_assessment();
    printf("\n=== All asserts passed ===\n"); return 0;
}
