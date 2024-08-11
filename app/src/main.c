// This file conains our application's entry point.

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

// We register a log module here so we can use LOG_INF, LOG_ERR, etc. in this
// module.
LOG_MODULE_REGISTER(app);

// This reads the GPIO settings of the green LED at compile time.
static const struct gpio_dt_spec green_led_spec = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);

int main(void)
{
    // First, we set the GPIO pin to output low.
    if (!gpio_is_ready_dt(&green_led_spec)) {
        LOG_ERR("gpio device not ready");
        return 0; // Return code is of no relevance here.
    }

    // Next, we set the pin for the green LED to output low.
    int err = gpio_pin_configure_dt(&green_led_spec, GPIO_OUTPUT_INACTIVE);
    if (err != 0) {
        LOG_ERR("failed to set gpio mode to output_inactive: %d", err);
        return 0;
    }

    while (true) {
        (void)gpio_pin_set_dt(&green_led_spec, 1); // Turn on the LED.
        LOG_INF("turned on led");
        k_msleep(CONFIG_BLINK_INTERVAL_MS / 2); // Sleep for half the period.
        (void)gpio_pin_set_dt(&green_led_spec, 0); // Turn the LED off again.
        LOG_INF("turned off LED");
        k_msleep(CONFIG_BLINK_INTERVAL_MS / 2); // Sleep for the second half of the period.
    }

    return 0;
}