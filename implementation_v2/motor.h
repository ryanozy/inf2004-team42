#ifndef MOTOR_H
#define MOTOR_H

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include <stdbool.h>
#include "pico/time.h"

#include "magnometer.h"

// Global variables
uint32_t left_encoder_count = 0;
uint32_t right_encoder_count = 0;
uint8_t left_encoder_pin = 2;
uint8_t right_encoder_pin = 3;
uint8_t enable_pin_A = 15;
uint8_t enable_pin_B = 10;
uint8_t input_1 = 14;
uint8_t input_2 = 13;
uint8_t input_3 = 12;
uint8_t input_4 = 11;

// Function prototypes
void move_forward(float speed);
void move_backward(float speed);
void turn_left(float speed, float angle);
void turn_right(float speed, float angle);
void stop_motors();
void init_motors(uint8_t left_motor_pin1, uint8_t left_motor_pin2, uint8_t right_motor_pin1, uint8_t right_motor_pin2, uint8_t left_motor_pwm_pin, uint8_t right_motor_pwm_pin, uint8_t encoder_left_pin, uint8_t encoder_right_pin);
uint16_t *get_encoder_data();

#endif // MOTOR_H

void set_speed(float left_motor_speed, float right_motor_speed)
{
    // Set the PWM channels
    pwm_set_gpio_level(enable_pin_A, left_motor_speed);
    pwm_set_gpio_level(enable_pin_B, right_motor_speed);
}


void move_forward(float speed)
{
    // Set the GPIO pins to high
    gpio_put(input_1, 1);
    gpio_put(input_2, 0);
    gpio_put(input_3, 1);
    gpio_put(input_4, 0);

    set_speed(speed, speed);

    // Set the PWM channels
    pwm_set_enabled(pwm_gpio_to_slice_num(enable_pin_A), true);
    pwm_set_enabled(pwm_gpio_to_slice_num(enable_pin_B), true);
}

void move_backward(float speed)
{
    // Set the GPIO pins to high
    gpio_put(input_1, 0);
    gpio_put(input_2, 1);
    gpio_put(input_3, 0);
    gpio_put(input_4, 1);

    set_speed(speed, speed);

    pwm_set_enabled(pwm_gpio_to_slice_num(enable_pin_A), true);
    pwm_set_enabled(pwm_gpio_to_slice_num(enable_pin_B), true);
}

void turn_left(float speed, float angle)
{
    // Set the GPIO pins to high
    gpio_put(input_1, 0);
    gpio_put(input_2, 1);
    gpio_put(input_3, 1);
    gpio_put(input_4, 0);

    set_speed(speed, speed);

    // Set the PWM channels
    pwm_set_enabled(pwm_gpio_to_slice_num(enable_pin_A), true);
    pwm_set_enabled(pwm_gpio_to_slice_num(enable_pin_B), true);

}

void turn_right(float speed, float angle)
{
    // Set the GPIO pins to high
    gpio_put(input_1, 1);
    gpio_put(input_2, 0);
    gpio_put(input_3, 0);
    gpio_put(input_4, 1);

    set_speed(speed, speed);

    // Set the PWM channels
    pwm_set_enabled(pwm_gpio_to_slice_num(enable_pin_A), true);
    pwm_set_enabled(pwm_gpio_to_slice_num(enable_pin_B), true);

}

void stop_motors()
{
    // Set the GPIO pins to low
    gpio_put(input_1, 0);
    gpio_put(input_2, 0);
    gpio_put(input_3, 0);
    gpio_put(input_4, 0);

    set_speed(0, 0);

    // Set the PWM channels
    pwm_set_chan_level(pwm_gpio_to_slice_num(enable_pin_A), PWM_CHAN_A, 0);
    pwm_set_chan_level(pwm_gpio_to_slice_num(enable_pin_B), PWM_CHAN_A, 0);

    pwm_set_enabled(pwm_gpio_to_slice_num(enable_pin_A), false);
    pwm_set_enabled(pwm_gpio_to_slice_num(enable_pin_B), false);
}

uint16_t *get_encoder_data()
{
    // Initialize the encoder data array
    static uint16_t encoder_data[2] = {0};

    // Get the encoder data
    encoder_data[0] = left_encoder_count;
    encoder_data[1] = right_encoder_count;

    // Reset the encoder count
    left_encoder_count = 0;
    right_encoder_count = 0;

    return encoder_data;
}

void init_motors(uint8_t left_motor_pin1, uint8_t left_motor_pin2, uint8_t right_motor_pin1, uint8_t right_motor_pin2, uint8_t left_motor_pwm_pin, uint8_t right_motor_pwm_pin, uint8_t encoder_left_pin, uint8_t encoder_right_pin)
{
    // change the global variables
    left_encoder_pin = encoder_left_pin;
    right_encoder_pin = encoder_right_pin;
    enable_pin_A = left_motor_pwm_pin;
    enable_pin_B = right_motor_pwm_pin;
    input_1 = left_motor_pin1;
    input_2 = left_motor_pin2;
    input_3 = right_motor_pin1;
    input_4 = right_motor_pin2;

    // Initialize the GPIO pins for the motors
    gpio_init(left_encoder_pin);
    gpio_init(right_encoder_pin);
    gpio_init(enable_pin_A);
    gpio_init(enable_pin_B);
    gpio_init(input_1);
    gpio_init(input_2);
    gpio_init(input_3);
    gpio_init(input_4);

    // Set the GPIO pins to output
    gpio_set_dir(left_encoder_pin, GPIO_IN);
    gpio_set_dir(right_encoder_pin, GPIO_IN);
    gpio_set_dir(input_1, GPIO_OUT);
    gpio_set_dir(input_2, GPIO_OUT);
    gpio_set_dir(input_3, GPIO_OUT);
    gpio_set_dir(input_4, GPIO_OUT);
    
    // Configure PWM 
    gpio_set_function(enable_pin_A, GPIO_FUNC_PWM);
    gpio_set_function(enable_pin_B, GPIO_FUNC_PWM);

    uint slice_A = pwm_gpio_to_slice_num(enable_pin_A);
    uint slice_B = pwm_gpio_to_slice_num(enable_pin_B);

    pwm_set_clkdiv(slice_A, 100.0); // set the clock divider to 100
    pwm_set_clkdiv(slice_B, 100.0); // set the clock divider to 100

    pwm_set_wrap(slice_A, 12500); // set the wrap to 12500
    pwm_set_wrap(slice_B, 12500); // set the wrap to 12500

    pwm_set_chan_level(slice_A, PWM_CHAN_A, 12500 / 2);
    pwm_set_chan_level(slice_B, PWM_CHAN_A, 12500 / 2);

    pwm_set_enabled(slice_A, false);
    pwm_set_enabled(slice_B, false);

}