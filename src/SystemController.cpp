#include "SystemController.h"
#include <Arduino.h>

// --- Task Function Prototypes ---
// These are the entry points for our FreeRTOS tasks.
void task_gps_entry(void *pvParameters);
void task_vision_entry(void *pvParameters);
void task_audio_entry(void *pvParameters);
void task_uplink_entry(void *pvParameters);
void task_logger_entry(void *pvParameters);

// --- Singleton Instance ---
SystemController& SystemController::getInstance() {
    static SystemController instance;
    return instance;
}

void SystemController::begin() {
    // Start serial for debugging
    Serial.begin(115200);
    while (!Serial) {
        delay(10); // Wait for serial to connect
    }
    Serial.println("--- Baltic Shoreline Monitor ---");
    Serial.println("SystemController: Initializing...");

    initContext();
    initHardware();
    createTasks();

    Serial.println("SystemController: Initialization complete. Tasks running.");
}

void SystemController::initContext() {
    Serial.println("SystemController: Creating queues and mutexes...");

    // Create queues for inter-task communication
    // Arguments: Queue length, size of each item
    context.gpsQueue = xQueueCreate(10, sizeof(int)); // Placeholder size
    context.visionQueue = xQueueCreate(5, sizeof(int)); // Placeholder size
    context.audioQueue = xQueueCreate(20, sizeof(int)); // Placeholder size

    // Create mutexes for shared resources
    context.i2cMutex = xSemaphoreCreateMutex();
    context.sdCardMutex = xSemaphoreCreateMutex();

    if (!context.gpsQueue || !context.visionQueue || !context.audioQueue || !context.i2cMutex || !context.sdCardMutex) {
        Serial.println("FATAL: Failed to create one or more system context objects!");
        // In a real scenario, you might want to halt or enter a safe mode here.
        while(1);
    }
}

void SystemController::initHardware() {
    Serial.println("SystemController: Initializing hardware peripherals...");
    // TODO: Initialize I2C, SPI, SD card, etc.
    // Wire.begin(SDA_PIN, SCL_PIN);
    // SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN);
}

void SystemController::createTasks() {
    Serial.println("SystemController: Creating FreeRTOS tasks...");

    // Create and launch each task, passing the system context to them.
    // Arguments: Task function, name, stack size, parameters, priority, task handle, core affinity

    xTaskCreatePinnedToCore(
        task_gps_entry,
        "GPS_Task",
        4096,
        &context,
        3,
        NULL,
        1
    );

    xTaskCreatePinnedToCore(
        task_vision_entry,
        "Vision_Task",
        4096,
        &context,
        4,
        NULL,
        1
    );
    
    xTaskCreatePinnedToCore(
        task_audio_entry,
        "Audio_Task",
        4096,
        &context,
        4,
        NULL,
        1
    );

    xTaskCreatePinnedToCore(
        task_uplink_entry,
        "Uplink_Task",
        4096,
        &context,
        3,
        NULL,
        1
    );

    xTaskCreatePinnedToCore(
        task_logger_entry,
        "Logger_Task",
        4096,
        &context,
        2,
        NULL,
        1
    );
}
