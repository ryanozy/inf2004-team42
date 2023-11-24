#ifndef INFRARED_H
#define INFRARED_H

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include <stdbool.h>
#include "pico/time.h"

// Define GPIO pins for infrared sensor


#define TICKS_PER_MICROSECOND 1
#define TIMEOUT_MICROSECONDS 5000000 // 5 seconds timeout

#define BARCODE_A "111010100010111"
#define BARCODE_F "101110111000101"
#define BARCODE_ASTERISK "100010111011101"

// Define variables for infrared sensor
uint32_t start_time_barcode_black = 0;
uint32_t stop_time_barcode_black = 0;
uint32_t start_time_barcode_white = 0;
uint32_t stop_time_barcode_white = 0;
uint32_t barcode_array[9] = {0};
uint32_t barcode_counter = 0;
uint32_t barcode_divider = 0;
bool barcode_started = false;
bool barcode_ended = false;
char barcode_string[27] = {0};

uint8_t left_sensor_pin = 20;
uint8_t right_sensor_pin = 21;
uint8_t barcode_sensor_pin = 22;

// Function prototypes
void init_infrared(int8_t left_line_sensor_pin, int8_t right_line_sensor_pin, int8_t barcode_sensor_pin);
void measure_barcode(char color[]);
void barcode_sensor_handler(uint gpio, uint32_t events);
void decode_barcode();
bool is_line_sensor_triggered(int8_t line_sensor_pin);

#endif // INFRARED_H

/**
 * @brief Function to decode the barcode
 *
 * Convert the barcode array to binary
 * Convert 2 to 0 and 222 to 000
 * No need to convert 1s
 * Check if barcode is F
 * Check if barcode is A
 * Check if barcode is *
 * Clear barcode array
 * Clear barcode string
 */
void decode_barcode()
{

    // get_line_sensor_value();

    if (barcode_counter == 9)
    {
        // Convert barcode array to binary
        // Convert 2 to 0 and 222 to 000
        // No need to convert 1s

        for (int i = 0; i < 9; i++)
        {
            if (barcode_array[i] == 2)
            {
                strcat(barcode_string, "0");
            }
            else if (barcode_array[i] == 222)
            {
                strcat(barcode_string, "000");
            }
            else if (barcode_array[i] == 1)
            {
                strcat(barcode_string, "1");
            }
            else if (barcode_array[i] == 111)
            {
                strcat(barcode_string, "111");
            }
        }

        // Check if barcode is F
        if (strcmp(barcode_string, BARCODE_F) == 0)
        {
            printf("Barcode is F\n");
            printf("Barcode String: %s\n", barcode_string);

            // Clear barcode array
            memset(barcode_array, 0, sizeof(barcode_array));
            barcode_counter = 0;
        }
        else if (strcmp(barcode_string, BARCODE_A) == 0)
        {
            printf("Barcode is A\n");
            printf("Barcode String: %s\n", barcode_string);

            // Clear barcode array
            memset(barcode_array, 0, sizeof(barcode_array));
            barcode_counter = 0;
        }
        else if (strcmp(barcode_string, BARCODE_ASTERISK) == 0)
        {
            printf("Barcode is *\n");
            printf("Barcode String: %s\n", barcode_string);

            // Clear barcode array
            memset(barcode_array, 0, sizeof(barcode_array));
            barcode_counter = 0;
        }

        // Clear barcode string
        memset(barcode_string, 0, sizeof(barcode_string));
    }
}

/**
 * @brief Function to measure the barcode
 *
 * If barcode counter is 9, then the barcode is complete. Shift the array to the left to make space for the new barcode
 * If barcode counter is 0, then the barcode has not started yet. Set the divider to the first time difference
 * If barcode counter is not 0, then the barcode has started. Check if the time difference is greater than or equal to 2 times the divider
 * If it is, then the barcode is thick. Else, the barcode is thin
 */
void measure_barcode(char color[])
{
    if (barcode_counter == 9)
    {
        // Shift the array to the left
        for (int i = 0; i < 8; i++)
        {
            barcode_array[i] = barcode_array[i + 1];
        }
        barcode_counter--;
    }

    if (strcmp(color, "black") == 0)
    {

        uint32_t time_difference = stop_time_barcode_black - start_time_barcode_black;
        // printf("Time Difference: %d\n", time_difference);

        if (barcode_counter == 0)
        {
            // Set the divider to the first time difference
            // Use this divider to determine if the barcode is thick or thin
            barcode_divider = time_difference;
            // printf("Barcode Divider: %d\n", barcode_divider);
        }

        // printf("time_difference / barcode_divider: %d\n", time_difference / barcode_divider);

        if (time_difference / barcode_divider >= 2)
        {

            barcode_array[barcode_counter] = 111;
            printf("Thick Black Barcode\n");
            barcode_counter++;
        }
        else
        {
            barcode_array[barcode_counter] = 1;
            printf("Thin Black Barcode\n");
            barcode_counter++;
        }
    }
    else if (strcmp(color, "white") == 0)
    {
        uint32_t time_difference = stop_time_barcode_white - start_time_barcode_white;
        // printf("Time Difference: %d\n", time_difference);
        // printf("time_difference / barcode_divider: %d\n", time_difference / barcode_divider);

        if (barcode_counter == 0)
        {

            // Barcode has not started yet
        }
        else
        {
            if (time_difference / barcode_divider >= 2)
            {
                barcode_array[barcode_counter] = 222;
                printf("Thick White Barcode\n");
                barcode_counter++;
            }
            else
            {
                barcode_array[barcode_counter] = 2;
                printf("Thin White Barcode\n");
                barcode_counter++;
            }
        }
    }
}

/**
 * @brief Barcode Sensor Interrupt Handler
 *
 * If events is GPIO_IRQ_EDGE_RISE, then the barcode is black --> Measure the previous white barcode
 * If events is GPIO_IRQ_EDGE_FALL, then the barcode is white --> Measure the previous black barcode
 */
void barcode_sensor_handler(uint gpio, uint32_t events)
{
    if (events == GPIO_IRQ_EDGE_RISE && gpio == barcode_sensor_pin)
    {
        start_time_barcode_black = time_us_32();
        stop_time_barcode_white = time_us_32();

        if (barcode_started == false)
        {
            barcode_started = true;
        }

        measure_barcode("white");
    }
    else if (events == GPIO_IRQ_EDGE_FALL && gpio == barcode_sensor_pin)
    {
        start_time_barcode_white = time_us_32();
        stop_time_barcode_black = time_us_32();

        measure_barcode("black");
    }
}

bool on_trigger_handler(uint gpio)
{
    printf("Triggered\n");
    printf("GPIO: %d\n", gpio);
    return true;
}


/**
 * @brief Function to initialize the infrared sensors
 *
 * Initialize the GPIO pins for the infrared sensors
 */
void init_infrared(int8_t left_line_sensor_pin, int8_t right_line_sensor_pin, int8_t barcode_sensor_pin)
{
    // Set the GPIO pins
    left_sensor_pin = left_line_sensor_pin;
    right_sensor_pin = right_line_sensor_pin;
    barcode_sensor_pin = barcode_sensor_pin;

    // Initialize GPIO pins
    gpio_init(left_line_sensor_pin);
    gpio_init(right_line_sensor_pin);
    gpio_init(barcode_sensor_pin);

    // Set GPIO pins to pull down
    gpio_set_dir(left_line_sensor_pin, GPIO_IN);
    gpio_set_dir(right_line_sensor_pin, GPIO_IN);
    gpio_set_dir(barcode_sensor_pin, GPIO_IN);
}