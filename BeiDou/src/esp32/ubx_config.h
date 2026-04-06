#pragma once
#include <Arduino.h>

// ─────────────────────────────────────────────────────────────────────────────
// UBX-CFG-GNSS: Enable GPS + BeiDou on u-blox M8/M9/M10
// Uses Stream& — compatible with HardwareSerial, SoftwareSerial, SerialUSB, etc.
// ─────────────────────────────────────────────────────────────────────────────

namespace UBX {

static void addChecksum(uint8_t *frame, uint8_t payloadLen) {
    uint8_t ckA = 0, ckB = 0;
    uint8_t total = 4 + payloadLen;
    for (uint8_t i = 2; i < 2 + total; i++) {
        ckA += frame[i];
        ckB += ckA;
    }
    frame[2 + total]     = ckA;
    frame[2 + total + 1] = ckB;
}

static void send(Stream &port, uint8_t *frame, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) port.write(frame[i]);
}

// Enable GPS (id=0) + BeiDou (id=3), disable SBAS / Galileo / QZSS / GLONASS
static void enableGPS_BeiDou(Stream &port) {
    uint8_t frame[60] = {
        0xB5, 0x62,
        0x06, 0x3E,
        0x34, 0x00,             // payload = 52 bytes
        0x00, 0x00, 0xFF, 0x06, // msgVer, numTrkChHw, numTrkChUse, numBlocks=6

        // gnssId  resTrkCh  maxTrkCh  reserved  flags[4] (LE)  bit0=enable
        0x00, 0x08, 0x10, 0x00,  0x01, 0x00, 0x01, 0x01, // GPS     ON
        0x01, 0x01, 0x03, 0x00,  0x00, 0x00, 0x01, 0x01, // SBAS    off
        0x02, 0x04, 0x08, 0x00,  0x00, 0x00, 0x01, 0x01, // Galileo off
        0x03, 0x08, 0x10, 0x00,  0x01, 0x00, 0x01, 0x01, // BeiDou  ON
        0x05, 0x00, 0x03, 0x00,  0x00, 0x00, 0x01, 0x01, // QZSS    off
        0x06, 0x08, 0x0E, 0x00,  0x00, 0x00, 0x01, 0x01, // GLONASS off

        0x00, 0x00 // checksum (filled in below)
    };
    addChecksum(frame, 52);
    send(port, frame, sizeof(frame));
    delay(500);
}

// Save config to module flash — survives power cycle
static void saveConfig(Stream &port) {
    uint8_t frame[21] = {
        0xB5, 0x62,
        0x06, 0x09,
        0x0D, 0x00,
        0x00, 0x00, 0x00, 0x00, // clearMask (nothing)
        0xFF, 0xFF, 0x00, 0x00, // saveMask  (all)
        0x00, 0x00, 0x00, 0x00, // loadMask  (nothing)
        0x17,                   // deviceMask (BBR+Flash+EEPROM+SPI)
        0x00, 0x00
    };
    addChecksum(frame, 13);
    send(port, frame, sizeof(frame));
    delay(300);
}

// Change u-blox UART baud rate. Call Serial.begin(newBaud) right after.
static void setBaud(Stream &port, uint32_t baud) {
    uint8_t frame[28] = {
        0xB5, 0x62,
        0x06, 0x00,
        0x14, 0x00,
        0x01, 0x00, 0x00, 0x00, // portId=UART1, reserved, txReady
        0xD0, 0x08, 0x00, 0x00, // mode (8N1)
        (uint8_t)(baud),
        (uint8_t)(baud >> 8),
        (uint8_t)(baud >> 16),
        (uint8_t)(baud >> 24),
        0x07, 0x00,             // inProtoMask  (UBX+NMEA+RTCM)
        0x03, 0x00,             // outProtoMask (UBX+NMEA)
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00
    };
    addChecksum(frame, 20);
    send(port, frame, sizeof(frame));
    delay(100);
}

} // namespace UBX
