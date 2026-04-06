// ─────────────────────────────────────────────────────────────────────────────
// BeiDou / Multi-GNSS — Raspberry Pi Pico (RP2040) + u-blox M8/M9/M10
//
// Requires: Arduino-Pico core by Earle Philhower
//   https://github.com/earlephilhower/arduino-pico
//   Board Manager URL: https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
//
// Wiring (3.3V native — NO level shifter needed):
//   u-blox VCC → 3V3(OUT) pin    u-blox GND → GND
//   u-blox TX  → GP1  (UART0 RX)
//   u-blox RX  → GP0  (UART0 TX)
//   u-blox PPS → GP2  (optional)
//
// Arduino IDE: Board = "Raspberry Pi Pico"
// ─────────────────────────────────────────────────────────────────────────────

#include "ubx_config.h"
#include "nmea_parser.h"

// RP2040 UART0: TX=GP0, RX=GP1
// Serial1 maps to UART0 in arduino-pico core
#define gnssSerial Serial1
#define PPS_PIN 2

NMEAParser gnss;
uint32_t   lastPrint = 0;

void setup() {
    Serial.begin(115200);
    delay(2000); // wait for USB serial to connect
    Serial.println("[BeiDou/Pico] Starting...");

    // Set UART0 pins explicitly for arduino-pico
    Serial1.setRX(1);
    Serial1.setTX(0);
    gnssSerial.begin(9600);
    delay(1000);

    Serial.println("[BeiDou] Enabling GPS + BeiDou...");
    UBX::enableGPS_BeiDou(gnssSerial);

    UBX::setBaud(gnssSerial, 38400);
    gnssSerial.end();
    delay(100);
    gnssSerial.begin(38400);

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
    Serial.printf("Course   : %.1f°\n", f.courseDeg);
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
