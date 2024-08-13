// This module implements status indicators (LEDs) and buttons for the measurements.

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>

#include "channels.h"

LOG_MODULE_REGISTER(io);

// This reads the GPIO settings of the LEDs at compile time.
static const struct gpio_dt_spec red_led_spec = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green_led_spec = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec blue_led_spec = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);

// Those are settings for the button.
static const struct gpio_dt_spec button_spec = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static struct gpio_callback button_callback;

// Handler for the button press.
static void on_button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins);

// Configuration function for LEDs.
static int configure_led(const struct gpio_dt_spec *spec)
{
    // First, we check whether the GPIO device is ready.
    if (!gpio_is_ready_dt(spec)) {
        LOG_ERR("gpio device not ready");
        return -ECANCELED; // Return code is of no relevance here.
    }

    // Next, we set the pin for the green LED to output low.
    int err = gpio_pin_configure_dt(spec, GPIO_OUTPUT_INACTIVE);
    if (err != 0) {
        LOG_ERR("failed to set gpio mode to output_inactive: %d", err);
        return -ECANCELED;
    }

    return 0;
}

// Cofigures the button.
static int configure_button(const struct gpio_dt_spec *spec, struct gpio_callback *callback)
{
    // First, we check whether the GPIO device is ready.
    if (!gpio_is_ready_dt(spec)) {
        LOG_ERR("gpio device not ready");
        return -ECANCELED; // Return code is of no relevance here.
    }

    // Configure the interrupt.
    int err = gpio_pin_configure_dt(spec, GPIO_INPUT);
    if (err != 0) {
        LOG_ERR("failed to set gpio interrupt mode %d", err);
        return -ECANCELED;
    }

    err = gpio_pin_interrupt_configure_dt(spec, GPIO_INT_EDGE_TO_ACTIVE);
    if (err != 0) {
        LOG_ERR("failed to enable interrupt: %d", err);
        return -ECANCELED;
    }

    gpio_init_callback(callback, on_button_pressed, BIT(spec->pin));
    err = gpio_add_callback(spec->port, callback);
    if (err != 0) {
        LOG_ERR("failed to add gpio callback: %d", err);
        return -ECANCELED;
    }

    return 0;
}

// Initializer function called before main starts.
// These functions can be used to set up module specific things.
int init_io(void)
{
    // Configure LEDs and button here.

    // If ok, return 0.
    return 0;
}

// The APPLICATION init level is the init level before main.
SYS_INIT(init_io, APPLICATION, 0);