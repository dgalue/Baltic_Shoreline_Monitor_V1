# 📘 ESP32-S3 Comprehensive System Guide: Meshtastic + Sensors + AI Integration

This comprehensive guide provides authoritative wiring, RTOS orchestration, and code scaffolding for a **single ESP32-S3** integrating multiple sensor systems with Meshtastic mesh networking capabilities.

---

## 🎯 System Overview

### Integrated Components
- **Meshtastic Mesh Networking** (LoRa-based, ESP-IDF + SX1262)
- **AI Vision Module** (Grove Vision AI V2 Himax HX6538 over I²C)
- **SSD1315 OLED Display** (I²C)
- **SD Card Storage** (SPI/SDMMC)
- **GPS Module** (Air530 via UART)
- **Hydrophone Audio Capture** (analog preamp → ADC)

### Core Architecture Principles
- **Core 0:** Meshtastic radio/routing tasks (leave untouched)
- **Core 1:** Custom FreeRTOS tasks (sensor, AI, logging, uplink)
- **Single-writer patterns** for shared resources
- **Event-driven communication** via FreeRTOS queues

> **Critical:** If flashing Meshtastic inside the same MCU, **pin your tasks to core 1** and keep radio/stack work unconstrained on core 0.

---

## 🔌 Hardware Configuration

### Pin Assignment (Authoritative)

#### I²C Bus (Shared)
- **SDA** → **GPIO5**
- **SCL** → **GPIO6**
- **Clock Speed:** 400 kHz
- **Devices:** AI Vision module, SSD1315 display

#### UART (GPS)
- **TX** → **GPIO43**
- **RX** → **GPIO44**
- **Baud Rate:** 9600/115200 (configurable)

#### SPI (SD Card)
- **SCK** → **GPIO7**
- **MISO** → **GPIO8**
- **MOSI** → **GPIO9**
- **CS** → **GPIO3**

#### Alternative SDMMC (4-bit mode)
- **CLK** → **GPIO14**
- **CMD** → **GPIO15**
- **D0** → **GPIO4**
- **D1** → **GPIO5**
- **D2** → **GPIO6**
- **D3** → **GPIO9**

#### Hydrophone (Analog)
- **Preamp:** INA332
- **Bias:** Mid-rail (2.5V)
- **Anti-aliasing:** RC filter (~7 kHz cutoff)
- **ADC Input:** ESP32-S3 ADC1 (DMA continuous mode)
- **Sample Rate:** 16 kHz, 12-bit

> **Note:** On some ESP32-S3 boards certain GPIOs are strapping/special. If boot issues occur with **GPIO3**, remap CS to a neutral pin and update this documentation.

### Peripheral Bindings Summary
- **AI Vision module:** I²C @ GPIO5/6
- **SSD1315 display:** I²C @ GPIO5/6 (address 0x3C)
- **SD card:** SPI @ GPIO7/8/9, **CS=GPIO3** OR SDMMC 4-bit mode
- **GPS (Air530):** UART @ GPIO43/44
- **Hydrophone ADC:** analog input → configured ADC channel

---

## ⚙️ Software Architecture (FreeRTOS)

### Task Map (Core, Priority, Ownership)

| Task | Core | Priority | Owns/Uses | Purpose |
|------|------|----------|-----------|----------|
| `Task_SenseAudio` | 1 | 4 | **ADC DMA** (read-only), `q_audio` (TX) | Stream hydrophone samples in 32 ms windows |
| `Task_AI_Audio` | 1 | 5 | `q_audio` (RX), `q_events` (TX) | MFCC → TFLM model → acoustic events |
| `Task_GPS` | 1 | 3 | **UART1** (RX), PPS GPIO ISR, timekeeper | Parse RMC/GGA; discipline timestamps |
| `Task_Vision` | 1 | 4 | **I²C** (shared, via `mtx_i2c`), `q_events` (TX) | Poll vision module (I²C) → detections |
| `Task_Display` | 1 | 2 | **I²C** (shared, via `mtx_i2c`) | Minimal UI/heartbeat; rate-limited |
| `Task_Logger` | 1 | 3 | **SPI/SD** (exclusive owner), `q_log` (RX) | Filesystem writes & rotation (only writer) |
| `Task_Uplink` | 1 | 4 | `q_events` (RX), `q_log` (TX), app-data API | Pack & send compact events; optional SD index |
| `Task_PowerMgr` | 1 | 3 | ADC (slow), GPIO gates | Duty-cycle vision/sensing; brown-out guards |

> **Important:** Keep **Meshtastic/radio/Wi-Fi/BLE** (if any) free to schedule on **core 0**. Never raise your task priority above time-critical radio tasks.

### Concurrency Rules
- **I²C is shared** → all I²C access must take **`mtx_i2c`**. Display must not starve the vision module.
- **SD card is single-writer** → **only `Task_Logger`** touches FATFS; others push buffers to `q_log`.
- **No blocking SPI/I²C** inside AI/audio hot paths. Always queue.
- **Watchdog** → subscribe long-running tasks to TWDT or yield at ≤ 100 ms.

---

## 📦 Messaging & Buffers

### Queue Configuration
- **`q_audio`:** 8× blocks of **512 samples** (int16) = 32 ms @ 16 kHz; 50% overlap optional
- **`q_events`:** 64× event structs (≤ 32 bytes each)
- **`q_log`:** 32× variable-size blobs (e.g., 256–4096 bytes)

### Event Payload Structure (Binary, Compact)
```c
typedef struct __attribute__((packed)) {
  uint8_t  version;        // 0x01
  uint8_t  type;           // 0=acoustic, 1=vision, 2=health
  uint8_t  confidence_q7;  // 0..100 mapped to 0..127
  uint8_t  flags;          // bitfield (lowpower, retrigger, etc.)
  uint32_t ts_unix;        // seconds
  int32_t  lat_e7;         // 1e-7 deg
  int32_t  lon_e7;         // 1e-7 deg
  uint16_t feat_hash;      // optional locality-sensitive hash
  uint32_t sd_offset;      // where to find snippet in SD (optional)
} event_t;
```
**Typical length:** 20–24 bytes. Suitable for Meshtastic app-data packets.

---

## 🚀 Initialization Sequence (Authoritative)

1. **Clock/PSRAM** (if any), basic NVS, logging setup
2. **GPIO** (set safe output levels; power-gate rails off)
3. **I²C** (`I2C_NUM_0 @ 400 kHz`), create **`mtx_i2c`**
4. **SPI bus**, **SD mount** → start `Task_Logger`
5. **UART (GPS)** + PPS GPIO ISR → start `Task_GPS`
6. **ADC continuous DMA** → start `Task_SenseAudio` and `Task_AI_Audio`
7. **Vision module** (I²C ping, config) → start `Task_Vision`
8. **Display** minimal UI → start `Task_Display`
9. **Uplink** + `Task_PowerMgr`
10. If present, **Meshtastic app-data binding**

**Fail-safe:** If SD mount fails, run degraded mode (events buffered in RAM + backpressure).

---

## 🔋 Power Management & Duty-Cycling
- **3.7V 9000mAh LiPo**

### Power Policies
- **Vision module:** default **OFF**. Enable N seconds every M minutes, or **on acoustic trigger**
- **Audio:** 5 s bursts every 2–5 min; increase duty on recent hits
- **Display:** off by default; pulse heartbeat icon every 5 s when enabled

### Target Performance
- **Average draw:** < 50 mA @ 3.3V
- **Keep Wi-Fi disabled:** BLE only for configuration
- **Gate Vision AI and preamp rails** with load switches

---

## 💾 Filesystem & Data Management

### File Structure
- **Path pattern:** `/sdcard/LOG/YYYYMMDD/HH.ndx` (index) + `/sdcard/LOG/YYYYMMDD/HH.bin` (data)
- **Rotation:** delete oldest day when **free < 10%** or **> N days**
- **`Task_Logger` writes append-only:** pre-alloc optional to reduce wear

### Logging Strategy
- **Daily folders, hourly files**
- **Index file:** `events.idx` with `{ts, type, sd_offset, len}`
- **Compression:** optional gzip/heatshrink blocks during idle time
- **Rotation:** delete oldest files when free space < 10%

---

## 💻 Code Implementation

### Common Defines & Handles
```c
// Pin definitions
#define I2C_SDA   5
#define I2C_SCL   6
#define UART_TX  43
#define UART_RX  44
#define SPI_SCK   7
#define SPI_MISO  8
#define SPI_MOSI  9
#define SPI_CS    3

// FreeRTOS handles
QueueHandle_t q_audio, q_events, q_log;
SemaphoreHandle_t mtx_i2c;

// Event structure
typedef struct __attribute__((packed)) {
  uint8_t  ver, type, conf_q7, flags;
  uint32_t ts_unix;
  int32_t  lat_e7, lon_e7;
  uint16_t feat_hash;
  uint32_t sd_offset;
} event_t;
```

### I²C Initialization (Shared Bus)
```c
void i2c_init(void){
  i2c_config_t c = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = I2C_SDA,
    .scl_io_num = I2C_SCL,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = 400000
  };
  ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &c));
  ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));
  mtx_i2c = xSemaphoreCreateMutex();
}
```

### UART (GPS) Initialization
```c
void gps_uart_init(void){
  uart_config_t u = {
    .baud_rate = 9600, .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE, .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
  };
  ESP_ERROR_CHECK(uart_param_config(UART_NUM_1, &u));
  ESP_ERROR_CHECK(uart_set_pin(UART_NUM_1, UART_TX, UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
  ESP_ERROR_CHECK(uart_driver_install(UART_NUM_1, 2048, 0, 0, NULL, 0));
}
```

### SPI + SD Card Initialization
```c
void sd_init(void){
  spi_bus_config_t bus = {
    .miso_io_num = SPI_MISO, .mosi_io_num = SPI_MOSI, .sclk_io_num = SPI_SCK,
    .quadwp_io_num = -1, .quadhd_io_num = -1
  };
  ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus, SPI_DMA_CH_AUTO));
  sdmmc_host_t host = SDSPI_HOST_DEFAULT();
  sdspi_slot_config_t slot = SDSPI_SLOT_CONFIG_DEFAULT();
  slot.gpio_cs = SPI_CS; slot.host_id = SPI2_HOST;

  esp_vfs_fat_sdmmc_mount_config_t mcfg = {
    .format_if_mount_failed = false, .max_files = 4, .allocation_unit_size = 16 * 1024
  };
  sdmmc_card_t *card;
  ESP_ERROR_CHECK(esp_vfs_fat_sdspi_mount("/sdcard", &host, &slot, &mcfg, &card));
}
```

### ADC Continuous Mode Setup
```c
adc_continuous_handle_t adc_handle;

void adc_init(void){
  adc_continuous_config_t adc_config = {
    .sample_freq_hz = 16000,
    .conv_mode = ADC_CONV_SINGLE_UNIT_1,
    .format = ADC_DIGI_OUTPUT_FORMAT_TYPE2,
    .pattern_num = 1,
  };
  ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &adc_handle));
  ESP_ERROR_CHECK(adc_continuous_start(adc_handle));
}
```

### Task Creation (Pinned to Core 1)
```c
void app_start_tasks(void){
  q_audio  = xQueueCreate(8, 512 * sizeof(int16_t));
  q_events = xQueueCreate(64, sizeof(event_t));
  q_log    = xQueueCreate(32, sizeof(void*));

  xTaskCreatePinnedToCore(Task_SenseAudio, "sense",  4096, NULL, 4, NULL, 1);
  xTaskCreatePinnedToCore(Task_AI_Audio,  "ai_aud", 8192, NULL, 5, NULL, 1);
  xTaskCreatePinnedToCore(Task_GPS,       "gps",    3072, NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(Task_Vision,    "vision", 4096, NULL, 4, NULL, 1);
  xTaskCreatePinnedToCore(Task_Display,   "disp",   3072, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(Task_Logger,    "logger", 4096, NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(Task_Uplink,    "uplink", 4096, NULL, 4, NULL, 1);
  xTaskCreatePinnedToCore(Task_PowerMgr,  "pwr",    3072, NULL, 3, NULL, 1);
}
```

### I²C Usage Pattern (Mutex-Protected)
```c
esp_err_t i2c_write_reg(uint8_t addr, uint8_t reg, uint8_t val){
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (addr<<1) | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(cmd, reg, true);
  i2c_master_write_byte(cmd, val, true);
  i2c_master_stop(cmd);
  
  xSemaphoreTake(mtx_i2c, portMAX_DELAY);
  esp_err_t r = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(50));
  xSemaphoreGive(mtx_i2c);
  
  i2c_cmd_link_delete(cmd);
  return r;
}
```

---

## 🤖 AI Processing Configuration

### Audio Processing
- **Window:** 32 ms (512 samples) with 50% overlap
- **Features:** 32 mel bins
- **Model:** Tiny TFLM CNN, int8 quantized
- **Inference duty:** < 20% CPU budget

### Vision Processing
- **Use Himax WiseEye2 module inference results directly**
- **Limit checks:** 1–2 inferences per interval (e.g., every 5–10 min)
- **Optionally wake on audio event trigger**
- **Power gated via load switch**

---

## 📡 Meshtastic Integration

### Application Data Integration
- **Use Application Data Packets** to send `event_t` on a dedicated channel
- **Event size:** ~22–24 bytes → safe for LoRa payload limits
- **Do not modify routing or protocol internals**
- **Compact event uplink** via Meshtastic Application Data Path

---

## 🛡️ Error Handling & Health Monitoring

### Error Recovery Strategies
- **I²C errors:** 3 NACKs → reset target, backoff 500 ms, log `E_I2C_xxx`
- **SD full:** drop low-priority events first
- **GPS loss:** >60 s → mark invalid lat/lon
- **Audio overload:** if `q_audio` ≥ 75%, drop windows
- **Power low:** raise health event

### Backpressure Management
- **Queue monitoring:** implement watermark-based flow control
- **Graceful degradation:** reduce sampling rates under stress
- **Watchdog integration:** ensure all tasks yield appropriately

---

## 🔧 Development Sequence

1. **Build Meshtastic ESP-IDF** for ESP32-S3
2. **Add components:** `sensing_audio`, `gps_time`, `vision_bridge`, `logger`, `uplink`
3. **Bring up SDMMC** and verify logging
4. **Add GPS + PPS alignment** → check RTC drift <1 ms
5. **Add ADC continuous sampling** → check timing stability
6. **Add MFCC + TFLM** → benchmark inference CPU load
7. **Add Vision AI I²C parsing** → confirm events generated
8. **Implement event uplink to Meshtastic** → test on real mesh
9. **Stress test** with simultaneous logging + LoRa TX
10. **Tune task priorities** and enable TWDT watchdog

---

## ⚠️ Critical Gotchas

- **Never block radio tasks:** keep your tasks ≤ priority 5
- **Only `Task_Logger` may write to SD**
- **No raw audio/video over LoRa:** transmit compact events only
- **Use mutexes for I²C/SPI buses**
- **Place ML arena and hot buffers in internal RAM:** use PSRAM only for cold storage
- **Verify GPIO strapping pins** before finalizing hardware

---

## 📋 Configuration Template

```yaml
hardware:
  mcu: esp32-s3
  pins:
    i2c: { sda: 5, scl: 6 }
    uart: { tx: 43, rx: 44 }
    spi: { sck: 7, miso: 8, mosi: 9, cs: 3 }
peripherals:
  vision: { iface: i2c, sda: 5, scl: 6 }
  display_ssd1315: { iface: i2c, sda: 5, scl: 6, addr: 0x3C }
  sdcard: { iface: spi, sck: 7, miso: 8, mosi: 9, cs: 3, mount: "/sdcard" }
  gps_air530: { iface: uart, tx: 43, rx: 44, baud: 9600 }
rtos:
  core_affinity:
    meshtastic_radio: 0
    app_tasks: 1
  tasks:
    - { name: Task_SenseAudio, prio: 4, core: 1 }
    - { name: Task_AI_Audio, prio: 5, core: 1 }
    - { name: Task_GPS, prio: 3, core: 1 }
    - { name: Task_Vision, prio: 4, core: 1 }
```

---

## 📚 Additional Resources

- **ESP-IDF Documentation:** [docs.espressif.com](https://docs.espressif.com)
- **Meshtastic Firmware:** [github.com/meshtastic/firmware](https://github.com/meshtastic/firmware)
- **FreeRTOS Reference:** [freertos.org](https://freertos.org)
- **TensorFlow Lite Micro:** [tensorflow.org/lite/microcontrollers](https://tensorflow.org/lite/microcontrollers)

---

*This document serves as the authoritative reference for ESP32-S3 Meshtastic + sensor integration. Keep it updated as the system evolves.*
