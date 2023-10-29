#include "magnometer.h" // Include the magnometer header file

#include <stdio.h> // Include standard input-output library


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

        int16_t raw_data[3] = get_magnometer_data();

        print("Magnometer data x: %d, y: %d, z: %d\n", raw_data[0], raw_data[1], raw_data[2]);

    }
}