#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <soc.h>
#include <zephyr/types.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
 
#define BLE_SERVICE_UUID 0xd4, 0x86, 0x48, 0x24, 0x54, 0xB3, 0x43, 0xA1, \
                        0xBC, 0x20, 0x97, 0x8F, 0xC3, 0x76, 0xC2, 0x75
 
#define RX_CHARACTERISTIC_UUID  0xA6, 0xE8, 0xC4, 0x60, 0x7E, 0xAA, 0x41, 0x6B, \
                                0x95, 0xD4, 0x9D, 0xCC, 0x08, 0x4F, 0xCF, 0x6A
 
/* Callback type for when new data is received. */
typedef void (*data_rx_cb_t)(uint8_t *data, uint8_t length);

typedef void (*on_data_received_cb)(const uint8_t *data, uint16_t len);
 
/* Callback struct used by the ble_service Service. */
struct ble_service_cb {
    /* Data received callback. */
    data_rx_cb_t    data_rx_cb;
};
 
struct ble_service_cb* get_ble_service_cb(void);
bool is_partying_time(void);
void ble_service_register_callback(on_data_received_cb callback);
void ble_service_register_sem(struct k_sem *sem);
void bt_ready(int err);