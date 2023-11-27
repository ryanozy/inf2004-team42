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
const static char *MAPPING = "m";
const static char *RUN = "r";

double front_heading = 0.0;

struct repeating_timer left_encoder_cool_down_timer;
struct repeating_timer right_encoder_cool_down_timer;
struct repeating_timer left_infrared_cool_down_timer;
struct repeating_timer right_infrared_cool_down_timer;

volatile uint32_t left_last_pulse_time = 0;
volatile uint32_t right_last_pulse_time = 0;

bool left_line_tiggered = false;
bool right_line_tiggered = false;

bool reset_left_encoder_cool_down(struct repeating_timer *t);
bool reset_right_encoder_cool_down(struct repeating_timer *t);
bool reset_left_infrared_cool_down(struct repeating_timer *t);
bool reset_right_infrared_cool_down(struct repeating_timer *t);

bool start_mapping = false;
bool start_running = false;

#define GRID_SIZE 4
typedef struct Node
{
    int x;
    int y;
    int value;
    struct Node *next;
    struct Node *prev;
    struct Node *left;
    struct Node *right;
    struct Node *up;
    struct Node *down;
} Node;

void explore(Node *currentNode, Node grid[GRID_SIZE][GRID_SIZE])
{
    // Mark the current node as visited
    currentNode->value = 1;

    // Print the current node's coordinates
    printf("Visited node: (%d, %d)\n", currentNode->x, currentNode->y);

    // Explore the adjacent nodes
    if (currentNode->left && currentNode->left->value == 0)
        explore(currentNode->left, grid);
    if (currentNode->right && currentNode->right->value == 0)
        explore(currentNode->right, grid);
    if (currentNode->up && currentNode->up->value == 0)
        explore(currentNode->up, grid);
    if (currentNode->down && currentNode->down->value == 0)
        explore(currentNode->down, grid);
}

void mapGrid()
{
    Node grid[GRID_SIZE][GRID_SIZE];

    // Initialize grid nodes
    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            grid[i][j].x = i;
            grid[i][j].y = j;
            grid[i][j].value = 0;
            grid[i][j].next = NULL;
            grid[i][j].prev = NULL;
            grid[i][j].left = (j > 0) ? &grid[i][j - 1] : NULL;
            grid[i][j].right = (j < GRID_SIZE - 1) ? &grid[i][j + 1] : NULL;
            grid[i][j].up = (i > 0) ? &grid[i - 1][j] : NULL;
            grid[i][j].down = (i < GRID_SIZE - 1) ? &grid[i + 1][j] : NULL;
        }
    }

    // Start exploring from the top-left node
    Node *startNode = &grid[0][0];
    explore(startNode, grid);
}


// Function is created in the wifi.h file and is used to control the vehicle
void motor_control(char *recv_buffer)
{

    printf("Received command: %s\n", recv_buffer);
    if (recv_buffer[0] == MOVE_FORWARD[0])
    {
        printf("Moving forward\n");
        move_forward(5000);
    }
    else if (recv_buffer[0] == MOVE_BACKWARD[0])
    {
        printf("Moving backward\n");
        move_backward(5000);
    }
    else if (recv_buffer[0] == TURN_LEFT[0])
    {
        printf("Turning left\n");
        turn_left(5000, 90);
    }
    else if (recv_buffer[0] == TURN_RIGHT[0])
    {
        printf("Turning right\n");
        turn_right(5000, 90);
    }
    else if (recv_buffer[0] == STOP[0])
    {
        printf("Stopping\n");
        stop_motors();
    }
    else if (recv_buffer[0] == MAPPING[0])
    {
        start_mapping = true;
    }
    else if (recv_buffer[0] == RUN[0])
    {
        start_running = true;
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
    if (gpio == ENCODER_LEFT_PIN)
    {
        // TODO: MOVE CODE OVER TO motor.h
        //  Left encoder triggered time
        uint32_t current_time = time_us_32();
        uint32_t time_since_last_pulse = current_time - left_last_pulse_time;

        if (events == GPIO_IRQ_EDGE_RISE)
        {
            left_encoder_count++;
        }

        if (time_since_last_pulse != 0)
        {
            left_encoder_speed = 1000000.0 / time_since_last_pulse;
        }

        float degrees_turned = left_encoder_count * 4.6;

        if (degrees_turned > set_heading && movement_direction == 'a')
        {
            stop_motors();
        }
        left_last_pulse_time = current_time;

        // Disable the interrupt for 100ms to prevent multiple interrupts from being triggered
        gpio_set_irq_enabled_with_callback(ENCODER_LEFT_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false, &interrupt_handler);
        add_repeating_timer_ms(200, reset_left_encoder_cool_down, NULL, &left_encoder_cool_down_timer);
    }

    if (gpio == ENCODER_RIGHT_PIN)
    {
        // Right encoder triggered time
        uint32_t current_time = time_us_32();
        uint32_t time_since_last_pulse = current_time - right_last_pulse_time;

        if (events == GPIO_IRQ_EDGE_RISE)
        {
            right_encoder_count++;
        }

        if (time_since_last_pulse != 0)
        {
            right_encoder_speed = 1000000.0 / time_since_last_pulse;
        }

        float degrees_turned = right_encoder_count * 4.6;

        if (degrees_turned > set_heading && movement_direction == 'd')
        {
            stop_motors();
        }

        right_last_pulse_time = current_time;

        // Disable the interrupt for 100ms to prevent multiple interrupts from being triggered
        gpio_set_irq_enabled_with_callback(ENCODER_RIGHT_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false, &interrupt_handler);
        add_repeating_timer_ms(100, reset_right_encoder_cool_down, NULL, &right_encoder_cool_down_timer);
    }

    if (gpio == LEFT_LINE_SENSOR_PIN)
    {
        if (events == GPIO_IRQ_EDGE_RISE)
        {
            // printf("Left line sensor black\n");
            left_line_tiggered = true;
        }
        else if (events == GPIO_IRQ_EDGE_FALL)
        {
            // printf("Left line sensor white\n");
            left_line_tiggered = false;
        }

        // Disable the interrupt for 100ms to prevent multiple interrupts from being triggered
        gpio_set_irq_enabled_with_callback(LEFT_LINE_SENSOR_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false, &interrupt_handler);
        add_repeating_timer_ms(100, reset_left_infrared_cool_down, NULL, &left_infrared_cool_down_timer);
    }

    if (gpio == RIGHT_LINE_SENSOR_PIN)
    {
        if (events == GPIO_IRQ_EDGE_RISE)
        {
            right_line_tiggered = true;
        }
        else if (events == GPIO_IRQ_EDGE_FALL)
        {
            right_line_tiggered = false;
        }

        // Disable the interrupt for 100ms to prevent multiple interrupts from being triggered
        gpio_set_irq_enabled_with_callback(RIGHT_LINE_SENSOR_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false, &interrupt_handler);
        add_repeating_timer_ms(200, reset_right_infrared_cool_down, NULL, &right_infrared_cool_down_timer);
    }

    if (gpio == BARCODE_SENSOR_PIN)
    {
        if (events == GPIO_IRQ_EDGE_RISE)
        {
            printf("Barcode sensor black\n");
        }
    }

    if (left_line_tiggered && right_line_tiggered)
    {
        printf("Both line sensors triggered\n");
        stop_motors();
        
    }
}

bool reset_left_encoder_cool_down(struct repeating_timer *t)
{
    gpio_set_irq_enabled_with_callback(ENCODER_LEFT_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &interrupt_handler);
    // disable the timer
    cancel_repeating_timer(t);
    return true;
}

bool reset_right_encoder_cool_down(struct repeating_timer *t)
{
    gpio_set_irq_enabled_with_callback(ENCODER_RIGHT_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &interrupt_handler);
    cancel_repeating_timer(t);
    return true;
}

bool reset_left_infrared_cool_down(struct repeating_timer *t)
{
    gpio_set_irq_enabled_with_callback(LEFT_LINE_SENSOR_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &interrupt_handler);
    cancel_repeating_timer(t);
    return true;
}

bool reset_right_infrared_cool_down(struct repeating_timer *t)
{
    gpio_set_irq_enabled_with_callback(RIGHT_LINE_SENSOR_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &interrupt_handler);
    cancel_repeating_timer(t);
    return true;
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

    // Configure the repeating timer to poll the Wi-Fi driver
    struct repeating_timer check_wifi;
    add_repeating_timer_ms(-500, check_wifi_status, NULL, &check_wifi);

    struct repeating_timer pid_timer;
    add_repeating_timer_ms(-5, pid_control, NULL, &pid_timer);

    while (1)
    {
        while (start_mapping)
        {
            // Mapping code
            mapGrid();
        }
    }

    return 0;
}