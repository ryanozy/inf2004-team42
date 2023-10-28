#ifndef ULTRASONIC_SENSOR_H
#define ULTRASONIC_SENSOR_H

#include <stdint.h>

// Function to initialize ultrasonic sensor
void ultrasonic_init();

// Function to measure distance in centimeters
float calculateDistance(float pulse_duration);

#endif // ULTRASONIC_SENSOR_H
