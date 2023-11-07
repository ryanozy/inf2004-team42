#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include <stdbool.h>
#include "pico/time.h"
#include <string.h>
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include "infrared.h"
#include "motor.h"
#include "ultrasonic_sensor.h"
#include "magnometer.h"
#include "wifi.h"

#define LEFT_MOTOR_PIN1 14
#define LEFT_MOTOR_PIN2 13
#define RIGHT_MOTOR_PIN1 12
#define RIGHT_MOTOR_PIN2 11
#define LEFT_MOTOR_PWM_PIN 15
#define RIGHT_MOTOR_PWM_PIN 10
#define ENCODEROUT_PIN 2
#define ENCODEROUT_PIN2 3

const static char *MOVE_FORWARD = "w";
const static char *MOVE_BACKWARD = "s";
const static char *TURN_LEFT = "a";
const static char *TURN_RIGHT = "d";

void motor_control(char recv_buffer[1])
{

    if (recv_buffer[0] == MOVE_FORWARD[0])
    {
        printf("Moving forward\n");
        move_forward(LEFT_MOTOR_PIN1, LEFT_MOTOR_PIN2, RIGHT_MOTOR_PIN1, RIGHT_MOTOR_PIN2);
    }
    else if (recv_buffer[0] == MOVE_BACKWARD[0])
    {
        printf("Moving backward\n");
        move_backward(LEFT_MOTOR_PIN1, LEFT_MOTOR_PIN2, RIGHT_MOTOR_PIN1, RIGHT_MOTOR_PIN2);
    }
    else if (recv_buffer[0] == TURN_LEFT[0])
    {
        printf("Turning left\n");
        turn_left(LEFT_MOTOR_PIN1, LEFT_MOTOR_PIN2, RIGHT_MOTOR_PIN1, RIGHT_MOTOR_PIN2);
    }
    else if (recv_buffer[0] == TURN_RIGHT[0])
    {
        printf("Turning right\n");
        turn_right(LEFT_MOTOR_PIN1, LEFT_MOTOR_PIN2, RIGHT_MOTOR_PIN1, RIGHT_MOTOR_PIN2);
    }
    else
    {
        printf("Stopping\n");
        stop_motor(LEFT_MOTOR_PIN1, LEFT_MOTOR_PIN2, RIGHT_MOTOR_PIN1, RIGHT_MOTOR_PIN2);
    }

}

int main()
{

    stdio_init_all();

    // Initialize the ultrasonic sensor
    ultrasonic_init();

    // Initialize the infrared sensor
    infrared_sensor_init();

    // Initialize the motor
    start_motor(LEFT_MOTOR_PIN1, LEFT_MOTOR_PIN2, RIGHT_MOTOR_PIN1, RIGHT_MOTOR_PIN2);

    init_encoder(ENCODEROUT_PIN, ENCODEROUT_PIN2);

    // Initialize the Wi-Fi driver
    if (wifi_init())
    {
        cyw43_arch_deinit();
        return 1;
    }

    // Initialize the TCP server and open the connection
    TCP_SERVER_T *state = create_tcp_server();
    if (!state)
    {
        cyw43_arch_deinit();
        return 1;
    }

    while (1)
    {
        // The main loop
        cyw43_arch_poll(); // Poll for Wi-Fi driver or lwIP work
        sleep_ms(1000);
    }

    return 0;
}