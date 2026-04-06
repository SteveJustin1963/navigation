// ─────────────────────────────────────────────────────────────────────────────
// BeiDou / Multi-GNSS — ESP32 + u-blox M8/M9/M10
//
// Wiring:
//   u-blox VCC → 3.3V    u-blox GND → GND
//   u-blox TX  → GPIO16  (Serial2 RX)
//   u-blox RX  → GPIO17  (Serial2 TX)  [3.3V native — no level shifter needed]
//   u-blox PPS → GPIO4   (optional, 1Hz timing pulse)
//
// Arduino IDE: Board = "ESP32 Dev Module"
// ─────────────────────────────────────────────────────────────────────────────

#include "ubx_config.h"
#include "nmea_parser.h"

#define GNSS_RX  16
#define GNSS_TX  17
#define PPS_PIN   4

NMEAParser gnss;
uint32_t   lastPrint = 0;

void setup() {
    Serial.begin(115200);
    Serial.println("\n[BeiDou/ESP32] Initializing...");

    Serial2.begin(9600, SERIAL_8N1, GNSS_RX, GNSS_TX);
    delay(1000);

    Serial.println("[BeiDou] Enabling GPS + BeiDou...");
    UBX::enableGPS_BeiDou(Serial2);

    // Bump to 38400 for faster NMEA throughput
    UBX::setBaud(Serial2, 38400);
    Serial2.end();
    delay(100);
    Serial2.begin(38400, SERIAL_8N1, GNSS_RX, GNSS_TX);

    UBX::saveConfig(Serial2);
    Serial.println("[BeiDou] Ready. Cold start: 30-90s outdoors.\n");

    pinMode(PPS_PIN, INPUT);
}

void loop() {
    while (Serial2.available())
        gnss.feed((char)Serial2.read());

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
