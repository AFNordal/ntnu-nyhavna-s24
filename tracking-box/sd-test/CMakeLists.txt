cmake_minimum_required(VERSION 3.12)

include(/include/pico-sdk/external/pico_sdk_import.cmake)
project(sd_test C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include(/include/pico-sdk/pico_sdk_init.cmake)
pico_sdk_init()

add_executable(${PROJECT_NAME} 
    src/main.cpp
    # src/hw_config.cpp
)
# pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/pio/uart.pio)

add_subdirectory(/include/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src build)

pico_add_extra_outputs(${PROJECT_NAME})

add_compile_definitions(PICO_STDIO_USB_RESET_MAGIC_BAUD_RATE=1200)
target_link_libraries(${PROJECT_NAME} 
    pico_stdlib
    no-OS-FatFS-SD-SDIO-SPI-RPi-Pico
    # hardware_spi
    # hardware_i2c
    # hardware_pio
    # hardware_dma
    # hardware_clocks
)

# Enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)