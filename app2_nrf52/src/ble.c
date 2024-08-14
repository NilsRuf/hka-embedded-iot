// This file implements the BLE part of the application.

// Read through the code and the comments to find out how to complete the sample.

#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>

#include "channels.h"

LOG_MODULE_REGISTER(ble);

// Current sensor measurement.
static struct sensor_msmt_message current_measurement = {0};

// TODO: Implement those four functions.
// Read callback functions for our characteristics.
// @param conn   The connection that is requesting to read
// @param attr   The attribute that's being read
// @param buf    Buffer to place the read result in
// @param len    Length of data to read
// @param offset Offset to start reading from
// Returns the number of bytes read or a BT error code.
static ssize_t read_temperature_bt(struct bt_conn *conn,
                            const struct bt_gatt_attr *attr,
					        void *buf, uint16_t len,
					        uint16_t offset);
static ssize_t read_pressure_bt(struct bt_conn *conn,
                         const struct bt_gatt_attr *attr,
					     void *buf, uint16_t len,
					     uint16_t offset);
static ssize_t read_humidity_bt(struct bt_conn *conn,
                        const struct bt_gatt_attr *attr,
					    void *buf, uint16_t len,
					    uint16_t offset);
static ssize_t read_gas_res_bt(struct bt_conn *conn,
                        const struct bt_gatt_attr *attr,
					    void *buf, uint16_t len,
					    uint16_t offset);

// Callback which is executed when a connection attempt has been handled.
// The err parameter indicates the success status of the connection attempt.
// TODO: Implement.
static void on_connected(struct bt_conn *conn, uint8_t err);

// Callback executed on device disconnect.
// The reason parameter indicates the reason for the disconnect.
// TODO: Implement. Make sure to start advertising again because it is disabled once a connection
// has been previously established.
static void on_disconnected(struct bt_conn *conn, uint8_t reason);

// We need to define a custom 128 bit UUID for the gas resistance characteristic because
// that does not exist.
static const struct bt_uuid_128 gas_resistance_uuid = BT_UUID_INIT_128(
    BT_UUID_128_ENCODE(0x00000001, 0x0000, 0x1000, 0x8000, 0x00805F9B34FB));

// Define the GATT service.
// The GATT service is based on the Environmental Sensing Service defined by the Bluetooth SIG.
// However, we need a custom service for gas resistance here as well.
BT_GATT_SERVICE_DEFINE(ess_svc,
    // This is the primary service we support
    BT_GATT_PRIMARY_SERVICE(BT_UUID_ESS),

    // Definition of a characteristic.
    // Each characteristic we need has a unique UUIDs. Except for the gas resistance, all UUIDs
    // are defined by BLE and accessible through BT_UUID_XXX macros
    // The first argument is the UUID, followed by access properties for the characteristic.
    // We always wand read and notify permissions so we can send updates to the connected device when
    // a value changes.
    // Next, we set the access permissions to read-only using BT_GATT_PERM_READ.
    // After that there is space for two function pointers. The first defines a read handler which is called
    // when a connected device wants to read out sensor measurement. The second one is for writing, so we
    // do not need that.
    // The last parameter is a pointer to our current measurement which is provided to the read handler.
    BT_GATT_CHARACTERISTIC(BT_UUID_TEMPERATURE,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ, read_temperature_bt, NULL, &current_measurement),
    // This is optional and defines a humanly readable name for the characteristic along with its access permissions.
    BT_GATT_CUD("Temperature", BT_GATT_PERM_READ),

    // Here, we define the gas resistance characteristic using the custom 128-bit UUID.
    BT_GATT_CHARACTERISTIC(&gas_resistance_uuid.uuid,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ, read_gas_res_bt, NULL, &current_measurement),
    BT_GATT_CUD("Gas Resistance", BT_GATT_PERM_READ),

    // TODO: Add the missing entries for pressure and humidity.
);

// Here are our read handlers.

static ssize_t read_temperature_bt(struct bt_conn *conn,
                            const struct bt_gatt_attr *attr,
					        void *buf, uint16_t len,
					        uint16_t offset)
{
    // Call bt_gatt_attr_read with the proper parameters here.
    // You can access the measurement value either globally or by using
    // attr->user_data.
    return 0; // Change return code to what bt_gatt_attr_read gives you.
}

// TODO: Implement those three functions.
static ssize_t read_pressure_bt(struct bt_conn *conn,
                         const struct bt_gatt_attr *attr,
					     void *buf, uint16_t len,
					     uint16_t offset)
{
    return 0; // Change return code to what bt_gatt_attr_read gives you.
}

static ssize_t read_humidity_bt(struct bt_conn *conn,
                        const struct bt_gatt_attr *attr,
					    void *buf, uint16_t len,
					    uint16_t offset)
{
    return 0; // Change return code to what bt_gatt_attr_read gives you.
}

static ssize_t read_gas_res_bt(struct bt_conn *conn,
                        const struct bt_gatt_attr *attr,
					    void *buf, uint16_t len,
					    uint16_t offset)
{
    return 0; // Change return code to what bt_gatt_attr_read gives you.
}

// Setting up advertising.
// This array defines the advertising data.
static const struct bt_data advertising_data[] = {
    // We do not advertise enhanced data rate.
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
    // This entry actually advertises our service.
    BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_ESS_VAL),

    // TODO: Add the device name using data bytes of type BT_DATA_NAME_COMPLETE.
    // The device name can be found in prj.conf.
    // Make sure to also provide the length of the name string as a third parameter.
};

// This function enables advertising.
static int start_advertising(void)
{
    // TODO: Start the advertising procedure using bt_le_adv_start.
    // Hint: We want to be connectable, so we need to specify BT_LE_ADV_CONN as the first parameter.
}

// Connection handling

// Callback which is executed when a connection attempt has been handled.
// TODO: Implement.
static void on_connected(struct bt_conn *conn, uint8_t err)
{

}

// Callback executed on device disconnect.
// TODO: Implement. Make sure to start advertising again because it is disabled once a connection
// has been previously established.
static void on_disconnected(struct bt_conn *conn, uint8_t reason)
{

}

// Callback structure which defines connection handlers.
static struct bt_conn_cb conn_callbacks = {
    .connected = on_connected,
    .disconnected = on_disconnected,
};

// Initializes the BLE module at boot.
static int ble_init(void)
{
    // First, we enable bluetooth.
    int err = bt_enable(NULL);
    if (err != 0) {
        LOG_ERR("failed to enable bluetooth: %d", err);
        return err;
    }

    // TODO: Register our connection handlers using bt_conn_cb_register.

    // TODO: Start advertising.

    return 0;
}

SYS_INIT(ble_init, APPLICATION, 99);