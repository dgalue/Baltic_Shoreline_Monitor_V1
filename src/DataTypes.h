#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <stdint.h>

/**
 * @brief Represents a single GPS reading.
 */
struct GpsReading {
    bool isValid;
    double latitude;
    double longitude;
    double altitude;
    double speed;
    uint32_t satellites;
    uint32_t hdop; // Horizontal Dilution of Precision
};

/**
 * @brief Represents a vision detection event.
 * (Placeholder for future implementation)
 */
struct VisionDetection {
    int objectId;
    float confidence;
};

/**
 * @brief Represents an acoustic detection event.
 * (Placeholder for future implementation)
 */
struct AcousticEvent {
    int eventId;
    float confidence;
};

#endif // DATA_TYPES_H
