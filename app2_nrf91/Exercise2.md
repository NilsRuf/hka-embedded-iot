# Exercise 2: Build a message-based system

The goal of this session is to read environmental sensor values and distribute them to different modules and chips.

> Note: Do not forget to enable the different modules in `CMakeLists.txt`!

## Task 1: Get the measurement
The Thingy91 has a gas and humidity sensor (BME360) which is controlled in `src/env_sensors.c`.
Get periodic measurements of this sensor (the interval is given in `prj.conf`) and log them.

Since we don't want to do this neither in the main thread context nor in an interrupt context we will create a thread for this. Do not use the system workqueue.

## Task 2: Add some messages
Familiarize yourself with the [Zephyr message bus (zbus)](https://docs.zephyrproject.org/latest/services/zbus/index.html), which is used for message passing between decoupled modules.

Create a channel in `src/channel.c` and `src/channel.h` and publish the measurement values.


## Task 3: Read the sensor values
In the IO module (`src/io.c`) reads the incoming measurement values by defining a listener on the measurement channel from the previous exercise.

Transfer the message out of the listener context (we do not want to populate our measurement thread) into either a workqueue item or a dedicated status indicator thread.

Indicate the temperature using the RGB led.
Values below 20 degrees celsius are good (green).
Between 20 and 25 degrees it is starting to get annoying (yellow).
Everything above 25 degrees is close to unbearable (red).

## Task 4: Get the measurement on demand
An impatient person might want to get the sensor values immediately instead of waiting ages for the next measurement.
When the button is pressed, a new sensor value should be read.
This should also reschedule the timer for the next measurement.

This task should be implemented by the IO module.
Define another zbus channel that transports a notification of this button press to the sensor module.
The sensor module then should take a new measurement.

# Task 5 Send the sensor values to the NRF52
In `src/comm.c` create another thread and a listener on the measurement channel.
In this new thread take the incoming measurement values and send them over UART to the NRF52 nearby in whatever format you prefer.
