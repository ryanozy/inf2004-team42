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

char strVal[50] = "";

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

double front_heading = 0.0;

void motor_control(char recv_buffer[1])
{

    if (recv_buffer[0] == MOVE_FORWARD[0])
    {
        move_forward(LEFT_MOTOR_PIN1, LEFT_MOTOR_PIN2, RIGHT_MOTOR_PIN1, RIGHT_MOTOR_PIN2);
        front_heading = get_heading();
    }
    else if (recv_buffer[0] == MOVE_BACKWARD[0])
    {
        move_backward(LEFT_MOTOR_PIN1, LEFT_MOTOR_PIN2, RIGHT_MOTOR_PIN1, RIGHT_MOTOR_PIN2);
    }
    else if (recv_buffer[0] == TURN_LEFT[0])
    {
        turn_left(LEFT_MOTOR_PIN1, LEFT_MOTOR_PIN2, RIGHT_MOTOR_PIN1, RIGHT_MOTOR_PIN2, 80.0);
    }
    else if (recv_buffer[0] == TURN_RIGHT[0])
    {
        turn_right(LEFT_MOTOR_PIN1, LEFT_MOTOR_PIN2, RIGHT_MOTOR_PIN1, RIGHT_MOTOR_PIN2, 80.0);
    }
    else
    {
        stop_motor(LEFT_MOTOR_PIN1, LEFT_MOTOR_PIN2, RIGHT_MOTOR_PIN1, RIGHT_MOTOR_PIN2);
    }
}

void check_dead_end()
{

    if (line_check_right == true && line_check_left == true)
    {
        printf("Dead end\n");
        stop_motor(LEFT_MOTOR_PIN1, LEFT_MOTOR_PIN2, RIGHT_MOTOR_PIN1, RIGHT_MOTOR_PIN2);
        // move_forward_by_distance(LEFT_MOTOR_PIN1, LEFT_MOTOR_PIN2, RIGHT_MOTOR_PIN1, RIGHT_MOTOR_PIN2, 5.0);
        turn_left(LEFT_MOTOR_PIN1, LEFT_MOTOR_PIN2, RIGHT_MOTOR_PIN1, RIGHT_MOTOR_PIN2, 80.0);
    }
    // else if (line_check_right == false && line_check_left == true)
    // {
    //     left_desired_speed_cm_s = 35;
    //     right_desired_speed_cm_s = 30;
    // }
    // else if (line_check_right == true && line_check_left == false)
    // {
    //     right_desired_speed_cm_s = 35;
    //     left_desired_speed_cm_s = 30;
    // }
    // else if (line_check_left == false && line_check_right == false)
    // {
    //     left_desired_speed_cm_s = 30;
    //     right_desired_speed_cm_s = 30;
    // }
}

void interrupt_handler(uint gpio, uint32_t events)
{
    if (gpio == BARCODE_SENSOR_PIN)
    {
        barcode_sensor_handler(gpio, events);
    }
    else if (gpio == LEFT_LINE_SENSOR_PIN || gpio == RIGHT_LINE_SENSOR_PIN)
    {
        line_sensor_handler(gpio, events);
        check_dead_end();
    }
    else if (gpio == ECHO_PIN)
    {
        on_echo_pin_change(gpio, events);
    }
    else if (gpio == ENCODEROUT_PIN || gpio == ENCODEROUT_PIN2)
    {
        gpio_callback(gpio, events);
    }
}

bool get_new_heading()
{

    float temp_heading = get_heading();
    if (temp_heading < 0)
    {
        temp_heading = temp_heading * -1.0;
    }

    if ((temp_heading - front_heading) > 0.2)
    {
        right_desired_speed_cm_s = 30;
        left_desired_speed_cm_s = 35;
    }
    else if ((temp_heading - front_heading) < -0.2)
    {

        left_desired_speed_cm_s = 35;
        right_desired_speed_cm_s = 30;
    }
    else
    {
        right_desired_speed_cm_s = 30;
        left_desired_speed_cm_s = 30;
    }

    return true;
}

bool getdistance()
{
    uint32_t pulse_duration = measurePulse();
    // Measure distance in centimeters
    float distance_cm = calculateDistance(pulse_duration);
    // Print the distance.
    printf("Distance: %f cm\n", distance_cm);

    return true;
}

bool clear_terminal()
{
    // printf("\033[2J");
    return true;
}

int main()
{

    stdio_init_all();

    // Initialize the infrared sensor
    infrared_sensor_init();

    // Initialize the ultrasonic sensor
    ultrasonic_init();

    init_i2c_bus();

    init_magnometer();

    // Initialize the motor
    start_motor(LEFT_MOTOR_PIN1, LEFT_MOTOR_PIN2, RIGHT_MOTOR_PIN1, RIGHT_MOTOR_PIN2);

    start_motor_pwm(LEFT_MOTOR_PWM_PIN, RIGHT_MOTOR_PWM_PIN);

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

    // move_forward(LEFT_MOTOR_PIN1, LEFT_MOTOR_PIN2, RIGHT_MOTOR_PIN1, RIGHT_MOTOR_PIN2);

    // Set up GPIO interrupt on rising edge
    gpio_set_irq_enabled_with_callback(RIGHT_LINE_SENSOR_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_handler);
    gpio_set_irq_enabled_with_callback(LEFT_LINE_SENSOR_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_handler);

    // Set up GPIO interrupt on falling edge
    gpio_set_irq_enabled_with_callback(RIGHT_LINE_SENSOR_PIN, GPIO_IRQ_EDGE_FALL, true, &interrupt_handler);
    gpio_set_irq_enabled_with_callback(LEFT_LINE_SENSOR_PIN, GPIO_IRQ_EDGE_FALL, true, &interrupt_handler);

    // Setup GPIO interrupt for barcode sensor
    gpio_set_irq_enabled_with_callback(BARCODE_SENSOR_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &interrupt_handler);

    struct repeating_timer speedtimer;
    add_repeating_timer_ms(-20, &check_wheel_moving, NULL, &speedtimer);

    struct repeating_timer pidtimer;
    add_repeating_timer_ms(-20, &pid, NULL, &pidtimer);

    struct repeating_timer getheading;
    add_repeating_timer_ms(-20, &get_new_heading, NULL, &getheading);

    struct repeating_timer terminaltimer;
    add_repeating_timer_ms(1000, &clear_terminal, NULL, &terminaltimer);

    // move_forward(LEFT_MOTOR_PIN1, LEFT_MOTOR_PIN2, RIGHT_MOTOR_PIN1, RIGHT_MOTOR_PIN2);
    stop_motor(LEFT_MOTOR_PIN1, LEFT_MOTOR_PIN2, RIGHT_MOTOR_PIN1, RIGHT_MOTOR_PIN2);

    while (1)
    {
        // The main loop
        getdistance();
        cyw43_arch_poll(); // Poll for Wi-Fi driver or lwIP work
        sleep_ms(1000);
    }

    return 0;
}