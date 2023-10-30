#ifndef INFRARED_SENSOR_H
#define INFRARED_SENSOR_H

// Define GPIO pins for infrared sensor
#define LEFT_LINE_SENSOR_PIN 2
#define RIGHT_LINE_SENSOR_PIN 3
#define BARCODE_SENSOR_PIN 4

// Function declarations
void infrared_sensor_init();
bool get_left_line_sensor_value();
bool readIRSensor();
bool recognize_barcode();

#endif