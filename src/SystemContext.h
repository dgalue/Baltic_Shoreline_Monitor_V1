#ifndef SYSTEM_CONTEXT_H
#define SYSTEM_CONTEXT_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

// Forward declarations for complex types
class GpsReading;
class VisionDetection;
class AcousticEvent;

/**
 * @brief Defines the core data structures and synchronization primitives used across the system.
 *
 * This struct acts as a central container for passing shared resources like queues and mutexes
 * to the various FreeRTOS tasks. It helps to decouple the tasks from one another and clarifies
 * the data flow within the application.
 */
struct SystemContext {
    // --- Queues for Inter-Task Communication ---

    /**
     * @brief Queue for passing GPS data from the GPS task to the uplink/logging tasks.
     *
     * - Producer: Task_GPS
     * - Consumers: Task_Uplink, Task_Logger
     */
    QueueHandle_t gpsQueue;

    /**
     * @brief Queue for passing vision detection events from the camera task.
     *
     * - Producer: Task_Vision
     * - Consumers: Task_Uplink, Task_Logger
     */
    QueueHandle_t visionQueue;

    /**
     * @brief Queue for passing acoustic events from the audio processing task.
     *
     * - Producer: Task_Audio
     * - Consumers: Task_Uplink, Task_Logger
     */
    QueueHandle_t audioQueue;

    // --- Mutexes for Shared Resource Protection ---

    /**
     * @brief Mutex to protect the shared I2C bus.
     *
     * The I2C bus is used by both the Grove Vision AI module and the SSD1315 OLED display.
     * Any task accessing the bus must first take this mutex to prevent concurrent access
     * and potential bus corruption.
     */
    SemaphoreHandle_t i2cMutex;

    /**
     * @brief Mutex to protect the SD card.
     *
     * While the architecture promotes a single-writer model for the SD card (Task_Logger),
     * this mutex provides an extra layer of safety if other tasks ever need to perform
     * direct read operations.
     */
    SemaphoreHandle_t sdCardMutex;
};

#endif // SYSTEM_CONTEXT_H
