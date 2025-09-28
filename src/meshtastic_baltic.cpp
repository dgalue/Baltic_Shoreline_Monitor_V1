#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <SPI.h>
#include <RadioLib.h>
#include <ArduinoJson.h>
#include <vector>
#include "meshtastic_baltic.h"

// Hardware Configuration for XIAO ESP32-S3 with Expansion Board
#define DISPLAY_SDA 5
#define DISPLAY_SCL 6
#define BUTTON_PIN 21
#define LED_PIN 48

// LoRa Configuration (SX1262 on expansion board)
#define LORA_MISO 8
#define LORA_MOSI 9
#define LORA_SCK 7
#define LORA_CS 41
#define LORA_RST 42
#define LORA_DIO1 39
#define LORA_BUSY 40
#define LORA_RXEN 38

// Meshtastic-compatible configuration
#define MESHTASTIC_FREQUENCY 915.0  // MHz (adjust for your region)
#define MESHTASTIC_BANDWIDTH 125.0  // kHz
#define MESHTASTIC_SPREADING_FACTOR 10
#define MESHTASTIC_CODING_RATE 5

// Device configuration
U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE, DISPLAY_SCL, DISPLAY_SDA);
SX1262 radio = new Module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY);

// Node information
// Struct definitions are in meshtastic_baltic.h

// Global variables
BalticNode myNode;
EnvironmentalData currentEnvData;
std::vector<BalticNode> nodeDatabase;
unsigned long lastTelemetryBroadcast = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastEnvironmentalRead = 0;
int displayMode = 0;
bool buttonPressed = false;
bool ledState = false;

// Function declarations are in meshtastic_baltic.h

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("=== Baltic Shoreline Meshtastic Node ===");
  Serial.println("Initializing hardware...");
  
  initializeHardware();
  initializeLoRa();
  
  // Generate unique node ID from MAC address
  myNode.nodeId = ESP.getEfuseMac() & 0xFFFFFF;
  myNode.nodeName = "Baltic-" + String(myNode.nodeId, HEX);
  myNode.shortName = "BS-" + String(myNode.nodeId & 0xFFF, HEX);
  myNode.latitude = 59.3293;  // Example: Stockholm coordinates
  myNode.longitude = 18.0686;
  myNode.lastSeen = millis();
  
  Serial.println("Node ID: " + String(myNode.nodeId, HEX));
  Serial.println("Node Name: " + myNode.nodeName);
  Serial.println("Baltic Shoreline Monitor ready!");
  
  // Initial broadcasts
  broadcastNodeInfo();
  updateDisplay();
}

void loop() {
  static unsigned long lastButtonCheck = 0;
  static bool lastButtonState = HIGH;
  
  // Button handling
  if (millis() - lastButtonCheck > 50) {
    lastButtonCheck = millis();
    bool currentButtonState = digitalRead(BUTTON_PIN);
    if (lastButtonState == HIGH && currentButtonState == LOW) {
      handleButtonPress();
    }
    lastButtonState = currentButtonState;
  }
  
  // Receive LoRa packets
  receiveMeshPacket();
  
  // Periodic environmental sensor reading
  if (millis() - lastEnvironmentalRead > 10000) {  // Every 10 seconds
    readEnvironmentalSensors();
    lastEnvironmentalRead = millis();
  }
  
  // Periodic telemetry broadcast
  if (millis() - lastTelemetryBroadcast > 300000) {  // Every 5 minutes
    broadcastTelemetry();
    lastTelemetryBroadcast = millis();
  }
  
  // Update display
  if (millis() - lastDisplayUpdate > 1000) {
    updateDisplay();
    lastDisplayUpdate = millis();
  }
  
  // LED heartbeat
  static unsigned long lastBlink = 0;
  if (millis() - lastBlink > 1000) {
    lastBlink = millis();
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  }
  
  delay(10);
}

void initializeHardware() {
  // Initialize pins
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Initialize I2C for display
  Wire.begin(DISPLAY_SDA, DISPLAY_SCL);
  
  // Initialize display
  display.begin();
  display.clearBuffer();
  display.setFont(u8g2_font_6x10_tf);
  display.drawStr(0, 20, "Initializing...");
  display.drawStr(0, 35, "Baltic Shoreline");
  display.drawStr(0, 50, "Meshtastic Node");
  display.sendBuffer();
  
  Serial.println("Hardware initialized successfully!");
}

void initializeLoRa() {
  // Initialize SPI for LoRa
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
  
  // Initialize LoRa radio
  Serial.print("Initializing SX1262... ");
  int state = radio.begin(MESHTASTIC_FREQUENCY, MESHTASTIC_BANDWIDTH, MESHTASTIC_SPREADING_FACTOR, MESHTASTIC_CODING_RATE, 0x12, 14, 8, 0, false);
  
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("success!");
    
    // Set RF switching (for SX1262 with RXEN/TXEN)
    radio.setRfSwitchPins(LORA_RXEN, RADIOLIB_NC);
    
    // Set interrupt action
    radio.setPacketReceivedAction([](void) {
      // Interrupt flag for packet received
    });
    
    // Start listening
    radio.startReceive();
    
  } else {
    Serial.println("failed, code " + String(state));
    while (true) {
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(LED_PIN, LOW);
      delay(100);
    }
  }
}

void readEnvironmentalSensors() {
  // Simulate environmental sensors for Baltic Sea monitoring
  // In a real implementation, you would read actual sensors here
  
  currentEnvData.waterTemperature = 12.5 + (random(-50, 50) / 10.0);  // Baltic Sea typical range
  currentEnvData.airTemperature = 15.2 + (random(-100, 100) / 10.0);
  currentEnvData.humidity = 65.0 + (random(-200, 200) / 10.0);
  currentEnvData.pressure = 1013.2 + (random(-50, 50) / 10.0);
  currentEnvData.windSpeed = 5.5 + (random(0, 100) / 10.0);
  currentEnvData.windDirection = random(0, 360);
  currentEnvData.waveHeight = 0.8 + (random(0, 150) / 100.0);
  currentEnvData.waterQuality = random(70, 95);  // Quality index 0-100
  currentEnvData.timestamp = millis();
  
  Serial.println("Environmental data updated:");
  Serial.println("  Water Temp: " + String(currentEnvData.waterTemperature) + "°C");
  Serial.println("  Air Temp: " + String(currentEnvData.airTemperature) + "°C");
  Serial.println("  Wave Height: " + String(currentEnvData.waveHeight) + "m");
}

void broadcastNodeInfo() {
  MeshPacket packet;
  packet.from = myNode.nodeId;
  packet.to = 0xFFFFFFFF;  // Broadcast
  packet.hopLimit = 3;
  packet.hopStart = 3;
  packet.id = random(0xFFFFFFFF);
  packet.payloadType = 1;  // Node info
  
  // Create JSON payload
  JsonDocument doc;
  doc["type"] = "nodeinfo";
  doc["id"] = myNode.nodeId;
  doc["name"] = myNode.nodeName;
  doc["short"] = myNode.shortName;
  doc["lat"] = myNode.latitude;
  doc["lon"] = myNode.longitude;
  doc["time"] = millis();
  
  serializeJson(doc, packet.payload);
  sendMeshPacket(packet);
  
  Serial.println("Broadcasted node info");
}

void broadcastTelemetry() {
  MeshPacket packet;
  packet.from = myNode.nodeId;
  packet.to = 0xFFFFFFFF;  // Broadcast
  packet.hopLimit = 3;
  packet.hopStart = 3;
  packet.id = random(0xFFFFFFFF);
  packet.payloadType = 2;  // Telemetry
  
  // Create JSON payload with Baltic environmental data
  JsonDocument doc;
  doc["type"] = "telemetry";
  doc["id"] = myNode.nodeId;
  doc["water_temp"] = currentEnvData.waterTemperature;
  doc["air_temp"] = currentEnvData.airTemperature;
  doc["humidity"] = currentEnvData.humidity;
  doc["pressure"] = currentEnvData.pressure;
  doc["wind_speed"] = currentEnvData.windSpeed;
  doc["wind_dir"] = currentEnvData.windDirection;
  doc["wave_height"] = currentEnvData.waveHeight;
  doc["water_quality"] = currentEnvData.waterQuality;
  doc["battery"] = 85;  // Simulated battery level
  doc["time"] = millis();
  
  serializeJson(doc, packet.payload);
  sendMeshPacket(packet);
  
  Serial.println("Broadcasted telemetry data");
}

void sendMeshPacket(MeshPacket& packet) {
  // Create packet header + JSON payload
  String fullPacket = String(packet.from, HEX) + ":" + String(packet.to, HEX) + ":" + 
                     String(packet.hopLimit) + ":" + String(packet.id, HEX) + ":" + packet.payload;
  
  Serial.println("Transmitting: " + fullPacket.substring(0, 50) + "...");
  
  int state = radio.transmit(fullPacket);
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("Packet sent successfully!");
  } else {
    Serial.println("Transmission failed, code " + String(state));
  }
  
  // Return to receive mode
  radio.startReceive();
}

void receiveMeshPacket() {
  String receivedData;
  int state = radio.readData(receivedData);
  
  if (state == RADIOLIB_ERR_NONE && receivedData.length() > 0) {
    float rssi = radio.getRSSI();
    float snr = radio.getSNR();
    
    Serial.println("Received packet: " + receivedData.substring(0, 50) + "...");
    Serial.println("RSSI: " + String(rssi) + " dBm, SNR: " + String(snr) + " dB");
    
    // Parse packet (simplified)
    int firstColon = receivedData.indexOf(':');
    int secondColon = receivedData.indexOf(':', firstColon + 1);
    int thirdColon = receivedData.indexOf(':', secondColon + 1);
    int fourthColon = receivedData.indexOf(':', thirdColon + 1);
    
    if (fourthColon > 0) {
      uint32_t fromId = strtoul(receivedData.substring(0, firstColon).c_str(), NULL, 16);
      String payload = receivedData.substring(fourthColon + 1);
      
      // Parse JSON payload
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload);
      
      if (!error) {
        String type = doc["type"];
        
        if (type == "nodeinfo") {
          BalticNode newNode;
          newNode.nodeId = doc["id"];
          newNode.nodeName = doc["name"].as<String>();
          newNode.shortName = doc["short"].as<String>();
          newNode.latitude = doc["lat"];
          newNode.longitude = doc["lon"];
          newNode.lastSeen = millis();
          newNode.rssi = rssi;
          newNode.snr = snr;
          
          addOrUpdateNode(newNode);
          Serial.println("Added node: " + newNode.nodeName);
        }
        else if (type == "telemetry") {
          Serial.println("Received telemetry from: " + String(fromId, HEX));
          // Process environmental telemetry data
        }
      }
    }
  }
  
  // Continue listening
  radio.startReceive();
}

void addOrUpdateNode(BalticNode& node) {
  // Find existing node or add new one
  for (auto& existingNode : nodeDatabase) {
    if (existingNode.nodeId == node.nodeId) {
      existingNode = node;  // Update existing
      return;
    }
  }
  
  // Add new node
  if (nodeDatabase.size() < 20) {  // Limit to 20 nodes
    nodeDatabase.push_back(node);
  }
}

void handleButtonPress() {
  displayMode = (displayMode + 1) % MAX_DISPLAY_MODES;
  Serial.println("Display mode: " + String(displayMode));
  
  // Flash LED
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
  
  updateDisplay();
}

void updateDisplay() {
  display.clearBuffer();
  
  switch (displayMode) {
    case STATUS_SCREEN:
      drawStatusScreen();
      break;
    case NODE_LIST:
      drawNodeList();
      break;
    case ENVIRONMENTAL_DATA:
      drawEnvironmentalData();
      break;
    case LORA_INFO:
      drawLoRaInfo();
      break;
    case BALTIC_MONITORING:
      drawBalticMonitoring();
      break;
  }
  
  display.sendBuffer();
}

void drawStatusScreen() {
  display.setFont(u8g2_font_6x10_tf);
  display.drawStr(0, 10, "Baltic Meshtastic");
  display.drawLine(0, 12, 127, 12);
  
  display.drawStr(0, 25, myNode.shortName.c_str());
  display.drawStr(0, 35, ("Nodes: " + String(nodeDatabase.size())).c_str());
  
  unsigned long uptime = millis() / 1000;
  display.drawStr(0, 45, ("Up: " + String(uptime) + "s").c_str());
  
  display.drawStr(0, 60, "Status (1/5)");
}

void drawNodeList() {
  display.setFont(u8g2_font_6x10_tf);
  display.drawStr(0, 10, "Mesh Network");
  display.drawLine(0, 12, 127, 12);
  
  int y = 25;
  int count = 0;
  for (const auto& node : nodeDatabase) {
    if (count >= 3) break;  // Show max 3 nodes
    
    String nodeStr = node.shortName + " " + String(node.rssi) + "dB";
    display.drawStr(0, y, nodeStr.c_str());
    y += 10;
    count++;
  }
  
  if (nodeDatabase.empty()) {
    display.drawStr(0, 25, "No nodes found");
  }
  
  display.drawStr(0, 60, "Nodes (2/5)");
}

void drawEnvironmentalData() {
  display.setFont(u8g2_font_6x10_tf);
  display.drawStr(0, 10, "Environment");
  display.drawLine(0, 12, 127, 12);
  
  display.drawStr(0, 25, ("H2O: " + String(currentEnvData.waterTemperature, 1) + "C").c_str());
  display.drawStr(0, 35, ("Air: " + String(currentEnvData.airTemperature, 1) + "C").c_str());
  display.drawStr(0, 45, ("Wave: " + String(currentEnvData.waveHeight, 1) + "m").c_str());
  
  display.drawStr(0, 60, "Environment (3/5)");
}

void drawLoRaInfo() {
  display.setFont(u8g2_font_6x10_tf);
  display.drawStr(0, 10, "LoRa Radio");
  display.drawLine(0, 12, 127, 12);
  
  display.drawStr(0, 25, ("Freq: " + String(MESHTASTIC_FREQUENCY, 0) + "MHz").c_str());
  display.drawStr(0, 35, ("SF: " + String(MESHTASTIC_SPREADING_FACTOR)).c_str());
  display.drawStr(0, 45, ("BW: " + String(MESHTASTIC_BANDWIDTH, 0) + "kHz").c_str());
  
  display.drawStr(0, 60, "Radio (4/5)");
}

void drawBalticMonitoring() {
  display.setFont(u8g2_font_6x10_tf);
  display.drawStr(0, 10, "Baltic Monitor");
  display.drawLine(0, 12, 127, 12);
  
  display.drawStr(0, 25, ("Wind: " + String(currentEnvData.windSpeed, 1) + "m/s").c_str());
  display.drawStr(0, 35, ("Quality: " + String(currentEnvData.waterQuality) + "%").c_str());
  display.drawStr(0, 45, ("Press: " + String(currentEnvData.pressure, 0) + "hPa").c_str());
  
  display.drawStr(0, 60, "Baltic (5/5)");
}