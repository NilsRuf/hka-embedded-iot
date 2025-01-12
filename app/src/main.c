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

// These are the button GPIO settings.
static const struct gpio_dt_spec m_button_spec = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);

// Interrupt callback handler for the button.
static struct gpio_callback m_button_callback;

/* ------------------------------------ Function prototypes ------------------------------------- */

// LED configuration
static int configure_led(const struct gpio_dt_spec *spec);

// Configures the button.
static int configure_button(const struct gpio_dt_spec *spec, struct gpio_callback *callback);

// Handler for button presses.
static void on_button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins);

// LED toggle timer expiry function.
static void on_led_timer_expired(struct k_timer *timer);

K_TIMER_DEFINE(m_my_timer, on_led_timer_expired, NULL);

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

    err = configure_button(&m_button_spec, &m_button_callback);
    if (err != 0) {
        LOG_ERR("failed to configure button: %d", err);
        return 0;
    }

    k_timer_start(&m_my_timer, K_NO_WAIT, LED_TIMEOUT);

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

static void on_led_timer_expired(struct k_timer *timer)
{
    ARG_UNUSED(timer);

    LOG_INF("toggling led");
    gpio_pin_toggle_dt(&m_led_spec);
}

/* ============================================================================================== */
/* -------------------------- Button-related configurations and ISRs. --------------------------- */
/* ============================================================================================== */

// Configures the button.
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

// Handler function for the button press.
static void on_button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    // We do not need those arguments.
    ARG_UNUSED(dev);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);

    static enum {LED_STATE_ON, LED_STATE_OFF} state = LED_STATE_ON;

    if (state == LED_STATE_ON) {
        k_timer_stop(&m_my_timer);
        state = LED_STATE_OFF;
        LOG_INF("button pressed - stopped leds");
    } else {
        k_timer_start(&m_my_timer, K_NO_WAIT, LED_TIMEOUT);
        state = LED_STATE_ON;
        LOG_INF("button pressed - started leds");
    }
}
