# This file is used by the build system to find sources you want to compile.

# Some basic setup stuff. We won't go into great detail.
cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(NONE)

target_sources(app PRIVATE
    src/main.c
    # src/channels.c
    # src/comm.c
    src/env_sensor.c
    # src/io.c
)

# Enable more warnings
target_compile_options(app PRIVATE -Wall -Wextra)