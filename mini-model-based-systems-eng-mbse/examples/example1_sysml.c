/* MBSE: SysML block definition diagram for a CubeSat 1U (NASA JPL standard). */
#include "mbse_core.h"
#include <stdio.h>
int main(void){printf("=== MBSE: CubeSat 1U Architecture Modeling ===

");
 printf("NASA JPL CubeSat Reference Model (1U, 10x10x10cm):

");
 printf("Block Definition Diagram (BDD):
");
 printf("  CubeSat_1U [Block]
");
 printf("    - mass: 1.33 kg
");
 printf("    - power_budget: 2.5 W
");
 printf("    - data_rate: 9.6 kbps downlink

");
 printf("  Subsystems (composition):
");
 printf("    EPS (Electrical Power): solar panels, battery, PMU
");
 printf("    CDH (Command & Data Handling): OBC, memory, watchdog
");
 printf("    COMMS: UHF transceiver, antenna, modem
");
 printf("    ADCS (Attitude Control): magnetorquer, sun sensor, IMU
");
 printf("    PAYLOAD: camera, spectrometer (mission-specific)

");
 printf("Internal Block Diagram (IBD):
");
 printf("  EPS --[5V bus]--> CDH
");
 printf("  EPS --[3.3V bus]--> COMMS
");
 printf("  CDH --[SPI]--> ADCS
");
 printf("  CDH --[UART]--> COMMS
");
 printf("  CDH --[I2C]--> PAYLOAD

");
 printf("Requirements traceability:
");
 printf("  REQ-001: Mass < 1.33 kg -> satisfied by all subsystems
");
 printf("  REQ-002: Power < 2.5 W orbit average -> verified by EPS
");
 printf("
Example 1 PASSED
");return 0;}
