set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.13...3.27)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(clock)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(clock
    clock.c
)

pico_enable_stdio_usb(clock 1)
pico_enable_stdio_uart(clock 0)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(clock pico_stdlib pico_multicore hardware_adc)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(clock)

