#include "sos_emergence.h"
#include <stdio.h>
int main(void) {
    printf("=== SoS Emergence Demo ===\n\n");
    SystemOfSystems* sos = sos_create("Swarm", SOS_COLLABORATIVE, "Coordinated movement");
    for (int i = 0; i < 5; i++) {
        char buf[32]; snprintf(buf, 32, "Drone-%d", i);
        ConstituentSystem* cs = gst_system_create(buf, GST_CYBERNETIC);
        cs->autonomy_level = 0.7;
        gst_system_add_interface(cs, "SwarmLink", 1);
        sos_add_constituent(sos, cs);
        gst_system_free(cs);
    }
    SoSArchitecture* arch = sos_arch_create(ARCH_MESH, 5);
    for (int i = 0; i < 5; i++)
        for (int j = i+1; j < 5; j++)
            sos_arch_connect(arch, i, j, INTEROP_DOMAIN, 50, 0.01, "SwarmProto");
    sos_arch_compute_metrics(arch);

    EmergentBehavior* eb = sos_emerge_create("Flocking", "Emergent formation flight", EMERGE_NOMINAL);
    for (int i = 0; i < 5; i++) sos_emerge_add_contributor(eb, i);
    EmergenceProfile* ep = sos_emerge_profile_create();
    sos_emerge_profile_add(ep, eb);
    sos_emerge_profile_analyze(ep, sos, arch);
    sos_emerge_profile_print(ep);

    bool novel = sos_detect_novel_emergence(sos, arch, 0.1);
    printf("\nNovel emergence detected: %s\n", novel ? "YES" : "NO");

    sos_emerge_profile_free(ep);
    sos_arch_free(arch);
    sos_free(sos);
    return 0;
}
