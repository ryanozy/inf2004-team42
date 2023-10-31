/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <hardware/pwm.h>
#include "motor.h"

#define MOTOR_LEFT_PIN1 14
#define MOTOR_LEFT_PIN2 13
#define MOTOR_RIGHT_PIN1 12
#define MOTOR_RIGHT_PIN2 11
#define MOTOR_LEFT_PWM_PIN 15
#define MOTOR_RIGHT_PWM_PIN 10


/**
 * @brief Main function to control motors with user input.
 *
 * This function initializes the motors and PWM for motor speed control. It then enters a loop
 * where the user can control the motors using the WASD keys and adjust the speed using the numbers 1-9.
 */

int main()
{
    // Initialize GPIO pins for motor control
    start_motor(MOTOR_LEFT_PIN1, MOTOR_LEFT_PIN2, MOTOR_RIGHT_PIN1, MOTOR_RIGHT_PIN2);

    // Initialize PWM for motor speed control
    start_motor_pwm(MOTOR_LEFT_PWM_PIN, MOTOR_RIGHT_PWM_PIN);

    // Initialize USB for standard I/O
    stdio_usb_init();

    while (true) {
        char input;
        printf("Control the motors with WASD\nAdjust speed with 1-9\n");
        scanf("%c", &input);

        switch (input) {
            case 'w':
                move_forward(MOTOR_LEFT_PIN1, MOTOR_LEFT_PIN2, MOTOR_RIGHT_PIN1, MOTOR_RIGHT_PIN2);
                break;

            case 'a':
                turn_left(MOTOR_LEFT_PIN1, MOTOR_LEFT_PIN2, MOTOR_RIGHT_PIN1, MOTOR_RIGHT_PIN2);
                break;

            case 's':
                move_backward(MOTOR_LEFT_PIN1, MOTOR_LEFT_PIN2, MOTOR_RIGHT_PIN1, MOTOR_RIGHT_PIN2);
                break;

            case 'd':
                turn_right(MOTOR_LEFT_PIN1, MOTOR_LEFT_PIN2, MOTOR_RIGHT_PIN1, MOTOR_RIGHT_PIN2);
                break;

            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                set_speed((input - '0') * 10, MOTOR_LEFT_PWM_PIN, MOTOR_RIGHT_PWM_PIN);
                break;

            default:
                printf("Invalid input!\n");
                break;
        }
    }

    return 0;
}