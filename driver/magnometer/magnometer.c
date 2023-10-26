#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

int main()
{
    stdio_init_all();

    // I2C bus initialization
    i2c_init(i2c0, 400000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    sleep_ms(10); // Give some time for the bus to settle

    while (true)
    {

        // LSM303DLHC ACCELEROMETER I2C address is 0x19 (25)
        uint8_t accel_addr = 0x19;

        // Select control register1(0x20)
        // X, Y, and Z-axis enable, power on mode, o/p data rate 10 Hz(0x27)
        uint8_t accel_config[] = {0x20, 0x27};
        i2c_write_blocking(i2c0, accel_addr, accel_config, 2, false);

        // Select control register4(0x23)
        // Full scale +/- 2g, continuous update(0x00)
        uint8_t accel_config4[] = {0x23, 0x00};
        i2c_write_blocking(i2c0, accel_addr, accel_config4, 2, false);
        sleep_ms(1000);

        // Read 6 bytes of data
        uint8_t reg[1] = {0x28};
        i2c_write_blocking(i2c0, accel_addr, reg, 1, true);

        uint8_t data[6];
        i2c_read_blocking(i2c0, accel_addr, data, 6, false);

        // Convert the data
        int16_t xAccl = (int16_t)((data[1] << 8) | data[0]);
        if (xAccl > 32767)
        {
            xAccl -= 65536;
        }

        int16_t yAccl = (int16_t)((data[3] << 8) | data[2]);
        if (yAccl > 32767)
        {
            yAccl -= 65536;
        }

        int16_t zAccl = (int16_t)((data[5] << 8) | data[4]);
        if (zAccl > 32767)
        {
            zAccl -= 65536;
        }

        // Output data to screen
        printf("Acceleration in X-Axis : %d\n", xAccl);
        printf("Acceleration in Y-Axis : %d\n", yAccl);
        printf("Acceleration in Z-Axis : %d\n", zAccl);

        // LSM303DLHC MAGNETOMETER I2C address is 0x1E (30)
        uint8_t mag_addr = 0x1E;

        // Select MR register(0x02)
        // Continuous conversion(0x00)
        uint8_t mag_config[] = {0x02, 0x00};
        i2c_write_blocking(i2c0, mag_addr, mag_config, 2, false);

        // Select CRA register(0x00)
        // Data output rate = 15Hz(0x10)
        uint8_t mag_cra_config[] = {0x00, 0x10};
        i2c_write_blocking(i2c0, mag_addr, mag_cra_config, 2, false);

        // Select CRB register(0x01)
        // Set gain = +/- 1.3g(0x20)
        uint8_t mag_crb_config[] = {0x01, 0x20};
        i2c_write_blocking(i2c0, mag_addr, mag_crb_config, 2, false);
        sleep_ms(1000);

        // Read 6 bytes of data
        uint8_t mag_reg[1] = {0x03};
        i2c_write_blocking(i2c0, mag_addr, mag_reg, 1, true);

        uint8_t mag_data[6];
        i2c_read_blocking(i2c0, mag_addr, mag_data, 6, false);

        // Convert the data
        int16_t xMag = (int16_t)((mag_data[0] << 8) | mag_data[1]);
        if (xMag > 32767)
        {
            xMag -= 65536;
        }

        int16_t yMag = (int16_t)((mag_data[4] << 8) | mag_data[5]);
        if (yMag > 32767)
        {
            yMag -= 65536;
        }

        int16_t zMag = (int16_t)((mag_data[2] << 8) | mag_data[3]);
        if (zMag > 32767)
        {
            zMag -= 65536;
        }

        // Output data to screen
        printf("Magnetic field in X-Axis: %d\n", xMag);
        printf("Magnetic field in Y-Axis: %d\n", yMag);
        printf("Magnetic field in Z-Axis: %d\n", zMag);
    }

    return 0;
}
