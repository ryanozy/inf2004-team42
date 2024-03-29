bool line_check_left = false;
bool line_check_right = false;
uint32_t start_time_barcode_black = 0;
uint32_t stop_time_barcode_black = 0;
uint32_t start_time_barcode_white = 0;
uint32_t stop_time_barcode_white = 0;
uint32_t barcode_array[9] = {0};
uint32_t barcode_counter = 0;
uint32_t barcode_divider = 0;
bool barcode_started = false;
bool barcode_ended = false;
char barcode_string[27] = {0};

void infrared_sensor_init();
void measure_barcode(char color[]);
void barcode_sensor_handler(uint gpio, uint32_t events);
void line_sensor_handler(uint gpio, uint32_t events);
void get_line_sensor_value();
void decode_barcode();


// Define GPIO pins for infrared sensor
#define LEFT_LINE_SENSOR_PIN 20
#define RIGHT_LINE_SENSOR_PIN 21
#define BARCODE_SENSOR_PIN 22

#define TICKS_PER_MICROSECOND 1
#define TIMEOUT_MICROSECONDS 5000000 // 5 seconds timeout

#define BARCODE_A "111010100010111"
#define BARCODE_F "101110111000101"
#define BARCODE_ASTERISK "100010111011101"

void decode_barcode()
{

    // get_line_sensor_value();

    if (barcode_counter == 9)
    {
        // Convert barcode array to binary
        // Convert 2 to 0 and 222 to 000
        // No need to convert 1s

        for (int i = 0; i < 9; i++)
        {
            if (barcode_array[i] == 2)
            {
                strcat(barcode_string, "0");
            }
            else if (barcode_array[i] == 222)
            {
                strcat(barcode_string, "000");
            }
            else if (barcode_array[i] == 1)
            {
                strcat(barcode_string, "1");
            }
            else if (barcode_array[i] == 111)
            {
                strcat(barcode_string, "111");
            }
        }

        // Check if barcode is F
        if (strcmp(barcode_string, BARCODE_F) == 0)
        {
            printf("Barcode is F\n");
            printf("Barcode String: %s\n", barcode_string);

            // Clear barcode array
            memset(barcode_array, 0, sizeof(barcode_array));
            barcode_counter = 0;
        }
        else if (strcmp(barcode_string, BARCODE_A) == 0)
        {
            printf("Barcode is A\n");
            printf("Barcode String: %s\n", barcode_string);

            // Clear barcode array
            memset(barcode_array, 0, sizeof(barcode_array));
            barcode_counter = 0;
        }
        else if (strcmp(barcode_string, BARCODE_ASTERISK) == 0)
        {
            printf("Barcode is *\n");
            printf("Barcode String: %s\n", barcode_string);

            // Clear barcode array
            memset(barcode_array, 0, sizeof(barcode_array));
            barcode_counter = 0;
        }

        // Clear barcode string
        memset(barcode_string, 0, sizeof(barcode_string));
    }

    // Clear the terminal
    // printf("\033[2J");
}

/**
 * @brief Function to measure the barcode
 *
 * If barcode counter is 9, then the barcode is complete. Shift the array to the left to make space for the new barcode
 * If barcode counter is 0, then the barcode has not started yet. Set the divider to the first time difference
 * If barcode counter is not 0, then the barcode has started. Check if the time difference is greater than or equal to 2 times the divider
 * If it is, then the barcode is thick. Else, the barcode is thin
 */
void measure_barcode(char color[])
{
    if (barcode_counter == 9)
    {
        // Shift the array to the left
        for (int i = 0; i < 8; i++)
        {
            barcode_array[i] = barcode_array[i + 1];
        }
        barcode_counter--;
    }

    if (strcmp(color, "black") == 0)
    {

        uint32_t time_difference = stop_time_barcode_black - start_time_barcode_black;
        // printf("Time Difference: %d\n", time_difference);

        if (barcode_counter == 0)
        {
            // Set the divider to the first time difference
            // Use this divider to determine if the barcode is thick or thin
            barcode_divider = time_difference;
            // printf("Barcode Divider: %d\n", barcode_divider);
        }

        // printf("time_difference / barcode_divider: %d\n", time_difference / barcode_divider);

        if (time_difference / barcode_divider >= 2)
        {

            barcode_array[barcode_counter] = 111;
            printf("Thick Black Barcode\n");
            barcode_counter++;
        }
        else
        {
            barcode_array[barcode_counter] = 1;
            printf("Thin Black Barcode\n");
            barcode_counter++;
        }
    }
    else if (strcmp(color, "white") == 0)
    {
        uint32_t time_difference = stop_time_barcode_white - start_time_barcode_white;
        // printf("Time Difference: %d\n", time_difference);
        // printf("time_difference / barcode_divider: %d\n", time_difference / barcode_divider);

        if (barcode_counter == 0)
        {

            // Barcode has not started yet
        }
        else
        {
            if (time_difference / barcode_divider >= 2)
            {
                barcode_array[barcode_counter] = 222;
                printf("Thick White Barcode\n");
                barcode_counter++;
            }
            else
            {
                barcode_array[barcode_counter] = 2;
                printf("Thin White Barcode\n");
                barcode_counter++;
            }
        }
    }
}

/**
 * @brief Barcode Sensor Interrupt Handler
 *
 * If events is GPIO_IRQ_EDGE_RISE, then the barcode is black --> Measure the previous white barcode
 * If events is GPIO_IRQ_EDGE_FALL, then the barcode is white --> Measure the previous black barcode
 */
void barcode_sensor_handler(uint gpio, uint32_t events)
{
    if (events == GPIO_IRQ_EDGE_RISE && gpio == BARCODE_SENSOR_PIN)
    {
        start_time_barcode_black = time_us_32();
        stop_time_barcode_white = time_us_32();

        if (barcode_started == false)
        {
            barcode_started = true;
        }

        measure_barcode("white");
    }
    else if (events == GPIO_IRQ_EDGE_FALL && gpio == BARCODE_SENSOR_PIN)
    {
        start_time_barcode_white = time_us_32();
        stop_time_barcode_black = time_us_32();

        measure_barcode("black");
    }
}

/**
 * @brief Line Sensor Interrupt Handler
 *
 * If events is GPIO_IRQ_EDGE_RISE, then the line is black
 * If events is GPIO_IRQ_EDGE_FALL, then the line is white
 * Update the line_check_left and line_check_right variables
 */
void line_sensor_handler(uint gpio, uint32_t events)
{
    if (events == GPIO_IRQ_EDGE_RISE && gpio == RIGHT_LINE_SENSOR_PIN)
    {
        line_check_right = true;
    }
    else if (events == GPIO_IRQ_EDGE_RISE && gpio == LEFT_LINE_SENSOR_PIN)
    {
        line_check_left = true;
    }
    else if (events == GPIO_IRQ_EDGE_FALL && gpio == RIGHT_LINE_SENSOR_PIN)
    {
        line_check_right = false;
    }
    else if (events == GPIO_IRQ_EDGE_FALL && gpio == LEFT_LINE_SENSOR_PIN)
    {
        line_check_left = false;
    }
}

/**
 * @brief Get the line sensor value
 *
 * Print the line sensor value
 */
void get_line_sensor_value()
{
    printf("Left Line Sensor: %s\n", line_check_left ? "true" : "false");
    printf("Right Line Sensor: %s\n", line_check_right ? "true" : "false");
}

/**
 * @brief Function to initialize the infrared sensors
 *
 * Initialize the GPIO pins for the infrared sensors
 */
void infrared_sensor_init()
{
    // Initialize GPIO pins
    gpio_init(LEFT_LINE_SENSOR_PIN);
    gpio_init(RIGHT_LINE_SENSOR_PIN);
    gpio_init(BARCODE_SENSOR_PIN);

    // Set GPIO pins to pull down
    gpio_set_dir(LEFT_LINE_SENSOR_PIN, GPIO_IN);
    gpio_set_dir(RIGHT_LINE_SENSOR_PIN, GPIO_IN);
    gpio_set_dir(BARCODE_SENSOR_PIN, GPIO_IN);
}