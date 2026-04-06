// ─────────────────────────────────────────────────────────────────────────────
// BeiDou / Multi-GNSS — nRF52840 (Adafruit Feather / XIAO) + u-blox M8/M9/M10
//
// Requires: Adafruit nRF52 core
//   Board Manager URL: https://adafruit.github.io/arduino-board-index/package_adafruit_index.json
//
// Wiring — Adafruit Feather nRF52840 Express:
//   u-blox VCC → 3.3V    u-blox GND → GND
//   u-blox TX  → Pin RX  (Serial1 RX — 3.3V native)
//   u-blox RX  → Pin TX  (Serial1 TX — 3.3V native)
//   u-blox PPS → Pin 5   (optional)
//
// Wiring — Seeed XIAO nRF52840:
//   u-blox TX  → D7 (RX)
//   u-blox RX  → D6 (TX)
//
// Arduino IDE: Board = "Adafruit Feather nRF52840 Express"
//              or "Seeed XIAO nRF52840"
//
// BLE bonus: GNSS data can be streamed over BLE Nordic UART Service (NUS)
// ─────────────────────────────────────────────────────────────────────────────

#include <bluefruit.h>          // Adafruit nRF52 BLE library
#include "ubx_config.h"
#include "nmea_parser.h"

#define gnssSerial Serial1
#define PPS_PIN 5

// ── BLE Nordic UART Service (streams GNSS data over BLE) ─────────────────────
BLEUart bleuart;

NMEAParser gnss;
uint32_t   lastPrint = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("[BeiDou/nRF52840] Starting...");

    // ── BLE setup ──────────────────────────────────────────────────────────
    Bluefruit.begin();
    Bluefruit.setName("BeiDou-GNSS");
    bleuart.begin();
    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    Bluefruit.Advertising.addTxPower();
    Bluefruit.Advertising.addService(bleuart);
    Bluefruit.Advertising.start(0); // advertise indefinitely
    Serial.println("[BLE] Advertising as 'BeiDou-GNSS'");

    // ── GNSS setup ─────────────────────────────────────────────────────────
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

    // Build a compact string for both Serial and BLE
    char line[80];
    snprintf(line, sizeof(line), "%.7f,%.7f,%.1f,%d,%d",
             f.lat, f.lon, f.altM, f.satellites, f.bdSatsInView);

    Serial.printf("Quality  : %s\n",    qualityStr(f.fixQuality));
    Serial.printf("Lat/Lon  : %.7f, %.7f\n", f.lat, f.lon);
    Serial.printf("Alt      : %.1f m\n", f.altM);
    Serial.printf("Sats     : %d used | BD in view: %d\n", f.satellites, f.bdSatsInView);
    Serial.printf("HDOP     : %.1f\n",  f.hdop);
    Serial.printf("Speed    : %.2f kn\n", f.speedKnots);
    Serial.printf("UTC      : %02d:%02d:%02d  %02d/%02d/%04d\n",
                  f.hour, f.minute, f.second, f.day, f.month, f.year);
    Serial.printf("BeiDou   : %s\n",   f.beidouSeen ? "YES" : "no");

    // Stream over BLE UART if connected
    if (Bluefruit.connected()) {
        bleuart.println(line);
    }
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
