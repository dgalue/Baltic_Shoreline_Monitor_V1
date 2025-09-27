#include "BalticShorelineMonitor.h"
#include <Wire.h>
#include <ArduinoJson.h>

BalticShorelineMonitor::BalticShorelineMonitor()
{
    startTime = millis();
}

void BalticShorelineMonitor::begin()
{
    Serial.println("Baltic Shoreline Monitor: Initializing...");
    
    initializeSensors();
    
    // Initialize data structures
    currentGPSData = GPSData();
    currentAudioData = AudioData();
    currentVisionData = VisionData();
    
    sensorsInitialized = true;
    
    Serial.println("Baltic Shoreline Monitor: Ready");
}

void BalticShorelineMonitor::initializeSensors()
{
    Serial.println("Initializing sensors...");
    
    // Initialize I2C for environmental sensors
    // Note: I2C initialization is done in main.cpp
    
    // TODO: Initialize specific sensors here
    // - Environmental sensors (BME280 or similar)
    // - Power monitoring (INA226 or similar)
    // - Grove Vision AI V2 communication
    
    Serial.println("Sensor initialization complete");
}

void BalticShorelineMonitor::readSensors()
{
    uint32_t currentTime = millis();
    
    if (currentTime - lastSensorRead < SENSOR_READ_INTERVAL) {
        return; // Too soon since last read
    }
    
    lastSensorRead = currentTime;
    
    Serial.println("Reading sensors...");
    
    // Read all sensor types
    readEnvironmentalSensors();
    readAudioSensor();
    readVisionSensor();
    readPowerSensors();
    
    Serial.println("Sensor reading complete");
}

void BalticShorelineMonitor::updateGPSData(const GPSData& gpsData)
{
    currentGPSData = gpsData;
}

void BalticShorelineMonitor::readGPSSensor()
{
    // GPS reading is handled in main.cpp and passed via updateGPSData()
}

void BalticShorelineMonitor::readAudioSensor()
{
    // TODO: Implement hydrophone audio analysis
    // For now, generate simulated data
    
    currentAudioData.frequency = 500.0 + (random(0, 1000) / 10.0); // 500-600 Hz
    currentAudioData.amplitude = 0.1 + (random(0, 500) / 10000.0); // 0.1-0.15
    currentAudioData.duration = 1.0 + (random(0, 100) / 100.0);    // 1.0-2.0 seconds
    currentAudioData.timestamp = millis();
    currentAudioData.isValid = true;
    
    // Simulate marine life detection
    if (random(0, 100) < 10) { // 10% chance of detecting something interesting
        currentAudioData.frequency = 1000.0 + random(0, 2000); // Higher frequency
        currentAudioData.amplitude = 0.3 + (random(0, 200) / 1000.0); // Stronger signal
        Serial.println("Audio: Detected interesting marine sound");
    }
}

void BalticShorelineMonitor::readVisionSensor()
{
    // TODO: Implement Grove Vision AI V2 communication
    // For now, generate simulated data
    
    currentVisionData.objectCount = random(0, 5);
    currentVisionData.averageSize = 10.0 + (random(0, 200) / 10.0); // 10-30 pixels
    currentVisionData.confidence = 0.7 + (random(0, 300) / 1000.0);  // 70-100%
    currentVisionData.timestamp = millis();
    currentVisionData.isValid = true;
    
    // Simulate debris detection
    if (random(0, 100) < 5) { // 5% chance of debris
        currentVisionData.objectCount = random(1, 3);
        currentVisionData.averageSize = 50.0 + random(0, 100); // Larger objects
        currentVisionData.confidence = 0.8 + (random(0, 200) / 1000.0);
        Serial.println("Vision: Detected potential marine debris");
    }
}

void BalticShorelineMonitor::readEnvironmentalSensors()
{
    // TODO: Implement real environmental sensors (BME280, DS18B20, etc.)
    // For now, generate realistic simulated data
    
    // Air temperature: 5-25°C (Baltic Sea range)
    temperature = 10.0 + (random(0, 1500) / 100.0);
    
    // Humidity: 60-95%
    humidity = 70.0 + (random(0, 2500) / 100.0);
    
    // Pressure: 980-1040 hPa
    pressure = 1000.0 + (random(0, 6000) / 100.0);
    
    // Water temperature: 2-20°C
    waterTemp = 5.0 + (random(0, 1500) / 100.0);
    
    Serial.printf("Environmental: T=%.1f°C, H=%.1f%%, P=%.1fhPa, WT=%.1f°C\\n", 
                  temperature, humidity, pressure, waterTemp);
}

void BalticShorelineMonitor::readPowerSensors()
{
    // TODO: Implement real power monitoring
    // For now, generate realistic simulated data
    
    // Battery voltage: 3.0-4.2V for Li-ion
    batteryVoltage = 3.5 + (random(0, 700) / 1000.0);
    
    // Calculate battery percentage (simplified)
    batteryPercent = ((batteryVoltage - 3.0) / 1.2) * 100.0;
    if (batteryPercent > 100.0) batteryPercent = 100.0;
    if (batteryPercent < 0.0) batteryPercent = 0.0;
    
    // Solar voltage: 0-6V depending on conditions
    int solarRandom = random(0, 100);
    if (solarRandom < 30) {
        solarVoltage = 0.0; // Night or very cloudy
        isCharging = false;
    } else if (solarRandom < 70) {
        solarVoltage = 2.0 + (random(0, 200) / 100.0); // Partial sun
        isCharging = (solarVoltage > 4.0);
    } else {
        solarVoltage = 4.5 + (random(0, 150) / 100.0); // Full sun
        isCharging = true;
    }
    
    Serial.printf("Power: Batt=%.2fV (%.0f%%), Solar=%.2fV, Charging=%s\\n",
                  batteryVoltage, batteryPercent, solarVoltage, isCharging ? "Yes" : "No");
}

String BalticShorelineMonitor::getSensorDataJson() const
{
    DynamicJsonDocument doc(1024);
    
    // Device identification
    doc["device"] = "BalticShorelineMonitor";
    doc["version"] = APP_VERSION;
    doc["timestamp"] = millis();
    doc["uptime"] = getUptime();
    
    // GPS data
    JsonObject gps = doc.createNestedObject("gps");
    gps["valid"] = currentGPSData.isValid;
    if (currentGPSData.isValid) {
        gps["lat"] = currentGPSData.latitude;
        gps["lon"] = currentGPSData.longitude;
        gps["alt"] = currentGPSData.altitude;
        gps["speed"] = currentGPSData.speed;
        gps["course"] = currentGPSData.course;
        gps["sats"] = currentGPSData.satellites;
        gps["hdop"] = currentGPSData.hdop;
    }
    
    // Environmental data
    JsonObject env = doc.createNestedObject("environmental");
    env["temperature"] = temperature;
    env["humidity"] = humidity;
    env["pressure"] = pressure;
    env["waterTemp"] = waterTemp;
    
    // Audio data
    JsonObject audio = doc.createNestedObject("audio");
    audio["valid"] = currentAudioData.isValid;
    if (currentAudioData.isValid) {
        audio["frequency"] = currentAudioData.frequency;
        audio["amplitude"] = currentAudioData.amplitude;
        audio["duration"] = currentAudioData.duration;
    }
    
    // Vision data
    JsonObject vision = doc.createNestedObject("vision");
    vision["valid"] = currentVisionData.isValid;
    if (currentVisionData.isValid) {
        vision["objects"] = currentVisionData.objectCount;
        vision["avgSize"] = currentVisionData.averageSize;
        vision["confidence"] = currentVisionData.confidence;
    }
    
    // Power data
    JsonObject power = doc.createNestedObject("power");
    power["battVoltage"] = batteryVoltage;
    power["battPercent"] = batteryPercent;
    power["solarVoltage"] = solarVoltage;
    power["charging"] = isCharging;
    
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

String BalticShorelineMonitor::getStatusJson() const
{
    DynamicJsonDocument doc(512);
    
    doc["device"] = "BalticShorelineMonitor";
    doc["status"] = "operational";
    doc["uptime"] = getUptime();
    doc["gpsValid"] = currentGPSData.isValid;
    doc["sensorsReady"] = sensorsInitialized;
    doc["batteryPercent"] = batteryPercent;
    doc["charging"] = isCharging;
    doc["transmissions"] = transmissionCount;
    doc["receptions"] = receptionCount;
    
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

void BalticShorelineMonitor::processReceivedMessage(const String& message)
{
    receptionCount++;
    
    Serial.println("Processing received message: " + message);
    
    // Try to parse as JSON
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, message);
    
    if (error) {
        Serial.println("Failed to parse received message as JSON");
        return;
    }
    
    // Process different message types
    String messageType = doc["type"] | "";
    
    if (messageType == "status_request") {
        Serial.println("Status request received - will respond on next transmission");
        // Note: In a real implementation, you might trigger an immediate response
    } else if (messageType == "config_update") {
        Serial.println("Configuration update received");
        // TODO: Handle configuration updates
    } else {
        Serial.println("Unknown message type received: " + messageType);
    }
}