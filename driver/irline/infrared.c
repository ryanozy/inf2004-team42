#include <stdio.h>
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include <stdbool.h>
#include "hardware/adc.h"
#include "pico/time.h"
#include <string.h>
#include "infrared.h"

void decode_barcode();

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

        decode_barcode();
        
    }

    return 0;
}
