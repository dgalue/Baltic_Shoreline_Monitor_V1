#include "SystemContext.h"
#include <Arduino.h>

/**
 * @brief FreeRTOS task for handling the Grove Vision AI camera.
 *
 * This task is responsible for:
 * 1. Periodically polling the camera for new detections.
 * 2. Acquiring the I2C mutex before communicating with the camera.
 * 3. Posting any detected events to the visionQueue.
 *
 * @param pvParameters A pointer to the SystemContext struct.
 */
void task_vision_entry(void *pvParameters) {
    SystemContext* context = (SystemContext*)pvParameters;
    Serial.println("Vision Task: Starting...");

    // TODO: Initialize the Vision AI module library here.

    for (;;) {
        // Acquire the I2C mutex before talking to the camera.
        if (xSemaphoreTake(context->i2cMutex, portMAX_DELAY) == pdTRUE) {
            
            // TODO: Implement the vision detection logic.
            // Example:
            // if (vision.hasDetection()) {
            //     VisionDetection detection = vision.getDetection();
            //     xQueueSend(context->visionQueue, &detection, 0);
            // }

            // Release the mutex so other tasks (like the display) can use the I2C bus.
            xSemaphoreGive(context->i2cMutex);
        }

        // The polling interval depends on the application's needs.
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
