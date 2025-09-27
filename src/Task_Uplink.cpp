#include "SystemContext.h"
#include "SystemController.h"
#include "mesh/Router.h"
#include "mesh/MeshTypes.h"
#include "meshtastic/mesh.pb.h"
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
                
                // 1. Format the data into a payload.
                // We will send the data as a PRIVATE_APP packet. This is the standard
                // way to send custom application data over the mesh.
                auto* appPacket = new AppPacket();
                appPacket->set_portnum(PortNum_PRIVATE_APP);
                appPacket->set_payload((uint8_t*)&received_gps_reading, sizeof(GpsReading));
                appPacket->set_destination(BROADCAST_ADDR); // Send to everyone on the mesh
                appPacket->set_want_ack(false);

                // 2. Send the packet.
                // The router will take ownership of the packet and delete it after sending.
                if (router->send(appPacket)) {
                    Serial.println("  - GPS data sent via Meshtastic.");
                } else {
                    Serial.println("  - Failed to send data via Meshtastic.");
                    delete appPacket; // We must delete the packet if the router rejected it.
                }

            } else {
                Serial.println("  - Received invalid GPS reading, not sending.");
            }
        }
    }
}
