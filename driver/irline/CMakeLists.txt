if (TARGET tinyusb_device)
add_executable(irline
        infrared.c infrared.h
        )

target_link_libraries(irline pico_stdlib hardware_adc hardware_pwm)

# create map/bin/hex file etc.
pico_add_extra_outputs(irline)

# add url via pico_set_program_url
example_auto_set_url(irline)

 # enable usb output, disable uart output
 pico_enable_stdio_usb(irline 1)
 pico_enable_stdio_uart(irline 0)

 # create map/bin/hex/uf2 file etc.
 pico_add_extra_outputs(irline)

     # add url via pico_set_program_url
     example_auto_set_url(irline)
     elseif(PICO_ON_DEVICE)
         message(WARNING "not building hello_usb because TinyUSB submodule is not initialized in the SDK")
     endif()
     


