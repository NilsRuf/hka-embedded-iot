// This file conains our application's entry point.

#include "zephyr/sys/time_units.h"
#include "zephyr/zbus/zbus.h"
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <channels.h>
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

// Current button press state.
static enum button_state {
    BUTTON_STATE_RELEASED,
    BUTTON_STATE_PRESSED,
} m_button_state = BUTTON_STATE_RELEASED;

// Time when the button was pressed the last time.
static k_ticks_t m_last_pressed = 0;

// Structure defining the work context for publishing a button press message.
static struct press_duration_context {
    struct k_work work;
    uint32_t press_duration_ms;
} m_press_duration_context;

/* ------------------------------------ Function prototypes ------------------------------------- */

// Configures the LED.
static int configure_led(const struct gpio_dt_spec *spec);

// Configures the button.
static int configure_button(const struct gpio_dt_spec *spec, struct gpio_callback *callback);

// Handler for button presses.
static void on_button_change(const struct device *dev, struct gpio_callback *cb, uint32_t pins);

// LED toggle timer expiry function.
static void on_led_timer_expired(struct k_timer *timer);

// Triggers a measurement.
static void trigger_measurement(struct k_timer *timer);

// Takes a sensor measurement and logs the result.
static void take_measurement(struct k_work *work);

// Publishes a button press duration.
static void publish_press_duration(struct k_work *work);

// Timer for the LED.
K_TIMER_DEFINE(m_my_timer, on_led_timer_expired, NULL);

// Timer for triggering measurements.
K_TIMER_DEFINE(m_msmt_timer, trigger_measurement, NULL);

// Work handler for taking measurement.
K_WORK_DEFINE(m_measurement_work, take_measurement);

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

    // Initizlize the button work item before the button reacts to interrupts.
    k_work_init(&m_press_duration_context.work, publish_press_duration);

    err = configure_button(&m_button_spec, &m_button_callback);
    if (err != 0) {
        LOG_ERR("failed to configure button: %d", err);
        return 0;
    }

    // Initialize the climate sensor (should be done in an appropriate module).
    err = env_sensor_climate_init();
    if (err != 0) {
        LOG_ERR("failed to init climate sensor: %d", err);
        return 0;
    }

    // Start the timers for LEDs and measurements. The latter one should be in another module.
    k_timer_start(&m_my_timer, K_NO_WAIT, LED_TIMEOUT);
    k_timer_start(&m_msmt_timer, K_NO_WAIT, K_MSEC(CONFIG_ENV_SENSOR_CLIMATE_UPDATE_INTERVAL_MS));

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

    err = gpio_pin_interrupt_configure_dt(spec, GPIO_INT_EDGE_BOTH);
    if (err != 0) {
        LOG_ERR("failed to enable interrupt: %d", err);
        return -ECANCELED;
    }

    gpio_init_callback(callback, on_button_change, BIT(spec->pin));
    err = gpio_add_callback(spec->port, callback);
    if (err != 0) {
        LOG_ERR("failed to add gpio callback: %d", err);
        return -ECANCELED;
    }

    return 0;
}

static void on_led_timer_expired(struct k_timer *timer)
{
    ARG_UNUSED(timer);

    LOG_INF("toggling LED");
    gpio_pin_toggle_dt(&m_led_spec);
}

// Handler function for the button change.
static void on_button_change(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    // We do not need those arguments.
    ARG_UNUSED(dev);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);

    k_ticks_t uptime = k_uptime_ticks();

    if (m_button_state == BUTTON_STATE_RELEASED) {
        // Button has just been pressed.
        m_button_state = BUTTON_STATE_PRESSED;
        m_last_pressed = uptime;
        return;
    }

    m_button_state = BUTTON_STATE_RELEASED;
    int64_t press_time_ms = k_ticks_to_ms_near64(uptime - m_last_pressed);

    if (press_time_ms <= CONFIG_PRESS_TIME_SHORT_MS) {
        LOG_INF("short press: %lldms", press_time_ms);
    } else {
        LOG_INF("long press: %lldms", press_time_ms);
    }

    // Publish the message inside a work queue and not in an interrupt handler.
    m_press_duration_context.press_duration_ms = (uint32_t)press_time_ms;
    k_work_submit(&m_press_duration_context.work);

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

static void trigger_measurement(struct k_timer *timer)
{
    ARG_UNUSED(timer);

    LOG_INF("triggering measurement");
    k_work_submit(&m_measurement_work);
}

static void take_measurement(struct k_work *work)
{
    ARG_UNUSED(work);

    struct env_sensor_climate_sample climate_sample = {0};

    int err = env_sensor_climate_get(&climate_sample);
    if (err) {
        LOG_ERR("sample fetch failed: %d", err);
    } else {
        LOG_INF("{temp: %d.%d celsius, humi: %d.%d%%}",
                climate_sample.temperature_celsius.val1,
                climate_sample.temperature_celsius.val2,
                climate_sample.humidity_percent.val1,
                climate_sample.humidity_percent.val2);
    }
}

static void publish_press_duration(struct k_work *work)
{
    struct press_duration_context *ctx = CONTAINER_OF(work, struct press_duration_context, work);
    struct sender_message msg = {
        .type = SENDER_MSG_TYPE_PRESS_DURATION,
        .press_duration_ms = ctx->press_duration_ms,
    };

    int err = zbus_chan_pub(&g_sender_chan, &msg, K_MSEC(500));
    if (err) {
        LOG_ERR("failed to publish press duration: %d", err);
    } else {
        LOG_INF("published press duration: %u", msg.press_duration_ms);
    }
}
