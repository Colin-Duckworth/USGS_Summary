#pragma once

void switch_init(void);

// SPDT — selects direct or delay-line path for phase ambiguity resolution
void switch1_thru(void);     // direct path     (C1=D10 LOW,  C2=D11 HIGH)
void switch1_delay(void);    // delay-line path (C1=D10 HIGH, C2=D11 LOW)

// SP6T — selects which port is routed into the reflectometer signal path
void switch2_antenna(void);  // RF1: antenna (measurement mode)
void switch2_open(void);     // RF2: OPEN calibration standard
void switch2_short(void);    // RF3: SHORT calibration standard
void switch2_load(void);     // RF4: LOAD (50 Ω) calibration standard
// RF5, RF6: terminated, unused
