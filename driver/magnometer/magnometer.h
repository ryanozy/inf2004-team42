
#ifndef MAGNOMETER_H
#define MAGNOMETER_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "math.h"

#define MAGNOMETER_ADDRESS 0x1E // Address of the magnometer
#define MAGNOMETER_CONFIG_REGISTER_A 0x00 // CRA_REG_M
#define MAGNOMETER_CONFIG_REGISTER_B 0x01 // CRB_REG_M
#define MAGNOMETER_MODE_REGISTER 0x02 // MR_REG_M
#define MAGNOMETER_DATA_REGISTER 0x03 // Starting address of data registers OUT_X_H_M

bool init_i2c_bus();
bool init_magnometer();
uint16_t *get_magnometer_data();
float get_heading();

#endif // MAGNOMETER_H

bool init_i2c_bus() {
    // Initialize the I2C bus
    i2c_init(i2c0, 400000);
    // Make the I2C pins available to picotool
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C); // GPIO4
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C); // GPIO5
    // Enable pull up on SDA/SCL lines
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
}

bool init_magnometer() {
    // initialize magnetometer
    uint8_t mag_config[] = {MAGNOMETER_MODE_REGISTER, MAGNOMETER_CONFIG_REGISTER_A}; // 0x02, 0x00
    i2c_write_blocking(i2c0, MAGNOMETER_ADDRESS, mag_config, 2, false); 
}

float get_heading() {
    
    float heading = 0;
    
    // Write to the magnetometer to tell it to read the data
    uint8_t mag_reg[1] = {MAGNOMETER_DATA_REGISTER};
    i2c_write_blocking(i2c0, MAGNOMETER_ADDRESS, mag_reg, 1, true);

    // Read the data from the magnetometer
    uint8_t mag_data[6];
    i2c_read_blocking(i2c0, MAGNOMETER_ADDRESS, mag_data, 6, false);
    
    // Convert the data to 16-bit signed values
    int16_t x = (mag_data[0] << 8) | mag_data[1];
    int16_t z = (mag_data[2] << 8) | mag_data[3];
    int16_t y = (mag_data[4] << 8) | mag_data[5];
    
    // Calculate the heading
    heading = atan2(y, x) * 180 / M_PI;
    
    // Normalize to 0-360
    if (heading < 0) {
        heading += 360;
    }
    
    return heading;
}

uint16_t *get_magnometer_data(){
    uint8_t mag_reg[1] = {MAGNOMETER_DATA_REGISTER};
    i2c_write_blocking(i2c0, MAGNOMETER_ADDRESS, mag_reg, 1, true);

    // Read the data from the magnetometer
    uint8_t mag_data[6];
    i2c_read_blocking(i2c0, MAGNOMETER_ADDRESS, mag_data, 6, false);
    
    // Convert the data to 16-bit signed values
    int16_t x = (mag_data[0] << 8) | mag_data[1];
    int16_t z = (mag_data[2] << 8) | mag_data[3];
    int16_t y = (mag_data[4] << 8) | mag_data[5];

    // put the data into a 16 bit unsigned int
    uint16_t data[3] = {x, y, z};


    return data;
}