#include <stdio.h>
#include "infrared_sensor.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include <stdbool.h>
#include "hardware/adc.h"
#include "pico/time.h"
#include <string.h>

#define SPEED_CMS 30

volatile bool line_check_left = false;
volatile bool line_check_right = false;
volatile uint32_t start_time_barcode_black = 0;
volatile uint32_t stop_time_barcode_black = 0;
volatile uint32_t start_time_barcode_white = 0;
volatile uint32_t stop_time_barcode_white = 0;
volatile uint32_t barcode_array[9] = {0};
volatile uint32_t barcode_counter = 0;
volatile uint32_t barcode_divider = 0;
volatile bool barcode_started = false;
volatile bool barcode_ended = false;
char barcode_string[27] = {0};

void infrared_sensor_init()
{
    gpio_init(LEFT_LINE_SENSOR_PIN);
    gpio_init(RIGHT_LINE_SENSOR_PIN);
    gpio_init(BARCODE_SENSOR_PIN);

    gpio_set_dir(LEFT_LINE_SENSOR_PIN, GPIO_IN);
    gpio_set_dir(RIGHT_LINE_SENSOR_PIN, GPIO_IN);
    gpio_set_dir(BARCODE_SENSOR_PIN, GPIO_IN);
}

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
        printf("Time Difference: %d\n", time_difference);

        if (barcode_counter == 0)
        {
            // Set the divider to the first time difference
            // Use this divider to determine if the barcode is thick or thin
            barcode_divider = time_difference;
            printf("Barcode Divider: %d\n", barcode_divider);
        }

        printf("time_difference / barcode_divider: %d\n", time_difference / barcode_divider);

        if (time_difference / barcode_divider >= 2)
        {

            barcode_array[barcode_counter] = 111;
            barcode_counter++;
        }
        else
        {
            barcode_array[barcode_counter] = 1;
            barcode_counter++;
        }
    }
    else if (strcmp(color, "white") == 0)
    {
        uint32_t time_difference = stop_time_barcode_white - start_time_barcode_white;
        printf("Time Difference: %d\n", time_difference);
        printf("time_difference / barcode_divider: %d\n", time_difference / barcode_divider);

        if (barcode_counter == 0)
        {

            // Barcode has not started yet
        }
        else
        {
            if (time_difference / barcode_divider >= 2)
            {
                barcode_array[barcode_counter] = 222;
                barcode_counter++;
            }
            else
            {
                barcode_array[barcode_counter] = 2;
                barcode_counter++;
            }
        }
    }
}

void barcode_sensor_handler(uint gpio, uint32_t events)
{
    if (events == GPIO_IRQ_EDGE_RISE && gpio == BARCODE_SENSOR_PIN)
    {
        start_time_barcode_black = time_us_32();
        stop_time_barcode_white = time_us_32();

        if (barcode_started == false)
        {
            barcode_started = true;
        }

        measure_barcode("white");
    }
    else if (events == GPIO_IRQ_EDGE_FALL && gpio == BARCODE_SENSOR_PIN)
    {
        start_time_barcode_white = time_us_32();
        stop_time_barcode_black = time_us_32();

        measure_barcode("black");
    }
}

void line_sensor_handler(uint gpio, uint32_t events)
{
    if (events == GPIO_IRQ_EDGE_RISE && gpio == RIGHT_LINE_SENSOR_PIN)
    {
        line_check_right = true;
    }
    else if (events == GPIO_IRQ_EDGE_RISE && gpio == LEFT_LINE_SENSOR_PIN)
    {
        line_check_left = true;
    }
    else if (events == GPIO_IRQ_EDGE_FALL && gpio == RIGHT_LINE_SENSOR_PIN)
    {
        line_check_right = false;
    }
    else if (events == GPIO_IRQ_EDGE_FALL && gpio == LEFT_LINE_SENSOR_PIN)
    {
        line_check_left = false;
    }
}

void get_line_sensor_value()
{
    printf("Left Line Sensor: %s\n", line_check_left ? "true" : "false");
    printf("Right Line Sensor: %s\n", line_check_right ? "true" : "false");
}

int main()
{
    infrared_sensor_init(); // Initialize infrared sensors

    stdio_init_all();

    // Set up GPIO interrupt on rising edge
    gpio_set_irq_enabled_with_callback(RIGHT_LINE_SENSOR_PIN, GPIO_IRQ_EDGE_RISE, true, &line_sensor_handler);
    gpio_set_irq_enabled_with_callback(LEFT_LINE_SENSOR_PIN, GPIO_IRQ_EDGE_RISE, true, &line_sensor_handler);

    // Set up GPIO interrupt on falling edge
    gpio_set_irq_enabled_with_callback(RIGHT_LINE_SENSOR_PIN, GPIO_IRQ_EDGE_FALL, true, &line_sensor_handler);
    gpio_set_irq_enabled_with_callback(LEFT_LINE_SENSOR_PIN, GPIO_IRQ_EDGE_FALL, true, &line_sensor_handler);

    // Setup GPIO interrupt for barcode sensor
    gpio_set_irq_enabled_with_callback(BARCODE_SENSOR_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &barcode_sensor_handler);

    while (1)
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
            } else if (strcmp(barcode_string, BARCODE_A) == 0) {
                printf("Barcode is A\n");
                printf("Barcode String: %s\n", barcode_string);
            } else if (strcmp(barcode_string, BARCODE_ASTERISK) == 0) {
                printf("Barcode is *\n");
                printf("Barcode String: %s\n", barcode_string);
            } 

            // Clear barcode string
            memset(barcode_string, 0, sizeof(barcode_string));
        }

        // Clear the terminal
        // printf("\033[2J");
    }

    return 0;
}
