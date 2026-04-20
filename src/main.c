/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/timer/system_timer.h>
#include <zephyr/fs/nvs.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/drivers/flash.h>


#include "ws2812.h"
#include "services/ble_service.h"

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

#define RGB(led, _r, _g, _b) \
    do {                     \
        (led).r = (_r);     \
        (led).g = (_g);     \
        (led).b = (_b);     \
    } while (0)

static const struct device *hc_sr04_dev = DEVICE_DT_GET_ANY(hc_sr04);
static struct nvs_fs fs;

#define NVS_PARTITION FIXED_PARTITION_ID(storage)
#define NVS_SECTOR_SIZE 4096
#define NVS_SECTOR_COUNT 6
#define NVS_ID_CONFIG 0

#define DELAY_TIME K_MSEC(100)
#define MEASURE_TIMEOUT_CNT 150 /* 200 ms * 150 = 30 seconds */

#define LOW_THRES_DEFAULT 10
#define HIGH_THRES_DEFAULT 100
#define LOW_THRES_LED_MIN 0
#define LOW_THRES_LED_MAX STRIP_NUM_PIXELS
#define GREEN_LEDS_THRESHOLD 10
#define YELLOW_LEDS_THRESHOLD 25

struct config {
	uint16_t low_threashold;
	uint16_t high_threashold;
};
 
/******************* Static definitions ***************************************/

/* A necessary definition for BLE stack */
static K_SEM_DEFINE(ble_init_ok, 0, 1);

static void measure_distance(void);
static void display(struct led_rgb *pixels, size_t count);
static void data_cb(const uint8_t *data, uint16_t len);
static void error(void);
static void init_cfg(void);
static void write_cfg(void);
static void worker(struct k_timer *timer_id);
static void stop_worker(struct k_timer *timer_id);

/******************************************************************************/
 
static struct config cfg;
static struct k_sem ble_init_ok;
static uint16_t cm;

struct k_timer measure_timer;
struct k_timer measure_timeout_timer;
K_TIMER_DEFINE(measure_timer, worker, NULL);
K_TIMER_DEFINE(measure_timeout_timer, stop_worker, NULL);

int
main(void)
{
    int err;
    cm = 500;

    init_cfg();

    LOG_INF("WS2812 PWM initialization");
    err = ws2812_pwm_init();
    if (err) {
        LOG_ERR("Failed to initialize WS2812 PWM");
        error();
    }

	ble_service_register_callback(data_cb);
	ble_service_register_sem(&ble_init_ok);

    err = bt_enable(bt_ready);
    if (err) {
        LOG_ERR("BLE initialization failed");
        error();
    }
 
    /*     
     * Bluetooth stack should be ready in less than 100 msec.
     *
     * We use this semaphore to wait for bt_enable to call bt_ready before we proceed
     * to the main loop. By using the semaphore to block execution we allow the RTOS to
     * execute other tasks while we wait.
     */
    err = k_sem_take(&ble_init_ok, K_MSEC(500));
    if (!err) {
        LOG_INF("Bluetooth initialized");
    } else {
        LOG_ERR("BLE initialization did not complete in time");
        error();
    }

    return 0;
}

static void
error(void)
{
    while (true) {
        LOG_ERR("Error!");
        k_sleep(K_MSEC(1000));
    }
}

void
data_cb(const uint8_t *buffer, uint16_t len)
{
    LOG_INF("Received data in main callback: len=%u", len);

	if (buffer[0] == 0x01) {
		/* Update config with new values from the app */
		if (len < 5) {
			LOG_WRN("Received data is too short to contain config, ignoring");
			return;
		}
		cfg.low_threashold = (buffer[1] << 8) | buffer[2];
		cfg.high_threashold = (buffer[3] << 8) | buffer[4];
		LOG_DBG("Updated config: low_threashold=%u, high_threashold=%u",
				cfg.low_threashold, cfg.high_threashold);
		write_cfg();
		return;
	}

	if (!((buffer[0] == 0xAA) && (buffer[1] == 0xBB) && (buffer[2] == 0xCC))) {
		LOG_WRN("Data does not match expected pattern, ignoring");
		return;
	}

    k_timer_start(&measure_timer, K_MSEC(100), K_MSEC(100));
    k_timer_start(&measure_timeout_timer, K_SECONDS(30), K_NO_WAIT);
}

static void
measure_distance(void)
{
    struct sensor_value distance;
    int err;
    int64_t total_um;

    /* Fetch sample (triggers measurement) */
    err = sensor_sample_fetch(hc_sr04_dev);
    if (err < 0) {
        LOG_ERR("Sensor fetch failed: %d", err);
        goto out;
    }

    /* Get distance value */
    err = sensor_channel_get(hc_sr04_dev, SENSOR_CHAN_DISTANCE, &distance);
    if (err < 0) {
        LOG_ERR("Sensor read failed: %d", err);
        goto out;
    }

    total_um = (int64_t)distance.val1 * 1000000 + distance.val2;
    cm = (total_um + 5000) / 10000;

    LOG_INF("Measured distance: %d.%06d m. %d cm", distance.val1, distance.val2, cm);

out:
    return;
}

static void
init_cfg(void)
{
	int err;
	int len = 0;
	const struct flash_area *fa;

    err = flash_area_open(NVS_PARTITION, &fa);
    if (err) {
        LOG_ERR("flash_area_open failed: %d", err);
        goto out;
    }

	fs.offset = fa->fa_off;
    fs.flash_device = flash_area_get_device(fa);
    fs.sector_size = NVS_SECTOR_SIZE;
    fs.sector_count = NVS_SECTOR_COUNT;

    err = nvs_mount(&fs);
    if (err) {
        LOG_ERR("nvs_mount failed: %d", err);
        goto out;
    }

	err = nvs_read(&fs, NVS_ID_CONFIG, &cfg, sizeof(cfg));
	if (err < 0) {
		LOG_ERR("Failed to read config from NVS: %d", err);
		/* Set default values if read fails */
		cfg.low_threashold = LOW_THRES_DEFAULT;
		cfg.high_threashold = HIGH_THRES_DEFAULT;
		len = nvs_write(&fs, NVS_ID_CONFIG, &cfg, sizeof(cfg));
		LOG_DBG("Bytes written to NVS: %d", len);
		LOG_INF("Default config written to NVS: low_threashold=%u, high_threashold=%u",
				cfg.low_threashold, cfg.high_threashold);
	} else {
		LOG_INF("Config loaded from NVS: low_threashold=%u, high_threashold=%u",
				cfg.low_threashold, cfg.high_threashold);
	}

out:
}

static void
write_cfg(void)
{
	int len = nvs_write(&fs, NVS_ID_CONFIG, &cfg, sizeof(cfg));
	if (len < 0) {
		LOG_ERR("Failed to write config to NVS");
	} else {
		LOG_DBG("Bytes written to NVS: %d", len);
		LOG_INF("Config written to NVS: low_threashold=%u, high_threashold=%u",
				cfg.low_threashold, cfg.high_threashold);
	}
}

static void
display(struct led_rgb *pixels, size_t count)
{
    uint8_t i, max;

    if (cm >= cfg.high_threashold) {
        max = 0;
    } else if (cm < cfg.low_threashold) {
        max = count;
    } else {
        uint32_t tmp = (uint32_t)count * (cfg.high_threashold - cm);
        max = (uint8_t)(tmp / (cfg.high_threashold - cfg.low_threashold));
    }

    for (i = 0; i < count; i++) {
        if (i < max) {
            if (i < GREEN_LEDS_THRESHOLD)
                RGB(pixels[i], 0x00, 0x0A, 0x00); /* green */
            else if (i < YELLOW_LEDS_THRESHOLD)
                RGB(pixels[i], 0x08, 0x08, 0x00); /* yellow */
            else
                RGB(pixels[i], 0x0A, 0x00, 0x00); /* red */
        } else {
            RGB(pixels[i], 0x00, 0x00, 0x00); /* off */
        }
    }

    LOG_DBG("Updating LEDs: cm=%d, max=%d", cm, max);
}

void
worker(struct k_timer *timer_id)
{
    measure_distance();
    ws2812_pwm_update(display);
}

void
stop_worker(struct k_timer *timer_id)
{
    k_timer_stop(&measure_timer);
    LOG_WRN("Measurement timeout reached, waiting for new data...");
    ws2812_clear();

}