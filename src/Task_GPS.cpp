#include "SystemContext.h"
#include <Arduino.h>
#include <TinyGPS++.h>

// The serial port for the GPS module
#define GPS_SERIAL_PORT Serial1
#define GPS_BAUD 115200 // Or 9600, depending on your module's default

// The pins for the GPS module (update according to your wiring)
#define GPS_RX_PIN 44
#define GPS_TX_PIN 43

TinyGPSPlus gps;

/**
 * @brief FreeRTOS task for handling the GPS module.
 *
 * This task is responsible for:
 * 1. Reading data from the GPS module over UART.
 * 2. Parsing the NMEA sentences using TinyGPS++.
 * 3. Posting the extracted location, time, and status to the gpsQueue.
 *
 * @param pvParameters A pointer to the SystemContext struct.
 */
void task_gps_entry(void *pvParameters) {
    SystemContext* context = (SystemContext*)pvParameters;
    Serial.println("GPS Task: Starting...");

    // Initialize the hardware serial port for the GPS
    GPS_SERIAL_PORT.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    Serial.println("GPS Task: Serial port initialized.");

    GpsReading reading;

    for (;;) {
        // Feed characters from the serial port to the TinyGPS++ object
        while (GPS_SERIAL_PORT.available() > 0) {
            gps.encode(GPS_SERIAL_PORT.read());
        }

        // Check if a new sentence has been fully parsed
        if (gps.location.isUpdated()) {
            reading.isValid = gps.location.isValid();
            if (reading.isValid) {
                reading.latitude = gps.location.lat();
                reading.longitude = gps.location.lng();
                reading.altitude = gps.altitude.meters();
                reading.speed = gps.speed.mps();
                reading.satellites = gps.satellites.value();
                reading.hdop = gps.hdop.value();

                // Send the new reading to the queue
                if (xQueueSend(context->gpsQueue, &reading, pdMS_TO_TICKS(100)) != pdPASS) {
                    Serial.println("GPS Task: Failed to send to gpsQueue.");
                } else {
                    Serial.printf("GPS Task: Lat: %.6f, Lng: %.6f, Sats: %d\n", reading.latitude, reading.longitude, reading.satellites);
                }
            }
        }

        // This task can run frequently to keep the GPS object fed with data.
        // TinyGPS++ handles the sentence parsing timing internally.
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
