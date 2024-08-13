// This header contains channel and structure definitions used by all the modules accessing zbus.

#ifndef _CHANNEL_H_
#define _CHANNEL_H_

#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/drivers/sensor.h>

// Message containing new measurement values.
// Should be distributed via sensor_channel.
struct sensor_msmt_message {
    int64_t uptime_ms; // System in milliseconds.
    struct sensor_value temperature; // Ambient temperature.
    struct sensor_value pressure; // Ambient pressure.
    struct sensor_value humidity; // Ambient humidity.
    struct sensor_value gas_res; // Gas sensor resistance.
};

// Declares a global sensor channel.
// This does not define the channel! Definition needs to happen in one of the modules.
ZBUS_CHAN_DECLARE(sensor_channel);


// Defines all possible IO request.
enum io_request {
    IO_REQUEST_TRIGGER_MSMT, // Forces new measurement to be taken.
};

// Request message sent by IO modules.
struct io_request_msg {
    enum io_request request; // Request kind.
};

// Channel for sending IO requests.
ZBUS_CHAN_DECLARE(io_channel);

#endif