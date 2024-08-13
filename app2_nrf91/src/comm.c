// This file implements the communication interface to the NRF52.

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>

#include "channels.h"

LOG_MODULE_REGISTER(comm);

// Handle to the UART device.
const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart0));

// Message queue for passing measurements to the comm thread.
K_MSGQ_DEFINE(comm_msgq, sizeof(struct sensor_msmt_message), CONFIG_COMM_MSGQ_SIZE, 4);

// Subscriber for sensor values.
static void on_new_measurement(const struct zbus_channel *chan);
ZBUS_LISTENER_DEFINE(comm_measurement_listener, on_new_measurement);
ZBUS_CHAN_ADD_OBS(sensor_channel, comm_measurement_listener, 1);

// Function to execute by the comm thread.
void comm_handler(void *_u1, void *_u2, void *_u3);

// Define the sensor thread and immediately start it.
K_THREAD_DEFINE(
    comm_thread,
    CONFIG_COMM_THREAD_STACK_SIZE,
    comm_handler, NULL, NULL, NULL,
    CONFIG_COMM_THREAD_PRIO, 0, 0);

// UART communication handler.
void comm_handler(void *_u1, void *_u2, void *_u3)
{
    ARG_UNUSED(_u1);
    ARG_UNUSED(_u2);
    ARG_UNUSED(_u3);

    if (!device_is_ready(uart)) {
        LOG_ERR("uart device is not ready");
        return;
    }

    while (true) {
        struct sensor_msmt_message current_message = {0};
        int err = k_msgq_get(&comm_msgq, &current_message, K_FOREVER);
        if (err != 0) {
            LOG_ERR("failed to read data from message queue: %d", err);
            continue;
        }

        // Now, send the frame over UART.
        // Since we are connected to another ARM Cortex-M controller we can just send the structure
        // as it is - without any special serialization.
        for (size_t i = 0; i < sizeof(current_message); i++) {
            // Simply write out all bytes in sequence.
            // Note: A more efficient way would be to use the asynchronous API using DMA transfers.
            uart_poll_out(uart, ((uint8_t *)&current_message)[i]);
        }

        LOG_INF("written message with timestamp %llu to serial line", current_message.uptime_ms);
    }
}

// Passes measurement structure to message queue.
static void on_new_measurement(const struct zbus_channel *chan)
{
    const struct sensor_msmt_message *msg = zbus_chan_const_msg(chan);

    int err = k_msgq_put(&comm_msgq, msg, K_NO_WAIT);
    if (err != 0) {
        LOG_ERR("failed to put message to message queue: %d", err);
    }
}