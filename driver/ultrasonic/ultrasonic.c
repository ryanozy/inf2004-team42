#include "ultrasonic_sensor.h" // Include the ultrasonic sensor header file
#include <stdio.h> // Include standard input-output library
#include "pico/stdlib.h" // Include the Pico standard library
#include "hardware/gpio.h" // Include the GPIO hardware library
#include "hardware/timer.h" // Include the timer hardware library

#define TRIGGER_PIN 15 // Define the GPIO pin for the ultrasonic sensor trigger
#define ECHO_PIN 14 // Define the GPIO pin for the ultrasonic sensor echo

volatile uint32_t pulse_start_time = 0; // Variable to store the start time of the echo pulse
volatile uint32_t pulse_end_time = 0; // Variable to store the end time of the echo pulse
volatile bool pulse_received = false; // Flag to indicate if an echo pulse has been received

bool checkPulseTimes(uint32_t start_time, uint32_t end_time) {
    // Check if start time is less than end time
    if (start_time < end_time) {
        return true; // Valid pulse times
    } else {
        return false; // Invalid pulse times
    }
}

void on_echo_pin_change(uint gpio, uint32_t events) {
    if (gpio_get(ECHO_PIN)) {
        pulse_start_time = time_us_32(); // Record the start time of the echo pulse
    } else {
        pulse_end_time = time_us_32(); // Record the end time of the echo pulse
        pulse_received = checkPulseTimes(pulse_start_time,pulse_end_time); // Set the flag to indicate that the echo pulse has been received
    }
}
 
void ultrasonic_init() {
    gpio_init(TRIGGER_PIN); // Initialize the trigger pin
    gpio_init(ECHO_PIN); // Initialize the echo pin
    gpio_set_dir(TRIGGER_PIN, GPIO_OUT); // Set trigger pin as output
    gpio_set_dir(ECHO_PIN, GPIO_IN); // Set echo pin as input
    gpio_set_irq_enabled_with_callback(ECHO_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &on_echo_pin_change);
    // Enable interrupt on both rising and falling edges of the echo pulse

    // Initialize the Pico SDK for standard input-output
    stdio_init_all();
}

uint32_t getPulseDuration() {
    pulse_received = false; // Reset the pulse received flag
    gpio_put(TRIGGER_PIN, true); // Send a trigger pulse
    sleep_us(10); // Wait for 10 microseconds
    gpio_put(TRIGGER_PIN, false); // Stop the trigger pulse

    // Wait for the echo pulse to be received
    while (!pulse_received) {
        tight_loop_contents(); // Wait without consuming CPU cycles
    }

    return pulse_end_time - pulse_start_time; // Return the duration of the echo pulse in microseconds
}

float calculateDistance(float pulse_duration) {
    // Speed of sound at sea level is approximately 343 meters per second or 34300 centimeters per second
    // Distance = (Speed of sound * Pulse duration) / 2
    return (float)(pulse_duration * 34300) / (2 * 1000000); // Convert microseconds to seconds (10^6) and calculate distance in centimeters
}

int main() {
    ultrasonic_init(); // Initialize ultrasonic sensor

    while (1) {
        float pulse_duration = getPulseDuration(); // Measure the duration of the echo pulse
        float distance_cm = calculateDistance(pulse_duration); // Calculate distance in centimeters
        printf("Distance: %.3f cm\n", distance_cm); // Print the distance with three decimal places
        sleep_ms(1000); // Wait for 1 second before measuring again
    }

    return 0; // Return 0 to indicate successful execution
}
