#include <stdio.h>
#include <stdlib.h>
#include "magnometer.h" // Include the magnometer header file
#include "pico/stdlib.h"

int main()
{
    stdio_init_all();

    // initialize I2C bus
    init_i2c_bus();

    // initialize magnetometer
    init_magnometer();

    // Test code to get and print the magnetometer and heading data
    while (1)
    {
        float heading = get_heading();

        printf("Heading: %f\n", heading);

        uint16_t* raw_data = get_magnometer_data();

        printf("Magnometer data x: %d, y: %d, z: %d\n", raw_data[0], raw_data[1], raw_data[2]);
        
        // Free the memory allocated by malloc
        free(raw_data);

        sleep_ms(50);

        // Clear the screen
        printf("\033[2J");
    }
}