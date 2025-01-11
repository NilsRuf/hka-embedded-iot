// This file implements the reading of environment sensors.

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#include <env_sensor.h>

#ifdef CONFIG_MOCK_ENV_SENSORS
#include <zephyr/random/random.h>
#endif

LOG_MODULE_REGISTER(env_sensor);

/* ============================================================================================== */
/* ---------------------------------- Module-wide sensor data. ---------------------------------- */
/* ============================================================================================== */

/* ------------------------------------ Climate sensor data. ------------------------------------ */

#ifndef CONFIG_MOCK_ENV_SENSORS
// Handle to the climate sensor device.
static const struct device *m_climate_sensor = DEVICE_DT_GET(DT_ALIAS(climate_sensor));
#endif

// Flag indicating init status of climate sensor.
static bool m_climate_sensor_initialized = false;

/* ============================================================================================== */
/* ------------------------ Implementation of climate sensor functions. ------------------------- */
/* ============================================================================================== */

int env_sensor_climate_init(void)
{
    if (m_climate_sensor_initialized) {
        // We are already initiazlied.
        return 0;
    }

#ifndef CONFIG_MOCK_ENV_SENSORS
    if (!device_is_ready(m_climate_sensor)) {
        LOG_ERR("sensor not ready - aborting");
        return -ECANCELED;
    }
#endif

    // Currently, there is nothing else to do here.
    m_climate_sensor_initialized = true;
    return 0;
}

int env_sensor_climate_get(struct env_sensor_climate_sample *sample)
{
    if (!sample) {
        return -EINVAL;
    }

    if (!m_climate_sensor_initialized) {
        return -ECANCELED;
    }

#ifndef CONFIG_MOCK_ENV_SENSORS
    int err = sensor_sample_fetch(m_climate_sensor);
    if (err != 0) {
        LOG_ERR("failed to read sensor value: %d", err);
        return err;
    }

    // These calls work once the sample has been fetched.
    (void)sensor_channel_get(
            m_climate_sensor, SENSOR_CHAN_AMBIENT_TEMP, &sample->temperature_celsius);
    (void)sensor_channel_get(m_climate_sensor, SENSOR_CHAN_HUMIDITY, &sample->humidity_percent);
#else
    // Here, we mock the sensor for now.
    static float s_prev_temp = 25.0;
    static float s_prev_hum = 30.0;

    const float max_temp = 35.0;
    const float min_temp = -5.0;
    const float max_hum = 100.0;
    const float min_hum = 0.0;

    int8_t delta_temp;
    sys_rand_get(&delta_temp, sizeof(delta_temp));
    float real_delta_temp = (float)delta_temp / 128.0f;
    s_prev_temp += real_delta_temp;
    s_prev_temp = MAX(min_temp, MIN(max_temp, s_prev_temp));
    sample->temperature_celsius.val1 = (int32_t)s_prev_temp;
    sample->temperature_celsius.val2 = (int32_t)((s_prev_temp - (float)(int32_t)s_prev_temp) * 1000000.0f);

    int8_t delta_hum;
    sys_rand_get(&delta_hum, sizeof(delta_hum));
    float real_delta_hum = (float)delta_hum / 128.0f;
    s_prev_hum += real_delta_hum;
    s_prev_hum = MAX(min_hum, MIN(max_hum, s_prev_hum));
    sample->humidity_percent.val1 = (int32_t)s_prev_hum;
    sample->humidity_percent.val2 = (int32_t)((s_prev_hum - (float)(int32_t)s_prev_hum) * 1000000.0f);

    LOG_WRN("mocked sensor sample");
#endif

    return 0;
}
