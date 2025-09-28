#ifndef MESHTASTIC_BALTIC_H
#define MESHTASTIC_BALTIC_H

#include <U8g2lib.h>
#include <RadioLib.h>
#include <ArduinoJson.h>
#include <vector>

// Display modes
enum DisplayMode {
    STATUS_SCREEN = 0,
    NODE_LIST = 1,
    ENVIRONMENTAL_DATA = 2,
    LORA_INFO = 3,
    BALTIC_MONITORING = 4,
    MAX_DISPLAY_MODES = 5
};

// Baltic node structure
struct BalticNode {
  uint32_t nodeId;
  String nodeName;
  String shortName;
  float latitude;
  float longitude;
  unsigned long lastSeen;
  int rssi;
  float snr;
};

// Environmental data structure (Baltic Shoreline specific)
struct EnvironmentalData {
  float waterTemperature;
  float airTemperature;
  float humidity;
  float pressure;
  float windSpeed;
  float windDirection;
  float waveHeight;
  int waterQuality;
  unsigned long timestamp;
};

// Meshtastic-compatible packet structure (simplified)
struct MeshPacket {
  uint32_t from;
  uint32_t to;
  uint8_t hopLimit;
  uint8_t hopStart;
  uint32_t id;
  uint8_t payloadType;
  String payload;
  unsigned long rxTime;
  int rssi;
  float snr;
};

// Function declarations
void initializeHardware();
void initializeLoRa();
void readEnvironmentalSensors();
void broadcastNodeInfo();
void broadcastTelemetry();
void sendMeshPacket(MeshPacket& packet);
void receiveMeshPacket();
void addOrUpdateNode(BalticNode& node);
void handleButtonPress();
void updateDisplay();
void drawStatusScreen();
void drawNodeList();
void drawEnvironmentalData();
void drawLoRaInfo();
void drawBalticMonitoring();

#endif // MESHTASTIC_BALTIC_H