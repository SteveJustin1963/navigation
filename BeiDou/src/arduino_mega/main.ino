// ─────────────────────────────────────────────────────────────────────────────
// BeiDou / Multi-GNSS — Arduino Mega 2560 + u-blox M8/M9/M10
//
// Wiring (uses hardware Serial1 — more reliable than SoftwareSerial):
//   u-blox VCC → 3.3V    u-blox GND → GND
//   u-blox TX  → Mega pin 19  (RX1 — no level shifter needed)
//   u-blox RX  → Mega pin 18  (TX1 — use voltage divider or TXB0104!)
//
// Level shifter for TX1→u-blox RX (5V→3.3V):
//   Pin18 ──[ 1kΩ ]──┬── u-blox RX
//                    [ 2kΩ ]
//                    GND
//
// Mega has 8KB RAM — can run at 38400 baud and handle more satellites.
// Arduino IDE: Board = "Arduino Mega or Mega 2560"
// ─────────────────────────────────────────────────────────────────────────────

#include "ubx_config.h"
#include "nmea_parser.h"

// Mega has Serial1 on pins 18/19 — no SoftwareSerial needed
#define gnssSerial Serial1

NMEAParser gnss;
uint32_t   lastPrint = 0;

void setup() {
    Serial.begin(115200);
    Serial.println(F("[BeiDou/Mega] Starting..."));

    gnssSerial.begin(9600);
    delay(1000);

    Serial.println(F("[BeiDou] Enabling GPS + BeiDou..."));
    UBX::enableGPS_BeiDou(gnssSerial);

    // Mega can handle 38400 reliably
    UBX::setBaud(gnssSerial, 38400);
    gnssSerial.end();
    delay(100);
    gnssSerial.begin(38400);

    UBX::saveConfig(gnssSerial);
    Serial.println(F("[BeiDou] Ready. Cold start: 30-90s outdoors.\n"));
}

void loop() {
    while (gnssSerial.available())
        gnss.feed((char)gnssSerial.read());

    if (millis() - lastPrint >= 2000) {
        lastPrint = millis();
        printFix(gnss.fix);
    }
}

void printPadded(uint8_t v) { if (v < 10) Serial.print('0'); Serial.print(v); }

void printFix(const GNSSFix &f) {
    Serial.println(F("─────────────────────────────"));
    if (!f.valid) {
        Serial.print(F("[BeiDou] No fix | BD sats: "));
        Serial.println(f.bdSatsInView);
        return;
    }
    Serial.print(F("Quality : ")); Serial.println(qualityStr(f.fixQuality));
    Serial.print(F("Lat     : ")); Serial.println(f.lat, 7);
    Serial.print(F("Lon     : ")); Serial.println(f.lon, 7);
    Serial.print(F("Alt     : ")); Serial.print(f.altM, 1); Serial.println(F(" m"));
    Serial.print(F("Sats    : ")); Serial.print(f.satellites);
    Serial.print(F(" | BD in view: ")); Serial.println(f.bdSatsInView);
    Serial.print(F("HDOP    : ")); Serial.println(f.hdop, 1);
    Serial.print(F("Speed   : ")); Serial.print(f.speedKnots, 2);
    Serial.print(F(" kn (")); Serial.print(f.speedKnots * 1.852, 1); Serial.println(F(" km/h)"));
    Serial.print(F("UTC     : "));
    printPadded(f.hour); Serial.print(':'); printPadded(f.minute); Serial.print(':'); printPadded(f.second);
    Serial.print(F("  ")); printPadded(f.day); Serial.print('/'); printPadded(f.month); Serial.print('/'); Serial.println(f.year);
    Serial.print(F("BeiDou  : ")); Serial.println(f.beidouSeen ? F("YES") : F("no"));
}

const __FlashStringHelper* qualityStr(uint8_t q) {
    switch (q) {
        case 1: return F("GPS/GNSS fix");
        case 2: return F("DGPS");
        case 4: return F("RTK Fixed (cm!)");
        case 5: return F("RTK Float");
        default: return F("No fix");
    }
}
