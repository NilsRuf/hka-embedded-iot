// This file implements the communication interface to the NRF52.

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>

#include "channels.h"

LOG_MODULE_REGISTER(comm);

// Handle to the UART device.
const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart0));

// Function to execute by the comm thread.
void comm_handler(void *_u1, void *_u2, void *_u3);

// Define the sensor thread and immediately start it.
K_THREAD_DEFINE(
    comm_thread,
    CONFIG_COMM_THREAD_STACK_SIZE,
    comm_handler, NULL, NULL, NULL,
    CONFIG_COMM_THREAD_PRIO, 0, 0);

// TODO: Use the Zephyr UART API (does not matter which one) to send data.