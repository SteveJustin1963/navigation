// ─────────────────────────────────────────────────────────────────────────────
// BeiDou / Multi-GNSS — SAMD21 (Arduino Zero / MKR boards) + u-blox M8/M9/M10
//
// Wiring — Arduino Zero:
//   u-blox VCC → 3.3V    u-blox GND → GND
//   u-blox TX  → Pin 0  (Serial1 RX — 3.3V native, no level shifter needed)
//   u-blox RX  → Pin 1  (Serial1 TX — 3.3V native, no level shifter needed)
//   u-blox PPS → Pin 4  (optional)
//
// Wiring — Arduino MKR boards (MKR WiFi 1010, MKR Zero, etc.):
//   Same pinout — Pin 13 = RX, Pin 14 = TX on MKR series
//   Check: https://docs.arduino.cc/hardware/mkr-wifi-1010
//
// NOTE: Arduino Zero has TWO USB ports:
//   - "Programming" port: use for uploading + Serial monitor (SerialUSB)
//   - "Native" port: SerialUSB on that port for production use
//
// Arduino IDE: Board = "Arduino Zero (Programming Port)"
//              or your MKR variant
// ─────────────────────────────────────────────────────────────────────────────

#include "ubx_config.h"
#include "nmea_parser.h"

// SAMD21 Arduino Zero: Serial1 on pins 0(RX)/1(TX)
// MKR boards: Serial1 on pins 13(RX)/14(TX) — same code works
#define gnssSerial Serial1
#define PPS_PIN 4

// SerialUSB for Zero's programming port, Serial for MKR boards
#define DEBUG_SERIAL SerialUSB

NMEAParser gnss;
uint32_t   lastPrint = 0;

void setup() {
    DEBUG_SERIAL.begin(115200);
    while (!DEBUG_SERIAL && millis() < 3000); // wait for USB (up to 3s)
    DEBUG_SERIAL.println("[BeiDou/SAMD21] Starting...");

    gnssSerial.begin(9600);
    delay(1000);

    DEBUG_SERIAL.println("[BeiDou] Enabling GPS + BeiDou...");
    UBX::enableGPS_BeiDou(gnssSerial);

    UBX::setBaud(gnssSerial, 38400);
    gnssSerial.end();
    delay(100);
    gnssSerial.begin(38400);

    UBX::saveConfig(gnssSerial);
    DEBUG_SERIAL.println("[BeiDou] Ready. Cold start: 30-90s outdoors.\n");

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

void printPadded(uint8_t v) { if (v < 10) DEBUG_SERIAL.print('0'); DEBUG_SERIAL.print(v); }

void printFix(const GNSSFix &f) {
    DEBUG_SERIAL.println("─────────────────────────────────");
    if (!f.valid) {
        DEBUG_SERIAL.print("[BeiDou] No fix | BD sats: ");
        DEBUG_SERIAL.println(f.bdSatsInView);
        return;
    }
    DEBUG_SERIAL.print("Quality  : "); DEBUG_SERIAL.println(qualityStr(f.fixQuality));
    DEBUG_SERIAL.print("Lat      : "); DEBUG_SERIAL.println(f.lat, 7);
    DEBUG_SERIAL.print("Lon      : "); DEBUG_SERIAL.println(f.lon, 7);
    DEBUG_SERIAL.print("Alt      : "); DEBUG_SERIAL.print(f.altM, 1); DEBUG_SERIAL.println(" m");
    DEBUG_SERIAL.print("Sats     : "); DEBUG_SERIAL.print(f.satellites);
    DEBUG_SERIAL.print(" | BD in view: "); DEBUG_SERIAL.println(f.bdSatsInView);
    DEBUG_SERIAL.print("HDOP     : "); DEBUG_SERIAL.println(f.hdop, 1);
    DEBUG_SERIAL.print("Speed    : "); DEBUG_SERIAL.print(f.speedKnots, 2);
    DEBUG_SERIAL.print(" kn ("); DEBUG_SERIAL.print(f.speedKnots * 1.852f, 1); DEBUG_SERIAL.println(" km/h)");
    DEBUG_SERIAL.print("UTC      : ");
    printPadded(f.hour); DEBUG_SERIAL.print(':');
    printPadded(f.minute); DEBUG_SERIAL.print(':');
    printPadded(f.second); DEBUG_SERIAL.print("  ");
    printPadded(f.day); DEBUG_SERIAL.print('/');
    printPadded(f.month); DEBUG_SERIAL.print('/');
    DEBUG_SERIAL.println(f.year);
    DEBUG_SERIAL.print("BeiDou   : "); DEBUG_SERIAL.println(f.beidouSeen ? "YES" : "no");
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
