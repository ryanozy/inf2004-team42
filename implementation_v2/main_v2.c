#include <stdio.h>
#include "pico/stdlib.h"
#include <stdbool.h>
#include "pico/time.h"
#include <string.h>
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include "motor.h"
#include "infrared.h"
// #include "ultrasonic_sensor.h"
#include "wifi.h"

#define LEFT_MOTOR_PIN1 14
#define LEFT_MOTOR_PIN2 13
#define RIGHT_MOTOR_PIN1 12
#define RIGHT_MOTOR_PIN2 11
#define LEFT_MOTOR_PWM_PIN 15
#define RIGHT_MOTOR_PWM_PIN 10
#define ENCODER_LEFT_PIN 2
#define ENCODER_RIGHT_PIN 3
#define LEFT_LINE_SENSOR_PIN 20
#define RIGHT_LINE_SENSOR_PIN 21
#define BARCODE_SENSOR_PIN 22

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
void motor_control(char *recv_buffer)
{

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

// Handles All the interrupts from the sensors
// Cannot use else if because the interrupts are not mutually exclusive
void interrupt_handler(uint gpio, uint32_t events)
{
    printf("Interrupt triggered\n");
    printf("GPIO: %d, Events: %d\n", gpio, events);

    // Check if there is any command from the server
    cyw43_arch_poll(); // Poll for Wi-Fi driver or lwIP work

    if (gpio == ENCODER_LEFT_PIN)
    {
        if (events == GPIO_IRQ_EDGE_RISE)
        {
            left_encoder_count++;
        }
        else if (events == GPIO_IRQ_EDGE_FALL)
        {
            left_encoder_count--;
        }
        printf("Left encoder count: %d\n", left_encoder_count);
    }
    
    if (gpio == ENCODER_RIGHT_PIN)
    {
        if (events == GPIO_IRQ_EDGE_RISE)
        {
            right_encoder_count++;
        }
        else if (events == GPIO_IRQ_EDGE_FALL)
        {
            right_encoder_count--;
        }
        printf("Right encoder count: %d\n", right_encoder_count);
    }
    
    if (gpio == LEFT_LINE_SENSOR_PIN)
    {
        if (events == GPIO_IRQ_EDGE_RISE)
        {
            printf("Left line sensor black\n");
        }
        else if (events == GPIO_IRQ_EDGE_FALL)
        {
            printf("Left line sensor white\n");
        }
    }
    
    if (gpio == RIGHT_LINE_SENSOR_PIN)
    {
        if (events == GPIO_IRQ_EDGE_RISE)
        {
            printf("Right line sensor black\n");
        }
        else if (events == GPIO_IRQ_EDGE_FALL)
        {
            printf("Right line sensor white\n");
        }
    }
    
    if (gpio == BARCODE_SENSOR_PIN)
    {
        if (events == GPIO_IRQ_EDGE_RISE)
        {
            printf("Barcode sensor black\n");
        }
        else if (events == GPIO_IRQ_EDGE_FALL)
        {
            printf("Barcode sensor white\n");
        }
    }
}

bool check_wifi_status(struct repeating_timer *t)
{
    cyw43_arch_poll(); // Poll for Wi-Fi driver or lwIP work
    return true;
}

int main()
{
    // Initialize the stdio driver
    stdio_init_all();

    // Initialize the motor
    init_motors(LEFT_MOTOR_PIN1, LEFT_MOTOR_PIN2, RIGHT_MOTOR_PIN1, RIGHT_MOTOR_PIN2, LEFT_MOTOR_PWM_PIN, RIGHT_MOTOR_PWM_PIN, ENCODER_LEFT_PIN, ENCODER_RIGHT_PIN);

    // Initialize the Wi-Fi driver
    start_wifi();

    // Initialize the infrared sensors
    init_infrared(LEFT_LINE_SENSOR_PIN, RIGHT_LINE_SENSOR_PIN, BARCODE_SENSOR_PIN);

    // Enable interrupt on line sensor and barcode sensor
    gpio_set_irq_enabled_with_callback(LEFT_LINE_SENSOR_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &interrupt_handler);
    gpio_set_irq_enabled_with_callback(RIGHT_LINE_SENSOR_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &interrupt_handler);
    gpio_set_irq_enabled_with_callback(BARCODE_SENSOR_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &interrupt_handler);

    // Enable interrupt on encoder
    gpio_set_irq_enabled_with_callback(ENCODER_LEFT_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &interrupt_handler);
    gpio_set_irq_enabled_with_callback(ENCODER_RIGHT_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &interrupt_handler);

    struct repeating_timer check_wifi;
    add_repeating_timer_ms(-1000, check_wifi_status, NULL, &check_wifi);

    while (1)
    {
        // DO NOTHING
        tight_loop_contents();
    }

    return 0;
}