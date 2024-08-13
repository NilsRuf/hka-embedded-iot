// This file includes the sensor handler.

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#include "channels.h"

LOG_MODULE_REGISTER(env_sensor);

// Handle to the sensor device.
const struct device *sensor = DEVICE_DT_GET_ONE(bosch_bme680);

// Function to execute by the sensor thread.
void sensor_handler(void *_u1, void *_u2, void *_u3);

// Define the sensor thread and immediately start it.
K_THREAD_DEFINE(
    env_sens_thread,
    CONFIG_SENSOR_THREAD_STACK_SIZE,
    sensor_handler, NULL, NULL, NULL,
    CONFIG_SENSOR_THREAD_PRIO, 0, 0);

// Sensor handler takes periodic measurements.
void sensor_handler(void *_u1, void *_u2, void *_u3)
{
    ARG_UNUSED(_u1);
    ARG_UNUSED(_u2);
    ARG_UNUSED(_u3);

    if (!device_is_ready(sensor)) {
        LOG_ERR("sensor not ready - aborting");
        return;
    }

    while (true) {
        // TODO: Take periodic measurements of the sensor every
        // CONFIG_SENSOR_UPDATE_INTERVAL_MS milliseconds.

        // Sensor values of each channel.
        // TODO: Also sample pressure, humidity and gas resolution.
        struct sensor_value temperature;
        sensor_sample_fetch(sensor);
        sensor_channel_get(sensor, SENSOR_CHAN_AMBIENT_TEMP, &temperature);

        // TODO: Publish the measurement to a channel.
    }
}