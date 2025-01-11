# Exercise 1: Getting to know Zephyr

After we have completed the getting started guide, do the following tasks.

## Task 1: Add timers
Our application turns a light on and off.
Currently, it is spending a lot of time idling around which prevents us from doing other tasks in the meantime.
Therefore, use [Zephyr's timer API](https://docs.zephyrproject.org/latest/kernel/services/timing/timers.html) to handle the blinking the LED in an event-based manner.

## Task 2: Add some input
The devkit also has a button.
Look up the devicetree of the board to find which one it is.

Configure the button to trigger an interrupt when pressed. You may find useful resources on how to do that [here](https://docs.zephyrproject.org/latest/hardware/peripherals/gpio.html).
When the button is pressed, the LED should stop blinking.
When pressed again, the original blinking pattern should start.

## Task 3: Multiple blinking patterns
Define different blinking patterns depending on whether the button was pressed for a short or long period of time.
You need to edge-trigger the GPIO pin.
Do you encounter any weird behavior? If so, why is that and how can you fix it?

## Task 4: Reading some sensors
Since you moved the blinking timers out of the main loop we can find something useful to do there.
What about periodically reading the environment climate sensor in `env_sensor.h`?

## Task 5: Periodic scheduling
Make the sensor reading task periodic as well by using Kconfig and the [system workqueue](https://docs.zephyrproject.org/latest/kernel/services/threads/workqueue.html#system-workqueue) of Zephyr.
