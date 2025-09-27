#include "SystemContext.h"
#include "SystemController.h"
#include "mesh/Router.h"
#include "mesh/MeshTypes.h"
#include "meshtastic/mesh.pb.h"
#include "NodeDB.h"
#include "configuration.h"
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

    // Get a pointer to the router instance
    Router* router = SystemController::getInstance().getRouter();
    if (!router) {
        Serial.println("Uplink Task: FATAL - Router not initialized!");
        vTaskDelete(NULL); // End this task
    }

    GpsReading received_gps_reading;

    for (;;) {
        // Block and wait for a message to appear on the gpsQueue.
        if (xQueueReceive(context->gpsQueue, &received_gps_reading, portMAX_DELAY) == pdTRUE) {
            Serial.println("Uplink Task: Received GPS reading.");

            if (received_gps_reading.isValid) {
                Serial.printf("  - Lat: %.6f, Lng: %.6f\n", received_gps_reading.latitude, received_gps_reading.longitude);
                
                // 1. Create a mesh packet using the router's allocator.
                // This ensures the packet has a unique ID and is properly formatted.
                meshtastic_MeshPacket* meshPacket = router->allocForSending();
                if (meshPacket) {
                    // 2. Fill in the packet details.
                    meshPacket->to = NODENUM_BROADCAST; // Send to everyone on the mesh
                    meshPacket->decoded.portnum = meshtastic_PortNum_PRIVATE_APP;
                    meshPacket->decoded.payload.size = sizeof(GpsReading);
                    memcpy(meshPacket->decoded.payload.bytes, &received_gps_reading, sizeof(GpsReading));
                    meshPacket->decoded.want_response = false;

                    // 3. Send the packet.
                    // The router takes ownership and will free the packet after sending.
                    ErrorCode result = router->send(meshPacket);
                    if (result == ERRNO_OK) {
                        Serial.println("  - GPS data sent via Meshtastic.");
                    } else {
                        Serial.printf("  - Failed to send data via Meshtastic. Error: %d\n", result);
                    }
                } else {
                    Serial.println("  - Failed to allocate mesh packet.");
                }

            } else {
                Serial.println("  - Received invalid GPS reading, not sending.");
            }
        }
    }
}
