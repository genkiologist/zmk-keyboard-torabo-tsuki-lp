/*
 * sensor-rotate の改造版（2026-11-18）。
 * Helix の SIQ エンコーダは、止まる位置が回した向きで1ステップずれる（順方向と逆方向で静止時の A/B 状態が違う）。
 * 標準の sensor-rotate は 24°（=4ステップ）ごとに切り捨てで数えるため、向きを変えた最初の1クリックが落ちる。
 * ここでは
 *   ・逆向きの値が来たら繰り越し分を捨てて数え直す
 *   ・しきい値を 1クリックの 3/4（3ステップ）にし、発火したら 1クリック分（4ステップ）を引く
 * ことで、1クリックが 3〜5 ステップにぶれても必ず1回だけ発火させる。
 */
#define DT_DRV_COMPAT zmk_behavior_sensor_rotate_detent

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/behavior_queue.h>
#include <zmk/keymap.h>
#include <zmk/sensors.h>
#include <zmk/virtual_key_position.h>
#include <zmk/events/position_state_changed.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

struct srd_config {
    struct zmk_behavior_binding cw_binding;
    struct zmk_behavior_binding ccw_binding;
    int tap_ms;
};

struct srd_data {
    int acc[ZMK_KEYMAP_SENSORS_LEN][ZMK_KEYMAP_LAYERS_LEN];
    int8_t last_dir[ZMK_KEYMAP_SENSORS_LEN][ZMK_KEYMAP_LAYERS_LEN];
    int triggers[ZMK_KEYMAP_SENSORS_LEN][ZMK_KEYMAP_LAYERS_LEN];
};

static int srd_accept_data(struct zmk_behavior_binding *binding,
                           struct zmk_behavior_binding_event event,
                           const struct zmk_sensor_config *sensor_config, size_t channel_data_size,
                           const struct zmk_sensor_channel_data *channel_data) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    struct srd_data *data = dev->data;
    const struct sensor_value value = channel_data[0].value;
    int idx = ZMK_SENSOR_POSITION_FROM_VIRTUAL_KEY_POSITION(event.position);
    int *acc = &data->acc[idx][event.layer];

    int delta = value.val1; /* 度。val2（百万分の1度）は EC11 では 0 */
    int step = 360 / sensor_config->triggers_per_rotation;
    int threshold = step * 3 / 4;
    int triggers = 0;

    if (delta == 0) {
        data->triggers[idx][event.layer] = 0;
        return 0;
    }

    /* 逆向きに回し始めたら繰り越しを捨てる。
       判定は「前回の回転の向き」で行う（acc の符号で判定すると、発火後に負になった繰り越しまで
       捨ててしまい、1クリックで2回発火することがある。11/18 実測で奥5クリック→6段） */
    int8_t dir = delta > 0 ? 1 : -1;
    int8_t *last = &data->last_dir[idx][event.layer];
    if (*last != 0 && dir != *last) {
        *acc = 0;
    }
    *last = dir;
    *acc += delta;

    while (*acc >= threshold) {
        *acc -= step;
        triggers++;
    }
    while (*acc <= -threshold) {
        *acc += step;
        triggers--;
    }

    LOG_DBG("detent: delta %d acc %d triggers %d", delta, *acc, triggers);
    data->triggers[idx][event.layer] = triggers;
    return 0;
}

static int srd_process(struct zmk_behavior_binding *binding, struct zmk_behavior_binding_event event,
                       enum behavior_sensor_binding_process_mode mode) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct srd_config *cfg = dev->config;
    struct srd_data *data = dev->data;
    int idx = ZMK_SENSOR_POSITION_FROM_VIRTUAL_KEY_POSITION(event.position);

    if (mode != BEHAVIOR_SENSOR_BINDING_PROCESS_MODE_TRIGGER) {
        data->triggers[idx][event.layer] = 0;
        return ZMK_BEHAVIOR_TRANSPARENT;
    }

    int triggers = data->triggers[idx][event.layer];
    struct zmk_behavior_binding b;
    if (triggers > 0) {
        b = cfg->cw_binding;
    } else if (triggers < 0) {
        triggers = -triggers;
        b = cfg->ccw_binding;
    } else {
        return ZMK_BEHAVIOR_TRANSPARENT;
    }

#if IS_ENABLED(CONFIG_ZMK_SPLIT)
    event.source = ZMK_POSITION_STATE_CHANGE_SOURCE_LOCAL;
#endif

    for (int i = 0; i < triggers; i++) {
        zmk_behavior_queue_add(&event, b, true, cfg->tap_ms);
        zmk_behavior_queue_add(&event, b, false, 0);
    }
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api srd_api = {
    .sensor_binding_accept_data = srd_accept_data,
    .sensor_binding_process = srd_process,
};

#define _SRD_ENTRY(idx, n)                                                                         \
    {                                                                                              \
        .behavior_dev = DEVICE_DT_NAME(DT_INST_PHANDLE_BY_IDX(n, bindings, idx)),                  \
        .param1 = COND_CODE_0(DT_INST_PHA_HAS_CELL_AT_IDX(n, bindings, idx, param1), (0),          \
                              (DT_INST_PHA_BY_IDX(n, bindings, idx, param1))),                     \
        .param2 = COND_CODE_0(DT_INST_PHA_HAS_CELL_AT_IDX(n, bindings, idx, param2), (0),          \
                              (DT_INST_PHA_BY_IDX(n, bindings, idx, param2))),                     \
    }

#define SRD_INST(n)                                                                                \
    static struct srd_config srd_config_##n = {                                                    \
        .cw_binding = _SRD_ENTRY(0, n),                                                            \
        .ccw_binding = _SRD_ENTRY(1, n),                                                           \
        .tap_ms = DT_INST_PROP_OR(n, tap_ms, 5),                                                   \
    };                                                                                             \
    static struct srd_data srd_data_##n = {};                                                      \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, &srd_data_##n, &srd_config_##n, POST_KERNEL,            \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &srd_api);

DT_INST_FOREACH_STATUS_OKAY(SRD_INST)

#endif
