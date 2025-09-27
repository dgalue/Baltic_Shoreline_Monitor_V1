// Use the simple Meshtastic display demo
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

// Hardware pin definitions
const int DISPLAY_SDA = 5;
const int DISPLAY_SCL = 6;
const int BUTTON_PIN = 21;
const int LED_PIN = 48;

// Display configuration
U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ DISPLAY_SCL, /* data=*/ DISPLAY_SDA);

// Node information
uint32_t myNodeId = 0;
String myNodeName = "XIAO-S3";
unsigned long bootTime = 0;
unsigned long lastDisplayUpdate = 0;
bool buttonPressed = false;
bool ledState = false;
int displayMode = 0;
int buttonPressCount = 0;

// Meshtastic-style display modes
enum DisplayModes {
  STATUS_SCREEN = 0,
  NODE_INFO = 1,
  HARDWARE_INFO = 2,
  LORA_SETTINGS = 3,
  MAX_MODES = 4
};

// Function declarations
void updateDisplay();
void handleButtonPress();
void drawStatusScreen();
void drawNodeInfo(); 
void drawHardwareInfo();
void drawLoRaSettings();

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=== XIAO ESP32-S3 Meshtastic Display Test ===");
  
  bootTime = millis();
  
  // Initialize pins
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Initialize I2C with custom pins
  Wire.begin(DISPLAY_SDA, DISPLAY_SCL);
  
  // Initialize display
  display.begin();
  display.clearBuffer();
  display.setFont(u8g2_font_6x10_tf);
  
  // Generate node ID from MAC address
  myNodeId = ESP.getEfuseMac() & 0xFFFFFF;
  myNodeName = "XIAO-" + String(myNodeId, HEX);
  
  Serial.println("Hardware initialized successfully!");
  Serial.println("Node ID: 0x" + String(myNodeId, HEX));
  Serial.println("Node Name: " + myNodeName);
  Serial.println("Display: SSD1306 128x64 on I2C");
  Serial.println("SDA: GPIO" + String(DISPLAY_SDA) + ", SCL: GPIO" + String(DISPLAY_SCL));
  Serial.println("Button: GPIO" + String(BUTTON_PIN));
  Serial.println("LED: GPIO" + String(LED_PIN));
  
  // Initial display update
  updateDisplay();
}

void loop() {
  static unsigned long lastButtonCheck = 0;
  static bool lastButtonState = HIGH;
  
  // Check button every 50ms
  if (millis() - lastButtonCheck > 50) {
    lastButtonCheck = millis();
    
    bool currentButtonState = digitalRead(BUTTON_PIN);
    if (lastButtonState == HIGH && currentButtonState == LOW) {
      handleButtonPress();
    }
    lastButtonState = currentButtonState;
  }
  
  // Update display every 1000ms
  if (millis() - lastDisplayUpdate > 1000) {
    updateDisplay();
  }
  
  // Blink LED to show activity
  static unsigned long lastBlink = 0;
  if (millis() - lastBlink > 500) {
    lastBlink = millis();
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  }
  
  // Small delay for stability
  delay(10);
}

void handleButtonPress() {
  Serial.println("Button pressed! Display mode: " + String(displayMode) + " -> " + String((displayMode + 1) % MAX_MODES));
  
  displayMode = (displayMode + 1) % MAX_MODES;
  buttonPressCount++;
  buttonPressed = true;
  
  // Immediate display update on button press
  updateDisplay();
  
  // Flash LED to indicate button press
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
}

void updateDisplay() {
  lastDisplayUpdate = millis();
  
  display.clearBuffer();
  
  switch (displayMode) {
    case STATUS_SCREEN:
      drawStatusScreen();
      break;
    case NODE_INFO:
      drawNodeInfo();
      break;
    case HARDWARE_INFO:
      drawHardwareInfo();
      break;
    case LORA_SETTINGS:
      drawLoRaSettings();
      break;
  }
  
  display.sendBuffer();
}

void drawStatusScreen() {
  display.setFont(u8g2_font_6x10_tf);
  
  // Title
  display.drawStr(0, 10, "Meshtastic Display");
  display.drawLine(0, 12, 127, 12);
  
  // Status info
  display.drawStr(0, 25, ("Node: " + myNodeName).c_str());
  
  unsigned long uptime = (millis() - bootTime) / 1000;
  String uptimeStr = "Uptime: " + String(uptime) + "s";
  display.drawStr(0, 35, uptimeStr.c_str());
  
  String buttonStr = "Button: " + String(buttonPressCount);
  display.drawStr(0, 45, buttonStr.c_str());
  
  // Mode indicator
  display.drawStr(0, 60, "Status Screen (1/4)");
}

void drawNodeInfo() {
  display.setFont(u8g2_font_6x10_tf);
  
  // Title
  display.drawStr(0, 10, "Node Information");
  display.drawLine(0, 12, 127, 12);
  
  // Node details
  display.drawStr(0, 25, ("ID: 0x" + String(myNodeId, HEX)).c_str());
  display.drawStr(0, 35, ("Name: " + myNodeName).c_str());
  display.drawStr(0, 45, "Role: Client");
  
  // Mode indicator
  display.drawStr(0, 60, "Node Info (2/4)");
}

void drawHardwareInfo() {
  display.setFont(u8g2_font_6x10_tf);
  
  // Title
  display.drawStr(0, 10, "Hardware Info");
  display.drawLine(0, 12, 127, 12);
  
  // Hardware details
  display.drawStr(0, 25, "ESP32-S3 XIAO");
  display.drawStr(0, 35, "Display: SSD1306");
  display.drawStr(0, 45, "I2C: 5(SDA) 6(SCL)");
  
  // Mode indicator
  display.drawStr(0, 60, "Hardware (3/4)");
}

void drawLoRaSettings() {
  display.setFont(u8g2_font_6x10_tf);
  
  // Title
  display.drawStr(0, 10, "LoRa Settings");
  display.drawLine(0, 12, 127, 12);
  
  // LoRa details (simulated)
  display.drawStr(0, 25, "Freq: 915 MHz");
  display.drawStr(0, 35, "BW: 125 kHz");
  display.drawStr(0, 45, "SF: 10, CR: 4/5");
  
  // Mode indicator
  display.drawStr(0, 60, "LoRa Config (4/4)");
}
