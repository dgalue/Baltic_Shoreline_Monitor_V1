#include <Arduino.h>
#include "SystemController.h"

/**
 * @brief Main setup function, called once at power-on or reset.
 */
void setup() {
    // Defer all initialization to the SystemController.
    // This keeps the main entry point clean and organized.
    SystemController::getInstance().begin();
}

/**
 * @brief Main loop function.
 *
 * In a FreeRTOS application, the main loop is typically empty or used for
 * very low-priority tasks. All the main work is done in the created tasks.
 * We can use it to feed the watchdog timer or for simple, non-critical status updates.
 */
void loop() {
    // The FreeRTOS scheduler is running the tasks.
    // This loop can be left empty.
    vTaskDelay(pdMS_TO_TICKS(1000)); // Yield to other tasks.
}
