#include <stdio.h>          // Include standard input-output library
#include "pico/stdlib.h"    // Include the Pico standard library
#include "hardware/gpio.h"  // Include the GPIO hardware library
#include "hardware/timer.h" // Include the timer hardware library

#define TRIGGER_PIN 19 // Define the GPIO pin for the ultrasonic sensor trigger
#define ECHO_PIN 18    // Define the GPIO pin for the ultrasonic sensor echo

uint32_t pulse_start_time = 0; // Variable to store the start time of the echo pulse
uint32_t pulse_end_time = 0;   // Variable to store the end time of the echo pulse
bool pulse_received = false;   // Flag to indicate if an echo pulse has been received
float distance_from_car = 0.0; // Variable to store the distance measured by the ultrasonic sensor

void interrupt_handler(uint gpio, uint32_t events);
bool checkPulseTimes(uint32_t start_time, uint32_t end_time);
void on_echo_pin_change(uint gpio, uint32_t events);
uint32_t getPulseDuration();
void ultrasonic_init();
float calculateDistance(float pulse_duration);

bool checkPulseTimes(uint32_t start_time, uint32_t end_time)
{
    // Check if start time is less than end time
    if (start_time < end_time)
    {
        return true; // Valid pulse times
    }
    else
    {
        return false; // Invalid pulse times
    }
}

void on_echo_pin_change(uint gpio, uint32_t events)
{
    if (gpio_get(ECHO_PIN))
    {
        pulse_start_time = time_us_32(); // Record the start time of the echo pulse
    }
    else
    {
        pulse_end_time = time_us_32();                                      // Record the end time of the echo pulse
        pulse_received = checkPulseTimes(pulse_start_time, pulse_end_time); // Set the flag to indicate that the echo pulse has been received
        if (pulse_received)
        {
            distance_from_car = calculateDistance(pulse_end_time - pulse_start_time); // Calculate the distance
        }
    }
}

void ultrasonic_init()
{
    gpio_init(TRIGGER_PIN);              // Initialize the trigger pin
    gpio_init(ECHO_PIN);                 // Initialize the echo pin
    gpio_set_dir(TRIGGER_PIN, GPIO_OUT); // Set trigger pin as output
    gpio_set_dir(ECHO_PIN, GPIO_IN);     // Set echo pin as input
    gpio_set_irq_enabled_with_callback(ECHO_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &interrupt_handler);
    // Enable interrupt on both rising and falling edges of the echo pulse
}

void shoot_pulse()
{
    pulse_received = false;       // Reset the pulse received flag
    gpio_put(TRIGGER_PIN, true);  // Send a trigger pulse
}

void stop_pulse()
{
    gpio_put(TRIGGER_PIN, false); // Stop the trigger pulse
}

bool ultrasonic_sensor_handler()
{
    shoot_pulse();

    if (pulse_received)
    {
        printf("Distance: %f\n", distance_from_car);
    }

    return true;
}

uint32_t getPulseDuration()
{
    pulse_received = false;       // Reset the pulse received flag
    gpio_put(TRIGGER_PIN, true);  // Send a trigger pulse
    sleep_us(10);                 // Wait for 10 microseconds
    gpio_put(TRIGGER_PIN, false); // Stop the trigger pulse

    // Wait for the echo pulse to be received or timeout after 1 second
    uint32_t timeout_start = time_us_32(); // Record the start time of the timeout
    while (!pulse_received && (time_us_32() - timeout_start) < 1000000)
    {
        tight_loop_contents(); // Wait without consuming CPU cycles
    }

    return pulse_received ? pulse_end_time - pulse_start_time : 0.0;
}

float calculateDistance(float pulse_duration)
{
    // Speed of sound at sea level is approximately 343 meters per second or 34300 centimeters per second
    // Distance = (Speed of sound * Pulse duration) / 2
    return (float)(pulse_duration * 34300) / (2 * 1000000); // Convert microseconds to seconds (10^6) and calculate distance in centimeters
}