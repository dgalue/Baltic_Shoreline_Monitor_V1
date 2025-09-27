#ifndef SYSTEM_CONTROLLER_H
#define SYSTEM_CONTROLLER_H

#include "SystemContext.h"

/**
 * @brief Manages the initialization and lifecycle of the entire system.
 *
 * This class follows the Singleton pattern to provide a single point of control for
 * setting up hardware, creating FreeRTOS tasks, and managing shared resources.
 */
class SystemController {
public:
    /**
     * @brief Get the single instance of the SystemController.
     */
    static SystemController& getInstance();

    /**
     * @brief Initialize the system.
     *
     * This method should be called once at startup from the main `setup()` function.
     * It performs the following actions:
     * 1. Initializes serial communication for debugging.
     * 2. Sets up the shared SystemContext, creating queues and mutexes.
     * 3. Initializes hardware peripherals (I2C, SPI, etc.).
     * 4. Creates and launches all the application's FreeRTOS tasks.
     */
    void begin();

private:
    // --- Private Constructor for Singleton Pattern ---
    SystemController() = default;
    SystemController(const SystemController&) = delete;
    SystemController& operator=(const SystemController&) = delete;

    // --- Initialization Methods ---
    void initContext();
    void initHardware();
    void createTasks();

    SystemContext context;
};

#endif // SYSTEM_CONTROLLER_H
