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
const static char *MAP = "m";
const static char *RACE = "r";

double front_heading = 0.0;

struct repeating_timer left_encoder_cool_down_timer;
struct repeating_timer right_encoder_cool_down_timer;
struct repeating_timer left_infrared_cool_down_timer;
struct repeating_timer right_infrared_cool_down_timer;

// Mapping variables
bool isMapping = false;
bool isRacing = false;
double start_mapping_time = 0.0;
double timeout = 20;

volatile uint32_t left_last_pulse_time = 0;
volatile uint32_t right_last_pulse_time = 0;

bool left_line_tiggered = false;
bool right_line_tiggered = false;

bool reset_left_encoder_cool_down(struct repeating_timer *t);
bool reset_right_encoder_cool_down(struct repeating_timer *t);
bool reset_left_infrared_cool_down(struct repeating_timer *t);
bool reset_right_infrared_cool_down(struct repeating_timer *t);

// Create a struct to hold Node data
struct Node
{
    bool is_wall_left;
    bool is_wall_right;
    bool is_wall_front;
    struct Node *left;
    struct Node *right;
    struct Node *front;
    struct Node *prev;
};

// Create a master node
struct Node *master_node;

// create a SAMPLE test 2d array to hold the nodes
struct Node *create_map()
{
    // create a 4x4 grid
    //  _   _ _
    // |  _|   |
    // | |  _| |
    // | | | | |
    // |_ _|  _|

    // create the nodes
    struct Node *map[4][4];

    // Initialize the map with nodes
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            map[i][j] = malloc(sizeof(struct Node));
            map[i][j]->left = NULL;
            map[i][j]->right = NULL;
            map[i][j]->front = NULL;
            map[i][j]->prev = NULL;
        }
    }

    // Connect the nodes
    // Row 1
    map[0][0]->front = NULL;
    map[0][0]->right = map[0][1];
    map[0][0]->prev = map[1][0];
    map[0][0]->left = NULL;
    map[0][0]->is_wall_left = true;
    map[0][0]->is_wall_right = false;
    map[0][0]->is_wall_front = true;

    map[0][1]->front = NULL;
    map[0][1]->right = map[0][2];
    map[0][1]->left = map[0][0];
    map[0][1]->prev = map[1][1];
    map[0][1]->is_wall_left = false;
    map[0][1]->is_wall_right = true;
    map[0][1]->is_wall_front = true;

    map[0][2]->front = NULL;
    map[0][2]->right = map[0][3];
    map[0][2]->left = map[0][1];
    map[0][2]->prev = map[1][2];
    map[0][2]->is_wall_left = true;
    map[0][2]->is_wall_right = false;
    map[0][2]->is_wall_front = true;

    map[0][3]->front = NULL;
    map[0][3]->right = NULL;
    map[0][3]->left = map[0][2];
    map[0][3]->prev = map[1][3];
    map[0][3]->is_wall_left = false;
    map[0][3]->is_wall_right = true;
    map[0][3]->is_wall_front = true;

    // Row 2
    map[1][0]->front = map[0][0];
    map[1][0]->right = map[1][1];
    map[1][0]->left = NULL;
    map[1][0]->prev = map[2][0];
    map[1][0]->is_wall_left = true;
    map[1][0]->is_wall_right = false;
    map[1][0]->is_wall_front = false;

    map[1][1]->front = map[0][1];
    map[1][1]->right = map[1][2];
    map[1][1]->left = map[1][0];
    map[1][1]->prev = map[2][1];
    map[1][1]->is_wall_left = false;
    map[1][1]->is_wall_right = false;
    map[1][1]->is_wall_front = false;

    map[1][2]->front = map[0][2];
    map[1][2]->right = map[1][3];
    map[1][2]->left = map[1][1];
    map[1][2]->prev = map[2][2];
    map[1][2]->is_wall_left = false;
    map[1][2]->is_wall_right = false;
    map[1][2]->is_wall_front = false;

    map[1][3]->front = map[0][3];
    map[1][3]->right = NULL;
    map[1][3]->left = map[1][2];
    map[1][3]->prev = map[2][3];
    map[1][3]->is_wall_left = false;
    map[1][3]->is_wall_right = true;
    map[1][3]->is_wall_front = false;

    // Row 3
    map[2][0]->front = map[1][0];
    map[2][0]->right = map[2][1];
    map[2][0]->left = NULL;
    map[2][0]->prev = map[3][0];
    map[2][0]->is_wall_left = true;
    map[2][0]->is_wall_right = false;
    map[2][0]->is_wall_front = false;

    map[2][1]->front = map[1][1];
    map[2][1]->right = map[2][2];
    map[2][1]->left = map[2][0];
    map[2][1]->prev = map[3][1];
    map[2][1]->is_wall_left = false;
    map[2][1]->is_wall_right = false;
    map[2][1]->is_wall_front = false;

    map[2][2]->front = map[1][2];
    map[2][2]->right = map[2][3];
    map[2][2]->left = map[2][1];
    map[2][2]->prev = map[3][2];
    map[2][2]->is_wall_left = false;
    map[2][2]->is_wall_right = false;
    map[2][2]->is_wall_front = false;

    map[2][3]->front = map[1][3];
    map[2][3]->right = NULL;
    map[2][3]->left = map[2][2];
    map[2][3]->prev = map[3][3];
    map[2][3]->is_wall_left = false;
    map[2][3]->is_wall_right = true;
    map[2][3]->is_wall_front = false;

    // Row 4
    map[3][0]->front = map[2][0];
    map[3][0]->right = map[3][1];
    map[3][0]->left = NULL;
    map[3][0]->prev = NULL;
    map[3][0]->is_wall_left = true;
    map[3][0]->is_wall_right = false;
    map[3][0]->is_wall_front = true;

    map[3][1]->front = map[2][1];
    map[3][1]->right = map[3][2];
    map[3][1]->left = map[3][0];
    map[3][1]->prev = NULL;

    map[3][1]->is_wall_left = false;
    map[3][1]->is_wall_right = false;
    map[3][1]->is_wall_front = true;

    map[3][2]->front = map[2][2];
    map[3][2]->right = map[3][3];
    map[3][2]->left = map[3][1];
    map[3][2]->prev = NULL;
    map[3][2]->is_wall_left = false;
    map[3][2]->is_wall_right = false;
    map[3][2]->is_wall_front = true;

    map[3][3]->front = map[2][3];
    map[3][3]->right = NULL;
    map[3][3]->left = map[3][2];
    map[3][3]->prev = NULL;
    map[3][3]->is_wall_left = false;
    map[3][3]->is_wall_right = true;
    map[3][3]->is_wall_front = true;

    return map[0][0];
}

void init_master_node()
{
    // Initialize the master node
    master_node = malloc(sizeof(struct Node));
    master_node->left = NULL;
    master_node->right = NULL;
    master_node->front = NULL;
    master_node->prev = NULL;
}

void start_racing()
{
    // Master node will be used to calculate the shortest path
    // Start maze solving algorithm
    // The robot will move and complete the maze
    isRacing = false;
}

struct Node *add_node_to_map(struct Node *prev_node, char direction)
{
    struct Node *new_node = malloc(sizeof(struct Node));
    new_node->left = NULL;
    new_node->right = NULL;
    new_node->front = NULL;
    new_node->prev = prev_node;

    if (direction == 'l')
    {
        prev_node->left = new_node;
    }
    else if (direction == 'r')
    {
        prev_node->right = new_node;
    }
    else if (direction == 'f')
    {
        prev_node->front = new_node;
    }

    return new_node;
}

bool checkWall(char direction)
{
    // wait till turn finish
    while (left_encoder_count < 20 || right_encoder_count < 20)
    {
        // do nothing
    }

    // move forward till hit wall
    while (!left_line_tiggered && !right_line_tiggered)
    {
        if (left_encoder_count > 20)
        {
            // no wall
            return false;
        }
    }
    return true;
}

void start_mapping()
{
    // Master node can be at any position outside the grid
    // Robot will move into the grid and start mapping.
    // The size of 1 grid is about 1 wheel rotation
    // The robot will move forward until it hits a wall
    printf("Starting mapping\n");

    move_forward(5000, 5625);

    // wait for the robot to hit a wall
    while (!left_line_tiggered && !right_line_tiggered)
    {
        if (left_encoder_count > 20 || right_encoder_count > 20)
        {
            // Add a node to the front of the master node
            struct Node *new_node = add_node_to_map(master_node, 'f');
            master_node = new_node;

            // Reset the encoder count
            left_encoder_count = 0;
            right_encoder_count = 0;
        }
    }

    master_node->is_wall_front = true;

    // Algorithm to map the maze
    if (master_node->is_wall_left)
    {
        master_node->is_wall_left = checkWall('l'); // check if there is a wall on the left
    }
    else if (master_node->is_wall_right)
    {
        master_node->is_wall_right = checkWall('r'); // check if there is a wall on the right
    }
    else if (master_node->is_wall_front)
    {
        master_node->is_wall_front = checkWall('f'); // check if there is a wall in front
    }

    // if time is up, stop mapping
    if (timeout < time_us_32() - start_mapping_time)
    {
        isMapping = false;
    }

    start_mapping();
}

// Function is created in the wifi.h file and is used to control the vehicle
void motor_control(char *recv_buffer)
{
    // Print the command received
    printf("Received command: %s\n", recv_buffer);
    if (recv_buffer[0] == MOVE_FORWARD[0])
    {
        printf("Moving forward\n");
        move_forward(5625, 5625);
    }
    else if (recv_buffer[0] == MOVE_BACKWARD[0])
    {
        printf("Moving backward\n");
        move_backward(5625, 5625);
    }
    else if (recv_buffer[0] == TURN_LEFT[0])
    {
        printf("Turning left\n");
        turn_left(5625, 5625, 90);
    }
    else if (recv_buffer[0] == TURN_RIGHT[0])
    {
        printf("Turning right\n");
        turn_right(5625, 5625, 90);
    }
    else if (recv_buffer[0] == STOP[0])
    {
        printf("Stopping\n");
        stop_motors();
        isMapping = false;
    }
    else if (recv_buffer[0] == MAP[0])
    {
        // Start mapping
        master_node = create_Sample_map();

        print_tree(master_node);
    }
    else if (recv_buffer[0] == RACE[0])
    {
        isMapping = true;
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
        add_repeating_timer_ms(200, reset_right_encoder_cool_down, NULL, &right_encoder_cool_down_timer);
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
            left_line_tiggered = false;
        }

        // Disable the interrupt for 100ms to prevent multiple interrupts from being triggered
        gpio_set_irq_enabled_with_callback(LEFT_LINE_SENSOR_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false, &interrupt_handler);
        add_repeating_timer_ms(5, reset_left_infrared_cool_down, NULL, &left_infrared_cool_down_timer);
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
        add_repeating_timer_ms(5, reset_right_infrared_cool_down, NULL, &right_infrared_cool_down_timer);
    }

    if (gpio == BARCODE_SENSOR_PIN)
    {
        if (events == GPIO_IRQ_EDGE_RISE)
        {
            printf("Barcode sensor black\n");
        }
    }

    // Check if the robot has hit a wall
    if (left_line_tiggered && right_line_tiggered)
    {
        printf("Both line sensors triggered\n");
        stop_motors();
        left_line_tiggered = false;
        right_line_tiggered = false;
    }
    else if (left_line_tiggered && !right_line_tiggered)
    {
        // Tilt Left
        set_speed(6225, 5625);
        left_line_tiggered = false;
        target_heading += 5;
    }
    else if (!left_line_tiggered && right_line_tiggered)
    {
        // Tilt Right
        set_speed(5625, 6225);
        right_line_tiggered = false;
        target_heading -= 5;
    }
    else if (!left_line_tiggered && !right_line_tiggered)
    {
        // Move forward
        set_speed(5625, 5625);
        target_heading = get_heading();
    }

    // if (movement_direction == 'w' || movement_direction == 's' || movement_direction == 'x')
    // {
    //     if (left_line_tiggered && right_line_tiggered)
    //     {
    //         printf("Both line sensors triggered\n");
    //         stop_motors();
    //         left_line_tiggered = false;
    //         right_line_tiggered = false;
    //     }
    //     else if (left_line_tiggered && !right_line_tiggered)
    //     {
    //         // Tilt Left
    //         set_speed(6225, 5625);
    //         left_line_tiggered = false;
    //         target_heading += 5;
    //     }
    //     else if (!left_line_tiggered && right_line_tiggered)
    //     {
    //         // Tilt Right
    //         set_speed(5625, 6225);
    //         right_line_tiggered = false;
    //         target_heading -= 5;
    //     }
    //     else if (!left_line_tiggered && !right_line_tiggered)
    //     {
    //         // Move forward
    //         set_speed(5625, 5625);
    //         target_heading = get_heading();
    //     }
    // }
}

// Cool down functions to prevent multiple interrupts from being triggered
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

    // Configure the repeating timer to control the motors using PID
    struct repeating_timer pid_timer;
    add_repeating_timer_ms(50, pid_control, NULL, &pid_timer);

    // Initialize the master node
    init_master_node();

    while (1)
    {
        // If wifi calls mapping mode or racing mode
        while (isMapping)
        {
            printf("Mapping\n");
            start_mapping_time = time_us_32();
            start_mapping();
            printf("Mapping complete\n");
            // print the master nodes search tree
            print_tree(master_node);
        }

        while (isRacing)
        {
            start_racing();
        }

        tight_loop_contents();
    }

    return 0;
}