#include "SystemContext.h"
#include <Arduino.h>

/**
 * @brief FreeRTOS task for logging data to the SD card.
 *
 * This task acts as the sole writer to the SD card to prevent corruption.
 * It is responsible for:
 * 1. Initializing the SD card.
 * 2. Waiting for data/events from other tasks.
 * 3. Writing the data to a log file.
 * 4. Managing log file rotation.
 *
 * @param pvParameters A pointer to the SystemContext struct.
 */
void task_logger_entry(void *pvParameters) {
    SystemContext* context = (SystemContext*)pvParameters;
    Serial.println("Logger Task: Starting...");

    // Acquire the SD card mutex. In a single-writer model, this task
    // could hold the mutex indefinitely after initialization.
    if (xSemaphoreTake(context->sdCardMutex, portMAX_DELAY) == pdTRUE) {
        
        // TODO: Initialize the SD card here.
        // if (!SD.begin(CS_PIN)) {
        //     Serial.println("Logger Task: SD card initialization failed!");
        //     // Handle error...
        // }

        // The mutex is now held by the logger task.
        Serial.println("Logger Task: SD card initialized and mutex acquired.");

    }

    for (;;) {
        // This task would also wait on the various data queues.
        // For simplicity, we'll just show a placeholder.
        
        // TODO:
        // 1. Wait for data on a queue (or queue set).
        // 2. Open the log file.
        // 3. Write the formatted data.
        // 4. Close the file.

        vTaskDelay(pdMS_TO_TICKS(2000)); // Placeholder delay
    }
}
