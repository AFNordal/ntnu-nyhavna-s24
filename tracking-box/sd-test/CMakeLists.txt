cmake_minimum_required(VERSION 3.12)

include(/include/pico-sdk/external/pico_sdk_import.cmake)
project(sd_test C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include(/include/pico-sdk/pico_sdk_init.cmake)
pico_sdk_init()


add_executable(${PROJECT_NAME} 
    # src/bmi270.hpp
    # inc/bmi270_config.h
    src/hw_config.cpp
    src/bmi270.cpp
    src/main.cpp
    src/battery.cpp
    src/sd_utils.cpp
    )

target_include_directories(${PROJECT_NAME}
    PUBLIC
    inc
)
# pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/pio/uart.pio)

add_subdirectory(/include/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src build)

pico_add_extra_outputs(${PROJECT_NAME})
    
add_compile_definitions(PICO_STDIO_USB_RESET_MAGIC_BAUD_RATE=1200)
target_link_libraries(${PROJECT_NAME} 
    pico_stdlib
    pico_multicore
    no-OS-FatFS-SD-SDIO-SPI-RPi-Pico
    hardware_i2c
    hardware_gpio
    hardware_adc
    hardware_irq
)

# Enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
