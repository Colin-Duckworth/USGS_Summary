// AD8302 reflectometer — Arduino UNO R4 Minima
// RFSoC is master; Arduino is slave.
// All communication via UART on Serial1 (D0/D1). See comms.h for protocol.

#include "config.h"
#include "adc.h"
#include "osl.h"
#include "comms.h"
#include "switch.h"

// ── State machine ─────────────────────────────────────────────────────────────
enum State {
    STATE_IDLE,
    STATE_OSL,          // collecting SHORT → OPEN → LOAD autonomously
    STATE_GAMMA_DELAY,  // first sweep: SPDT on delay line
    STATE_GAMMA_THRU    // second sweep: SPDT on thru path
};

static State state    = STATE_IDLE;
static int   freq_idx = 0;
static int   osl_std  = 0;   // 0=SHORT, 1=OPEN, 2=LOAD

// ── Entry points ──────────────────────────────────────────────────────────────
void setup(void)
{
    RFSOC_SERIAL.begin(UART_BAUD);
    adc_init();
    switch_init();
}

void loop(void)
{
    char buf[32];
    if (!comms_readLine(buf, sizeof(buf))) return;

    double  freq_hz = 0.0;
    Command cmd     = comms_parseCommand(buf, &freq_hz);

    switch (cmd) {

        // ── OSL calibration ───────────────────────────────────────────────────
        case CMD_OSL_BEGIN:
            state    = STATE_OSL;
            osl_std  = 0;
            freq_idx = 0;
            switch1_thru();   // force thru path — SOL must match the path used for the final gamma magnitude
            switch2_short();
            comms_sendACK();
            break;

        // ── Gamma characterization ────────────────────────────────────────────
        case CMD_GAMMA_BEGIN:
            osl_load_from_eeprom();
            switch2_antenna();
            switch1_delay();
            state    = STATE_GAMMA_DELAY;
            freq_idx = 0;
            comms_sendACK();
            break;

        // ── Per-frequency measurement ─────────────────────────────────────────
        case CMD_FREQ: {
            float gain_dB, phase_deg, vmag_mV, vphs_mV;

            if (state == STATE_OSL)
            {
                if (freq_idx >= N_CAL_POINTS) { comms_sendNACK(); break; }
                osl_store_sample(osl_std, freq_idx++, freq_hz);

                if (freq_idx == N_CAL_POINTS) {
                    freq_idx = 0;
                    osl_std++;
                    if      (osl_std == 1) { switch2_open(); delay(STEP_DELAY_MS); comms_sendACK(); }
                    else if (osl_std == 2) { switch2_load(); delay(STEP_DELAY_MS); comms_sendACK(); }
                    else {
                        // all three standards collected
                        osl_compute_error_terms();
                        delay(STEP_DELAY_MS);
                        comms_sendCalDone();
                        state = STATE_IDLE;
                    }
                    // Manual standard-swap window — ACK/CAL_DONE already sent above, so
                    // this doesn't risk the RFSoC's 5s ACK_TIMEOUT_S. Any FREQ command the
                    // RFSoC sends during this delay just sits in the UART FIFO until we get
                    // back to comms_readLine().
                    delay(20000);
                } else {
                    delay(STEP_DELAY_MS);
                    comms_sendACK();
                }
            }
            else if (state == STATE_GAMMA_DELAY)
            {
                if (freq_idx >= N_SWEEP_POINTS) { comms_sendNACK(); break; }
                delay(DELAY_RF_SETTLE_MS);
                delayMicroseconds(DELAY_AD8302_US);
                sample(&gain_dB, &phase_deg, &vmag_mV, &vphs_mV);
                gamma_delay_raw[freq_idx].gain_dB   = gain_dB;
                gamma_delay_raw[freq_idx].phase_deg = phase_deg;
                freq_idx++;
                delay(STEP_DELAY_MS);
                comms_sendACK();

                if (freq_idx == N_SWEEP_POINTS) {
                    switch1_thru();
                    state    = STATE_GAMMA_THRU;
                    freq_idx = 0;
                }
            }
            else if (state == STATE_GAMMA_THRU)
            {
                if (freq_idx >= N_SWEEP_POINTS) { comms_sendNACK(); break; }
                delay(DELAY_RF_SETTLE_MS);
                delayMicroseconds(DELAY_AD8302_US);
                sample(&gain_dB, &phase_deg, &vmag_mV, &vphs_mV);
                gamma_thru_raw[freq_idx].gain_dB   = gain_dB;
                gamma_thru_raw[freq_idx].phase_deg = phase_deg;
                freq_idx++;
                delay(STEP_DELAY_MS);
                comms_sendACK();

                if (freq_idx == N_SWEEP_POINTS) {
                    osl_apply_corrections();
                    delay(STEP_DELAY_MS);
                    comms_sendSweepData();
                    state    = STATE_IDLE;
                    freq_idx = 0;
                }
            }
            else
            {
                comms_sendNACK();
            }
            break;
        }

        default:
            comms_sendNACK();
            break;
    }
}
