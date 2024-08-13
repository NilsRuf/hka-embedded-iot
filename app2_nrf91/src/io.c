// This module implements status indicators (LEDs) and buttons for the measurements.

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>

#include "channels.h"

LOG_MODULE_REGISTER(io);

// Debounce interval of the button in milliseconds.
#define BUTTON_DEBOUNCE_INTERVAL_MS 200

// Work item for handling button presses.
struct button_pressed_worker {
    struct k_work work; // Work item to be executed.
    enum io_request request; // Carries the request to be sent.
};

// Worker for controlling the LEDs.
struct led_control_worker {
    struct k_work work; // Work item to be executed.
    struct sensor_msmt_message sensor_message; // Message containing the sensor value.
};

// Define the IO channel.
ZBUS_CHAN_DEFINE(
    io_channel,
    struct io_request_msg,
    NULL, NULL,
    ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(0));

// This reads the GPIO settings of the LEDs at compile time.
static const struct gpio_dt_spec red_led_spec = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green_led_spec = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);

// Those are settings for the button.
static const struct gpio_dt_spec button_spec = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static struct gpio_callback button_callback;

// Handler for the button press.
static void on_button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins);
static void reactivate_button(struct k_timer *timer);
static void button_pressed_handler(struct k_work *work);
static struct button_pressed_worker button_worker = {0};
K_SEM_DEFINE(button_press_possible, 1, 1); // Protects the work item from being issued twice.
K_TIMER_DEFINE(button_reactivate_timer, reactivate_button, NULL);

// Listener listening for new sensor values.
static void on_measurement_received(const struct zbus_channel *chan);
ZBUS_LISTENER_DEFINE(io_measurement_listener, on_measurement_received);
ZBUS_CHAN_ADD_OBS(sensor_channel, io_measurement_listener, 1);

// Worker for controlling the LEDs.
static void led_control_handler(struct k_work *work);
static struct led_control_worker led_control_worker;
K_SEM_DEFINE(led_control_ready, 1, 1); // Protects the work item from being issued twice.

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

// Delegates the handling of the button press to a work item.
static void on_button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);

    // Disable the interrupt to debounce the timer.
    int err = gpio_pin_interrupt_configure_dt(&button_spec, GPIO_INT_DISABLE);
    if (err != 0) {
        LOG_ERR("failed to enable interrupt: %d", err);
        return;
    }
    k_timer_start(&button_reactivate_timer, K_MSEC(BUTTON_DEBOUNCE_INTERVAL_MS), K_NO_WAIT);
    
    if (k_sem_take(&button_press_possible, K_NO_WAIT) != 0) {
        LOG_WRN("ignoring button press");
        return;
    }

    LOG_INF("new measurement explicitly requested");
    button_worker.request = IO_REQUEST_TRIGGER_MSMT;
    k_work_submit(&button_worker.work);
}

static void reactivate_button(struct k_timer *timer)
{
    ARG_UNUSED(timer);

    int err = gpio_pin_interrupt_configure_dt(&button_spec, GPIO_INT_EDGE_TO_ACTIVE);
    if (err != 0) {
        LOG_ERR("failed to enable interrupt: %d", err);
    }    
}

// Sends a notification about the button press event.
static void button_pressed_handler(struct k_work *work)
{
    struct button_pressed_worker *worker = CONTAINER_OF(work, struct button_pressed_worker, work);
    struct io_request_msg message = {.request = worker->request};

    int err = zbus_chan_pub(&io_channel, &message, K_NO_WAIT);
    if (err != 0) {
        LOG_ERR("failed to publish measurement trigger request: %d", err);
    }

    k_sem_give(&button_press_possible);
}

// Delegates the measurement value to the work queue.
static void on_measurement_received(const struct zbus_channel *chan)
{
    if (k_sem_take(&led_control_ready, K_NO_WAIT) != 0) {
        LOG_WRN("ignoring measurement value");
        return;
    }

    const struct sensor_msmt_message *message = zbus_chan_const_msg(chan);
    led_control_worker.sensor_message = *message;
    k_work_submit(&led_control_worker.work);
}

// Handles the LEDs.
static void led_control_handler(struct k_work *work)
{
    struct led_control_worker *worker = CONTAINER_OF(work, struct led_control_worker, work);
    struct sensor_msmt_message *sens = &worker->sensor_message;

    if (sens->temperature.val1 < CONFIG_TEMPERATURE_MAX_GOOD) {
        (void)gpio_pin_set_dt(&red_led_spec, 0);
        (void)gpio_pin_set_dt(&green_led_spec, 1);
        LOG_INF("temperature is good");
    } else if (sens->temperature.val1 < CONFIG_TEMPERATURE_MAX_OKAY) {
        (void)gpio_pin_set_dt(&red_led_spec, 1);
        (void)gpio_pin_set_dt(&green_led_spec, 1);
        LOG_INF("temperature is warm");
    } else {
        (void)gpio_pin_set_dt(&red_led_spec, 1);
        (void)gpio_pin_set_dt(&green_led_spec, 0);
        LOG_INF("temperature is hot");
    }

    LOG_INF("received measurement from %llums:\n\ttemperature = %d.%d\n\thumidity = %d.%d\n\t"
        "pressure = %d.%d\n\tgas_res = %d.%d",
        sens->uptime_ms, sens->temperature.val1, sens->temperature.val2,
        sens->humidity.val1, sens->humidity.val2, sens->pressure.val1, sens->pressure.val2,
        sens->gas_res.val1, sens->gas_res.val2);

    k_sem_give(&led_control_ready);
}

// Initializer function called before main starts.
// These functions can be used to set up module specific things.
static int init_io(void)
{
    // Configure LEDs and button here.
    int err = 0;

    err = configure_led(&red_led_spec);
    if (err != 0) {
        LOG_ERR("failed to configure red led: %d", err);
        return err;
    }

    err = configure_led(&green_led_spec);
    if (err != 0) {
        LOG_ERR("failed to configure green led: %d", err);
        return err;
    }


    err = configure_button(&button_spec, &button_callback);
    if (err != 0) {
        LOG_ERR("failed to configure button: %d", err);
        return err;
    }

    k_work_init(&button_worker.work, button_pressed_handler);
    k_work_init(&led_control_worker.work, led_control_handler);

    // If ok, return 0.
    return err;
}

// The APPLICATION init level is the init level before main.
SYS_INIT(init_io, APPLICATION, 0);