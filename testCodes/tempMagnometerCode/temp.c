#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <math.h>

// #define ACC_ADDR 0x19 // Address of the accelerometer
#define MAG_ADDR 0x1E // Address of the magnetometer

#define MAG_MR_REG 0x02        // Mode register for the magnetometer
#define MAG_MR_MODE 0x00       // Continuous conversion mode for the magnetometer
#define MAG_STARTING_REG 0x03  // Starting register for the magnetometer

/*
 * Code to initialize the I2C bus
 */
void init_i2c_bus()
{
    // Initialize the I2C bus
    i2c_init(i2c0, 400000);
    // Make the I2C pins available to picotool
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C); // GPIO4
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C); // GPIO5
    // Enable pull up on SDA/SCL lines
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
}

/*
 * Calculate the heading from the magnetometer data and return it
 */

int main()
{
    // initialize stdio
    stdio_init_all();

    // initialize I2C bus
    init_i2c_bus();

    // initialize magnetometer
    uint8_t mag_config[] = {MAG_MR_REG, MAG_MR_MODE};
    i2c_write_blocking(i2c0, MAG_ADDR, mag_config, 2, false);

    // Test code to get and print the magnetometer and heading data
    while (1)
    {
        // Read 6 bytes of data
        uint8_t mag_reg[1] = {MAG_STARTING_REG};
        i2c_write_blocking(i2c0, MAG_ADDR, mag_reg, 1, true);

        uint8_t mag_data[6];
        i2c_read_blocking(i2c0, MAG_ADDR, mag_data, 6, false);

        // Break down the data
        int16_t xMag = (mag_data[0] << 8) | mag_data[1];
        int16_t zMag = (mag_data[2] << 8) | mag_data[3];
        int16_t yMag = (mag_data[4] << 8) | mag_data[5];


        float heading = atan2(yMag, xMag) * 180.0 / M_PI;

        // normalize to 0-360
        if (heading < 0)
        {
            heading += 360;
        }

        // Output data to screen
        printf("Magnetometer data: %d, %d, %d\n", xMag, yMag, zMag);
        printf("Heading: %f\n", heading);

        sleep_ms(1000);
    }
}