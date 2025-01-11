// This file defines the interface for the environment sensors.

#ifndef _ENV_SENSOR_H_
#define _ENV_SENSOR_H_

#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>

/// Represents a sensor sample for climate data (temperature and humidity).
struct env_sensor_climate_sample {
    struct sensor_value temperature_celsius; ///< Ambient temperature in degrees celsius.
    struct sensor_value humidity_percent;    ///< Humidity in percent.
};

/// Initializes the climate sensor.
/// \return 0 on success or negative errno code.
int env_sensor_climate_init(void);

/// Gets a sample from the climate sensor.
/// \param[out] sample Pointer to structure in which to save the measurement value.
/// \return 0 on success or negative errno code.
int env_sensor_climate_get(struct env_sensor_climate_sample *sample);

/// Initializes the light sensor.
/// \return 0 on success or negative errno code.
int env_sensor_ldo_init(void);

#endif
