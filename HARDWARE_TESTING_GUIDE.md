# Baltic Shoreline Monitor - Hardware Testing Guide

## Quick Start Testing (No External Sensors)

### 1. Flash the Firmware

Connect your Seeed XIAO ESP32-S3 to your computer via USB-C and run:

```bash
# Upload firmware to the device
pio run -e seeed_xiao_esp32s3 --target upload

# Start serial monitor to see output
pio device monitor -e seeed_xiao_esp32s3
```

**Expected Output:**
```
Baltic Shoreline Monitor - Starting...
Baltic Shoreline Monitor: Initializing...
Initializing sensors...
Sensor initialization complete
Baltic Shoreline Monitor: Ready
Ready!
Reading sensors...
Sensor reading complete
Sensor data: {"device":"BalticShorelineMonitor","version":"1.0.0","timestamp":5023,"uptime":5023, ...}
```

### 2. What You'll See (Simulated Data)

The firmware runs with simulated sensor data, so you'll see:
- **GPS Data**: Simulated coordinates (currently invalid since no real GPS)
- **Environmental Data**: Realistic temperature, humidity, pressure values
- **Audio Data**: Simulated hydrophone readings with frequency/amplitude
- **Vision Data**: Simulated object detection results
- **Power Data**: Simulated battery voltage and solar charging status

## Pin Connections for Real Sensors

### Essential Connections (Seeed XIAO ESP32-S3)

```
Power:
- VCC: 3.3V or 5V (depending on sensor)
- GND: Ground

I2C Bus (Shared):
- SDA: Pin 4 (D4)
- SCL: Pin 5 (D5)

SPI Bus for LoRa SX1262:
- CS:   Pin 7 (D7)
- MOSI: Pin 10 (D10) - ESP32-S3 default
- MISO: Pin 9 (D9)   - ESP32-S3 default  
- SCK:  Pin 8 (D8)   - ESP32-S3 default
- DIO1: Pin 9 (D9)   - Interrupt pin
- RST:  Pin 8 (D8)   - Reset pin
- BUSY: Pin 10 (D10) - Busy status

GPS Module (UART):
- RX: Pin 44 (GPS_RX_PIN) - Connect to GPS TX
- TX: Pin 43 (GPS_TX_PIN) - Connect to GPS RX
- VCC: 3.3V
- GND: Ground

SD Card (SPI):
- CS:   Pin 21
- MOSI: Pin 35
- MISO: Pin 37
- SCK:  Pin 36

User Interface:
- Button: Pin 0 (Built-in boot button)
- LED:    Pin 21 (Status indicator)
```

### OLED Display (I2C - 0x3C)
- Connect to shared I2C bus (pins 4&5)
- 0.96" SSD1306 OLED recommended

### Grove Vision AI V2
- Connect to I2C bus or configure for serial communication
- Refer to Grove documentation for specific protocol

## Testing Steps by Component

### 1. Basic Functionality Test
```bash
# Flash and monitor
pio run -e seeed_xiao_esp32s3 -t upload -t monitor
```

**Look for:**
- ✅ Successful initialization message
- ✅ JSON sensor data every 5 seconds
- ✅ No error messages or crashes
- ✅ Stable operation over time

### 2. GPS Module Test

**Hardware:** Connect GPS module to pins 43/44

**Expected Changes:**
- GPS data changes from `"valid":false` to `"valid":true`
- Real latitude/longitude coordinates appear
- Satellite count > 0
- HDOP values become reasonable (< 10.0)

**Troubleshooting:**
- GPS needs clear sky view for first fix (can take 5-15 minutes)
- Indoor testing may not get GPS lock
- Check UART connections and baud rate (9600)

### 3. LoRa Radio Test

**Hardware:** Connect SX1262 LoRa module

**Expected Changes:**
- No "FAILED" messages during LoRa initialization
- Ready for mesh networking (future feature)

### 4. SD Card Test

**Hardware:** Connect SD card module

**Expected Changes:**
- SD initialization success message
- Log file creation on SD card
- Sensor data logging to `/baltmon.log`

## Serial Monitor Output Analysis

### Normal Operation
```
Baltic Shoreline Monitor - Starting...
Version: 1.0.0
Initializing hardware...
Initializing sensors...
Sensor initialization complete
Baltic Shoreline Monitor: Ready
Ready!
Reading sensors...
Environmental: T=15.2°C, H=78.3%, P=1015.2hPa, WT=12.1°C
Power: Batt=3.67V (58%), Solar=4.23V, Charging=Yes
Sensor reading complete
Sensor data: {"device":"BalticShorelineMonitor","version":"1.0.0",...}
```

### Error Indicators
- **Memory issues**: Stack overflow, heap corruption
- **Hardware failures**: Sensor initialization failures
- **Communication errors**: I2C/SPI/UART timeout messages

## Performance Metrics

**Current Measurements:**
- **RAM Usage**: ~18.6KB (5.7% of 320KB)
- **Flash Usage**: ~264KB (4.0% of 6.5MB)
- **Loop Cycle**: 5 seconds per sensor reading
- **Data Rate**: ~1KB JSON per cycle

## Next Development Steps

### Immediate (Current Code)
1. **Verify Basic Operation**: Confirm stable 5-second cycles
2. **Power Consumption**: Measure current draw
3. **Memory Stability**: Run for extended periods

### Hardware Integration
1. **GPS Integration**: Replace simulated GPS with real readings
2. **LoRa Networking**: Implement mesh communication
3. **Sensor Drivers**: Add real environmental sensors
4. **Power Management**: Implement sleep modes

### Testing Checklist

- [ ] Firmware uploads successfully
- [ ] Serial output shows regular sensor readings
- [ ] No crashes or resets during extended operation
- [ ] JSON data format is valid and complete
- [ ] GPS module provides valid coordinates (outdoor test)
- [ ] LoRa module initializes without errors
- [ ] SD card logging works (if connected)
- [ ] Power consumption is reasonable for solar operation

## Troubleshooting

### Upload Issues
```bash
# If upload fails, try holding BOOT button during upload
# or reset the device and try again
pio run -e seeed_xiao_esp32s3 -t upload --upload-port COM[X]
```

### Serial Monitor Issues
```bash
# Specify exact port if auto-detection fails
pio device monitor --port COM[X] --baud 115200
```

### Memory Issues
- Monitor for stack overflows or heap fragmentation
- Reduce JSON buffer sizes if needed
- Check for memory leaks in long-running tests

## Safety Notes

- **Power Supply**: Ensure stable 3.3V/5V supply
- **ESD Protection**: Handle ESP32-S3 with anti-static precautions
- **Connections**: Double-check wiring before powering on
- **GPS Antenna**: Use proper GPS antenna for outdoor testing

The firmware is designed to be robust and should handle missing sensors gracefully, so you can start with just the ESP32-S3 board and add components incrementally!