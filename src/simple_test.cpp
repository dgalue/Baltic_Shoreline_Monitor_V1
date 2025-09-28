#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("=== Baltic XIAO ESP32-S3 Test ===");
  Serial.println("Hardware test starting...");
  
  // Initialize LED
  pinMode(48, OUTPUT);
  
  Serial.println("Setup complete!");
}

void loop() {
  static unsigned long lastUpdate = 0;
  static bool ledState = false;
  
  if (millis() - lastUpdate > 1000) {
    lastUpdate = millis();
    ledState = !ledState;
    digitalWrite(48, ledState);
    
    Serial.print("Test running... LED: ");
    Serial.println(ledState ? "ON" : "OFF");
  }
}