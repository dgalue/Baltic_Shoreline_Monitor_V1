#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <stdint.h>

/**
 * @brief GPS data structure for position and navigation info
 */
struct GPSData {
    bool isValid = false;
    double latitude = 0.0;
    double longitude = 0.0;
    double altitude = 0.0;
    double speed = 0.0;
    double course = 0.0;
    uint32_t satellites = 0;
    double hdop = 0.0;
    uint32_t timestamp = 0;
};

/**
 * @brief Audio data structure for hydrophone readings
 */
struct AudioData {
    bool isValid = false;
    double frequency = 0.0;
    double amplitude = 0.0;
    double duration = 0.0;
    uint32_t timestamp = 0;
};

/**
 * @brief Vision data structure for Grove Vision AI V2 analysis
 */
struct VisionData {
    bool isValid = false;
    uint32_t objectCount = 0;
    double averageSize = 0.0;
    double confidence = 0.0;
    uint32_t timestamp = 0;
};

#endif // DATA_TYPES_H
