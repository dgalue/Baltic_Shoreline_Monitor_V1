#include "SystemContext.h"
#include <Arduino.h>

/**
 * @brief FreeRTOS task for handling the hydrophone audio stream.
 *
 * This task is responsible for:
 * 1. Setting up the ADC for continuous sampling.
 * 2. Collecting audio samples into buffers.
 * 3. Running the TinyML model for acoustic event detection.
 * 4. Posting any detected events to the audioQueue.
 *
 * @param pvParameters A pointer to the SystemContext struct.
 */
void task_audio_entry(void *pvParameters) {
    SystemContext* context = (SystemContext*)pvParameters;
    Serial.println("Audio Task: Starting...");

    // TODO: Initialize ADC, DMA, and the TFLM model.

    for (;;) {
        // This task will likely be event-driven, waiting for a DMA interrupt
        // that signifies a full audio buffer is ready for processing.
        
        // 1. Wait for notification from ADC/DMA ISR.
        // 2. Process the audio buffer with the ML model.
        // 3. If the model detects an event:
        //    AcousticEvent event = ...;
        //    xQueueSend(context->audioQueue, &event, 0);

        vTaskDelay(pdMS_TO_TICKS(100)); // Placeholder delay
    }
}
