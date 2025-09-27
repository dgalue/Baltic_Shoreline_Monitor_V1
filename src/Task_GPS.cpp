#include "SystemContext.h"
#include <Arduino.h>

/**
 * @brief FreeRTOS task for handling the GPS module.
 *
 * This task is responsible for:
 * 1. Reading data from the GPS module over UART.
 * 2. Parsing the NMEA sentences.
 * 3. Posting the extracted location, time, and status to the gpsQueue.
 *
 * @param pvParameters A pointer to the SystemContext struct.
 */
void task_gps_entry(void *pvParameters) {
    SystemContext* context = (SystemContext*)pvParameters;
    Serial.println("GPS Task: Starting...");

    // TODO: Initialize the GPS hardware/library here.

    for (;;) {
        // TODO: Implement the GPS reading and parsing logic.
        // Example:
        // if (gps.available()) {
        //     GpsReading reading = gps.getReading();
        //     xQueueSend(context->gpsQueue, &reading, portMAX_DELAY);
        // }

        // Run this task at a reasonable interval, e.g., once per second.
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
