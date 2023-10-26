#ifndef ULTRASONIC_SENSOR_H
#define ULTRASONIC_SENSOR_H

#include <stdint.h>

// Function to initialize ultrasonic sensor
void ultrasonic_init();

// Function to measure distance in centimeters
uint32_t ultrasonic_measure_distance_cm();

#endif // ULTRASONIC_SENSOR_H
