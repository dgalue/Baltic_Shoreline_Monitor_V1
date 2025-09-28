# Baltic Shoreline Monitor - AI Coding Agent Instructions

## Project Architecture Overview

This is a **dual-approach ESP32-S3 firmware project** for marine environmental monitoring buoys with LoRa mesh networking:

1. **Standalone approach** (`src/main.cpp` + `lib/BalticShorelineMonitor/`) - Clean, modular implementation
2. **Meshtastic integration** (`src/meshtastic_baltic.*`) - Full mesh networking with display interface

The system monitors Baltic Sea conditions using multiple sensors and transmits data via LoRa mesh network.

## Critical Build Workflow

### Primary Build Targets
- **Standard build**: `pio run -e seeed_xiao_esp32s3` (recommended for development)
- **Meshtastic build**: `pio run -e baltic_meshtastic` (full mesh networking)
- **Upload**: Use `upload_baltic_meshtastic.bat` for Windows (handles ESP32-S3 boot sequence)

### Windows-Specific Requirements
- PowerShell commands use backslashes: `build\integration_test.exe`
- Manual boot sequence required: Hold BOOT button during upload until "Writing at..." appears
- COM port auto-detection via `COM*` wildcard in `platformio.ini`

## Data Architecture Patterns

### Core Data Types (`lib/BalticShorelineMonitor/DataTypes.h`)
All sensor data uses **timestamped structs** with `isValid` flags:
- `GPSData`: lat/lon/altitude/speed/course/satellites/hdop
- `AudioData`: frequency/amplitude/duration (hydrophone analysis)  
- `VisionData`: objectCount/averageSize/confidence (Grove Vision AI V2)

### JSON Serialization Pattern
The `BalticShorelineMonitor::getSensorDataJson()` method creates structured JSON with:
- Device metadata (version, uptime, transmission counts)
- All sensor readings with timestamps
- Environmental data (air/water temp, humidity, pressure)
- Power status (battery/solar voltages, charging state)

## Hardware Integration Points

### Pin Configuration (Seeed XIAO ESP32-S3)
- **I2C Bus**: SDA=Pin 4, SCL=Pin 5 (shared by OLED + Grove Vision AI V2)
- **LoRa SX1262**: SPI bus with specific CS/DIO pins in `meshtastic_baltic.cpp`  
- **GPS/Hydrophone**: UART interfaces (simulated in current implementation)

### Sensor Simulation Strategy
Current code uses **realistic simulated data** for all sensors. When implementing real sensors:
1. Replace simulation logic in `BalticShorelineMonitor::read*Sensor()` methods
2. Keep the same data structure interfaces (`GPSData`, `AudioData`, `VisionData`)
3. Maintain JSON serialization compatibility

## Mesh Networking Patterns

### Meshtastic Integration (`src/meshtastic_baltic.*`)
- **Node discovery**: Maintains `std::vector<BalticNode>` with RSSI/SNR tracking
- **Display modes**: 5-screen UI (status/nodes/environmental/LoRa/Baltic monitoring)
- **Packet structure**: Uses `MeshPacket` with Meshtastic-compatible fields
- **Environmental broadcasting**: Sends Baltic-specific telemetry every 60 seconds

### Standalone LoRa (`lib/BalticShorelineMonitor/`)
- **RadioLib integration**: Direct SX1262 control without Meshtastic protocol overhead
- **JSON payload**: Structured sensor data ready for mesh transmission
- **Modular design**: Easy to integrate with any mesh protocol

## Development Testing Patterns

### Host Integration Testing
- **Cross-platform build**: `g++ -std=c++17 tests/integration_test.cpp -Isrc -o build/integration_test`
- **Single translation unit**: Test automatically includes firmware sources
- **No RTOS dependencies**: Core sensor logic testable on desktop

### Serial Monitoring
- **Standard monitoring**: `pio device monitor -b 115200`
- **Expected output pattern**: "Baltic Shoreline Monitor - Starting..." → JSON sensor data every 5 seconds
- **Debugging**: All sensor readings logged to serial with timestamps

## Memory and Performance Constraints

### Resource Usage (ESP32-S3)
- **Current footprint**: 5.7% RAM (18,612 bytes), 4.0% Flash (264,049 bytes)
- **Sensor intervals**: 5-second reading cycle, 60-second mesh broadcasts
- **JSON payload size**: ~500-800 bytes per transmission (optimize for LoRa limits)

### Power Management (Solar Buoy)
- Battery/solar voltage monitoring in `BalticShorelineMonitor` class
- Sleep mode implementation needed for extended operation
- Environmental sensor power cycling for efficiency

## Project-Specific Conventions

### File Organization
- **Library approach**: Core functionality in `lib/BalticShorelineMonitor/` for reusability
- **Dual main files**: `main.cpp` (includes via header) vs `meshtastic_baltic.cpp` (standalone)
- **Documentation**: Hardware guides in root, assets in `docs/assets/`

### Naming Patterns
- **Classes**: PascalCase (`BalticShorelineMonitor`, `MeshPacket`)
- **Methods**: camelCase with descriptive prefixes (`getSensorDataJson`, `readGPSSensor`)
- **Constants**: UPPER_SNAKE_CASE (`SENSOR_READ_INTERVAL`)
- **Structs**: Descriptive suffixes (`GPSData`, `EnvironmentalData`)

### Error Handling
- **Graceful degradation**: Invalid sensor data marked with `isValid = false`
- **Simulation fallback**: Missing hardware doesn't break core functionality
- **Serial logging**: All major operations logged for debugging