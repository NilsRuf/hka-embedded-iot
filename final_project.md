# Final Project

The goal of the final project is to create a sensor device that perioically sends out measurement values via LoRa.
It is built based on what we have covered in the practice sessions in the lab. This code can be reused.

First, implement the **mandatory requirements**.
Then, choose **at least one** optional requirement.

Submit your code before Sunday, 19.01.2025 17:00.

## Mandatory requirements

* Code quality
    * The code has to compile without warnings from **your** code (Zephyr warnings not originating from your code don't matter).
    * Every return value needs to be checked for errors or casted to void with a comment explaining why this is okay.
    * Every function in a header file must contain descriptions for each parameter and the return value.
    * General coding best practices (split your code into functions, sensible variable naming, a few comments, etc.) should be applied.
* Architecture
    * Focus on a clean design with your code structured in modules (different C files) and different threads (where it makes sense).
    * Design a proper way of communication between the threads (e.g. message bus or something else)
* Logging
    * All system events (measurements, config changes, sending data, etc. must be logged to the console).
    * All errors have to be logged at least once (either with a warning or error log - depending on the severity).
* Measurements
    * The device has a standard measurement interval of 10 seconds. This value must be configurable via a Kconfig option.
    * The device has a _fast mode_ in which triggers a measurement every 5 seconds. It is activated by pressing the button and deactivated by pressing the button again.
    * Extend the `env_sensor` module with the magnetometer readings for the X and Y axes (hint: sensor channels `SENSOR_CHAN_MAGN_X` and `SENSOR_CHAN_MAGN_Y` are required)
        * To do this you can extend the API in a similar way to the climate sensor structs and functions.
    * Let the LED blink for 250ms after each measurement.
* Sending measurements via LoRa
    * Periodically send the measurement values (ambient temperature, humidity, and the two magnetometer readings) via LoRa.
        * You can use the integral part of the humidity and only one decimal digit for the ambient temperature. Use three digits for the magnetometer readings.
    * Use the example string for formatting: `{i:N,t:25.6,h:53,x:0.873,y:1.247}` where `i` denotes your ID (first char of your first name).
    * If a specific sensor value could not be retrieved, send e.g. `y:?` in this case.
    * Let the LED blink for one second.
    * Log the string you send!
* Documentation
    * Create a markdown file under `docs` explaining your architecture (e.g. which modules exist, what they do, how they interact).
    * Add a figure showing the different modules/threads and their interaction.
    * Keep it short - just the essentials ;)

## Optional requirements

* Compass
    * Use the magnetometer readings of the X and Y axes to calculate the direction. It is easier than it sounds, I promise.
    * Replace the `x` and `y` values in your LoRa packets with `d:SW`, etc. for the direction (here south-west).
* Shell input
    * Make the update interval configurable via the serial terminal. You can use [Zephyr's shell subsystem](https://docs.zephyrproject.org/latest/services/shell/index.html) for this or simply read the commands on your own.
    * Support two commands: `slow <x>` and `fast <x>` setting the respective update interval to `<x>` seconds respectively.
    * Always check that the fast update interval is faster than the slow interval.
    * **Validate your input!**
* Persist the current mesurement interval using [Zephyr's settings](https://docs.zephyrproject.org/latest/services/settings/index.html) subsystem.
* Implement your own idea!
    * If you have an own idea which is cool, implement and document it.
