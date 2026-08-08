#include "src/command_parser.h"
#include "src/control_loop.h"
#include "src/sensors.h"

// ---------------------------------------------------------
// Command format (final):
//
// CFG_WATER_SCHEDULE:<hours>,<quantity_ml>
// CFG_WATER_THRESHOLD:<moisture_pct>
// CMD_RESET
// CMD_REQUEST_STATUS
// ---------------------------------------------------------

// Utility: trim + uppercase
static String normalize(const String &s) {
    String out = s;
    out.trim();
    out.toUpperCase();
    return out;
}

// ---------------------------------------------------------
// Main parser
// ---------------------------------------------------------
void command_parse(const String &cmdRaw) {
    String cmd = normalize(cmdRaw);

    // --- Schedule config ---
    if (cmd.startsWith("CFG_WATER_SCHEDULE")) {
        int colon = cmd.indexOf(':');
        if (colon < 0) return;

        String payload = cmd.substring(colon + 1);
        int comma = payload.indexOf(',');
        if (comma < 0) return;

        uint32_t hours = payload.substring(0, comma).toInt();
        uint8_t quantity_ml = payload.substring(comma + 1).toInt();

        apply_schedule_config(hours, quantity_ml);
        return;
    }

    // --- Threshold config ---
    if (cmd.startsWith("CFG_WATER_THRESHOLD")) {
        int colon = cmd.indexOf(':');
        if (colon < 0) return;

        uint8_t moisture_pct = cmd.substring(colon + 1).toInt();
        apply_threshold_config(moisture_pct);
        return;
    }

    // --- Reset ---
    if (cmd == "CMD_RESET") {
        apply_reset();
        return;
    }

    // --- Status ---
    if (cmd == "CMD_REQUEST_STATUS") {
        send_status();
        return;
    }

    // Unknown command → ignore silently
}

// ---------------------------------------------------------
// Apply schedule mode
// ---------------------------------------------------------
void apply_schedule_config(uint32_t hours, uint8_t quantity_ml) {
    currentMode = MODE_SCHEDULE;
    schedule_hours = hours;
    ::quantity_ml = quantity_ml;

    disable_schedule_timer();
    configure_schedule_timer(hours);
}

// ---------------------------------------------------------
// Apply threshold mode
// ---------------------------------------------------------
void apply_threshold_config(uint8_t moisture_pct) {
    currentMode = MODE_THRESHOLD;
    threshold_pct = moisture_pct;

    disable_schedule_timer();
}

// ---------------------------------------------------------
// Reset system
// ---------------------------------------------------------
void apply_reset() {
    currentMode = MODE_IDLE;
    disable_schedule_timer();
    pump_off();
}

// ---------------------------------------------------------
// Status packet (simple version)
// ---------------------------------------------------------
void send_status() {
    // You will replace this with your actual packet format later
    Serial.print("STATUS:");
    Serial.print("MODE=");
    Serial.print((int)currentMode);
    Serial.print(",SCHED_HRS=");
    Serial.print(schedule_hours);
    Serial.print(",QTY=");
    Serial.print(quantity_ml);
    Serial.print(",THRESH=");
    Serial.print(threshold_pct);
    Serial.print(",SENSORS_OK=");
    Serial.println(sensors_ok_flags());
}
