cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(hx_lcd_matriz C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# Add main.c as the only source file
add_executable(${PROJECT_NAME}
    hx_lcd_matriz.c
)

# Add include directory for the HX711 library headers
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
)

# Add all source files from library (assuming they're in src directory)
target_sources(${PROJECT_NAME} PRIVATE
    src/common.c
    src/hx711.c
    src/hx711_multi.c
    src/util.c
    src/lcd_i2c.c
    src/user_management.c
    src/keypad.c
)

# Add required libraries
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    hardware_pio
    hardware_dma
    hardware_gpio
    hardware_i2c
)

# enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${PROJECT_NAME})