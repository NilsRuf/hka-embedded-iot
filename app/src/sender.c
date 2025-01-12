// This file implements the LoRa sending thread.

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/lora.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>

#include <channels.h>

LOG_MODULE_REGISTER(sender);

/* -------------------------------- Define a new handler thread. -------------------------------- */

// The actual thread function.
void sender_thread(void *_p1, void *_p2, void *_p3);

// Define the thread with its name, stack size, and entry point.
K_THREAD_DEFINE(
        m_sender_thread,
        CONFIG_SENDER_THREAD_STACK_SIZE,
        sender_thread, NULL, NULL, NULL,
        CONFIG_SENDER_THREAD_PRIO,
        0, 0);

/* --------------------------------- Define the sender channel. --------------------------------- */

// Define global zbus channel.
ZBUS_CHAN_DEFINE(g_sender_chan, struct sender_message, NULL, NULL, ZBUS_OBSERVERS_EMPTY, {0});

/* -------------------------------- Define a channel subscriber. -------------------------------- */

// Defines a message subscriber that gets all the messages from a channel.
ZBUS_MSG_SUBSCRIBER_DEFINE(m_send_sub);

// Add the new message subscriber to the list of channel observers for g_sender_chan.
ZBUS_CHAN_ADD_OBS(g_sender_chan, m_send_sub, 2);

/* ------------------------------------ LoRa configuration. ------------------------------------- */

// Configur LoRa module. This sets the physical layer parameters.
static int configure_lora(const struct device *lora);


/* ============================================================================================== */
/* ----------------------- Alternative for sending a trigger to a thread. ----------------------- */
/* ============================================================================================== */

/* K_SEM_DEFINE(m_msg_notifier, 0, 1); */

/* void trigger(void) { */
/*     // ... */
/*     k_sem_give(&m_msg_notifier); */
/* } */

/* void thread(void) { */
/*     while (true) { */
/*         int ret = k_sem_take(&m_msg_notifier, K_FOREVER); */
/*         // do something */
/*     } */
/* } */


/* ============================================================================================== */
/* ----------------------------------- Thread implementation. ----------------------------------- */
/* ============================================================================================== */

void sender_thread(void *_p1, void *_p2, void *_p3)
{
    ARG_UNUSED(_p1);
    ARG_UNUSED(_p2);
    ARG_UNUSED(_p3);

    LOG_INF("sender thread spawned");

    // First, we configure the LoRa module.
    const struct device *lora = DEVICE_DT_GET(DT_ALIAS(lora0));
    int err = 0;

    err = configure_lora(lora);
    if (err) {
        LOG_ERR("failed to configure lora: %d", err);
        return;
    }

    // Now, we loop forever and consume messages.
    while (true) {
        const struct zbus_channel *chan;
        struct sender_message msg = {0};
        err = zbus_sub_wait_msg(&m_send_sub, &chan, &msg, K_FOREVER);
        if (err) {
            LOG_ERR("failed to read msg: %d - aborting", err);
            return;
        }

        if (&g_sender_chan != chan) {
            // This must never happen!
            LOG_ERR("received from wrong channel %p", chan);
            continue;
        }

        // We validate the message type here.
        if (msg.type != SENDER_MSG_TYPE_PRESS_DURATION) {
            LOG_ERR("illegal sender message type");
            continue;
        }

        LOG_INF("button press was %ums", msg.press_duration_ms);

        // It would be better to statically allocate the message.
        char send_buffer[12];
        snprintk(send_buffer, sizeof(send_buffer), "{d:%u}", msg.press_duration_ms);

        err = lora_send(lora, send_buffer, strlen(send_buffer));
        if (err) {
            LOG_ERR("failed to send packet %s: %d", send_buffer, err);
        } else {
            LOG_INF("sent packet: %s", send_buffer);
        }
    }
}

/* ------------------------------------ LoRa configuration. ------------------------------------- */

static int configure_lora(const struct device *lora)
{
    if (!device_is_ready(lora)) {
        LOG_ERR("lora device not ready");
        return -ECANCELED;
    }

    struct lora_modem_config config = {
        .frequency = 865100000,
        .bandwidth = BW_125_KHZ,
        .datarate = SF_10,
        .coding_rate = CR_4_5,
        .iq_inverted = false,
        .public_network = false,
        .tx_power = 4,
        .tx = true,
    };

    return lora_config(lora, &config);
}
