# This file is used by the build system to find sources you want to compile.

# Some basic setup stuff. We won't go into great detail.
cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(NONE)

target_sources(app PRIVATE
    src/main.c
    src/env_sensor.c)

# Add everything under include/ to the include path.
zephyr_include_directories(include)

# Enable more warnings
target_compile_options(app PRIVATE -Wall -Wextra)
