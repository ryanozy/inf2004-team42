#ifndef INFRARED_SENSOR_H
#define INFRARED_SENSOR_H

#include "pico/stdlib.h"

// Define GPIO pins for infrared sensor
#define LEFT_LINE_SENSOR_PIN 2
#define RIGHT_LINE_SENSOR_PIN 3
#define BARCODE_SENSOR_PIN 4

#define TICKS_PER_MICROSECOND 1
#define TIMEOUT_MICROSECONDS 5000000  // 5 seconds timeout

// Function declarations
void infrared_sensor_init();
bool get_line_sensor_value();
bool recognize_barcode();
void line_sensor_handler(uint gpio, uint32_t events);

#endif