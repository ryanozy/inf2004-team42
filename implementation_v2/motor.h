#ifndef MOTOR_H
#define MOTOR_H

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include <stdbool.h>
#include "pico/time.h"
#include <math.h>

#include "magnometer.h"

// Global variables
uint8_t left_encoder_pin = 2;
uint8_t right_encoder_pin = 3;
uint8_t enable_pin_A = 15;
uint8_t enable_pin_B = 10;
uint8_t input_1 = 14;
uint8_t input_2 = 13;
uint8_t input_3 = 12;
uint8_t input_4 = 11;

// Function prototypes
void move_forward(float left_motor_speed, float right_motor_speed);
void move_backward(float left_motor_speed, float right_motor_speed);
void turn_left(float left_motor_speed, float right_motor_speed, float angle);
void turn_right(float left_motor_speed, float right_motor_speed, float angle);
void stop_motors();
void init_motors(uint8_t left_motor_pin1, uint8_t left_motor_pin2, uint8_t right_motor_pin1, uint8_t right_motor_pin2, uint8_t left_motor_pwm_pin, uint8_t right_motor_pwm_pin, uint8_t encoder_left_pin, uint8_t encoder_right_pin);

float start_heading = 0.0;
float current_heading = 0.0;
float target_heading = 0.0;
float set_heading = 0.0;
volatile int left_encoder_count = 0;
volatile int right_encoder_count = 0;
double left_encoder_speed = 0.0;
double right_encoder_speed = 0.0;

volatile char movement_direction = 'x'; // w = forward, s = backward, a = left, d = right, x = stop

float kp = 0.1;  // Proportional gain
float ki = 0.01; // Integral gain
float kd = 0.01; // Derivative gain
float I = 0.0;   // Integral term
float P = 0.0;   // Proportional term
float D = 0.0;   // Derivative term

#define SPEED_RIGHT 5625
#define SPEED_LEFT 5625
#endif // MOTOR_H

void set_speed(float left_motor_speed, float right_motor_speed)
{
    // Set the PWM channels
    pwm_set_gpio_level(enable_pin_A, left_motor_speed);
    pwm_set_gpio_level(enable_pin_B, right_motor_speed);
}

float calculate_new_heading(float current_heading, float angle, bool is_left_turn)
{
    if (!is_left_turn)
    {
        float new_heading = current_heading + angle;
        if (new_heading > 360)
        {
            new_heading = new_heading - 360;
        }
        return new_heading;
    }
    else
    {
        float new_heading = current_heading - angle;
        if (new_heading < 0)
        {
            new_heading = 360 + new_heading;
        }
        return new_heading;
    }
}

bool pid_control()
{
    if (movement_direction == 'w')
    {
        // Get current heading
        current_heading = get_heading();

        // Calculate the error
        float error = target_heading - current_heading;
        float left_wheel_speed = left_encoder_speed;
        float right_wheel_speed = right_encoder_speed;
        float wheel_speed_error = left_wheel_speed - right_wheel_speed;

        // Calculate the PID
        P = kp * error;
        I = I + ki * error;
        D = kd * wheel_speed_error;

        float PID = P + I + D;

        printf("PID: %f\n", PID);
        printf("Left wheel speed: %f\n", left_wheel_speed);
        printf("Right wheel speed: %f\n", right_wheel_speed);
        printf("heading: %f\n", current_heading);

        // Calculate the new motor speeds
        if (PID > 1)
        {
            // Turn left
            float left_motor_speed = SPEED_LEFT + PID;
            float right_motor_speed = SPEED_RIGHT;

            set_speed(left_motor_speed, right_motor_speed);
        }
        else if (PID < -1)
        {
            // change pid to positive
            PID = fabs(PID);
            float left_motor_speed = SPEED_LEFT;
            float right_motor_speed = SPEED_RIGHT + PID;

            set_speed(left_motor_speed, right_motor_speed);
        }
        else
        {
            // Maintain the same heading
            set_speed(SPEED_LEFT, SPEED_RIGHT);
        }

        return true;
    }
    else if (movement_direction == 's')
    {
        // Get current heading
        current_heading = get_heading();

        // Calculate the error
        float error = target_heading - current_heading;
        float left_wheel_speed = left_encoder_speed;
        float right_wheel_speed = right_encoder_speed;
        float wheel_speed_error = left_wheel_speed - right_wheel_speed;

        // Calculate the PID
        P = kp * error;
        I = I + ki * error;
        D = kd * wheel_speed_error;

        float PID = P + I + D;

        printf("PID: %f\n", PID);
        printf("Left wheel speed: %f\n", left_wheel_speed);
        printf("Right wheel speed: %f\n", right_wheel_speed);
        printf("heading: %f\n", current_heading);

        // Calculate the new motor speeds
        if (PID < 0)
        {
            // Turn left
            float left_motor_speed = SPEED_LEFT + PID;
            float right_motor_speed = SPEED_RIGHT;

            set_speed(left_motor_speed, right_motor_speed);
        }
        else if (PID > 0)
        {
            // change pid to positive
            PID = fabs(PID);
            float left_motor_speed = SPEED_LEFT;
            float right_motor_speed = SPEED_RIGHT + PID;

            set_speed(left_motor_speed, right_motor_speed);
        }
        else
        {
            // Maintain the same heading
            set_speed(SPEED_LEFT, SPEED_RIGHT);
        }

        return true;
    }
    else if (movement_direction == 'a')
    {
        // Get current heading
        current_heading = get_heading();

        // Calculate the error
        // float error = target_heading - current_heading;
        float error = 0;
        float left_wheel_speed = left_encoder_speed;
        float right_wheel_speed = right_encoder_speed;
        float wheel_speed_error = left_wheel_speed - right_wheel_speed;

        // Calculate the PID
        P = kp * error;
        I = I + ki * error;
        D = kd * wheel_speed_error;

        float PID = P + I + D;

        printf("PID: %f\n", PID);
        printf("Left wheel speed: %f\n", left_wheel_speed);
        printf("Right wheel speed: %f\n", right_wheel_speed);
        printf("heading: %f\n", current_heading);
        printf("target heading: %f\n", target_heading);

        // Calculate the new motor speeds
        if (PID > 0)
        {
            // Turn left
            float left_motor_speed = SPEED_LEFT + PID;
            float right_motor_speed = SPEED_RIGHT;

            set_speed(left_motor_speed, right_motor_speed);
        }
        else if (PID < 0)
        {
            // change pid to positive
            PID = fabs(PID);
            float left_motor_speed = SPEED_LEFT;
            float right_motor_speed = SPEED_RIGHT + PID;

            set_speed(left_motor_speed, right_motor_speed);
        }
        else
        {
            // Maintain the same heading
            set_speed(SPEED_LEFT, SPEED_RIGHT);
        }

        float heading_difference = fabs(current_heading - target_heading);
        if (heading_difference < 20.0)
        {
            stop_motors();
        }

        return true;
    }
    else if (movement_direction == 'd')
    {
        // Get current heading
        current_heading = get_heading();

        // Calculate the error
        float error = 0;
        float left_wheel_speed = left_encoder_speed;
        float right_wheel_speed = right_encoder_speed;
        float wheel_speed_error = left_wheel_speed - right_wheel_speed;

        // Calculate the PID
        P = kp * error;
        I = I + ki * error;
        D = kd * wheel_speed_error;

        float PID = P + I + D;

        printf("PID: %f\n", PID);
        printf("Left wheel speed: %f\n", left_wheel_speed);
        printf("Right wheel speed: %f\n", right_wheel_speed);
        printf("heading: %f\n", current_heading);
        printf("target heading: %f\n", target_heading);

        // Calculate the new motor speeds
        if (PID < 0)
        {
            // Turn left
            float left_motor_speed = SPEED_LEFT + PID;
            float right_motor_speed = SPEED_RIGHT;

            set_speed(left_motor_speed, right_motor_speed);
        }
        else if (PID > 0)
        {
            // change pid to positive
            PID = fabs(PID);
            float left_motor_speed = SPEED_LEFT;
            float right_motor_speed = SPEED_RIGHT + PID;

            set_speed(left_motor_speed, right_motor_speed);
        }
        else
        {
            // Maintain the same heading
            set_speed(SPEED_LEFT, SPEED_RIGHT);
        }

        float heading_difference = fabs(current_heading - target_heading);
        if (heading_difference < 20.0)
        {
            stop_motors();
        }

        return true;
    }
    return true;
}

void reset_values()
{
    // Reset the encoder counts
    left_encoder_count = 0;
    right_encoder_count = 0;

    // Reset the encoder speeds
    left_encoder_speed = 0.0;
    right_encoder_speed = 0.0;

    // Reset the PID variables
    I = 0.0;
    P = 0.0;
    D = 0.0;

    // Reset the heading variables
    start_heading = 0.0;
    current_heading = 0.0;
    target_heading = 0.0;
}

void move_forward(float left_motor_speed, float right_motor_speed)
{
    // Reset Values
    reset_values();

    // Set the GPIO pins to high
    gpio_put(input_1, 1);
    gpio_put(input_2, 0);
    gpio_put(input_3, 1);
    gpio_put(input_4, 0);

    set_speed(left_motor_speed, right_motor_speed);

    // Set the PWM channels
    pwm_set_enabled(pwm_gpio_to_slice_num(enable_pin_A), true);
    pwm_set_enabled(pwm_gpio_to_slice_num(enable_pin_B), true);

    // Get the current heading
    start_heading = get_heading();
    target_heading = calculate_new_heading(start_heading, 0, false); // Maintain the same heading
    movement_direction = 'w';
}

void move_backward(float left_motor_speed, float right_motor_speed)
{
    // Reset Values
    reset_values();

    // Set the GPIO pins to high
    gpio_put(input_1, 0);
    gpio_put(input_2, 1);
    gpio_put(input_3, 0);
    gpio_put(input_4, 1);

    set_speed(left_motor_speed, right_motor_speed);

    pwm_set_enabled(pwm_gpio_to_slice_num(enable_pin_A), true);
    pwm_set_enabled(pwm_gpio_to_slice_num(enable_pin_B), true);

    // Get the current heading
    start_heading = get_heading();
    target_heading = calculate_new_heading(start_heading, 0, false); // Maintain the same heading
    movement_direction = 's';
}

void turn_left(float left_motor_speed, float right_motor_speed, float angle)
{
    // Reset Values
    reset_values();

    // Set the GPIO pins to high
    gpio_put(input_1, 0);
    gpio_put(input_2, 1);
    gpio_put(input_3, 1);
    gpio_put(input_4, 0);

    set_speed(left_motor_speed, right_motor_speed);

    // Set the PWM channels
    pwm_set_enabled(pwm_gpio_to_slice_num(enable_pin_A), true);
    pwm_set_enabled(pwm_gpio_to_slice_num(enable_pin_B), true);

    // Get the current heading
    start_heading = get_heading();
    target_heading = calculate_new_heading(start_heading, angle, true);
    set_heading = angle;
    movement_direction = 'a';
}

void turn_right(float left_motor_speed, float right_motor_speed, float angle)
{
    // Reset Values
    reset_values();

    // Set the GPIO pins to high
    gpio_put(input_1, 1);
    gpio_put(input_2, 0);
    gpio_put(input_3, 0);
    gpio_put(input_4, 1);

    set_speed(left_motor_speed, right_motor_speed);

    // Set the PWM channels
    pwm_set_enabled(pwm_gpio_to_slice_num(enable_pin_A), true);
    pwm_set_enabled(pwm_gpio_to_slice_num(enable_pin_B), true);

    // Get the current heading
    start_heading = get_heading();
    target_heading = calculate_new_heading(start_heading, angle, false);
    set_heading = angle;
    movement_direction = 'd';
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

    reset_values();

    // Reset the movement direction
    movement_direction = 'x';
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

    // initialize magnetometer
    init_magnometer();
}