#include <Arduino.h>
#include "BalticShorelineMonitor.h"

BalticShorelineMonitor monitor;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("Baltic Shoreline Monitor - Starting...");
    monitor.begin();
    Serial.println("Ready!");
}

void loop() {
    monitor.readSensors();
    
    String data = monitor.getSensorDataJson();
    Serial.println("Sensor data: " + data);
    
    delay(5000);
}
