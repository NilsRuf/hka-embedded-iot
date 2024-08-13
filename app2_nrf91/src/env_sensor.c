// This file includes the sensor handler.

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#include "channels.h"

LOG_MODULE_REGISTER(env_sensor);

// Define the zbus channel
ZBUS_CHAN_DEFINE(
    sensor_channel, struct sensor_msmt_message,
    NULL, NULL, ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(0));

// Handle to the sensor device.
const struct device *sensor = DEVICE_DT_GET_ONE(bosch_bme680);

// Function to execute by the sensor thread.
static void sensor_handler(void *_u1, void *_u2, void *_u3);

// Binary semaphore to trigger measurements.
K_SEM_DEFINE(trigger_measurement, 0, 1);

// Timer which triggers the semaphore periodically to trigger new measurements.
static void on_measurement_due(struct k_timer *timer);
K_TIMER_DEFINE(measurement_trigger, on_measurement_due, NULL);

// Listener for the button press.
static void on_button_trigger(const struct zbus_channel *chan);
ZBUS_LISTENER_DEFINE(button_trigger_listener, on_button_trigger);
ZBUS_CHAN_ADD_OBS(io_channel, button_trigger_listener, 1);

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

    // Schedule the timer.
    k_timer_start(&measurement_trigger, K_NO_WAIT, K_MSEC(CONFIG_SENSOR_UPDATE_INTERVAL_MS));

    while (true) {
        // Wait until we need to sample a new measurement.
        k_sem_take(&trigger_measurement, K_FOREVER);

        // Sensor values of each channel.
        struct sensor_msmt_message meas_msg = {0};
        meas_msg.uptime_ms = k_uptime_get();
    
        int err = sensor_sample_fetch(sensor);
        if (err != 0) {
            LOG_ERR("failed to read sensor value: %d", err);
            continue;
        }
        
        // These calls work once the sample has been fetched.
        (void)sensor_channel_get(sensor, SENSOR_CHAN_AMBIENT_TEMP, &meas_msg.temperature);
        (void)sensor_channel_get(sensor, SENSOR_CHAN_PRESS, &meas_msg.pressure);
        (void)sensor_channel_get(sensor, SENSOR_CHAN_HUMIDITY, &meas_msg.humidity);
        (void)sensor_channel_get(sensor, SENSOR_CHAN_GAS_RES, &meas_msg.gas_res);

        LOG_INF("fetched new sensor sample @ %llums", meas_msg.uptime_ms);

        // Publish the measurement to a channel.
        err = zbus_chan_pub(&sensor_channel, &meas_msg, K_NO_WAIT);
        if (err != 0) {
            LOG_ERR("failed to publish measurement to channel: %d", err);
        }
    }
}

// This function simply triggers the measurement.
static void on_measurement_due(struct k_timer *timer)
{
    ARG_UNUSED(timer);

    k_sem_give(&trigger_measurement);
}

static void on_button_trigger(const struct zbus_channel *chan)
{
    const struct io_request_msg *msg = zbus_chan_const_msg(chan);

    switch (msg->request) {
        case IO_REQUEST_TRIGGER_MSMT:
            // We simply start the timer again and trigger it immediately.
            k_timer_start(
                &measurement_trigger, K_NO_WAIT, K_MSEC(CONFIG_SENSOR_UPDATE_INTERVAL_MS));
            break;
        default:
            break;
    }
}