/* 診断用（パネル版）: P1.10(42,A) / P1.02(34,B) / P1.11(43,MOTION) / P1.09(41) の論理レベルを 1ms ごとに読み、変化したときだけログに出す */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(pin_poll, LOG_LEVEL_INF);

static const struct device *gpio1 = DEVICE_DT_GET(DT_NODELABEL(gpio1));
static int last = -1;
static uint32_t changes;

static void poll_fn(struct k_work *w);
static K_WORK_DELAYABLE_DEFINE(poll_work, poll_fn);

static void poll_fn(struct k_work *w) {
    gpio_port_value_t v;
    if (gpio_port_get_raw(gpio1, &v) == 0) {
        int cur = (((v >> 9) & 1) << 3) | (((v >> 10) & 1) << 2) | (((v >> 2) & 1) << 1) | ((v >> 11) & 1);
        if (cur != last) {
            changes++;
            if (changes < 2000) {
                LOG_INF("A42=%d B34=%d MOTION43=%d P41=%d changes=%u", (cur >> 2) & 1, (cur >> 1) & 1, cur & 1, (cur >> 3) & 1, changes);
            }
            last = cur;
        }
    }
    k_work_reschedule(&poll_work, K_MSEC(1));
}

static void hb_fn(struct k_work *w);
static K_WORK_DELAYABLE_DEFINE(hb_work, hb_fn);
static void hb_fn(struct k_work *w) {
    LOG_INF("heartbeat: A42=%d B34=%d MOTION43=%d P41=%d changes=%u", (last >> 2) & 1, (last >> 1) & 1, last & 1, (last >> 3) & 1, changes);
    k_work_reschedule(&hb_work, K_SECONDS(5));
}

static int pin_poll_init(void) {
    k_work_reschedule(&poll_work, K_MSEC(100));
    k_work_reschedule(&hb_work, K_SECONDS(5));
    return 0;
}
SYS_INIT(pin_poll_init, APPLICATION, 99);
