// ─────────────────────────────────────────────────────────────────────────────
// BeiDou / Multi-GNSS — Teensy 3.x / 4.x + u-blox M8/M9/M10
//
// Teensy is the best platform for high-update-rate / RTK use:
// - Up to 7 hardware UARTs (Serial1–Serial7)
// - 3.3V native on all pins — no level shifter needed
// - 600MHz (T4.1) — can handle 115200 baud + heavy processing
//
// Wiring — Teensy 4.0 / 4.1 (Serial1):
//   u-blox VCC → 3.3V    u-blox GND → GND
//   u-blox TX  → Pin 0  (RX1 — 3.3V native)
//   u-blox RX  → Pin 1  (TX1 — 3.3V native)
//   u-blox PPS → Pin 2  (optional)
//
// Wiring — Teensy 3.2 / 3.5 / 3.6:
//   Same pinout — Pin 0 = RX1, Pin 1 = TX1
//
// Arduino IDE: Install Teensyduino from https://www.pjrc.com/teensy/teensyduino.html
//              Board = "Teensy 4.0" (or your version)
// ─────────────────────────────────────────────────────────────────────────────

#include "ubx_config.h"
#include "nmea_parser.h"

#define gnssSerial Serial1
#define PPS_PIN 2

// Teensy can handle 115200 baud reliably
#define GNSS_FAST_BAUD 115200

NMEAParser gnss;
uint32_t   lastPrint = 0;

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("[BeiDou/Teensy] Starting...");

    gnssSerial.begin(9600);
    delay(1000);

    Serial.println("[BeiDou] Enabling GPS + BeiDou...");
    UBX::enableGPS_BeiDou(gnssSerial);

    // Teensy handles 115200 easily — maximize throughput
    UBX::setBaud(gnssSerial, GNSS_FAST_BAUD);
    gnssSerial.end();
    delay(100);
    gnssSerial.begin(GNSS_FAST_BAUD);

    UBX::saveConfig(gnssSerial);
    Serial.println("[BeiDou] Ready. Cold start: 30-90s outdoors.\n");

    pinMode(PPS_PIN, INPUT);
}

void loop() {
    while (gnssSerial.available())
        gnss.feed((char)gnssSerial.read());

    if (millis() - lastPrint >= 2000) {
        lastPrint = millis();
        printFix(gnss.fix);
    }
}

void printFix(const GNSSFix &f) {
    Serial.println("─────────────────────────────────");
    if (!f.valid) {
        Serial.printf("[BeiDou] No fix | BD sats in view: %d\n", f.bdSatsInView);
        return;
    }
    Serial.printf("Quality  : %s\n",    qualityStr(f.fixQuality));
    Serial.printf("Lat      : %.7f\n",  f.lat);
    Serial.printf("Lon      : %.7f\n",  f.lon);
    Serial.printf("Alt      : %.1f m\n", f.altM);
    Serial.printf("Sats     : %d used | BD in view: %d\n", f.satellites, f.bdSatsInView);
    Serial.printf("HDOP     : %.1f\n",  f.hdop);
    Serial.printf("Speed    : %.2f kn (%.2f km/h)\n", f.speedKnots, f.speedKnots * 1.852f);
    Serial.printf("Course   : %.1f\n",  f.courseDeg);
    Serial.printf("UTC      : %02d:%02d:%02d  %02d/%02d/%04d\n",
                  f.hour, f.minute, f.second, f.day, f.month, f.year);
    Serial.printf("BeiDou   : %s\n",   f.beidouSeen ? "YES" : "no");
}

const char* qualityStr(uint8_t q) {
    switch (q) {
        case 1: return "GPS/GNSS fix";
        case 2: return "DGPS";
        case 4: return "RTK Fixed (cm!)";
        case 5: return "RTK Float";
        default: return "No fix";
    }
}
