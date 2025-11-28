# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(blink_basic)

target_sources(app PRIVATE
    src/main.c
    src/sensors/rgb_sensor.c
    src/sensors/humidity_sensor.c
    src/sensors/accelerometer_sensor.c
    src/sensors/light_sensor.c
    src/sensors/soil_sensor.c
    src/sensors/gps_sensor.c
    src/sensors/leds.c
    src/sensors/led1.c
    src/helpers/plantcare_state.c
    src/helpers/plantcare_config.c
    src/helpers/sensor_thread.c
    src/helpers/plantcare_mode_test.c
    src/helpers/plantcare_units.c
    src/sensors/button.c
    src/helpers/plantcare_mode_normal.c
)

target_include_directories(app PRIVATE)

target_include_directories(app PRIVATE
    src
    src/helpers
)