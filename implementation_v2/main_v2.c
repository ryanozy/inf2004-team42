#include <stdio.h>
#include "pico/stdlib.h"
#include <stdbool.h>
#include "pico/time.h"
#include <string.h>
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"

// #include "infrared.h"
#include "motor.h"
#include "ultrasonic_sensor.h"
#include "wifi.h"

#define LEFT_MOTOR_PIN1 14
#define LEFT_MOTOR_PIN2 13
#define RIGHT_MOTOR_PIN1 12
#define RIGHT_MOTOR_PIN2 11
#define LEFT_MOTOR_PWM_PIN 15
#define RIGHT_MOTOR_PWM_PIN 10
#define ENCODER_LEFT_PIN 2
#define ENCODER_RIGHT_PIN 3

// WIFI Commands
const static char *MOVE_FORWARD = "w";
const static char *MOVE_BACKWARD = "s";
const static char *TURN_LEFT = "a";
const static char *TURN_RIGHT = "d";
const static char *STOP = "x";


struct repeating_timer speedtimer;
struct repeating_timer pidtimer;
struct repeating_timer getheading;

double front_heading = 0.0;

// Function is created in the wifi.h file and is used to control the vehicle
void motor_control(char *recv_buffer){

    printf("Received command: %s\n", recv_buffer);
    if (recv_buffer[0] == MOVE_FORWARD[0])
    {
        printf("Moving forward\n");
        move_forward(12500);
    }
    else if (recv_buffer[0] == MOVE_BACKWARD[0])
    {
        printf("Moving backward\n");
        move_backward(12500);
    }
    else if (recv_buffer[0] == TURN_LEFT[0])
    {
        printf("Turning left\n");
        turn_left(12500, 90);
    }
    else if (recv_buffer[0] == TURN_RIGHT[0])
    {
        printf("Turning right\n");
        turn_right(12500, 90);
    }
    else if (recv_buffer[0] == STOP[0])
    {
        printf("Stopping\n");
        stop_motors();
    }
    else
    {
        printf("Invalid command or command already executing\n");
    }
}

void interrupt_handler(uint gpio, uint32_t events)
{
    printf("Interrupt triggered\n");
    printf("GPIO: %d, Events: %d\n", gpio, events);
}

int main()
{
    // Initialize the stdio driver
    stdio_init_all();

    // Initialize the motor
    init_motors(LEFT_MOTOR_PIN1, LEFT_MOTOR_PIN2, RIGHT_MOTOR_PIN1, RIGHT_MOTOR_PIN2, LEFT_MOTOR_PWM_PIN, RIGHT_MOTOR_PWM_PIN, ENCODER_LEFT_PIN, ENCODER_RIGHT_PIN);

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