 Inertial Navigation and Guidance (ING) is a self-contained method for determining a vehicle's position, orientation, and velocity without relying on external references like GPS, visual landmarks, or radio signals. It is fundamental to aerospace, maritime, submarine, and missile systems.

## Core Principle

The system operates on **Newton's laws of motion**, specifically by measuring acceleration and rotation rates and integrating them over time to track movement from a known starting point.

## Key Components

### 1. Inertial Measurement Unit (IMU)
The IMU contains sensors that measure motion relative to inertial space:

- **Accelerometers**: Measure specific force (acceleration minus gravity) along three orthogonal axes. They cannot directly sense gravity—only the reaction force when supported against it.
- **Gyroscopes (Gyros)**: Measure angular rotation rates around three axes (roll, pitch, yaw). Modern systems use ring laser gyros (RLG) or fiber optic gyros (FOG), while older systems used spinning mass gyros.

### 2. Navigation Computer
Processes raw sensor data through **dead reckoning**:
- Integrates angular rates to track orientation (attitude)
- Removes gravity from accelerometer readings using known orientation
- Double-integrates corrected accelerations to calculate position changes

## How It Works

1. **Initialization**: The system is aligned to a known reference frame (e.g., local level or Earth-centered inertial) and given initial position/velocity.
2. **Sensing**: As the vehicle moves, accelerometers detect linear acceleration and gyros detect rotation.
3. **Integration**: The computer continuously integrates these measurements:
   - Gyro data → orientation angles
   - Acceleration (corrected for gravity and Earth rotation) → velocity → position
4. **Output**: Real-time position, velocity, and attitude data fed to guidance systems or pilots.

## The Fundamental Problem: Error Growth

Inertial navigation suffers from **unbounded error accumulation**. Small sensor imperfections compound over time:

| Error Source | Effect |
|-------------|--------|
| Accelerometer bias | Position error grows as **t²** |
| Gyro drift | Orientation error causes incorrect gravity subtraction, making position error grow as **t³** |

Example: A gyro bias of just 0.01°/hour can produce nautical-mile-level position errors after hours of operation.

## Types of Inertial Systems

**Platform Systems (Gimbaled)**: The IMU is mounted on a stabilized platform kept level and oriented using motors. Complex but isolates sensors from vehicle rotations.

**Strapdown Systems**: Sensors are fixed directly to the vehicle body. The computer mathematically transforms measurements to the navigation frame. Lighter, more reliable, and now dominant due to computing advances.

## Guidance vs. Navigation

- **Navigation**: Determining *where you are* (position/velocity)
- **Guidance**: Using that knowledge to determine *where to go* (steering commands to reach a target or follow a path)

Inertial guidance combines navigation with pre-programmed mission data to autonomously steer missiles, spacecraft, or aircraft to their destinations.

## Modern Applications & Mitigation

Pure inertial systems are rare today. They are typically **aided** by:
- **GPS** (when available) to bound position drift
- **Terrain matching**, **star trackers**, or **Doppler radar** for updates
- **Kalman filtering** to optimally blend sensor data

Critical applications still require pure inertial capability for environments where external signals are denied (submarines, ballistic missiles, or jammed battlefields).

## Summary

Inertial navigation provides autonomous, jam-proof positioning by sensing motion directly. Its Achilles' heel is error growth over time, making it most effective when combined with external updates or used for short-duration, high-dynamic missions where self-containment is paramount.




///



# First, let's create a comprehensive technical document combining all sources
# and adding the Arduino Nano implementation details

documentation = """
# Inertial Navigation and Guidance (ING): Technical Deep Dive & DIY Arduino Lab

## 1. Fundamental Physics

### Reference Frames
ING requires careful frame management:
- **Body Frame (b-frame)**: Fixed to the vehicle, axes align with IMU sensors
- **Navigation Frame (n-frame)**: Local level (North-East-Down or East-North-Up)
- **Earth-Centered Inertial (ECI)**: Non-rotating frame for space applications
- **Earth-Centered Earth-Fixed (ECEF)**: Rotates with Earth

### Core Equations

**Attitude Update (from gyroscopes):**
The rotation vector ω = [ωx, ωy, ωz]ᵀ is integrated to update the Direction Cosine Matrix (DCM) or quaternion:

q̇ = ½ * q ⊗ ω

Where q is the quaternion [q0, q1, q2, q3] representing orientation.

**Velocity Update:**
v̇ⁿ = Cᵇₙ * fᵇ - (2ωᵢₑⁿ + ωₑₙⁿ) × vⁿ + gⁿ

Where:
- Cᵇₙ: DCM transforming body to navigation frame
- fᵇ: Specific force measured by accelerometers
- ωᵢₑⁿ: Earth rotation rate (~7.2921×10⁻⁵ rad/s)
- ωₑₙⁿ: Transport rate (frame rotation due to movement over curved Earth)
- gⁿ: Local gravity vector

**Position Update:**
For local-level approximation (short distances):
ṗⁿ = vⁿ

### Error Propagation (Critical)


| Sensor Error | Position Error Growth | Physical Meaning |
|-------------|----------------------|------------------|
| Accelerometer bias (bₐ) | δp ≈ ½ bₐ t² | Constant false acceleration integrates to quadratic position drift |
| Gyro bias (bω) | δp ≈ ⅙ g bω t³ | Tilt error causes gravity to leak into horizontal channels |
| Accelerometer scale factor | δp ∝ a t² | Gain error on measured acceleration |
| Gyro scale factor | δp ∝ ω t³ | Angular rate mismeasurement compounds |
| Random walk (ARW/VRW) | δp ∝ t^(3/2) | White noise integrated |

For MEMS sensors (typical Arduino IMUs):
- Gyro bias: ~10-50 °/hour (vs 0.001°/hr for navigation-grade)
- Position drift: ~1-10 meters per MINUTE

This makes pure INS on Arduino only viable for short-duration (<30s) or aided navigation.

## 2. Sensor Technologies

### Gyroscopes
| Type | Principle | Bias Stability | Cost | Arduino Compatible? |
|------|-----------|---------------|------|---------------------|
| Mechanical (spinning mass) | Conservation of angular momentum | 0.001°/hr | $$$$ | No |
| Ring Laser Gyro (RLG) | Sagnac effect (interference of counter-propagating laser beams) | 0.001°/hr | $$$$ | No |
| Fiber Optic Gyro (FOG) | Sagnac effect in fiber coil | 0.01°/hr | $$$ | No |
| MEMS (vibrating structure) | Coriolis force on vibrating mass | 10-50°/hr | $ | Yes |
| MEMS (tuning fork) | Coriolis-induced capacitance change | 10-50°/hr | $ | Yes |

### Accelerometers
| Type | Principle | Bias Stability | Arduino Compatible? |
|------|-----------|---------------|---------------------|
| Pendulous force balance | Torque rebalance | 10 μg | No |
| MEMS capacitive | Displacement of proof mass changes capacitance | 1-5 mg | Yes |
| MEMS piezoelectric | Crystal strain generates voltage | Medium | Yes |
| MEMS thermal | Convection change due to acceleration | Low | Rare |

## 3. System Architectures

### Gimbaled (Stabilized Platform)
- IMU mounted on motorized gimbals maintained level by servos
- Isolates sensors from vehicle rotations
- High precision but mechanical complexity, weight, cost
- Used in ballistic missiles, submarines

### Strapdown
- IMU rigidly attached to vehicle
- All frame transformations computed mathematically
- Requires high-rate computation (typically >100 Hz attitude update)
- Dominant in modern systems including our Arduino build

### Aided INS (Recommended for Arduino)
- GPS + INS fusion via Kalman Filter or Complementary Filter
- GPS bounds position drift
- INS provides high-rate output between GPS updates
- Handles GPS outages (tunnels, urban canyons)

## 4. Arduino Nano DIY Inertial Navigation Lab

### Bill of Materials (Low Budget: ~$15-25)

| Component | Model | Purpose | Cost |
|-----------|-------|---------|------|
| Microcontroller | Arduino Nano (ATmega328P) | Processing, integration | $3-5 |
| IMU | MPU-6050 (GY-521 module) | 3-axis accel + 3-axis gyro | $2-4 |
| Magnetometer (optional) | HMC5883L or QMC5883L | Heading reference (yaw drift correction) | $1-3 |
| Barometer (optional) | BMP280 | Altitude aiding | $2-3 |
| GPS Module (optional but recommended) | NEO-6M or NEO-M8N | Position aiding | $8-15 |
| Display (optional) | 0.96" OLED I2C | Real-time data visualization | $3-5 |
| Breadboard + wires | - | Prototyping | $2-3 |
| Enclosure | 3D printed or project box | Protection when mounted | $0-5 |

**Total without GPS/OLED: ~$8-15**
**Total with GPS/OLED: ~$20-30**

### Wiring Diagram

```
Arduino Nano          MPU-6050 (GY-521)
-----------           ---------------
5V        ---------->  VCC
GND       ---------->  GND
A4 (SDA)  ---------->  SDA
A5 (SCL)  ---------->  SCL
D2        ---------->  INT (optional, for interrupt-driven sampling)

Arduino Nano          NEO-6M GPS
-----------           ----------
5V        ---------->  VCC
GND       ---------->  GND
D3 (RX)   ---------->  TX (via voltage divider or 3.3V GPS)
D4 (TX)   ---------->  RX

Arduino Nano          0.96" OLED (I2C)
-----------           ---------------
5V        ---------->  VCC
GND       ---------->  GND
A4 (SDA)  ---------->  SDA
A5 (SCL)  ---------->  SCL

Note: NEO-6M is 3.3V logic. Use voltage divider (1kΩ + 2kΩ) on TX→Nano 
or use 3.3V Arduino Nano version.
```

### Arduino Code


# Create the Arduino code file
arduino_code = '''/*
 * Arduino Nano Inertial Navigation System (INS) - Educational Lab
 * 
 * Hardware: Arduino Nano + MPU-6050 + Optional GPS/OLED
 * 
 * Features:
 * - Real-time attitude estimation (roll, pitch, yaw) using Madgwick/Mahony filter
 * - Dead reckoning position estimation (demonstrates drift)
 * - GPS fusion (if available) to correct drift
 * - Serial output for plotting/logging
 * 
 * IMPORTANT LIMITATIONS:
 * - MEMS sensors drift significantly (~1-10m per minute)
 * - This is EDUCATIONAL - not suitable for real navigation
 * - Position is only meaningful for short durations (<30s) without GPS
 */

#include <Wire.h>
#include <math.h>

// ==================== CONFIGURATION ====================
#define USE_GPS         0   // Set to 1 if NEO-6M GPS connected
#define USE_OLED        0   // Set to 1 if OLED display connected
#define USE_MAGNETOMETER 0  // Set to 1 if HMC5883L/QMC5883L connected
#define SAMPLE_RATE_HZ  100 // IMU sample rate (max ~200Hz for MPU-6050)

// MPU-6050 I2C Address
const int MPU_ADDR = 0x68;

// Calibration offsets (determined experimentally - see calibration section)
float accelOffset[3] = {0, 0, 0};
float gyroOffset[3]  = {0, 0, 0};

// Madgwick filter parameters
float beta = 0.6f;      // Filter gain (higher = faster response, more noise)
float q[4] = {1.0f, 0.0f, 0.0f, 0.0f}; // Quaternion [w, x, y, z]

// Navigation state
float velocity[3] = {0, 0, 0};    // m/s in navigation frame (NED)
float position[3] = {0, 0, 0};    // meters from start in navigation frame
float attitude[3] = {0, 0, 0};    // roll, pitch, yaw in radians

// Timing
unsigned long lastMicros = 0;
float dt = 0.01f;                 // seconds (100Hz)

// Raw sensor data
int16_t rawAccel[3], rawGyro[3];
float accel[3], gyro[3];          // Converted to m/s² and rad/s

// Earth parameters
const float g = 9.80665f;         // Standard gravity m/s²
const float deg2rad = PI / 180.0f;
const float rad2deg = 180.0f / PI;

// GPS variables (if enabled)
#if USE_GPS
  #include <SoftwareSerial.h>
  #include <TinyGPS++.h>
  SoftwareSerial gpsSerial(3, 4); // RX, TX
  TinyGPSPlus gps;
  bool gpsValid = false;
  double gpsLat = 0, gpsLon = 0, gpsAlt = 0;
  float gpsVelN = 0, gpsVelE = 0, gpsVelD = 0;
#endif

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000); // Wait for serial or timeout
  
  Wire.begin();
  Wire.setClock(400000); // Fast I2C (400kHz)
  
  Serial.println(F("========================================"));
  Serial.println(F("Arduino Nano Inertial Navigation System"));
  Serial.println(F("========================================"));
  
  // Initialize MPU-6050
  if (!initMPU6050()) {
    Serial.println(F("ERROR: MPU-6050 not found! Check wiring."));
    while (1) {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      delay(100);
    }
  }
  
  Serial.println(F("MPU-6050 initialized successfully"));
  
  // Calibrate sensors (keep device still for 3 seconds)
  calibrateSensors();
  
  #if USE_GPS
    gpsSerial.begin(9600);
    Serial.println(F("GPS initialized"));
  #endif
  
  Serial.println(F("\\nSystem ready. Move device to begin navigation."));
  Serial.println(F("Format: Time,Roll,Pitch,Yaw,PosN,PosE,PosD,VelN,VelE,VelD"));
  
  lastMicros = micros();
}

// ==================== MAIN LOOP ====================
void loop() {
  unsigned long currentMicros = micros();
  dt = (currentMicros - lastMicros) / 1000000.0f;
  
  // Maintain consistent sample rate
  if (dt < (1.0f / SAMPLE_RATE_HZ)) return;
  lastMicros = currentMicros;
  
  // 1. Read sensors
  readMPU6050();
  
  // 2. Update attitude using Madgwick filter
  updateAttitude(accel[0], accel[1], accel[2], 
                 gyro[0], gyro[1], gyro[2], dt);
  
  // 3. Convert quaternion to Euler angles
  quaternionToEuler(q[0], q[1], q[2], q[3], 
                    attitude[0], attitude[1], attitude[2]);
  
  // 4. Perform dead reckoning navigation
  updateNavigation(accel[0], accel[1], accel[2], dt);
  
  // 5. Read GPS if available
  #if USE_GPS
    readGPS();
    if (gpsValid) {
      correctWithGPS();
    }
  #endif
  
  // 6. Output data
  outputData();
}

// ==================== MPU-6050 FUNCTIONS ====================
bool initMPU6050() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); // PWR_MGMT_1
  Wire.write(0x00); // Wake up
  if (Wire.endTransmission(true) != 0) return false;
  
  delay(100);
  
  // Configure accelerometer range (±2g = 16384 LSB/g)
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1C); // ACCEL_CONFIG
  Wire.write(0x00); // ±2g
  Wire.endTransmission(true);
  
  // Configure gyroscope range (±250°/s = 131 LSB/(°/s))
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1B); // GYRO_CONFIG
  Wire.write(0x00); // ±250°/s
  Wire.endTransmission(true);
  
  // Configure Digital Low Pass Filter (DLPF)
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1A); // CONFIG
  Wire.write(0x03); // DLPF = 42Hz (good balance)
  Wire.endTransmission(true);
  
  // Set sample rate divider
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x19); // SMPLRT_DIV
  Wire.write(0x00); // 1kHz / (1 + 0) = 1kHz
  Wire.endTransmission(true);
  
  return true;
}

void readMPU6050() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // Start at ACCEL_XOUT_H
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true); // 14 bytes
  
  rawAccel[0] = (Wire.read() << 8) | Wire.read(); // X
  rawAccel[1] = (Wire.read() << 8) | Wire.read(); // Y
  rawAccel[2] = (Wire.read() << 8) | Wire.read(); // Z
  Wire.read(); Wire.read(); // Skip temperature
  rawGyro[0]  = (Wire.read() << 8) | Wire.read(); // X
  rawGyro[1]  = (Wire.read() << 8) | Wire.read(); // Y
  rawGyro[2]  = (Wire.read() << 8) | Wire.read(); // Z
  
  // Convert to physical units
  // Accel: ±2g range, 16384 LSB/g → convert to m/s²
  accel[0] = (rawAccel[0] - accelOffset[0]) * g / 16384.0f;
  accel[1] = (rawAccel[1] - accelOffset[1]) * g / 16384.0f;
  accel[2] = (rawAccel[2] - accelOffset[2]) * g / 16384.0f;
  
  // Gyro: ±250°/s range, 131 LSB/(°/s) → convert to rad/s
  gyro[0] = (rawGyro[0] - gyroOffset[0]) * deg2rad / 131.0f;
  gyro[1] = (rawGyro[1] - gyroOffset[1]) * deg2rad / 131.0f;
  gyro[2] = (rawGyro[2] - gyroOffset[2]) * deg2rad / 131.0f;
}

void calibrateSensors() {
  Serial.print(F("Calibrating sensors... keep device still "));
  
  const int samples = 500;
  long sumAccel[3] = {0, 0, 0};
  long sumGyro[3] = {0, 0, 0};
  
  for (int i = 0; i < samples; i++) {
    readMPU6050();
    sumAccel[0] += rawAccel[0];
    sumAccel[1] += rawAccel[1];
    sumAccel[2] += rawAccel[2];
    sumGyro[0]  += rawGyro[0];
    sumGyro[1]  += rawGyro[1];
    sumGyro[2]  += rawGyro[2];
    delay(5);
    if (i % 50 == 0) Serial.print(F("."));
  }
  
  accelOffset[0] = sumAccel[0] / (float)samples;
  accelOffset[1] = sumAccel[1] / (float)samples;
  accelOffset[2] = sumAccel[2] / (float)samples; // Should be ~1g when level
  gyroOffset[0]  = sumGyro[0]  / (float)samples;
  gyroOffset[1]  = sumGyro[1]  / (float)samples;
  gyroOffset[2]  = sumGyro[2]  / (float)samples;
  
  Serial.println(F(" DONE"));
  Serial.print(F("Accel offsets: ")); 
  Serial.print(accelOffset[0]); Serial.print(F(", "));
  Serial.print(accelOffset[1]); Serial.print(F(", "));
  Serial.println(accelOffset[2]);
  Serial.print(F("Gyro offsets: ")); 
  Serial.print(gyroOffset[0]); Serial.print(F(", "));
  Serial.print(gyroOffset[1]); Serial.print(F(", "));
  Serial.println(gyroOffset[2]);
}

// ==================== MADGWICK FILTER ====================
/*
 * Madgwick's IMU algorithm for attitude estimation
 * Fuses accelerometer and gyroscope data
 * 
 * Paper: "An efficient orientation filter for inertial and 
 *         inertial/magnetic sensor arrays" (2010)
 * 
 * This is a gradient descent algorithm that minimizes the error
 * between measured and estimated gravity direction.
 */
void updateAttitude(float ax, float ay, float az,
                    float gx, float gy, float gz, float dt) {
  
  float q0 = q[0], q1 = q[1], q2 = q[2], q3 = q[3];
  
  // Normalize accelerometer measurement
  float recipNorm = 1.0f / sqrt(ax*ax + ay*ay + az*az);
  ax *= recipNorm;
  ay *= recipNorm;
  az *= recipNorm;
  
  // Gradient descent algorithm corrective step
  float s0 = 4.0f * q0 * q2 + 2.0f * q2 * ax + 4.0f * q0 * q1 - 2.0f * q1 * ay;
  float s1 = 4.0f * q1 * q3 - 2.0f * q3 * ax + 4.0f * q0 * q2 - 2.0f * q2 * ay - 4.0f * q1 + 8.0f * q1 * q1 + 8.0f * q2 * q2 + 4.0f * az;
  float s2 = 4.0f * q0 * q3 + 2.0f * q3 * ay + 4.0f * q2 * q2 - 2.0f * q2 * ax - 4.0f * q2 + 8.0f * q1 * q2 + 8.0f * q3 * q3 + 4.0f * az;
  float s3 = 4.0f * q1 * q2 - 2.0f * q1 * ax + 4.0f * q2 * q3 - 2.0f * q3 * ay;
  
  recipNorm = 1.0f / sqrt(s0*s0 + s1*s1 + s2*s2 + s3*s3);
  s0 *= recipNorm;
  s1 *= recipNorm;
  s2 *= recipNorm;
  s3 *= recipNorm;
  
  // Apply feedback step
  float qDot0 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz) - beta * s0;
  float qDot1 = 0.5f * ( q0 * gx + q2 * gz - q3 * gy) - beta * s1;
  float qDot2 = 0.5f * ( q0 * gy - q1 * gz + q3 * gx) - beta * s2;
  float qDot3 = 0.5f * ( q0 * gz + q1 * gy - q2 * gx) - beta * s3;
  
  // Integrate quaternion rate
  q0 += qDot0 * dt;
  q1 += qDot1 * dt;
  q2 += qDot2 * dt;
  q3 += qDot3 * dt;
  
  // Normalize quaternion
  recipNorm = 1.0f / sqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3);
  q[0] = q0 * recipNorm;
  q[1] = q1 * recipNorm;
  q[2] = q2 * recipNorm;
  q[3] = q3 * recipNorm;
}

void quaternionToEuler(float w, float x, float y, float z,
                       float &roll, float &pitch, float &yaw) {
  // Roll (x-axis rotation)
  float sinr_cosp = 2.0f * (w * x + y * z);
  float cosr_cosp = 1.0f - 2.0f * (x * x + y * y);
  roll = atan2(sinr_cosp, cosr_cosp);
  
  // Pitch (y-axis rotation)
  float sinp = 2.0f * (w * y - z * x);
  if (fabs(sinp) >= 1.0f)
    pitch = copysign(PI / 2.0f, sinp); // Use 90 degrees if out of range
  else
    pitch = asin(sinp);
  
  // Yaw (z-axis rotation)
  float siny_cosp = 2.0f * (w * z + x * y);
  float cosy_cosp = 1.0f - 2.0f * (y * y + z * z);
  yaw = atan2(siny_cosp, cosy_cosp);
}

// ==================== NAVIGATION FUNCTIONS ====================
void updateNavigation(float ax, float ay, float az, float dt) {
  /*
   * Strapdown INS mechanization:
   * 1. Transform specific force from body to navigation frame
   * 2. Remove gravity
   * 3. Integrate to get velocity
   * 4. Integrate velocity to get position
   * 
   * For this educational demo, we use a simplified local-level
   * frame (North-East-Down) with flat-Earth approximation.
   */
  
  // Quaternion to DCM elements (body to navigation)
  float q0 = q[0], q1 = q[1], q2 = q[2], q3 = q[3];
  
  float c11 = q0*q0 + q1*q1 - q2*q2 - q3*q3;
  float c12 = 2.0f * (q1*q2 - q0*q3);
  float c13 = 2.0f * (q1*q3 + q0*q2);
  float c21 = 2.0f * (q1*q2 + q0*q3);
  float c22 = q0*q0 - q1*q1 + q2*q2 - q3*q3;
  float c23 = 2.0f * (q2*q3 - q0*q1);
  float c31 = 2.0f * (q1*q3 - q0*q2);
  float c32 = 2.0f * (q2*q3 + q0*q1);
  float c33 = q0*q0 - q1*q1 - q2*q2 + q3*q3;
  
  // Transform acceleration to navigation frame
  float aN = c11 * ax + c12 * ay + c13 * az;
  float aE = c21 * ax + c22 * ay + c23 * az;
  float aD = c31 * ax + c32 * ay + c33 * az;
  
  // Remove gravity (Down direction)
  aD -= g;
  
  // Simple integration (trapezoidal would be better)
  velocity[0] += aN * dt;
  velocity[1] += aE * dt;
  velocity[2] += aD * dt;
  
  position[0] += velocity[0] * dt;
  position[1] += velocity[1] * dt;
  position[2] += velocity[2] * dt;
  
  // Damping to prevent unbounded drift (educational only)
  // In real systems, this would be replaced by GPS aiding
  velocity[0] *= 0.999f;
  velocity[1] *= 0.999f;
  velocity[2] *= 0.999f;
}

// ==================== GPS FUNCTIONS ====================
#if USE_GPS
void readGPS() {
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }
  
  if (gps.location.isUpdated() && gps.location.isValid()) {
    gpsValid = true;
    gpsLat = gps.location.lat();
    gpsLon = gps.location.lng();
    gpsAlt = gps.altitude.meters();
    
    // Convert to local NED frame (simplified - assumes small distances)
    // In practice, use proper geodetic transformations
    static double originLat = 0, originLon = 0;
    if (originLat == 0) {
      originLat = gpsLat;
      originLon = gpsLon;
    }
    
    float dLat = (gpsLat - originLat) * 111320.0f; // meters per degree latitude
    float dLon = (gpsLon - originLon) * 111320.0f * cos(originLat * deg2rad);
    
    gpsVelN = gps.speed.kmph() / 3.6f * cos(gps.course.deg() * deg2rad);
    gpsVelE = gps.speed.kmph() / 3.6f * sin(gps.course.deg() * deg2rad);
  }
}

void correctWithGPS() {
  /*
   * Simple complementary filter for GPS/INS fusion
   * Weighted average: trust GPS for low-frequency, INS for high-frequency
   * 
   * In production, use Extended Kalman Filter (EKF)
   */
  const float alpha = 0.02f; // GPS correction weight (0-1)
  
  // Correct position
  position[0] = (1.0f - alpha) * position[0] + alpha * gpsVelN; // Simplified
  position[1] = (1.0f - alpha) * position[1] + alpha * gpsVelE;
  
  // Correct velocity
  velocity[0] = (1.0f - alpha) * velocity[0] + alpha * gpsVelN;
  velocity[1] = (1.0f - alpha) * velocity[1] + alpha * gpsVelE;
}
#endif

// ==================== OUTPUT FUNCTIONS ====================
void outputData() {
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint < 100) return; // 10Hz output rate
  lastPrint = millis();
  
  // CSV format for easy plotting
  Serial.print(millis() / 1000.0f, 3); Serial.print(F(","));
  Serial.print(attitude[0] * rad2deg, 2); Serial.print(F(",")); // Roll
  Serial.print(attitude[1] * rad2deg, 2); Serial.print(F(",")); // Pitch
  Serial.print(attitude[2] * rad2deg, 2); Serial.print(F(",")); // Yaw
  Serial.print(position[0], 3); Serial.print(F(",")); // PosN
  Serial.print(position[1], 3); Serial.print(F(",")); // PosE
  Serial.print(position[2], 3); Serial.print(F(",")); // PosD
  Serial.print(velocity[0], 3); Serial.print(F(",")); // VelN
  Serial.print(velocity[1], 3); Serial.print(F(",")); // VelE
  Serial.print(velocity[2], 3); // VelD
  
  #if USE_GPS
    Serial.print(F(",")); Serial.print(gpsLat, 6);
    Serial.print(F(",")); Serial.print(gpsLon, 6);
  #endif
  
  Serial.println();
}
'''

with open('/mnt/agents/output/Arduino_INS.ino', 'w') as f:
    f.write(arduino_code)

print("Arduino code saved successfully")

