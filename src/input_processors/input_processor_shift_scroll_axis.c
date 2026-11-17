/*
 * Shift なし＝縦スクロールだけ、Shift あり＝横スクロールだけ（11/17 田上さん依頼）。
 * xy_to_scroll_mapper の後に置く（その時点で X→HWHEEL、Y→WHEEL になっている）。
 */
#define DT_DRV_COMPAT zmk_input_processor_shift_scroll_axis

#include <zephyr/device.h>
#include <zephyr/input/input.h>
#include <drivers/input_processor.h>
#include <dt-bindings/zmk/modifiers.h>
#include <zmk/hid.h>

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static int shift_scroll_axis_handle_event(const struct device *dev, struct input_event *event,
                                          uint32_t param1, uint32_t param2,
                                          struct zmk_input_processor_state *state) {
    if (event->type != INPUT_EV_REL) {
        return ZMK_INPUT_PROC_CONTINUE;
    }
    bool shift = (zmk_hid_get_explicit_mods() & (MOD_LSFT | MOD_RSFT)) != 0;
    if ((event->code == INPUT_REL_WHEEL && shift) || (event->code == INPUT_REL_HWHEEL && !shift)) {
        event->value = 0;
    }
    return ZMK_INPUT_PROC_CONTINUE;
}

static const struct zmk_input_processor_driver_api shift_scroll_axis_driver_api = {
    .handle_event = shift_scroll_axis_handle_event,
};

#define SHIFT_SCROLL_AXIS_INST(n)                                                                  \
    DEVICE_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                                  \
                          CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &shift_scroll_axis_driver_api);

DT_INST_FOREACH_STATUS_OKAY(SHIFT_SCROLL_AXIS_INST)

#endif
