# DIY Inertial Navigation Lab — Low-Budget Arduino Nano Version

**Purpose:** Build a practical small inertial-navigation and motion-logging lab using cheap parts. This is not a precision aircraft-grade INS. It is a learning platform that shows how an IMU senses motion, how drift appears, how to reduce it, and how to use the data on a moving object such as a toy car, bicycle, trolley, robot chassis, model boat, or test sled.

---

## 1. Cleaned-up concept

An **Inertial Navigation and Guidance system**, often shortened to **ING** or **INS/IMU navigation**, estimates an object's:

- **Attitude:** roll, pitch, yaw
- **Velocity:** how fast it is moving in each axis
- **Position:** where it has moved from a known starting point

It does this without needing GPS, radio beacons, cameras, or landmarks. It measures motion directly and then integrates those measurements over time.

The core idea is simple:

```text
Gyroscope rate  -> integrate -> angle/orientation
Acceleration    -> integrate -> velocity
Velocity        -> integrate -> position
```

The hard part is that tiny errors also get integrated. That is why low-cost DIY inertial navigation is useful for **short-term motion**, **attitude measurement**, **vibration logging**, **vehicle behaviour logging**, and **sensor-fusion experiments**, but not for long-term accurate position without extra references.

---

## 2. Main parts of an inertial navigation system

### 2.1 IMU — inertial measurement unit

The IMU is the sensor package. A cheap IMU module usually contains:

| Sensor | Measures | Common cheap device |
|---|---|---|
| 3-axis accelerometer | Specific force on X/Y/Z axes | MPU6050, MPU9250, BMI160 |
| 3-axis gyroscope | Rotation rate around X/Y/Z axes | MPU6050, MPU9250, BMI160 |
| 3-axis magnetometer, optional | Compass heading relative to Earth's magnetic field | HMC5883L, QMC5883L, MPU9250 built-in mag |

The accelerometer does **not** simply measure movement. When sitting still on a table it reads about **1 g upward**, because it senses support force. To get useful motion acceleration, gravity must be removed using orientation information.

### 2.2 Navigation computer

The navigation computer can be an Arduino Nano, Raspberry Pi Pico, ESP32, laptop, TEC-1/Z80 project, or any microcontroller that can:

1. Read sensors at a regular sample rate.
2. Calibrate sensor offsets.
3. Estimate attitude.
4. Remove gravity from acceleration.
5. Integrate acceleration into velocity.
6. Integrate velocity into position.
7. Log or transmit the result.

For a low-budget lab, the Arduino Nano is enough to demonstrate the mechanism.

### 2.3 Strapdown arrangement

In a **strapdown INS**, the IMU is fixed directly to the moving object. It rotates with the object. Software converts body-frame measurements into navigation-frame measurements.

```text
Moving object body frame:
  X = forward
  Y = right
  Z = down/up depending on convention

Navigation frame:
  X = east/forward reference
  Y = north/side reference
  Z = vertical reference
```

Cheap labs use strapdown mounting because it is simple and has no gimbals.

### 2.4 Gimballed arrangement

Older high-end systems used a mechanically stabilised platform. Motors kept the IMU level while the vehicle moved around it. This reduces some computation but adds mechanical complexity. For a home lab, avoid this at first.

---

## 3. Why drift happens

The main limitation is error growth.

| Error source | What happens | Approximate growth |
|---|---|---|
| Accelerometer bias | A false acceleration appears even when still | Position error grows like **t²** |
| Gyro bias | Orientation slowly tilts | Gravity subtraction becomes wrong; position error can grow like **t³** |
| Noise | Random jitter in velocity/position | Wandering error |
| Misalignment | X/Y/Z axes are not perfectly aligned to the vehicle | Cross-axis errors |
| Vibration | Mechanical vibration corrupts acceleration | False movement |
| Timing jitter | Sample time is not constant | Bad integration |

Example: if the accelerometer has only **0.02 g** of false acceleration, that is about:

```text
0.02 × 9.81 = 0.196 m/s²
```

After 10 seconds, position error from that alone can be roughly:

```text
s = 0.5 × a × t²
s = 0.5 × 0.196 × 10²
s = 9.8 metres
```

That is why a low-cost Nano/MPU6050 system should be treated as a **motion experiment and logger**, not as a precision navigator.

---

## 4. Low-budget DIY lab hardware

### 4.1 Basic build

| Part | Purpose | Notes |
|---|---|---|
| Arduino Nano ATmega328P | Main controller | Cheap clone is fine |
| MPU6050 module | 3-axis accelerometer + 3-axis gyro | I²C module, 3.3/5 V boards common |
| USB cable | Power + serial logging | Also uploads code |
| Breadboard or perfboard | Prototype wiring | Use soldered wiring for moving tests |
| Small plastic box | Enclosure | Protects wiring |
| Foam tape or rubber washers | Vibration isolation | Do not make it too soft |
| Velcro/zip ties | Attach to object | Keep orientation repeatable |

### 4.2 Better lab build with data logging

| Add-on | Purpose | Recommended low-cost part |
|---|---|---|
| microSD card module | Save CSV data while moving | SPI microSD module |
| NEO-6M GPS | Absolute position update outdoors | Helps show drift correction |
| OLED display | Live roll/pitch/speed display | 0.96 inch I²C SSD1306 |
| HC-05 Bluetooth | Wireless serial to laptop/phone | Useful on moving object |
| Wheel encoder | Distance/speed correction | Slotted opto or Hall sensor |
| Buzzer/LED | Status output | Shows calibration/recording state |
| Power bank | Stable 5 V supply | Safer than loose batteries |

### 4.3 Minimum wiring: Arduino Nano + MPU6050

Most MPU6050 breakout boards have VCC, GND, SCL, SDA, INT.

| MPU6050 pin | Arduino Nano pin |
|---|---|
| VCC | 5 V, or 3.3 V if your board requires it |
| GND | GND |
| SDA | A4 |
| SCL | A5 |
| INT | D2 optional, not used in the simple code |

Keep I²C wires short. For a moving object, solder the wires or use locking connectors. Breadboard jumpers shake loose easily.

---

## 5. Mechanical build: how to make the mechanism

### 5.1 IMU mounting plate

Make a small sensor plate from:

- 2 mm acrylic
- FR-4 offcut
- thin plywood
- aluminium angle
- 3D-printed bracket

Mark arrows on it:

```text
X arrow = forward direction of moving object
Y arrow = right side
Z arrow = up or down, but keep it consistent in software notes
```

Mount the IMU flat on this plate. Do not let the IMU board flex.

### 5.2 Vibration control

For a trolley, RC car, bicycle, or robot:

1. Put the IMU near the object's centre of mass if possible.
2. Avoid mounting directly beside motors, gears, chains, or vibrating panels.
3. Use thin foam tape or rubber washers.
4. Do not make a floppy mount; that creates delayed motion and false readings.
5. Twist power wires and keep motor wires away from the IMU.

### 5.3 Repeatable alignment

A good low-budget trick is to make a **keyed mount**:

```text
[vehicle deck]
     |
     |-- small rectangular recess or two locator screws
     |
[IMU plate]
     |-- always fits the same way
```

This prevents the IMU being mounted differently each test.

### 5.4 Moving-object installation examples

#### Robot car or trolley

- Mount the IMU on the chassis top plate.
- X-axis points forward.
- Put the Nano and battery in the same box.
- Add an SD card module or Bluetooth serial.
- Optional: add wheel encoder for better speed/distance.

#### Bicycle

- Mount the box on the frame, not on handlebars.
- Frame mounting has less steering vibration.
- Use a power bank in a frame bag.
- Do not rely on position integration for road navigation; use it for lean angle, braking, acceleration, bumps, and cornering behaviour.

#### Model boat

- Mount high enough to avoid water.
- Seal enclosure with silicone or gasket tape.
- Log roll/pitch/yaw rate and acceleration.
- Add GPS if outdoor and open sky.

#### RC car

- Mount away from motor and ESC.
- Use foam isolation.
- Log acceleration, braking, cornering, vibration, and attitude.
- Use an SD card because USB cable logging is impractical while driving.

---

## 6. What the data means

The Nano can output CSV like this:

```csv
time_ms,ax_g,ay_g,az_g,gx_dps,gy_dps,gz_dps,roll_deg,pitch_deg,vx,vy,px,py
1000,0.01,-0.02,1.01,0.4,-0.2,0.1,1.2,-0.8,0.00,0.00,0.00,0.00
```

| Field | Meaning | Use |
|---|---|---|
| time_ms | Timestamp | Plot against time |
| ax_g, ay_g, az_g | Raw acceleration in g | Detect bumps, braking, turning |
| gx_dps, gy_dps, gz_dps | Gyro rates in degrees/sec | Detect rotation and vibration |
| roll_deg | Tilt left/right | Bike lean, chassis roll |
| pitch_deg | Nose up/down | Braking/dive, climbing |
| vx, vy | Estimated velocity | Short-term only |
| px, py | Estimated position | Demonstration only unless aided |

Useful experiments:

1. **Stationary drift test:** Leave it still for 5 minutes and watch velocity/position wander.
2. **Push-cart test:** Push a trolley 1 metre and stop. Compare estimated distance.
3. **Braking test:** Put it on a bike or cart and record deceleration.
4. **Cornering test:** Look at lateral acceleration and roll angle.
5. **Vibration test:** Compare smooth floor, carpet, concrete, and road.
6. **Zero velocity update test:** Stop the object and force velocity back to zero.
7. **GPS-aided test:** Compare IMU-only drift against GPS track outdoors.

---

## 7. Arduino Nano example code

This example uses only:

- Arduino Nano
- MPU6050 module
- USB serial monitor or serial logger

It performs:

- Sensor initialisation
- Simple gyro/accelerometer calibration
- Complementary filter for roll and pitch
- Gravity removal using roll/pitch
- Very simple X/Y velocity and position integration
- Zero-velocity reset when the object seems stationary
- CSV output

**Important:** This is teaching code. It is not a precision INS. Yaw is not corrected without a magnetometer, GPS, optical flow, or wheel encoder.

```cpp
#include <Wire.h>

// MPU6050 I2C address
const uint8_t MPU = 0x68;

// Raw sensor values
int16_t rawAx, rawAy, rawAz;
int16_t rawGx, rawGy, rawGz;

// Calibration offsets
float axOff = 0, ayOff = 0, azOff = 0;
float gxOff = 0, gyOff = 0, gzOff = 0;

// Scales for default MPU6050 settings:
// accel +/-2g = 16384 LSB/g
// gyro  +/-250 deg/s = 131 LSB/(deg/s)
const float ACC_SCALE = 16384.0;
const float GYRO_SCALE = 131.0;
const float G = 9.80665;

// Attitude estimate
float rollDeg = 0.0;
float pitchDeg = 0.0;

// Simple dead-reckoning variables in navigation frame
float vx = 0.0, vy = 0.0;
float px = 0.0, py = 0.0;

unsigned long lastMicros;

void writeMPU(uint8_t reg, uint8_t data) {
  Wire.beginTransmission(MPU);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission(true);
}

void readMPU() {
  Wire.beginTransmission(MPU);
  Wire.write(0x3B); // first accel register
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, (uint8_t)14, (uint8_t)true);

  rawAx = (Wire.read() << 8) | Wire.read();
  rawAy = (Wire.read() << 8) | Wire.read();
  rawAz = (Wire.read() << 8) | Wire.read();

  // Temperature registers, not used here
  Wire.read(); Wire.read();

  rawGx = (Wire.read() << 8) | Wire.read();
  rawGy = (Wire.read() << 8) | Wire.read();
  rawGz = (Wire.read() << 8) | Wire.read();
}

void calibrateMPU() {
  const int N = 1000;
  long sax = 0, say = 0, saz = 0;
  long sgx = 0, sgy = 0, sgz = 0;

  Serial.println(F("Keep the unit flat and still. Calibrating..."));
  delay(1000);

  for (int i = 0; i < N; i++) {
    readMPU();
    sax += rawAx;
    say += rawAy;
    saz += rawAz;
    sgx += rawGx;
    sgy += rawGy;
    sgz += rawGz;
    delay(3);
  }

  axOff = (float)sax / N;
  ayOff = (float)say / N;

  // If Z points up while flat, rawAz should be about +1g.
  // Remove only the extra offset, not gravity itself.
  azOff = ((float)saz / N) - ACC_SCALE;

  gxOff = (float)sgx / N;
  gyOff = (float)sgy / N;
  gzOff = (float)sgz / N;

  Serial.println(F("Calibration complete."));
}

bool looksStationary(float ax_g, float ay_g, float az_g,
                     float gx_dps, float gy_dps, float gz_dps) {
  float amag = sqrt(ax_g * ax_g + ay_g * ay_g + az_g * az_g);
  float gmag = sqrt(gx_dps * gx_dps + gy_dps * gy_dps + gz_dps * gz_dps);

  // Tweak these thresholds for your vehicle.
  bool accelNearOneG = abs(amag - 1.0) < 0.04;
  bool gyroQuiet = gmag < 2.0;

  return accelNearOneG && gyroQuiet;
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Wake MPU6050
  writeMPU(0x6B, 0x00);

  // Accel range +/-2g
  writeMPU(0x1C, 0x00);

  // Gyro range +/-250 deg/s
  writeMPU(0x1B, 0x00);

  // Digital low-pass filter roughly 44 Hz accel / 42 Hz gyro
  writeMPU(0x1A, 0x03);

  calibrateMPU();

  lastMicros = micros();

  Serial.println(F("time_ms,ax_g,ay_g,az_g,gx_dps,gy_dps,gz_dps,roll_deg,pitch_deg,vx,vy,px,py"));
}

void loop() {
  unsigned long now = micros();
  float dt = (now - lastMicros) / 1000000.0;
  lastMicros = now;

  // Avoid bad integration if serial monitor pauses/reset timing
  if (dt <= 0 || dt > 0.2) dt = 0.01;

  readMPU();

  // Convert raw readings to physical units
  float ax_g = ((float)rawAx - axOff) / ACC_SCALE;
  float ay_g = ((float)rawAy - ayOff) / ACC_SCALE;
  float az_g = ((float)rawAz - azOff) / ACC_SCALE;

  float gx_dps = ((float)rawGx - gxOff) / GYRO_SCALE;
  float gy_dps = ((float)rawGy - gyOff) / GYRO_SCALE;
  float gz_dps = ((float)rawGz - gzOff) / GYRO_SCALE;

  // Accelerometer-only roll/pitch estimate.
  // Sign conventions may need swapping depending on your mounting.
  float rollAcc  = atan2(ay_g, az_g) * 180.0 / PI;
  float pitchAcc = atan2(-ax_g, sqrt(ay_g * ay_g + az_g * az_g)) * 180.0 / PI;

  // Complementary filter.
  // Gyro is good short-term. Accelerometer corrects long-term tilt drift.
  const float alpha = 0.98;
  rollDeg  = alpha * (rollDeg  + gx_dps * dt) + (1.0 - alpha) * rollAcc;
  pitchDeg = alpha * (pitchDeg + gy_dps * dt) + (1.0 - alpha) * pitchAcc;

  // Convert roll/pitch to radians
  float roll = rollDeg * PI / 180.0;
  float pitch = pitchDeg * PI / 180.0;

  // Convert acceleration from g to m/s^2
  float ax = ax_g * G;
  float ay = ay_g * G;
  float az = az_g * G;

  // Simple gravity compensation using roll and pitch only.
  // This assumes yaw is not needed for the basic X/Y lab demo.
  // For accurate navigation, use full 3D rotation with yaw correction.
  float sinR = sin(roll);
  float cosR = cos(roll);
  float sinP = sin(pitch);
  float cosP = cos(pitch);

  // Rotate body acceleration approximately into a level frame.
  // This is simplified for low-cost demonstration.
  float axLevel = ax * cosP + az * sinP;
  float ayLevel = ax * sinR * sinP + ay * cosR - az * sinR * cosP;

  // Remove tiny noise by deadband.
  if (abs(axLevel) < 0.12) axLevel = 0;
  if (abs(ayLevel) < 0.12) ayLevel = 0;

  // Zero-velocity update when stationary.
  // This prevents velocity from running away while stopped.
  if (looksStationary(ax_g, ay_g, az_g, gx_dps, gy_dps, gz_dps)) {
    vx = 0;
    vy = 0;
  } else {
    vx += axLevel * dt;
    vy += ayLevel * dt;
  }

  px += vx * dt;
  py += vy * dt;

  // CSV output
  Serial.print(millis()); Serial.print(',');
  Serial.print(ax_g, 4); Serial.print(',');
  Serial.print(ay_g, 4); Serial.print(',');
  Serial.print(az_g, 4); Serial.print(',');
  Serial.print(gx_dps, 3); Serial.print(',');
  Serial.print(gy_dps, 3); Serial.print(',');
  Serial.print(gz_dps, 3); Serial.print(',');
  Serial.print(rollDeg, 2); Serial.print(',');
  Serial.print(pitchDeg, 2); Serial.print(',');
  Serial.print(vx, 3); Serial.print(',');
  Serial.print(vy, 3); Serial.print(',');
  Serial.print(px, 3); Serial.print(',');
  Serial.println(py, 3);

  delay(10); // about 100 Hz loop
}
```

---

## 8. Arduino IDE installation and use

1. Install the Arduino IDE.
2. Plug in the Arduino Nano by USB.
3. Select board:

```text
Tools -> Board -> Arduino Nano
```

4. Select processor. For many clone Nanos:

```text
Tools -> Processor -> ATmega328P (Old Bootloader)
```

5. Select the correct serial port.
6. Paste the code into a new sketch.
7. Upload.
8. Open Serial Monitor or Serial Plotter at:

```text
115200 baud
```

9. Keep the unit flat and still during calibration.
10. After the CSV header appears, start moving the object.

To save data, use a serial logger on the laptop, or add an SD card module.

---

## 9. How to use the lab step by step

### Experiment 1 — stationary drift

1. Place the unit flat on a desk.
2. Upload the code.
3. Let it run for 5 minutes.
4. Save the CSV.
5. Plot `vx`, `vy`, `px`, and `py`.

Expected result: even though the unit did not move, velocity and position will slowly wander. This demonstrates inertial drift.

### Experiment 2 — one-metre push

1. Tape a one-metre ruler or mark 1 metre on the floor.
2. Put the unit on a trolley.
3. Start logging.
4. Push forward 1 metre and stop.
5. Compare the final `px` value to the real 1 metre.

Expected result: it may be close for very short tests, but it will not stay accurate for long.

### Experiment 3 — braking and acceleration

1. Mount it on a bicycle frame, cart, or RC car.
2. Record a short start/stop run.
3. Plot `ax_g` and `pitch_deg`.

Use the data to see:

- acceleration surge
- braking deceleration
- chassis pitch/dive
- road vibration

### Experiment 4 — cornering

1. Drive a slow circle.
2. Plot `ay_g`, `roll_deg`, and `gz_dps`.
3. Compare left vs right turns.

Use the data to see lateral acceleration and yaw rate.

### Experiment 5 — improve with wheel encoder

A wheel encoder gives real distance. The IMU gives fast acceleration and attitude. Together they are much better than either alone.

Simple fusion idea:

```text
IMU acceleration = fast short-term motion
Wheel encoder    = slower but bounded distance/speed
Correction       = gently pull IMU velocity toward encoder velocity
```

---

## 10. Optional add-ons

### 10.1 microSD logging

Attach a microSD module:

| SD module | Nano |
|---|---|
| VCC | 5 V or 3.3 V depending module |
| GND | GND |
| CS | D10 |
| MOSI | D11 |
| MISO | D12 |
| SCK | D13 |

Then write the same CSV lines to a file instead of only Serial. Use the Arduino `SD.h` library.

### 10.2 GPS aiding

A NEO-6M GPS module can connect to software serial:

| GPS | Nano |
|---|---|
| VCC | 5 V, if module supports it |
| GND | GND |
| TX | D4 RX for SoftwareSerial |
| RX | D3 TX for SoftwareSerial, optional |

GPS is slow, often 1 Hz to 10 Hz, but does not drift like pure IMU integration. Outdoors, you can compare:

```text
IMU-only path: smooth but drifting
GPS path: noisy but bounded
Fused path: smoother and less drifting
```

### 10.3 Magnetometer for yaw

A 6-axis MPU6050 has no compass. It can estimate roll and pitch from gravity, but yaw drifts badly. Add a magnetometer if heading matters.

Low-cost options:

- QMC5883L
- HMC5883L clone module
- MPU9250 module instead of MPU6050

Magnetometers are disturbed by motors, steel frames, magnets, and power wires, so placement and calibration matter.

### 10.4 Wheel encoder

For ground vehicles, a wheel encoder is often more useful than GPS indoors.

Options:

- slotted opto sensor and encoder disk
- Hall sensor and small magnet
- salvaged mouse wheel encoder
- quadrature encoder module

Use the encoder to correct speed and distance drift.

---

## 11. How to analyse the data

Save the serial output as a `.csv` file. Then plot it in a spreadsheet or Python.

Useful plots:

| Plot | Shows |
|---|---|
| `ax_g` vs time | acceleration/braking |
| `ay_g` vs time | cornering force |
| `az_g` vs time | bumps/vibration |
| `roll_deg` vs time | lean/body roll |
| `pitch_deg` vs time | climbing/braking dive |
| `px` vs `py` | rough path estimate |
| gyro values vs time | rotation/vibration |

Python plotting example:

```python
import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("imu_log.csv")

plt.figure()
plt.plot(df["time_ms"] / 1000, df["ax_g"], label="ax_g")
plt.plot(df["time_ms"] / 1000, df["ay_g"], label="ay_g")
plt.plot(df["time_ms"] / 1000, df["az_g"], label="az_g")
plt.xlabel("Time (s)")
plt.ylabel("Acceleration (g)")
plt.legend()
plt.grid(True)
plt.show()

plt.figure()
plt.plot(df["px"], df["py"])
plt.xlabel("X position estimate (m)")
plt.ylabel("Y position estimate (m)")
plt.axis("equal")
plt.grid(True)
plt.show()
```

---

## 12. Practical limitations and fixes

### Problem: position runs away while sitting still

Fixes:

- improve calibration
- add zero-velocity update
- use stronger acceleration deadband
- improve vibration isolation
- keep temperature stable

### Problem: pitch/roll are wrong

Fixes:

- check IMU axis direction
- swap signs in roll/pitch equations
- recalibrate on a flat surface
- ensure the module is not mounted upside down unless code is adjusted

### Problem: yaw heading drifts

Fixes:

- add magnetometer
- add GPS course outdoors
- add wheel encoder/steering model for ground vehicle
- use known course correction points

### Problem: readings are noisy on motorised vehicle

Fixes:

- move IMU away from motor/ESC
- use twisted wires
- add capacitors to power rail
- shield motor wires
- mount IMU on a firm damped plate
- use digital low-pass filter settings

---

## 13. Upgrade path

### Stage 1 — Nano + MPU6050

Learn the basics:

- raw acceleration
- gyro rate
- roll/pitch
- drift
- basic CSV logging

### Stage 2 — Nano + MPU6050 + SD card

Make it mobile:

- log without USB cable
- mount on moving object
- collect real data

### Stage 3 — add wheel encoder or GPS

Reduce drift:

- compare inertial and external reference
- correct velocity/position
- learn sensor fusion

### Stage 4 — use ESP32, Raspberry Pi Pico, or laptop

More processing power:

- better filters
- quaternions
- Madgwick/Mahony AHRS
- Kalman filter
- live Bluetooth/Wi-Fi dashboard

### Stage 5 — full navigation lab

Combine:

- IMU
- GPS
- wheel encoder
- magnetometer
- barometer
- SD logging
- map display
- correction algorithms

---

## 14. Safe use notes

Use this as a data logger and educational navigation experiment. Do not use a cheap DIY inertial system for safety-critical control, road navigation, aircraft control, weapon guidance, or any system where drift or failure could injure people.

For moving-object tests:

- keep speeds low at first
- secure the electronics
- avoid loose USB cables near wheels
- protect batteries from impact
- do not look at a laptop while riding/driving
- do not mount sharp or heavy boxes where they can become projectiles

---

## 15. Summary

A low-cost Arduino Nano inertial lab can teach the real mechanisms of inertial navigation:

1. Gyros measure rotation rate.
2. Accelerometers measure specific force.
3. Software estimates attitude.
4. Gravity is subtracted.
5. Acceleration is integrated into velocity.
6. Velocity is integrated into position.
7. Errors accumulate quickly.
8. External references such as GPS, wheel encoders, magnetometers, or known stops are used to control drift.

The best DIY result is not a perfect navigator. The best result is a **working motion laboratory** that lets you see acceleration, tilt, vibration, drift, and sensor fusion in real data from a moving object.
