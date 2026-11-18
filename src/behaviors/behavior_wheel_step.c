/*
 * 押すたびにホイールを param1（MOVE_X/MOVE_Y で符号化）だけ1回だけ送る behavior。
 * &msc は速度×時間で動くため、エンコーダの短いタップではタイミング次第で 0〜2 段にぶれる。
 * こちらはタイマーを使わず、1クリック=指定段数をそのまま送る（2026-11-18）。
 */
#define DT_DRV_COMPAT zmk_behavior_wheel_step

#include <zephyr/device.h>
#include <zephyr/input/input.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <dt-bindings/zmk/pointing.h>

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static int wheel_step_pressed(struct zmk_behavior_binding *binding,
                              struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    int16_t x = MOVE_X_DECODE(binding->param1);
    int16_t y = MOVE_Y_DECODE(binding->param1);

    if (x != 0) {
        input_report_rel(dev, INPUT_REL_HWHEEL, x, y == 0, K_FOREVER);
    }
    if (y != 0) {
        input_report_rel(dev, INPUT_REL_WHEEL, y, true, K_FOREVER);
    }
    return ZMK_BEHAVIOR_OPAQUE;
}

static int wheel_step_released(struct zmk_behavior_binding *binding,
                               struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api wheel_step_api = {
    .binding_pressed = wheel_step_pressed,
    .binding_released = wheel_step_released,
};

#define WS_INST(n)                                                                                 \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                                \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &wheel_step_api);

DT_INST_FOREACH_STATUS_OKAY(WS_INST)

#endif
