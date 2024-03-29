#define ENCODEROUT_PIN 2
#define ENCODEROUT_PIN2 3
#define DISTANCE_BETWEEN_NOTCHES_CM 1.0525
#define DEGREES_PER_NOTCH 4.5

// PID Constants
#define KP 0.1
#define KI 10
#define KD 1

double integral_left = 0;
double prev_error_left = 0;
double derivative_left = 0;

double integral_right = 0;
double prev_error_right = 0;
double derivative_right = 0;

int left_desired_speed_cm_s = 30;
int right_desired_speed_cm_s = 30;

bool turning_left = false;
bool turning_right = false;

float degree_turned = 0;
float degree_to_turn = 0;
bool inching = false;
double distance_after_blackline[2] = {0, 0};
float distance_to_inch = 0;

uint32_t time_of_prev_notch[2] = {0, 0};
__long_double_t distance[2] = {0, 0};
double speed[2] = {0, 0};
__long_double_t prev_distance[2] = {0, 0};

void interrupt_handler(uint gpio, uint32_t events);
void start_motor(int leftmotor_pin1, int leftmotor_pin2, int rightmotor_pin1, int rightmotor_pin2);
void start_motor_pwm(int leftmotor_pwm_pin, int rightmotor_pwm_pin);
void stop_motor(int leftmotor_pin1, int leftmotor_pin2, int rightmotor_pin1, int rightmotor_pin2);
void move_forward(int leftmotor_pin1, int leftmotor_pin2, int rightmotor_pin1, int rightmotor_pin2);
void move_backward(int leftmotor_pin1, int leftmotor_pin2, int rightmotor_pin1, int rightmotor_pin2);
void turn_left(int leftmotor_pin1, int leftmotor_pin2, int rightmotor_pin1, int rightmotor_pin2, float degree);
void turn_right(int leftmotor_pin1, int leftmotor_pin2, int rightmotor_pin1, int rightmotor_pin2, float degree);
void set_speed(int speed, int leftmotor_pwm_pin, int rightmotor_pwm_pin);
void calculate_speed(uint32_t time_of_notch, int encoder_number);
void gpio_callback(uint gpio, uint32_t events);
bool check_wheel_moving(struct repeating_timer *t);
bool pid(struct repeating_timer *t);
void init_encoder(int encoder_pin, int encoder_pin2);

/**
 * @brief PID function
 *
 * This function is called every 50ms to calculate the PID values for the left and right motors.
 * The PID values are then used to set the speed of the motors.
 * This function ensures motors are moving at the desired speed.
 */
bool pid(struct repeating_timer *t)
{
    double error_left = left_desired_speed_cm_s - speed[0];
    integral_left += error_left;
    derivative_left = error_left - prev_error_left;
    prev_error_left = error_left;

    double error_right = right_desired_speed_cm_s - speed[1];
    integral_right += error_right;
    derivative_right = error_right - prev_error_right;
    prev_error_right = error_right;

    double left_motor_speed = KP * error_left + KI * integral_left + KD * derivative_left;
    double right_motor_speed = KP * error_right + KI * integral_right + KD * derivative_right;

    pwm_set_gpio_level(15, left_motor_speed);
    pwm_set_gpio_level(10, right_motor_speed);

    // printf("Left Motor Speed: %lfcm/s\nRight Motor Speed: %lfcm/s\n", left_motor_speed, right_motor_speed);

    return true;
}

/**
 * @brief Main function to calculate speed of wheels.
 *
 * This function is called on a interrupt when the encoder notch is detected. (Rising Edge)
 * It calculates the speed of the wheel and distance travelled, updating the global variables.
 */
void calculate_speed(uint32_t time_of_notch, int encoder_number)
{
    // If Encoder Pin is 2, index is 0
    // If Encoder Pin is 3, index is 1
    int index = encoder_number == ENCODEROUT_PIN ? 0 : 1;

    if (turning_left == true)
    {
        degree_turned += DEGREES_PER_NOTCH;
        if (degree_turned >= degree_to_turn)
        {
            turning_left = false;
            degree_turned = 0;
            stop_motor(14, 13, 12, 11);
            printf("Turned left by %d degrees\n", degree_to_turn);
        }
    }
    else if (turning_right == true)
    {
        degree_turned += DEGREES_PER_NOTCH;
        if (degree_turned >= degree_to_turn)
        {
            turning_right = false;
            degree_turned = 0;
            stop_motor(14, 13, 12, 11);
            printf("Turned right by %d degrees\n", degree_to_turn);
        }
    }

    if (inching == true)
    {

        distance_after_blackline[index] += DISTANCE_BETWEEN_NOTCHES_CM;

        if (distance_after_blackline[index] >= distance_to_inch)
        {
            inching = false;
            distance_after_blackline[1] = 0;
            distance_after_blackline[0] = 0;
            stop_motor(14, 13, 12, 11);
            turn_left(14, 13, 12, 11, 50.0);
        }
    }
    else
    {

        if (time_of_prev_notch[index] == 0)
        {
            time_of_prev_notch[index] = time_of_notch;
        }
        else
        {
            double time_between_notches_s = (time_of_notch - time_of_prev_notch[index]) / 1000000.0;
            speed[index] = (DISTANCE_BETWEEN_NOTCHES_CM) / time_between_notches_s;
            distance[index] += DISTANCE_BETWEEN_NOTCHES_CM;

            time_of_prev_notch[index] = time_of_notch;
        }
    }
}

/**
 * @brief Callback function for GPIO interrupts.
 *
 * Called when the encoder notch is detected. (Rising Edge)
 * Calls calculate_speed() to calculate the speed of the wheel and pass the time of the notch.
 */
void gpio_callback(uint gpio, uint32_t events)
{
    if ((gpio == ENCODEROUT_PIN || gpio == ENCODEROUT_PIN2) && events == GPIO_IRQ_EDGE_RISE)
    {
        calculate_speed(time_us_32(), gpio);
    }
}

/**
 * @brief Check if wheel is moving.
 *
 * This function is called every 50ms to check if the wheel is moving.
 * If the wheel is not moving, the speed is set to 0.
 *
 */
bool check_wheel_moving(struct repeating_timer *t)
{
    // printf("Left Motor Speed: %lfcm/s\nRight Motor Speed: %lfcm/s\n", speed[0], speed[1]);
    // printf("Left Motor Distance: %lfcm\nRight Motor Distance: %lfcm\n", distance[0], distance[1]);

    // Check if distance not changed
    if (prev_distance[0] == distance[0])
    {
        speed[0] = 0.0;
    }

    if (prev_distance[1] == distance[1])
    {
        speed[1] = 0.0;
    }

    prev_distance[0] = distance[0];
    prev_distance[1] = distance[1];

    return true;
}

/**
 * @brief Initialize GPIO pins for motor control.
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
 * @brief Initialize PWM for motor speed control.
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
    pwm_set_chan_level(slice_num_0, PWM_CHAN_A, 12500 / 3.5);

    // Left Motor
    pwm_set_chan_level(slice_num_1, PWM_CHAN_B, 12500 / 3);

    // Set the PWM running for both slices
    pwm_set_enabled(slice_num_0, true);
    pwm_set_enabled(slice_num_1, true);
}

/**
 * @brief Stop the motors.
 *
 * This function is called to stop the motors.
 * It sets all the GPIO pins to 0.
 */
void stop_motor(int leftmotor_pin1, int leftmotor_pin2, int rightmotor_pin1, int rightmotor_pin2)
{
    gpio_put(leftmotor_pin1, 0);
    gpio_put(leftmotor_pin2, 0);
    gpio_put(rightmotor_pin1, 0);
    gpio_put(rightmotor_pin2, 0);
}

/**
 * @brief Move the robot forward.
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
 * @brief Move the robot backward.
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
 * @brief Turn the robot left.
 *
 * @param leftmotor_pin1   GPIO pin for the left motor, forward control.
 * @param leftmotor_pin2   GPIO pin for the left motor, reverse control.
 * @param rightmotor_pin1  GPIO pin for the right motor, forward control.
 * @param rightmotor_pin2  GPIO pin for the right motor, reverse control.
 */
void turn_left(int leftmotor_pin1, int leftmotor_pin2, int rightmotor_pin1, int rightmotor_pin2, float degree)
{
    gpio_put(leftmotor_pin1, 0);
    gpio_put(leftmotor_pin2, 1);
    gpio_put(rightmotor_pin1, 0);
    gpio_put(rightmotor_pin2, 1);
    turning_left = true;
    degree_to_turn = degree;
}

/**
 * @brief Turn the robot right.d
 *
 * @param leftmotor_pin1   GPIO pin for the left motor, forward control.
 * @param leftmotor_pin2   GPIO pin for the left motor, reverse control.
 * @param rightmotor_pin1  GPIO pin for the right motor, forward control.
 * @param rightmotor_pin2  GPIO pin for the right motor, reverse control.
 */
void turn_right(int leftmotor_pin1, int leftmotor_pin2, int rightmotor_pin1, int rightmotor_pin2, float degree)
{
    gpio_put(leftmotor_pin1, 1);
    gpio_put(leftmotor_pin2, 0);
    gpio_put(rightmotor_pin1, 1);
    gpio_put(rightmotor_pin2, 0);
    turning_right = true;
    degree_to_turn = degree;
}

void move_forward_by_distance(int leftmotor_pin1, int leftmotor_pin2, int rightmotor_pin1, int rightmotor_pin2, float distance_cm)
{
    inching = true;
    distance_after_blackline[0] = 0;
    distance_after_blackline[1] = 0;
    distance_to_inch = distance_cm;
    move_forward(leftmotor_pin1, leftmotor_pin2, rightmotor_pin1, rightmotor_pin2);
}

/**
 * @brief Set the motor speed as a percentage.
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

/**
 * @brief Initialize the encoder pins.
 *
 * This function is called in main() to initialize the encoder pins.
 * It sets the GPIO pins as inputs and enables interrupts on rising edge.
 */
void init_encoder(int encoder_pin, int encoder_pin2)
{
    gpio_init(encoder_pin);
    gpio_init(encoder_pin2);

    gpio_set_dir(encoder_pin, GPIO_IN);
    gpio_set_dir(encoder_pin2, GPIO_IN);

    gpio_pull_up(encoder_pin);
    gpio_pull_up(encoder_pin2);

    gpio_set_irq_enabled_with_callback(encoder_pin, GPIO_IRQ_EDGE_RISE, true, &interrupt_handler);
    gpio_set_irq_enabled_with_callback(encoder_pin2, GPIO_IRQ_EDGE_RISE, true, &interrupt_handler);
}
