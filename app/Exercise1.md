# Exercise 1: Getting to know Zephyr

After we have completed the getting started guide, do the following tasks.

## Task 1: Make it colorful
The Thingy91 actually has a three-colored LED.
Also make the other LEDs blink, each with a different period.

## Task 2: Add timers
Currently, our application is spending a lot of time idling around which prevents us from doing other tasks in the meantime.
Therefore, use [Zephyr's timer API](https://docs.zephyrproject.org/latest/kernel/services/timing/timers.html) to handle the blinking LEDs in an event-based manner.
You may use multiple timers.

## Task 3: Add some input
The Thingy91 also has a button.
Look up the devicetree of the board to find which one it is.

Configure the button to trigger an interrupt when pressed. You may find useful resources on how to do that [here](https://docs.zephyrproject.org/latest/hardware/peripherals/gpio.html).
When the button is pressed, all LEDs should stop blinking.
When pressed again, the original blinking pattern should start.

## Task 4: Using the color sensors
The Thingy91 also has color sensors (three of them for each color).
Find out how to control them and take measurements in specific intervals.
Log the measurements with the logging API.

Hint: The color sensors are also controlled using GPIOs.