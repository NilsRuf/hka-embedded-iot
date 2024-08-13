// This header contains channel and structure definitions used by all the modules accessing zbus.

#ifndef _CHANNEL_H_
#define _CHANNEL_H_

#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/drivers/sensor.h>

// Message containing new measurement values.
// Should be distributed via sensor_channel.
struct sensor_msmt_message {
    // TODO: fill in the blanks
};

// Declares a global sensor channel.
// This does not define the channel! Definition needs to happen in one of the modules.
// Add listeners with ZBUS_LISTENER_DEFINE and ZBUS_ADD_OBS wherever you need them.
ZBUS_CHAN_DECLARE(sensor_channel);

// TOOD: Further channels and structures here.

#endif