#include <stdio.h>
#include "infrared_sensor.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include <stdbool.h>
#include "hardware/adc.h"

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

bool get_left_line_sensor_value()
{
    // Assuming the sensor is connected to a digital pin
    int digitalValue = gpio_get(LEFT_LINE_SENSOR_PIN);

    if (digitalValue == 1)
    {
        printf("Black line detected!\n");
        return true;
    }
    else
    {
        printf("No black line detected.\n");
        return false;
    }
}

bool get_right_line_sensor_value()
{
    // Assuming the sensor is connected to a digital pin
    int digitalValue = gpio_get(RIGHT_LINE_SENSOR_PIN);

    if (digitalValue == 1)
    {
        printf("Black line detected!\n");
        return true;
    }
    else
    {
        printf("No black line detected.\n");
        return false;
    }
}

int main()
{
    infrared_sensor_init(); // Initialize infrared sensors

    stdio_init_all();

    while (1)
    {

        bool left_line_value = get_left_line_sensor_value();
        printf("Left Line Sensor: %s\n", left_line_value ? "true" : "false");

        // Print the value of the right line sensor
        bool right_line_value = get_right_line_sensor_value();
        printf("Right Line Sensor: %s\n", right_line_value ? "true" : "false");

        // else
        // {
        //     bool left_line_value = get_left_line_sensor_value();
        //     printf("Left Line Sensor: %s\n", left_line_value);
        //     // Print the value of the right line sensor
        //     bool right_line_value = get_right_line_sensor_value();
        //     printf("Right Line Sensor: %s\n", right_line_value);
        // }

        sleep_ms(1000); // Wait for 1 second before the next iteration
    }

    return 0;
}
