// ─────────────────────────────────────────────────────────────────────────────
// BeiDou / Multi-GNSS — STM32 (Blue Pill / Nucleo) + u-blox M8/M9/M10
//
// Requires: STM32duino core (Arduino_Core_STM32)
//   Board Manager URL: https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
//
// Wiring — Blue Pill (STM32F103C8T6):
//   u-blox VCC → 3.3V    u-blox GND → GND
//   u-blox TX  → PA10   (USART1 RX — 3.3V native, no level shifter needed)
//   u-blox RX  → PA9    (USART1 TX — 3.3V native, no level shifter needed)
//   u-blox PPS → PB0    (optional)
//
// Wiring — Nucleo boards:
//   Use the same PA9/PA10 pins or check your board's Serial1 mapping.
//
// Arduino IDE: Board = "Generic STM32F1 series" → "BluePill F103C8"
//              or your specific Nucleo variant
// ─────────────────────────────────────────────────────────────────────────────

#include "ubx_config.h"
#include "nmea_parser.h"

// STM32duino: Serial1 = USART1 (PA9=TX, PA10=RX on Blue Pill)
#define gnssSerial Serial1
#define PPS_PIN PB0

NMEAParser gnss;
uint32_t   lastPrint = 0;

void setup() {
    Serial.begin(115200);   // USB CDC serial (or USART via ST-Link)
    delay(1000);
    Serial.println("[BeiDou/STM32] Starting...");

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
        Serial.print("[BeiDou] No fix | BD sats: ");
        Serial.println(f.bdSatsInView);
        return;
    }
    Serial.print("Quality  : "); Serial.println(qualityStr(f.fixQuality));
    Serial.print("Lat      : "); Serial.println(f.lat, 7);
    Serial.print("Lon      : "); Serial.println(f.lon, 7);
    Serial.print("Alt      : "); Serial.print(f.altM, 1); Serial.println(" m");
    Serial.print("Sats     : "); Serial.print(f.satellites);
    Serial.print(" | BD in view: "); Serial.println(f.bdSatsInView);
    Serial.print("HDOP     : "); Serial.println(f.hdop, 1);
    Serial.print("Speed    : "); Serial.print(f.speedKnots, 2);
    Serial.print(" kn ("); Serial.print(f.speedKnots * 1.852f, 1); Serial.println(" km/h)");
    char buf[32];
    snprintf(buf, sizeof(buf), "UTC      : %02d:%02d:%02d  %02d/%02d/%04d",
             f.hour, f.minute, f.second, f.day, f.month, f.year);
    Serial.println(buf);
    Serial.print("BeiDou   : "); Serial.println(f.beidouSeen ? "YES" : "no");
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
