// This file conains our application's entry point.

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <env_sensor.h>

// We register a log module here so we can use LOG_INF, LOG_ERR, etc. in this
// module.
LOG_MODULE_REGISTER(app);

/* ============================================================================================== */
/* ---------------------------------- Some global definitions ----------------------------------- */
/* ============================================================================================== */

// We define some shorthands for our timeouts.
#define LED_TIMEOUT K_MSEC(CONFIG_BLINK_INTERVAL_MS)

// This reads the GPIO settings of the LEDs at compile time.
static const struct gpio_dt_spec m_led_spec = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

/* ------------------------------------ Function prototypes ------------------------------------- */

// LED configuration
static int configure_led(const struct gpio_dt_spec *spec);

/* --------------------------------------- Main function. --------------------------------------- */

int main(void)
{
    int err = 0;

    // First initialize all GPIOs.
    err = configure_led(&m_led_spec);
    if (err != 0) {
        LOG_ERR("failed to configure green led: %d", err);
        return 0;
    }

    // TODO: Configure the button here as well...

    while (true) {
        gpio_pin_toggle_dt(&m_led_spec);
        k_sleep(LED_TIMEOUT);
    }

    return 0;
}

/* ============================================================================================== */
/* ------------------------------ Configuration function for LEDs. ------------------------------ */
/* ============================================================================================== */

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

/* ============================================================================================== */
/* -------------------------- Button-related configurations and ISRs. --------------------------- */
/* ============================================================================================== */

// Configures the button.
static int configure_button(const struct gpio_dt_spec *spec, struct gpio_callback *callback)
{
    // TODO
    return 0;
}

// Handler function for the button press.
static void on_button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    // We do not need those arguments.
    ARG_UNUSED(dev);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);

    // TODO
}
