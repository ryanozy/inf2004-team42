add_executable(motor motor_control.c motor.h)

target_link_libraries(motor pico_stdlib hardware_pwm hardware_gpio)

# enable usb output 
pico_enable_stdio_usb(motor 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(motor)

# add url via pico_set_program_url
example_auto_set_url(motor)