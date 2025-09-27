#include "SystemContext.h"
#include <Arduino.h>

/**
 * @brief FreeRTOS task for handling data uplink via Meshtastic.
 *
 * This task is responsible for:
 * 1. Waiting for events from the sensor/AI tasks (GPS, vision, audio).
 * 2. Formatting the event data into a compact payload.
 * 3. Sending the payload over the Meshtastic mesh network.
 *
 * @param pvParameters A pointer to the SystemContext struct.
 */
void task_uplink_entry(void *pvParameters) {
    SystemContext* context = (SystemContext*)pvParameters;
    Serial.println("Uplink Task: Starting...");

    // TODO: Initialize the Meshtastic API/interface here.

    for (;;) {
        // This task can be entirely event-driven, blocking on multiple queues.
        // A more advanced implementation would use a QueueSet to wait on
        // gpsQueue, visionQueue, and audioQueue simultaneously.

        // For now, we'll check one queue as an example.
        int received_data; // Placeholder for actual data struct
        if (xQueueReceive(context->visionQueue, &received_data, portMAX_DELAY) == pdTRUE) {
            Serial.println("Uplink Task: Received vision event. Preparing to send.");
            
            // TODO:
            // 1. Format the received_data into a byte buffer.
            // 2. Call the Meshtastic API to send the data.
            //    meshtastic.sendData(buffer, size);
        }
        
        // No delay needed here if xQueueReceive blocks indefinitely (portMAX_DELAY)
    }
}
