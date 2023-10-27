add_executable(pid_motor pid_motor.c motor.h)

target_link_libraries(pid_motor pico_stdlib hardware_timer hardware_pwm hardware_gpio)

#enable usb output
pico_enable_stdio_usb(pid_motor 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(pid_motor)

# add url via pico_set_program_url
example_auto_set_url(pid_motor)