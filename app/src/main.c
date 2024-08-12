// This file conains our application's entry point.

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

// We register a log module here so we can use LOG_INF, LOG_ERR, etc. in this
// module.
LOG_MODULE_REGISTER(app);

// We define some shorthands for our timeouts.
#define RED_LED_TIMEOUT K_MSEC(CONFIG_BLINK_INTERVAL_RED_MS)
#define GREEN_LED_TIMEOUT K_MSEC(CONFIG_BLINK_INTERVAL_GREEN_MS)
#define BLUE_LED_TIMEOUT K_MSEC(CONFIG_BLINK_INTERVAL_BLUE_MS)

// This reads the GPIO settings of the LEDs at compile time.
static const struct gpio_dt_spec red_led_spec = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green_led_spec = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec blue_led_spec = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);

// Those are settings for the button.
static const struct gpio_dt_spec button_spec = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static struct gpio_callback button_callback;

// Handler for the timer expiration and the button press.
static void on_led_timer_expired(struct k_timer *timer);
static void on_button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins);

// Define the timers.
K_TIMER_DEFINE(red_timer, on_led_timer_expired, NULL);
K_TIMER_DEFINE(green_timer, on_led_timer_expired, NULL);
K_TIMER_DEFINE(blue_timer, on_led_timer_expired, NULL);

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

// The timers are started here. The second parameter specifies the periodicity.
static void start_led_timers(void)
{
    k_timer_user_data_set(&red_timer, (void *)&red_led_spec);
    k_timer_start(&red_timer, K_NO_WAIT, RED_LED_TIMEOUT);

    k_timer_user_data_set(&green_timer, (void *)&green_led_spec);
    k_timer_start(&green_timer, K_NO_WAIT, GREEN_LED_TIMEOUT);

    k_timer_user_data_set(&blue_timer, (void *)&blue_led_spec);
    k_timer_start(&blue_timer, K_NO_WAIT, BLUE_LED_TIMEOUT);

    LOG_INF("led timers started");
}

// This function stops all timers.
static void stop_led_timers(void)
{
    k_timer_stop(&red_timer);
    k_timer_stop(&green_timer);
    k_timer_stop(&blue_timer);

    (void)gpio_pin_set_dt(&red_led_spec, 0);
    (void)gpio_pin_set_dt(&green_led_spec, 0);
    (void)gpio_pin_set_dt(&blue_led_spec, 0);
}

static void on_led_timer_expired(struct k_timer *timer)
{
    // Let's get the current handle.
    const struct gpio_dt_spec *spec = timer->user_data;

    // Then we toggle the LED.
    gpio_pin_toggle_dt(spec);
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

// Handler function for the button press.
static void on_button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    // We do not need those arguments.
    ARG_UNUSED(dev);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);

    static enum {LED_STATE_ON, LED_STATE_OFF} state = LED_STATE_ON;

    if (state == LED_STATE_ON) {
        stop_led_timers();
        state = LED_STATE_OFF;
        LOG_INF("button pressed - stopped leds");
    } else {
        start_led_timers();
        state = LED_STATE_ON;
        LOG_INF("button pressed - started leds");
    }
}

int main(void)
{
    // First initialize all GPIOs.
    int err = configure_led(&red_led_spec);
    if (err != 0) {
        LOG_ERR("failed to configure red led: %d", err);
        return 0;
    }

    err = configure_led(&green_led_spec);
    if (err != 0) {
        LOG_ERR("failed to configure green led: %d", err);
        return 0;
    }

    err = configure_led(&blue_led_spec);
    if (err != 0) {
        LOG_ERR("failed to configure blue led: %d", err);
        return 0;
    }

    err = configure_button(&button_spec, &button_callback);
    if (err != 0) {
        LOG_ERR("failed to configure button: %d", err);
    }

    start_led_timers();

    return 0;
}