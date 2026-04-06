#pragma once
#include <Arduino.h>

// ─────────────────────────────────────────────────────────────────────────────
// NMEA Parser — BeiDou ($BD*) and combined GNSS ($GN*) sentences
// Handles: GGA, RMC, GSV
// Portable — works on AVR, ARM, ESP32, RP2040, nRF52, SAMD
// ─────────────────────────────────────────────────────────────────────────────

struct GNSSFix {
    double   lat        = 0.0;
    double   lon        = 0.0;
    float    altM       = 0.0;
    float    hdop       = 99.9;
    float    speedKnots = 0.0;
    float    courseDeg  = 0.0;
    uint8_t  fixQuality = 0;   // 0=none 1=GNSS 2=DGPS 4=RTK-fixed 5=RTK-float
    uint8_t  satellites = 0;
    uint8_t  hour = 0, minute = 0, second = 0;
    uint8_t  day  = 0, month  = 0;
    uint16_t year = 0;
    uint8_t  bdSatsInView = 0;
    bool     valid      = false;
    bool     beidouSeen = false;
};

class NMEAParser {
public:
    GNSSFix fix;

    // Feed one character at a time.
    // Returns true when a complete, valid sentence was parsed.
    bool feed(char c) {
        if (c == '$') { _idx = 0; _buf[_idx++] = c; return false; }
        if (_idx == 0) return false;
        if (c == '\r' || c == '\n') {
            if (_idx < 6) { _idx = 0; return false; }
            _buf[_idx] = '\0';
            bool ok = parse(_buf);
            _idx = 0;
            return ok;
        }
        if (_idx < (int)sizeof(_buf) - 1) _buf[_idx++] = c;
        else _idx = 0; // overflow — reset
        return false;
    }

private:
    char _buf[100];
    int  _idx = 0;

    bool validateChecksum(const char *s) {
        const char *star = strrchr(s, '*');
        if (!star || strlen(star) < 3) return false;
        uint8_t expected = (uint8_t)strtol(star + 1, nullptr, 16);
        uint8_t calc = 0;
        for (const char *p = s + 1; p < star; p++) calc ^= (uint8_t)*p;
        return calc == expected;
    }

    bool getField(const char *s, int n, char *dst, uint8_t dstLen) {
        int field = 0;
        const char *p = s;
        while (*p && field < n) { if (*p++ == ',') field++; }
        if (!*p) return false;
        uint8_t i = 0;
        while (*p && *p != ',' && *p != '*' && i < dstLen - 1) dst[i++] = *p++;
        dst[i] = '\0';
        return i > 0;
    }

    double nmeaToDecimal(const char *val, char hemi) {
        if (!val || !*val) return 0.0;
        double raw = atof(val);
        int    deg = (int)(raw / 100.0);
        double result = deg + (raw - deg * 100.0) / 60.0;
        if (hemi == 'S' || hemi == 'W') result = -result;
        return result;
    }

    void parseTime(const char *t) {
        if (!t || strlen(t) < 6) return;
        fix.hour   = (t[0]-'0')*10 + (t[1]-'0');
        fix.minute = (t[2]-'0')*10 + (t[3]-'0');
        fix.second = (t[4]-'0')*10 + (t[5]-'0');
    }

    void parseDate(const char *d) {
        if (!d || strlen(d) < 6) return;
        fix.day   = (d[0]-'0')*10 + (d[1]-'0');
        fix.month = (d[2]-'0')*10 + (d[3]-'0');
        fix.year  = 2000 + (d[4]-'0')*10 + (d[5]-'0');
    }

    bool parse(const char *s) {
        if (!validateChecksum(s)) return false;
        char type[8] = {0};
        if (sscanf(s, "$%7[^,]", type) != 1) return false;

        bool isBD = (strncmp(type, "BD", 2) == 0);
        if (isBD) fix.beidouSeen = true;
        const char *sen = type + 2;

        char f[20], lat[16], latH[4], lon[16], lonH[4];

        if (strcmp(sen, "GGA") == 0) {
            getField(s, 1, f,    sizeof(f));    parseTime(f);
            getField(s, 2, lat,  sizeof(lat));
            getField(s, 3, latH, sizeof(latH));
            getField(s, 4, lon,  sizeof(lon));
            getField(s, 5, lonH, sizeof(lonH));
            fix.lat = nmeaToDecimal(lat, latH[0]);
            fix.lon = nmeaToDecimal(lon, lonH[0]);
            getField(s, 6, f, sizeof(f)); fix.fixQuality = atoi(f);
            getField(s, 7, f, sizeof(f)); fix.satellites = atoi(f);
            getField(s, 8, f, sizeof(f)); fix.hdop       = atof(f);
            getField(s, 9, f, sizeof(f)); fix.altM       = atof(f);
            return true;
        }

        if (strcmp(sen, "RMC") == 0) {
            char status[4];
            getField(s, 1, f,      sizeof(f));     parseTime(f);
            getField(s, 2, status, sizeof(status));
            getField(s, 3, lat,    sizeof(lat));
            getField(s, 4, latH,   sizeof(latH));
            getField(s, 5, lon,    sizeof(lon));
            getField(s, 6, lonH,   sizeof(lonH));
            fix.valid      = (status[0] == 'A');
            fix.lat        = nmeaToDecimal(lat, latH[0]);
            fix.lon        = nmeaToDecimal(lon, lonH[0]);
            getField(s, 7, f, sizeof(f)); fix.speedKnots = atof(f);
            getField(s, 8, f, sizeof(f)); fix.courseDeg  = atof(f);
            getField(s, 9, f, sizeof(f)); parseDate(f);
            return true;
        }

        if (strcmp(sen, "GSV") == 0 && isBD) {
            getField(s, 3, f, sizeof(f));
            fix.bdSatsInView = atoi(f);
            return true;
        }

        return false;
    }
};
