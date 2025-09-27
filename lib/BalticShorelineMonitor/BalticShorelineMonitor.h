#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <TinyGPSPlus.h>
#include "DataTypes.h"

/**
 * Baltic Shoreline Monitor - Main sensor management class
 * 
 * Manages all environmental sensors for the marine monitoring buoy:
 * - GPS tracking for precise position monitoring
 * - Hydrophone audio analysis for marine life detection
 * - Grove Vision AI V2 for coastal monitoring
 * - Environmental sensors (temperature, humidity, pressure)
 * - Power management (battery, solar charging)
 * 
 * Designed to work with LoRa mesh networking for remote data transmission
 */
class BalticShorelineMonitor
{
public:
    BalticShorelineMonitor();
    
    // Initialization
    void begin();
    
    // Sensor management
    void readSensors();
    void updateGPSData(const GPSData& gpsData);
    
    // Data access
    GPSData getGPSData() const { return currentGPSData; }
    AudioData getAudioData() const { return currentAudioData; }
    VisionData getVisionData() const { return currentVisionData; }
    
    // Environmental data access
    float getTemperature() const { return temperature; }
    float getHumidity() const { return humidity; }
    float getPressure() const { return pressure; }
    float getWaterTemperature() const { return waterTemp; }
    float getBatteryVoltage() const { return batteryVoltage; }
    float getSolarVoltage() const { return solarVoltage; }
    
    // Data serialization
    String getSensorDataJson() const;
    String getStatusJson() const;
    
    // Message processing for LoRa communication
    void processReceivedMessage(const String& message);
    
    // Status
    bool isGPSValid() const { return currentGPSData.isValid; }
    bool areSensorsReady() const { return sensorsInitialized; }
    uint32_t getUptime() const { return millis() - startTime; }
    
private:
    void initializeSensors();
    void readGPSSensor();
    void readAudioSensor();
    void readVisionSensor();
    void readEnvironmentalSensors();
    void readPowerSensors();
    
    // Sensor data
    GPSData currentGPSData;
    AudioData currentAudioData;
    VisionData currentVisionData;
    
    // Environmental data
    float temperature = 0.0;        // Air temperature (°C)
    float humidity = 0.0;           // Relative humidity (%)
    float pressure = 0.0;           // Atmospheric pressure (hPa)
    float waterTemp = 0.0;          // Water temperature (°C)
    
    // Power management data
    float batteryVoltage = 0.0;     // Battery voltage (V)
    float solarVoltage = 0.0;       // Solar panel voltage (V)
    float batteryPercent = 0.0;     // Battery charge level (%)
    bool isCharging = false;        // Solar charging status
    
    // Status
    bool sensorsInitialized = false;
    uint32_t startTime = 0;
    uint32_t lastSensorRead = 0;
    
    // Statistics
    uint32_t transmissionCount = 0;
    uint32_t receptionCount = 0;
    
    // Configuration
    static const uint32_t SENSOR_READ_INTERVAL = 5000; // 5 seconds
};