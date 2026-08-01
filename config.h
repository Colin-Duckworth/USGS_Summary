#pragma once
#include <Arduino.h>
#include <EEPROM.h>

// ── Frequency plan ────────────────────────────────────────────────────────────
// OSL calibration: fixed 5 MHz grid, flight-invariant
#define F_CAL_MIN_HZ      40000000.0
#define F_CAL_MAX_HZ     600000000.0
#define F_CAL_STEP_HZ      5000000.0
#define N_CAL_POINTS             113   // (600-40)/5 + 1

// Gamma sweep: 1 MHz grid, driven by RFSoC FREQ commands
#define F_SWEEP_MIN_HZ    40000000.0
#define F_SWEEP_MAX_HZ   600000000.0
#define F_SWEEP_STEP_HZ    1000000.0
#define N_SWEEP_POINTS           561   // (600-40)/1 + 1

// ── Pin assignments ───────────────────────────────────────────────────────────
#define VMAG_PIN  A0
#define VPHS_PIN  A1

// SPDT (switch1): phase ambiguity resolution — experimentally verified
// C1=D10, C2=D11.  C1=0,C2=1 → thru (short, non-delayed cable)
//                  C1=1,C2=0 → delay (longer, delayed cable)
//                  C1=C2 (00 or 11) → undefined, never drive this
#define SPDT_A_PIN  10
#define SPDT_B_PIN  11

// SP6T (switch2): calibration standard / antenna selection — experimentally verified
// Control lines: C1=D5, C2=D6, C3=D7
// C1 C2 C3 → port
// 0  0  0  → antenna
// 1  0  1  → short
// 1  1  0  → open
// 1  0  0  → load (50 Ω)
// all other combinations → terminated ports, never activate
#define SP6T_C1_PIN  5
#define SP6T_C2_PIN  6
#define SP6T_C3_PIN  7

// ── ADC configuration ─────────────────────────────────────────────────────────
// R4 Minima uses AR_EXTERNAL — the AD8302 eval board provides stable ~2.4 V on AREF.
// Measure actual AREF with a DMM and update AREF_MV to zero any gain offset.
#define AREF_MV         1852.0f
#define ADC_RESOLUTION  1024.0f   // 10-bit Arduino default mode
#define N_SAMPLES           32    // readings taken per channel per measurement
#define TRIM                 8    // discard lowest 8 and highest 8; average middle 16

// ── Settling delays ───────────────────────────────────────────────────────────
// All values are 2× the worst-case datasheet spec for margin.
// SP6T (JSW6-33DR+):  datasheet 2 µs → 4 µs applied after any switch2_*() call
// SPDT (ZMSW-1211):   datasheet 5 µs → 10 µs applied after any switch1_*() call
// AD8302:             VPHS 120° settle to 1% = 500 ns → 1 µs applied before each sample
#define DELAY_SP6T_US    4
#define DELAY_SPDT_US   10
#define DELAY_AD8302_US  1
#define ADC_SAMPLE_DELAY_US  50   // deliberate spacing between each of the N_SAMPLES readings

// RF signal path settling — RFSoC DAC output + analog chain need real time
// to stabilize after a new tone starts, separate from the AD8302's own
// ~1us internal settling (DELAY_AD8302_US above). Tune empirically —
// starting at 10ms.
#define DELAY_RF_SETTLE_MS  10

// ── Bench probing delay ───────────────────────────────────────────────────────
// Bench-debug only: holds state after each measurement (tone loaded, switch
// settled, sample taken) before acknowledging, so the output can be probed
// on a scope. Must stay under the RFSoC script's ACK_TIMEOUT_S (5 s) or every
// step will read back as a timeout. Set to 0 for normal/flight operation.
#define STEP_DELAY_MS 1

// ── Phase ambiguity resolution ────────────────────────────────────────────────
// Characterised with VNA. Update if delay line is changed.
#define DELAY_DEG_PER_MHZ  0.25f   // degrees of extra phase added by SPDT delay path per MHz

// ── AD8302 transfer-function constants ────────────────────────────────────────
#define MAG_SLOPE_MV_PER_DB   29.0f
#define MAG_CENTER_MV        900.0f
#define PHS_SLOPE_MV_PER_DEG  10.0f
#define PHS_AT_0DEG_MV      1800.0f

// ── UART ──────────────────────────────────────────────────────────────────────
#define UART_BAUD     160979
#define RFSOC_SERIAL  Serial1   // hardware UART on D0 (RX) / D1 (TX)

// ── EEPROM ────────────────────────────────────────────────────────────────────
// error_terms[N_CAL_POINTS] occupies (113 * 24) = 2712 bytes starting at addr 0.
// R4 Minima EEPROM emulation provides 8 KB — well within budget.
#define CAL_EEPROM_ADDR  0

// Raw SOL gain/phase, per standard per frequency — for post-hoc debugging after
// disconnecting from the RFSoC. 3 standards * N_CAL_POINTS * sizeof(RawSample)
// = 3 * 113 * 8 = 2712 bytes, placed immediately after the error_terms block.
// Total EEPROM use: 5424 bytes, still well within the 8 KB budget.
#define OSL_RAW_EEPROM_ADDR  (CAL_EEPROM_ADDR + sizeof(ErrorTerms) * N_CAL_POINTS)

// ── Data structures ───────────────────────────────────────────────────────────
struct GammaPoint {
    float gamma_mag;
    float gamma_phase_deg;
    // frequency reconstructed from index: F_SWEEP_MIN_HZ + i * F_SWEEP_STEP_HZ
};

struct ErrorTerms {
    float e00_re,     e00_im;
    float e11_re,     e11_im;
    float delta_e_re, delta_e_im;
};

struct RawSample {
    float gain_dB;
    float phase_deg;
};

// ── is_mirrored bit array ─────────────────────────────────────────────────────
// One bit per sweep point. ceil(561/8) = 71 bytes.
#define IS_MIRRORED_BYTES  ((N_SWEEP_POINTS + 7) / 8)
#define IS_MIRRORED_SET(arr, i)  ((arr)[(i)/8] |=  (1u << ((i)%8)))
#define IS_MIRRORED_GET(arr, i)  (((arr)[(i)/8] >> ((i)%8)) & 1u)

// ── Shared globals (defined in osl.cpp) ───────────────────────────────────────
extern ErrorTerms error_terms[N_CAL_POINTS];
extern GammaPoint sweep_results[N_SWEEP_POINTS];
extern RawSample  gamma_delay_raw[N_SWEEP_POINTS];
extern RawSample  gamma_thru_raw[N_SWEEP_POINTS];
extern uint8_t    is_mirrored[IS_MIRRORED_BYTES];