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

    GpsReading received_gps_reading;

    for (;;) {
        // This task can be entirely event-driven. We will block and wait for
        // a message to appear on the gpsQueue.
        // The last parameter, portMAX_DELAY, means the task will sleep indefinitely
        // until a message is available, which is very efficient.
        if (xQueueReceive(context->gpsQueue, &received_gps_reading, portMAX_DELAY) == pdTRUE) {
            Serial.println("Uplink Task: Received GPS reading.");

            if (received_gps_reading.isValid) {
                Serial.printf("  - Lat: %.6f, Lng: %.6f\n", received_gps_reading.latitude, received_gps_reading.longitude);
                
                // TODO:
                // 1. Format the received_gps_reading into a byte buffer suitable for LoRa.
                //    For example, using Protocol Buffers (protobuf) or a custom struct.
                //
                //    Example payload:
                //    byte payload[16];
                //    memcpy(payload, &received_gps_reading.latitude, sizeof(double));
                //    memcpy(payload + sizeof(double), &received_gps_reading.longitude, sizeof(double));

                // 2. Call the Meshtastic API to send the data.
                //    meshtastic.sendData(payload, sizeof(payload));
                
                Serial.println("  - (Simulating sending data via Meshtastic)");
            } else {
                Serial.println("  - Received invalid GPS reading, not sending.");
            }
        }
        
        // No delay is needed here because xQueueReceive with portMAX_DELAY handles all the waiting.
    }
}
