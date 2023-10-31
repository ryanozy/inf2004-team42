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
void get_line_sensor_value();
bool recognize_barcode();
void line_sensor_handler(uint gpio, uint32_t events);

// Define the barcode 39 library array
// Each element is a sequence of max 27 characters
// 0 = Thin White, 000 = Thick White, 1 = Thin Black, 111 = Thick Black

#define BARCODE_A "111010100010111"
#define BARCODE_F "101110111000101"
#define BARCODE_ASTERISK "100010111011101"

#endif