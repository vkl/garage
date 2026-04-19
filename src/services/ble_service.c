#include <stddef.h>
#include <string.h>
#include <errno.h>

#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>
#include <zephyr/types.h>

#include <soc.h>
 
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/addr.h>
#include <zephyr/bluetooth/gatt.h>

#include "ble_service.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ble_service, LOG_LEVEL_DBG);
 
#define DEVICE_NAME             CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN         (sizeof(DEVICE_NAME) - 1)

#define BT_UUID_MY_SERVICE      BT_UUID_DECLARE_128(BLE_SERVICE_UUID)
#define BT_UUID_MY_SERVICE_RX   BT_UUID_DECLARE_128(RX_CHARACTERISTIC_UUID)
 
bool party_mode = false;
 
static on_data_received_cb user_callback = NULL;
static struct k_sem *ble_init_ok;

/* Define the connection we'll be establishing */
static struct bt_conn *my_connection;
 
/* Array containing the advertising data */
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, BLE_SERVICE_UUID),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

/* Array containing the scan response data */
static const struct bt_data sd[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static const struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM(
    	BT_LE_ADV_OPT_CONN | BT_LE_ADV_OPT_SCANNABLE,
        BT_GAP_ADV_FAST_INT_MIN_2,
        BT_GAP_ADV_FAST_INT_MAX_2,
        NULL);

/***************************** Static functions *****************************/

static void connected(struct bt_conn *conn, uint8_t err);
static void disconnected(struct bt_conn *conn, uint8_t reason);
static bool le_param_req(struct bt_conn *conn, struct bt_le_conn_param *param);
static void le_param_updated(struct bt_conn *conn, uint16_t interval, uint16_t latency, uint16_t timeout);

/* Wire up all the callbacks we defined above to the bt struct */
static struct bt_conn_cb conn_callbacks = {
    .connected         = connected,
    .disconnected      = disconnected,
    .le_param_req      = le_param_req,
    .le_param_updated  = le_param_updated
};

/***************************** Public functions ****************************/

void
bt_ready(int err)
{
    if (err) {
        LOG_ERR("BLE init failed with error code %d", err);
        return;
    }
 
    /* Configure connection callbacks */
    bt_conn_cb_register(&conn_callbacks);
 
    /* Start advertising */
    err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (err) {
        LOG_ERR("Advertising failed to start (err %d)", err);
        return;
    }
 
    LOG_INF("Advertising successfully started");
 
    k_sem_give(ble_init_ok);
}

/* Who's trying to party? */
bool
is_partying_time(void)
{
    return party_mode;
}

void
ble_service_register_sem(struct k_sem *sem)
{
    ble_init_ok = sem;
}
 
void
ble_service_register_callback(on_data_received_cb callback)
{
    user_callback = callback;
}
 
/* This function is called whenever the RX Characteristic has been written to by a Client */
static ssize_t
on_receive(struct bt_conn *conn,
        const struct bt_gatt_attr *attr,
        const void *buf,
        uint16_t len,
        uint16_t offset,
        uint8_t flags)
{
    /* Data coming in off the BLE write */
    const uint8_t *buffer = (const uint8_t *)buf;
 
    if (user_callback != NULL) {
        user_callback(buffer, len);
    }
 
    return len;
}
 
/* This function is called whenever the CCCD register has been changed by the client*/
static void
on_cccd_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    ARG_UNUSED(attr);
    switch(value) {
        case BT_GATT_CCC_NOTIFY:
            // Start sending stuff!
            break;
        case BT_GATT_CCC_INDICATE:
            // Start sending stuff via indications
            break;
        case 0:
            // Stop sending stuff
            break;
        default:
            printk("Error, CCCD has been set to an invalid value");
    }
}
 
/* BLE Service Declaration and Registration */
BT_GATT_SERVICE_DEFINE(ble_service,
BT_GATT_PRIMARY_SERVICE(BT_UUID_MY_SERVICE),
BT_GATT_CHARACTERISTIC(BT_UUID_MY_SERVICE_RX,
                   BT_GATT_CHRC_WRITE | BT_GATT_CHRC_WRITE_WITHOUT_RESP,
                   BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
                   NULL, on_receive, NULL),
BT_GATT_CCC(on_cccd_changed,
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);

/***********************************************************************/

static void
connected(struct bt_conn *conn, uint8_t err)
{
    struct bt_conn_info info;
    char addr[BT_ADDR_LE_STR_LEN];
    my_connection = conn;
 
    if (err) {
		LOG_WRN("Connection failed (err %u)", err);
        return;
    } else if (bt_conn_get_info(conn, &info)) {
        LOG_ERR("Could not parse connection info");
    } else {
        bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
        LOG_INF("Connection established to %s", addr);
    }
}

static void
disconnected(struct bt_conn *conn, uint8_t reason)
{
    int err;
    LOG_INF("Disconnected (reason %u)", reason);

    /* Restart advertising */
    err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (err) {
        LOG_ERR("Advertising failed to restart (err %d)", err);
        return;
    }

    LOG_INF("Advertising restarted");
}
 
/* Callback for BLE update request */
static bool
le_param_req(struct bt_conn *conn, struct bt_le_conn_param *param)
{
    /* If acceptable params, return true, otherwise return false. */
    return true;
}
 
static void
le_param_updated(struct bt_conn *conn, uint16_t interval, uint16_t latency, uint16_t timeout)
{
    struct bt_conn_info info;
    char addr[BT_ADDR_LE_STR_LEN];
 
    if(bt_conn_get_info(conn, &info)) {
        LOG_ERR("Could not parse connection info");
    } else {
        bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
        LOG_INF("Connection parameters updated for %s", addr);
    }
}
