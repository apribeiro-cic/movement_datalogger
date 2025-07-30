
cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PICO_BOARD pico_w CACHE STRING "Board type")
include(pico_sdk_import.cmake)
project(movement_datalogger C CXX ASM)
pico_sdk_init()

include_directories( ${CMAKE_SOURCE_DIR}/lib ) # Inclui os files .h na pasta lib

add_subdirectory(lib/FatFs_SPI)

add_executable(${PROJECT_NAME}  
        movement_datalogger.c
        hw_config.c
        lib/sd_card.c
        lib/ssd1306.c
        lib/mpu6050.c
        )

target_include_directories(movement_datalogger PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/lib
)

target_link_libraries(${PROJECT_NAME} 
        pico_stdlib 
        FatFs_SPI
        hardware_rtc
        hardware_clocks
        hardware_adc
        hardware_i2c
        hardware_gpio
        hardware_pio
        hardware_pwm
        )

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

pico_add_extra_outputs(${PROJECT_NAME})
