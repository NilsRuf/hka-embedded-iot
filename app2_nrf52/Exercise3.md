# Exercise 3: Building a BLE device

This tasks builds upon the previous exercise.
The goal is now to create a BLE device which sends our measurements to a remote device.

> Note: To build the software for this board you have to call `west build -p -b thingy91_nrf52840`.

[Here](https://github.com/zephyrproject-rtos/zephyr/tree/main/samples/bluetooth/peripheral) is a Zephyr Bluetooth sample which might help.

[Here](https://docs.zephyrproject.org/latest/doxygen/html/group__zbus__apis.html) you can find the Zephyr zbus API.

## Task 1: Get the sensor values
Finish exercise 2 (or take the provided solution for the NRF91) and implement the NRF52 side of the UART connection.

## Task 2: Distribute the sensor values to the BLE module
First, uncomment the `src/ble.c` entry in `CMakeLists.txt`.

Take the incoming measurement values and use zbus channels again to send them to the BLE module in `ble.c`.
Remember to properly handle the incoming messages from `zbus_chan_pub()` inside a work queue or thread **on both sides**.

## Task 3: Finish the BLE module.
There are still some blanks in the BLE module.
Fill them in, the comments in there will guide you.
In the end, you should be able to find your device popping up in your Bluetooth scan list.

## Bonus Task 4:
There are two accelerometers on the Thingy91.
Use the ADXL362 together with the sensor trigger API to detect motion (hint `SENSOR_TRIG_MOTION`).
When motion is detected, trigger a new sensor sample.