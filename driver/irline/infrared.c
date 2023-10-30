#include <stdio.h>
#include "infrared_sensor.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include <stdbool.h>
#include "hardware/adc.h"
#include "pico/time.h"

volatile bool line_check = false;

void infrared_sensor_init()
{
    gpio_init(LEFT_LINE_SENSOR_PIN);
    gpio_init(RIGHT_LINE_SENSOR_PIN);
    gpio_init(BARCODE_SENSOR_PIN);

    gpio_set_dir(LEFT_LINE_SENSOR_PIN, GPIO_IN);
    gpio_set_dir(RIGHT_LINE_SENSOR_PIN, GPIO_IN);
    gpio_set_dir(BARCODE_SENSOR_PIN, GPIO_IN);

    // adc_init();
    // adc_gpio_init(LEFT_LINE_SENSOR_PIN);
    // adc_gpio_init(RIGHT_LINE_SENSOR_PIN);
}

void line_sensor_handler(uint gpio, uint32_t events)
{
    if (events == GPIO_IRQ_EDGE_RISE)
    {
        printf("Black line detected!\n");
        line_check = true;
    }
}

bool get_line_sensor_value()
{
    bool line_detected = line_check;
    line_check = false; // Reset the flag
    return line_detected;
}

bool recognize_barcode()
{
    uint32_t start_time;
    int digitalValue;
    uint32_t pulse_widths[100]; // Adjust the size based on your needs
    int pulse_count = 0;

    printf("Waiting for the start of the barcode (falling edge)...\n");

    // Wait for the start of the barcode (falling edge)
    while (gpio_get(BARCODE_SENSOR_PIN) == 1)
    {
        // Wait for falling edge
    }

    printf("Falling edge detected. Starting measurement...\n");

    uint32_t timeout_start = time_us_32();

    while (1)
    {
        // Measure the width of the black bar (falling edge to rising edge)
        start_time = time_us_32();
        digitalValue = gpio_get(BARCODE_SENSOR_PIN);

        while (digitalValue == 0)
        {
            // Wait for rising edge
            digitalValue = gpio_get(BARCODE_SENSOR_PIN);

            // Check for timeout
            if (time_us_32() - timeout_start > TIMEOUT_MICROSECONDS)
            {
                printf("Timeout reached while waiting for rising edge\n");
                return false; // Timeout reached, exit function
            }
        }

        pulse_widths[pulse_count++] = (time_us_32() - start_time) / TICKS_PER_MICROSECOND;

        printf("Measured black bar. Width: %u us\n", pulse_widths[pulse_count - 1]);

        // Measure the width of the white space (rising edge to falling edge)
        start_time = time_us_32();
        digitalValue = gpio_get(BARCODE_SENSOR_PIN);

        while (digitalValue == 1)
        {
            // Wait for falling edge
            digitalValue = gpio_get(BARCODE_SENSOR_PIN);

            // Check for timeout
            if (time_us_32() - timeout_start > TIMEOUT_MICROSECONDS)
            {
                printf("Timeout reached while waiting for falling edge\n");
                return false; // Timeout reached, exit function
            }
        }

        pulse_widths[pulse_count++] = (time_us_32() - start_time) / TICKS_PER_MICROSECOND;

        printf("Measured white space. Width: %u us\n", pulse_widths[pulse_count - 1]);

        // Repeat the process until the end of the barcode

        // Process pulse widths, decode barcode, etc.
        for (int i = 0; i < pulse_count; i++)
        {
            printf("Pulse Width %d: %u us\n", i, pulse_widths[i]);
        }
    }

    // Decode the barcode based on measured widths
    // ...

    return true; // Placeholder for success
}

int main()
{
    infrared_sensor_init(); // Initialize infrared sensors

    stdio_init_all();

    // Set up GPIO interrupt on rising edge
    gpio_set_irq_enabled_with_callback(RIGHT_LINE_SENSOR_PIN, GPIO_IRQ_EDGE_RISE, true, &line_sensor_handler);

    while (1)
    {
        bool barcode_check = recognize_barcode();
        // printf("After Barcode Detection\n");
        printf("Barcode Recognition: %s\n", barcode_check ? "true" : "false");
        
        tight_loop_contents();

        sleep_ms(1000); // Wait for 1 second before the next iteration
    }

    return 0;
}
