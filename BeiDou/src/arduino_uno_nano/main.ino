// ─────────────────────────────────────────────────────────────────────────────
// BeiDou / Multi-GNSS — Arduino Uno / Nano + u-blox M8/M9/M10
//
// Wiring:
//   u-blox VCC → 3.3V    u-blox GND → GND
//   u-blox TX  → D4  (SoftwareSerial RX — no level shifter needed)
//   u-blox RX  → D3  (SoftwareSerial TX — use voltage divider or TXB0104!)
//
// Level shifter for D3→u-blox RX (5V→3.3V):
//   D3 ──[ 1kΩ ]──┬── u-blox RX
//                 [ 2kΩ ]
//                 GND
//
// IMPORTANT: Uno/Nano only has 2KB RAM. Baud stays at 9600 (SoftwareSerial limit).
// Arduino IDE: Board = "Arduino Uno" or "Arduino Nano"
// ─────────────────────────────────────────────────────────────────────────────

#include <SoftwareSerial.h>
#include "ubx_config.h"
#include "nmea_parser.h"

SoftwareSerial gnssSerial(4, 3); // RX=D4, TX=D3

NMEAParser gnss;
uint32_t   lastPrint = 0;

void setup() {
    Serial.begin(115200);
    Serial.println(F("[BeiDou/Uno] Starting..."));

    gnssSerial.begin(9600);
    delay(1000);

    Serial.println(F("[BeiDou] Enabling GPS + BeiDou..."));
    UBX::enableGPS_BeiDou(gnssSerial);
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
