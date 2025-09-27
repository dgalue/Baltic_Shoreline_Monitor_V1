# Hardware Component Validation Guide

This guide documents the standalone bring-up tests for each peripheral used by the
Baltic Shoreline Monitor. Run these procedures on real hardware to validate new
boards, confirm repairs, or qualify firmware changes before merging them into the
main application.

---

## How to Use This Guide

1. Work on one peripheral at a time with every other device disconnected unless
the procedure explicitly calls for a companion radio or sensor.
2. Create a fresh PlatformIO project (or reuse an existing scratch project)
targeting **seeed_xiao_esp32s3**.
3. Install the libraries called out in each section with `pio lib install`.
4. Copy the firmware sketch into `src/main.cpp`, build with `pio run`, and flash
with `pio run -t upload`.
5. Monitor the serial console at 115200 bps unless a different baud rate is
specified.
6. Capture the observed behaviour in the corresponding result table. Include
the test date, the firmware commit (or sketch hash) that exercised the hardware,
and a concise summary of the outcome.

### Common Setup

- Power peripherals from the XIAO ESP32S3 3V3 and GND pins. The USB supply is
sufficient for the individual tests in this document.
- Use short jumper wires and keep high-gain analog lines (hydrophone output) away
from switching regulators or digital busses.
- If a test depends on outdoor conditions (GPS reception, solar charging), plan
a suitable environment ahead of time.

### Documentation Standards

- Use ISO-8601 dates (`YYYY-MM-DD`).
- Reference firmware with a short Git hash or release tag.
- When a test fails, record the failure mode and open a tracking issue.

---

## Test Summary

| Component | Interface | Last Run | Firmware Commit | Result Summary |
| --- | --- | --- | --- | --- |
| Air530 GPS Module | UART1 @ 9600 bps | 2025-09-17 | 60ec05e | Locked a clean 3D fix outdoors after no indoor acquisition; HDOP 0.9, altitude ≈ 8 m. |
| Grove Vision AI V2 | I²C (0x62) | 2025-09-19 | b7130f2 | Frequent `Invoke failed` responses with sporadic single-box detections (target 0 score ≈ 50). |
| MicroSD Card | SPI @ 20 MHz | 2025-09-20 | 60ec05e | Init succeeded; wrote and read `SD card write test`. |
| SSD1315 OLED | I²C (0x3C) | 2025-09-21 | 60ec05e | OLED test pattern rendered. |
| Hydrophone + Preamp | ADC1 continuous | _pending_ | _pending_ | Acoustic capture test pending. |
| SX1262 LoRa Radio | Internal SPI | 2025-09-22 | 60ec05e | Console showed init success; initial and periodic transmit status 0. |

Update the summary table after every hardware run so that the system status is
immediately visible to reviewers and manufacturing partners.

---

## Component Test Procedures

### Air530 GPS Module (UART)

**Interface:** UART1 (GPS TX → GPIO44, GPS RX → GPIO43) @ 9600 bps

#### Required Parts
- Seeed XIAO ESP32S3 (LoRa variant or standard)
- Air530 (or compatible) GPS receiver
- Jumper wires and optional external antenna

#### Wiring

| Air530 Pin | XIAO ESP32S3 Pin | Notes |
| --- | --- | --- |
| VCC | 3V3 | Module tolerates 3.3–5 V; stay at 3.3 V for XIAO compatibility. |
| GND | GND | Common ground. |
| TX | GPIO44 (RX1) | GPS UART output. |
| RX | GPIO43 (TX1) | Leave disconnected if not sending commands. |
| PPS (optional) | GPIO10 | Optional 1PPS for time-discipline tests. |

#### Firmware Sketch

```cpp
#include <Arduino.h>

HardwareSerial GPS(1);

constexpr int GPS_RX_PIN = 44;  // XIAO pin receiving GPS TX
constexpr int GPS_TX_PIN = 43;  // XIAO pin driving GPS RX

void setup() {
  Serial.begin(115200);
  GPS.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.println("Air530 smoke test ready");
}

void loop() {
  while (GPS.available()) {
    Serial.write(GPS.read());
  }

  // Passthrough for NMEA command experiments if needed.
  while (Serial.available()) {
    GPS.write(Serial.read());
  }
}
```

This sketch bridges UART1 to the USB console so you can observe raw NMEA sentences
(`$GPGGA`, `$GPRMC`, etc.) as soon as the receiver powers up.

#### Test Steps

1. Flash the sketch and open the serial monitor at 115200 bps.
2. Verify that the console prints `Air530 smoke test ready` followed by NMEA
sentences.
3. Start indoors to confirm that the receiver reports `fix status = 0` and
HDOP values greater than 5.
4. Move the hardware outdoors with a clear view of the sky. Watch for the first
`GGA` sentence reporting `fix status = 1` and a decreasing HDOP value.
5. Log the time-to-first-fix, HDOP, and altitude in the result table.

#### Expected Results

- Indoor testing typically shows no valid fix and large HDOP values ( ≥ 99.9 ).
- Outdoors you should obtain a 3D fix within 30–90 s, HDOP below 1.5, and a
steady altitude reading consistent with your site.

#### Result Log

| Date | Firmware Commit | Observed Behavior |
| --- | --- | --- |
| 2025-09-17 | 60ec05e | No fix while indoors; once outdoors the receiver acquired a 3D lock in ~40 s with HDOP 0.9 and altitude ≈ 8 m. |

---

### Grove Vision AI V2 (I²C)

**Interface:** I²C @ 400 kHz (address 0x62)

#### Required Parts
- Seeed XIAO ESP32S3
- Grove Vision AI V2 camera module
- Grove-to-male jumper cable or breadboard adapter

#### Wiring

| Vision AI Pin | XIAO ESP32S3 Pin | Notes |
| --- | --- | --- |
| VCC | 3V3 | Module draws ~110 mA during inference. |
| GND | GND | Common ground. |
| SDA | GPIO5 | Shared I²C bus. |
| SCL | GPIO6 | Shared I²C bus. |
| INT (optional) | GPIO4 | Optional interrupt for event-driven capture. |

#### Firmware Sketch

```cpp
#include <Arduino.h>
#include <Wire.h>
#include <Seeed_Arduino_SSCMA.h>

constexpr int SDA_PIN = 5;
constexpr int SCL_PIN = 6;

SSCMA vision;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);

  if (!vision.begin(&Wire)) {
    Serial.println("Vision AI init failed");
    while (true) {
      delay(1000);
    }
  }

  vision.setDefaultModel();
  Serial.println("Vision AI ready");
}

void loop() {
  if (vision.invoke(1, false, false) == 0) {
    auto &boxes = vision.boxes();
    auto &classes = vision.classes();

    Serial.printf("Detected %u boxes\n", static_cast<unsigned int>(boxes.size()));
    for (size_t i = 0; i < boxes.size(); ++i) {
      const auto &box = boxes[i];
      Serial.printf(
          "  Box %u target %u score %u x=%u y=%u w=%u h=%u\n",
          static_cast<unsigned int>(i),
          static_cast<unsigned int>(box.target),
          static_cast<unsigned int>(box.score),
          static_cast<unsigned int>(box.x),
          static_cast<unsigned int>(box.y),
          static_cast<unsigned int>(box.w),
          static_cast<unsigned int>(box.h));
    }

    Serial.printf("Detected %u classes\n", static_cast<unsigned int>(classes.size()));
    for (size_t i = 0; i < classes.size(); ++i) {
      const auto &cls = classes[i];
      Serial.printf("  Class %u target %u score %u\n",
                    static_cast<unsigned int>(i),
                    static_cast<unsigned int>(cls.target),
                    static_cast<unsigned int>(cls.score));
    }
  } else {
    Serial.println("Invoke failed");
  }

  delay(200);
}
```

The module runs whichever model is currently flashed in its firmware. Successful
`vision.invoke(...)` calls populate the bounding-box and classification vectors,
which the sketch prints with raw confidence scores (0–100) for each detected
target.

#### Test Steps

1. Connect the module while the ESP32-S3 is powered off to avoid hot-plug
transients.
2. Flash the sketch and open the serial console.
3. Confirm that initialization succeeds (`Vision AI ready`). If it fails, inspect
the I²C wiring and power rails.
4. Present a high-contrast object or the bundled classification card to the
camera and confirm detections increment on the console.
5. Record detection counts, confidence scores, and any anomalies.

#### Expected Results

- Successful initialization prints `Vision AI ready` and produces object
  reports once the model recognises targets.
- Idle power should stay below 110 mA. If the board browns out, verify that your
USB supply can source sufficient current.

If repeated `Invoke failed` messages appear, re-check the 3V3 rail and consider
reflashing the module firmware; marginal power or stale firmware can cause the
inference routine to abort mid-cycle.

#### Result Log

| Date | Firmware Commit | Observed Behavior |
| --- | --- | --- |
| 2025-09-19 | b7130f2 | Majority of invokes printed `Invoke failed`; every few iterations one pass succeeded, reporting `Detected 1 boxes` with target 0 score 50 before the failures returned, pointing to unstable inference likely tied to power or firmware. |

---

### MicroSD Card (SPI)

**Interface:** SPI @ up to 20 MHz (SCK → GPIO7, MOSI → GPIO9, MISO → GPIO8, CS → GPIO3)

#### Required Parts
- Seeed XIAO ESP32S3 with expansion board or breakouts exposing SPI pins
- microSD card (FAT32 formatted)

#### Wiring

| microSD Pin | XIAO ESP32S3 Pin | Notes |
| --- | --- | --- |
| VCC | 3V3 | Use a level-shifted socket if the card requires 3.3 V I/O. |
| GND | GND | Common ground. |
| CS | GPIO3 | Chip-select (active low). |
| MOSI | GPIO9 | Data out from ESP32-S3. |
| MISO | GPIO8 | Data into ESP32-S3. |
| SCK | GPIO7 | SPI clock. |

#### Firmware Sketch

```cpp
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

constexpr uint8_t SD_CS_PIN = 3;
constexpr uint8_t SD_SCK_PIN = 7;
constexpr uint8_t SD_MISO_PIN = 8;
constexpr uint8_t SD_MOSI_PIN = 9;
constexpr char TEST_FILE[] = "/baltic.txt";
constexpr uint32_t SD_SPI_FREQ = 12000000;

void setup() {
  Serial.begin(115200);
  const uint32_t serial_start = millis();
  while (!Serial && (millis() - serial_start) < 4000) {
    delay(10);
  }

  Serial.println("Starting microSD smoke test");

  SPI.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
  pinMode(SD_CS_PIN, OUTPUT);
  digitalWrite(SD_CS_PIN, HIGH);

  if (!SD.begin(SD_CS_PIN, SPI, SD_SPI_FREQ)) {
    Serial.println("SD init failed");
    while (true) {
      delay(500);
    }
  }

  Serial.println("SD init succeeded");

  if (SD.exists(TEST_FILE)) {
    SD.remove(TEST_FILE);
  }

  File f = SD.open(TEST_FILE, FILE_WRITE);
  if (f) {
    if (f.println("SD card write test") > 0) {
      f.close();
      Serial.println("Write OK");
    } else {
      f.close();
      Serial.println("Write failed (println)");
    }
  } else {
    Serial.println("Write failed (open)");
  }

  f = SD.open(TEST_FILE, FILE_READ);
  if (f) {
    Serial.print("Read back: ");
    while (f.available()) {
      Serial.write(f.read());
    }
    Serial.println();
    f.close();
  } else {
    Serial.println("Read failed (open)");
  }

  Serial.println("microSD test complete");
}

void loop() {
  delay(1000);
}
```

#### Test Steps

1. Insert a known-good microSD card and connect the SPI lines as listed above.
2. Flash the sketch, open the serial console, and reset the board so you catch the startup banner (the firmware waits up to four seconds for USB Serial to come up).
3. Verify that initialization succeeds and `Write OK` is printed.
4. Confirm that the read-back string matches `SD card write test`.
5. Eject the card and check the file on a PC if deeper validation is required.

If you see `Write failed (open)` or `Read failed (open)` double-check the CS wiring and keep the leading `/` in `TEST_FILE` so the ESP32 SD driver opens the file from the root of the card.

#### Expected Results

- The console prints `Starting microSD smoke test`, `SD init succeeded`,
`Write OK`, and echoes the file contents.
- Initialization succeeds consistently. Repeated failures typically indicate
incorrect chip-select wiring or cards that require 1.8 V signalling.
- After several power cycles the file should persist and contain the expected
payload.

#### Result Log

| Date | Firmware Commit | Observed Behavior |
| --- | --- | --- |
| 2025-09-20 | 60ec05e | Console log showed `SD init succeeded`, `Write OK`, and `Read back: SD card write test` before `microSD test complete`. |

---

### SSD1315 OLED Display (I²C)

**Interface:** I²C @ 400 kHz (address 0x3C)

#### Required Parts
- Seeed XIAO ESP32S3
- Grove SSD1315 128×64 OLED or equivalent

#### Wiring

| OLED Pin | XIAO ESP32S3 Pin | Notes |
| --- | --- | --- |
| VCC | 3V3 | Display draws ~20 mA. |
| GND | GND | Common ground. |
| SDA | GPIO5 | Shared I²C bus. |
| SCL | GPIO6 | Shared I²C bus. |

#### Firmware Sketch

```cpp
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

constexpr uint8_t SCREEN_WIDTH = 128;
constexpr uint8_t SCREEN_HEIGHT = 64;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(115200);
  Wire.begin(5, 6);  // SDA, SCL

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed");
    while (true) {
      delay(1000);
    }
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Baltic");
  display.println("Monitor");
  display.display();
  Serial.println("OLED test pattern rendered");
}

void loop() {
  delay(1000);
}
```

#### Test Steps

1. Flash the sketch and observe the OLED. It should display the words
“Baltic” and “Monitor”.
2. Confirm that the serial console prints `OLED test pattern rendered`.
3. Power-cycle the board to ensure the display initialises reliably.

#### Expected Results

- The screen renders the static text without flickering.
- If the display stays blank, check the I²C pull-ups on the expansion board.

#### Result Log

| Date | Firmware Commit | Observed Behavior |
| --- | --- | --- |
| 2025-09-21 | 60ec05e | OLED test pattern rendered. |

---

### Hydrophone + Preamp (Analog ADC)

**Interface:** ESP32-S3 ADC1 channel 0 (GPIO1) sampled at 12-bit resolution

#### Required Parts
- Seeed XIAO ESP32S3
- INA332 (or equivalent) hydrophone preamplifier with mid-rail bias
- Hydrophone element and shielded audio cable

#### Wiring

| Signal | XIAO ESP32S3 Pin | Notes |
| --- | --- | --- |
| Preamp VCC | 3V3 | Ensure the preamp supply is filtered. |
| Preamp GND | GND | Tie the shield to ground at a single point. |
| Preamp Output | GPIO1 (ADC1_CH0) | Center the signal around 1.65 V. |
| Optional Power Gate | GPIO2 | Use for muting the preamp between bursts. |

#### Firmware Sketch

```cpp
#include <Arduino.h>
#include <driver/adc.h>

constexpr adc_channel_t HYDROPHONE_CHANNEL = ADC_CHANNEL_0;  // GPIO1

void setup() {
  Serial.begin(115200);
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(HYDROPHONE_CHANNEL, ADC_ATTEN_DB_11);  // 0-3.3 V
  Serial.println("Hydrophone level monitor ready");
}

void loop() {
  constexpr size_t kSamples = 256;
  int min_val = 4095;
  int max_val = 0;
  int32_t accum = 0;

  for (size_t i = 0; i < kSamples; ++i) {
    int sample = adc1_get_raw(HYDROPHONE_CHANNEL);
    min_val = min(min_val, sample);
    max_val = max(max_val, sample);
    accum += sample;
  }

  float avg = accum / static_cast<float>(kSamples);
  float peak_to_peak_volts = (max_val - min_val) * 3.3f / 4095.0f;
  Serial.printf("avg=%0.1f counts, p-p=%0.2f V\n", avg, peak_to_peak_volts);
  delay(200);
}
```

#### Test Steps

1. Power the preamp and confirm that its output is biased near half the supply
(~1.65 V).
2. Flash the sketch and open the serial console.
3. Observe the reported average value (should hover near 2048 counts) and
peak-to-peak voltage (<0.05 V in quiet water, >0.2 V when tapping the sensor).
4. Submerge the hydrophone in water and gently tap near it to confirm that the
peak-to-peak reading responds accordingly.
5. If available, capture the UART output to a CSV file for offline FFT analysis.

#### Expected Results

- Background noise yields a peak-to-peak level below 50 mV.
- Exciting the hydrophone (taps, tone generator) produces visibly higher
peak-to-peak values without saturating the ADC (stay < 3.0 Vpp).
- Excessive DC offsets indicate the bias network needs adjustment.

#### Result Log

| Date | Firmware Commit | Observed Behavior |
| --- | --- | --- |
| _pending_ | | |

---

### SX1262 LoRa Radio (Integrated SPI)

**Interface:** Internal SPI (CS → GPIO41, Busy → GPIO40, Reset → GPIO42, DIO1 → GPIO39)

#### Required Parts
- Seeed XIAO ESP32S3 **LoRa** variant (the radio is built in)
- Optional: second Meshtastic/LoRa node tuned to the same frequency for reception
verification

#### Firmware Sketch

```cpp
#include <Arduino.h>
#include <RadioLib.h>

SX1262 radio = new Module(41, 39, 42, 40);  // CS, DIO1, RESET, BUSY

void setup() {
  Serial.begin(115200);

  int state = radio.begin(915.0);
  if (state != RADIOLIB_ERR_NONE) {
    Serial.printf("Radio init failed: %d\n", state);
    while (true) {
      delay(1000);
    }
  }

  Serial.println("Radio ready, sending test packet");
  state = radio.transmit("hello baltic shoreline");
  Serial.printf("Initial transmit status: %d\n", state);
}

void loop() {
  delay(5000);
  int state = radio.transmit("ping");
  Serial.printf("Periodic transmit status: %d\n", state);
}
```

#### Test Steps

1. Flash the sketch and open the serial console.
2. Verify that `Radio ready, sending test packet` is printed with status `0`.
3. Monitor the periodic `ping` transmissions. Status `0` indicates success;
non-zero codes should be cross-referenced with the RadioLib error table.
4. If possible, confirm reception on a second Meshtastic node or SDR to verify RF
performance and antenna matching.

#### Expected Results

- Successful initialization and transmission yield status code `0`.
- If you observe `-5` (timeout) or similar errors, inspect the antenna connection
and ensure the regional frequency (e.g., 868 vs 915 MHz) matches your hardware.

#### Result Log

| Date | Firmware Commit | Observed Behavior |
| --- | --- | --- |
| 2025-09-22 | 60ec05e | Radio ready, sending test packet.<br>`Initial transmit status: 0`.<br>`Periodic transmit status: 0` (first loop).<br>`Periodic transmit status: 0` (second loop). |

---

## Test Log Guidelines

After executing any of the tests above:

- Update the component’s result table with the observed behaviour and add a new
row rather than overwriting history.
- Copy a condensed summary into the **Test Summary** table at the top of this
document for rapid status checks.
- File follow-up tickets for anomalies and link them next to the relevant table
row so regressions are easy to audit.

Maintaining accurate hardware test records ensures fielded buoys can be
assembled and serviced with confidence.
