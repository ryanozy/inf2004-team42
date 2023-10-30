// Function Definitions for Motor Control
void start_motor(int leftmotor_pin1, int leftmotor_pin2, int rightmotor_pin1, int rightmotor_pin2);
void start_motor_pwm(int leftmotor_pwm_pin, int rightmotor_pwm_pin);
void move_forward(int leftmotor_pin1, int leftmotor_pin2, int rightmotor_pin1, int rightmotor_pin2);
void move_backward(int leftmotor_pin1, int leftmotor_pin2, int rightmotor_pin1, int rightmotor_pin2);
void turn_left(int leftmotor_pin1, int leftmotor_pin2, int rightmotor_pin1, int rightmotor_pin2);
void turn_right(int leftmotor_pin1, int leftmotor_pin2, int rightmotor_pin1, int rightmotor_pin2);
void set_speed(int speed, int leftmotor_pwm_pin, int rightmotor_pwm_pin);

/**
 * Initialize GPIO pins for motor control.
 *
 * @param leftmotor_pin1   GPIO pin for the left motor, forward control.
 * @param leftmotor_pin2   GPIO pin for the left motor, reverse control.
 * @param rightmotor_pin1  GPIO pin for the right motor, forward control.
 * @param rightmotor_pin2  GPIO pin for the right motor, reverse control.
 */
void start_motor(int leftmotor_pin1, int leftmotor_pin2, int rightmotor_pin1, int rightmotor_pin2)
{

    gpio_init(leftmotor_pin1);
    gpio_init(leftmotor_pin2);
    gpio_init(rightmotor_pin1);
    gpio_init(rightmotor_pin2);

    gpio_set_dir(leftmotor_pin1, GPIO_OUT);
    gpio_set_dir(leftmotor_pin2, GPIO_OUT);
    gpio_set_dir(rightmotor_pin1, GPIO_OUT);
    gpio_set_dir(rightmotor_pin2, GPIO_OUT);
}

/**
 * Initialize PWM for motor speed control.
 *
 * @param leftmotor_pwm_pin   GPIO pin for the left motor's PWM input.
 * @param rightmotor_pwm_pin  GPIO pin for the right motor's PWM input.
 */
void start_motor_pwm(int leftmotor_pwm_pin, int rightmotor_pwm_pin)
{

    // Tell GPIO 0 and 1 they are allocated to the PWM
    gpio_set_function(rightmotor_pwm_pin, GPIO_FUNC_PWM);
    gpio_set_function(leftmotor_pwm_pin, GPIO_FUNC_PWM);

    // Find out which PWM slice is connected to GPIO 0 (it's slice 0)
    uint slice_num_0 = pwm_gpio_to_slice_num(rightmotor_pwm_pin);

    // Find out which PWM slice is connected to GPIO 1 (it's slice 1)
    uint slice_num_1 = pwm_gpio_to_slice_num(leftmotor_pwm_pin);

    pwm_set_clkdiv(slice_num_0, 100);
    pwm_set_clkdiv(slice_num_1, 100);

    pwm_set_wrap(slice_num_0, 12500);
    pwm_set_wrap(slice_num_1, 12500);

    // Right Motor
    pwm_set_chan_level(slice_num_0, PWM_CHAN_A, 12500 / 2);

    // Left Motor
    pwm_set_chan_level(slice_num_1, PWM_CHAN_B, 12500 / 2);

    // Set the PWM running for both slices
    pwm_set_enabled(slice_num_0, true);
    pwm_set_enabled(slice_num_1, true);
    
}

/**
 * Move the robot forward.
 *
 * @param leftmotor_pin1   GPIO pin for the left motor, forward control.
 * @param leftmotor_pin2   GPIO pin for the left motor, reverse control.
 * @param rightmotor_pin1  GPIO pin for the right motor, forward control.
 * @param rightmotor_pin2  GPIO pin for the right motor, reverse control.
 */
void move_forward(int leftmotor_pin1, int leftmotor_pin2, int rightmotor_pin1, int rightmotor_pin2)
{
    gpio_put(leftmotor_pin1, 1);
    gpio_put(leftmotor_pin2, 0);
    gpio_put(rightmotor_pin1, 0);
    gpio_put(rightmotor_pin2, 1);
}

/**
 * Move the robot backward.
 *
 * @param leftmotor_pin1   GPIO pin for the left motor, forward control.
 * @param leftmotor_pin2   GPIO pin for the left motor, reverse control.
 * @param rightmotor_pin1  GPIO pin for the right motor, forward control.
 * @param rightmotor_pin2  GPIO pin for the right motor, reverse control.
 */
void move_backward(int leftmotor_pin1, int leftmotor_pin2, int rightmotor_pin1, int rightmotor_pin2)
{
    gpio_put(leftmotor_pin1, 0);
    gpio_put(leftmotor_pin2, 1);
    gpio_put(rightmotor_pin1, 1);
    gpio_put(rightmotor_pin2, 0);
}

/**
 * Turn the robot left.
 *
 * @param leftmotor_pin1   GPIO pin for the left motor, forward control.
 * @param leftmotor_pin2   GPIO pin for the left motor, reverse control.
 * @param rightmotor_pin1  GPIO pin for the right motor, forward control.
 * @param rightmotor_pin2  GPIO pin for the right motor, reverse control.
 */
void turn_left(int leftmotor_pin1, int leftmotor_pin2, int rightmotor_pin1, int rightmotor_pin2)
{
    gpio_put(leftmotor_pin1, 0);
    gpio_put(leftmotor_pin2, 1);
    gpio_put(rightmotor_pin1, 0);
    gpio_put(rightmotor_pin2, 1);
}

/**
 * Turn the robot right.
 *
 * @param leftmotor_pin1   GPIO pin for the left motor, forward control.
 * @param leftmotor_pin2   GPIO pin for the left motor, reverse control.
 * @param rightmotor_pin1  GPIO pin for the right motor, forward control.
 * @param rightmotor_pin2  GPIO pin for the right motor, reverse control.
 */
void turn_right(int leftmotor_pin1, int leftmotor_pin2, int rightmotor_pin1, int rightmotor_pin2)
{
    gpio_put(leftmotor_pin1, 1);
    gpio_put(leftmotor_pin2, 0);
    gpio_put(rightmotor_pin1, 1);
    gpio_put(rightmotor_pin2, 0);
}

/**
 * Set the motor speed as a percentage.
 *
 * @param speed              Speed percentage (0 to 100).
 * @param leftmotor_pwm_pin  GPIO pin for the left motor's PWM input.
 * @param rightmotor_pwm_pin GPIO pin for the right motor's PWM input.
 */
void set_speed(int speed, int leftmotor_pwm_pin, int rightmotor_pwm_pin)
{
    float duty_cycle = speed / 100.0;

    pwm_set_gpio_level(leftmotor_pwm_pin, 12500 * duty_cycle);
    pwm_set_gpio_level(rightmotor_pwm_pin, 12500 * duty_cycle);

    printf("Speed set to %d%%\n", speed);
}