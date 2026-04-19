#include <zephyr/logging/log.h>
#include <zephyr/drivers/led_strip.h>

#include "ws2812.h"

LOG_MODULE_REGISTER(ws2812, LOG_LEVEL_DBG);

#define WS2812_PIN          20
#define PWM_TOP             20  /* 16MHz / 800kHz = 20 */
#define WS2812_BITS_PER_LED 24
#define STRIP_NUM_PIXELS    32
#define RESET_SLOTS         800 /* 16MHz / 20 = 800kHz; 800kHz = 1.25us; 50us / 1.25us = 40; 40 * 20 = 800 */

static nrfx_pwm_t pwm = NRFX_PWM_INSTANCE(NRF_PWM0);

static uint16_t pwm_seq[(STRIP_NUM_PIXELS * WS2812_BITS_PER_LED) + RESET_SLOTS];
static struct led_rgb pixels[STRIP_NUM_PIXELS];

static void encode_byte(uint8_t byte, uint16_t *buf, int *idx);

int
ws2812_pwm_init(void)
{
    int err = -1;
    nrfx_pwm_config_t config = {
        .output_pins = {
            WS2812_PIN,
            NRF_PWM_PIN_NOT_CONNECTED,
            NRF_PWM_PIN_NOT_CONNECTED,
            NRF_PWM_PIN_NOT_CONNECTED
        },
        .irq_priority = 6,
        .base_clock = NRF_PWM_CLK_16MHz,
        .count_mode = NRF_PWM_MODE_UP,
        .top_value = PWM_TOP,
        .load_mode    = NRF_PWM_LOAD_COMMON,
        .step_mode    = NRF_PWM_STEP_AUTO,
    };

    err = nrfx_pwm_init(&pwm, &config, NULL, NULL);
    if (err) {
        LOG_ERR("Failed to initialize PWM: %d", err);
        goto out;
    }

    err = 0;

    irq_disable(PWM0_IRQn);
out:
    return err;
}

void
ws2812_pwm_update(ws2812_update_cb_t update_cb)
{
    int idx = 0;

    if (update_cb != NULL) {
        LOG_DBG("Updating WS2812 LEDs");
        update_cb(pixels, STRIP_NUM_PIXELS);
    }

    for (size_t i = 0; i < STRIP_NUM_PIXELS; i++) {
        /* WS2812 expects GRB order */
        encode_byte(pixels[i].g, pwm_seq, &idx);
        encode_byte(pixels[i].r, pwm_seq, &idx);
        encode_byte(pixels[i].b, pwm_seq, &idx);
    }

    /* Reset signal (low) */
    for (int i = 0; i < RESET_SLOTS; i++) {
        pwm_seq[idx++] = (1 << 15);
    }

    nrf_pwm_sequence_t const seq = {
        .values.p_common = pwm_seq,
        .length = NRF_PWM_VALUES_LENGTH(pwm_seq),
        .repeats = 0,
        .end_delay = 0
    };

    nrfx_pwm_simple_playback(&pwm, &seq, 1, 0);
}

void
ws2812_clear(void)
{
    LOG_DBG("Clearing WS2812 LEDs");
    for (size_t i = 0; i < STRIP_NUM_PIXELS; i++) {
        pixels[i].r = 0;
        pixels[i].g = 0;
        pixels[i].b = 0;
    }
    ws2812_pwm_update(NULL);
}

static void
encode_byte(uint8_t byte, uint16_t *buf, int *idx)
{
    for (int i = 7; i >= 0; i--) {
        if (byte & (1 << i)) {
            buf[(*idx)++] = (1 << 15) | 14; // ~0.9µs HIGH
        } else {
            buf[(*idx)++] = (1 << 15) | 6; // ~0.35µs HIGH
        }
    }
}