// Defines all channels for module communication.

#ifndef _CHANNELS_H_
#define _CHANNELS_H_

#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>

/// This enum defines the message type for outgoing messages.
enum sender_message_type {
    SENDER_MSG_TYPE_INVALID = 0,    ///< Type is invalid (uninitialized message).
    SENDER_MSG_TYPE_PRESS_DURATION, ///< Button press duration message.
};

/// Defines a message with contents to be sent.
struct sender_message {
    enum sender_message_type type; ///< Message type.

    // This union holds the contents of different message types.
    union {
        uint32_t press_duration_ms; ///< Button press duration in milliseconds.
    };
};

/// Declares the sender channel.
/// This channel is defined in sender.c and made available to everyone who wants to send something.
ZBUS_CHAN_DECLARE(g_sender_chan);

#endif
