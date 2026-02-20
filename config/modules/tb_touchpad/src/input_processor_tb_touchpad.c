/*
 * Copyright (c) 2026
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_input_processor_tb_touchpad

#include <zephyr/device.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <drivers/input_processor.h>
#include <zmk/endpoints.h>
#include <zmk/hid.h>

#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define TB_TOUCHPAD_CONTACTS ZMK_HID_TOUCHPAD_CONTACT_MAX
#define TB_TOUCHPAD_COORD_MIN 0
#define TB_TOUCHPAD_COORD_MID (ZMK_HID_TOUCHPAD_COORD_MAX / 2)

struct tb_touchpad_runtime {
    int32_t x[TB_TOUCHPAD_CONTACTS];
    int32_t y[TB_TOUCHPAD_CONTACTS];
    int64_t last_sync_ms[TB_TOUCHPAD_CONTACTS];
    bool active[TB_TOUCHPAD_CONTACTS];
};

static struct tb_touchpad_runtime runtime = {
    .x = {TB_TOUCHPAD_COORD_MID, TB_TOUCHPAD_COORD_MID},
    .y = {TB_TOUCHPAD_COORD_MID, TB_TOUCHPAD_COORD_MID},
    .last_sync_ms = {0, 0},
    .active = {false, false},
};

static inline uint16_t clamp_coord(int32_t v) {
    if (v < TB_TOUCHPAD_COORD_MIN) {
        return TB_TOUCHPAD_COORD_MIN;
    }

    if (v > ZMK_HID_TOUCHPAD_COORD_MAX) {
        return ZMK_HID_TOUCHPAD_COORD_MAX;
    }

    return (uint16_t)v;
}

static void emit_touchpad_report(uint8_t updated_contact) {
    int64_t now = k_uptime_get();
    runtime.active[updated_contact] = true;
    runtime.last_sync_ms[updated_contact] = now;

    for (uint8_t i = 0; i < TB_TOUCHPAD_CONTACTS; i++) {
        if (runtime.active[i] &&
            (now - runtime.last_sync_ms[i]) > CONFIG_ZMK_INPUT_PROCESSOR_TB_TOUCHPAD_ACTIVE_MS) {
            runtime.active[i] = false;
        }

        if (runtime.active[i]) {
            zmk_hid_touchpad_contact_set(i, true, true, clamp_coord(runtime.x[i]),
                                         clamp_coord(runtime.y[i]));
        } else {
            zmk_hid_touchpad_contact_clear(i);
        }
    }

    (void)zmk_endpoint_send_touchpad_report();
}

static int tb_touchpad_handle_event(const struct device *dev, struct input_event *event,
                                    uint32_t param1, uint32_t param2,
                                    struct zmk_input_processor_state *state) {
    ARG_UNUSED(dev);
    ARG_UNUSED(state);

    if (!IS_ENABLED(CONFIG_ZMK_POINTING_HID_TOUCHPAD)) {
        return ZMK_INPUT_PROC_CONTINUE;
    }

    if (event->type != INPUT_EV_REL) {
        return ZMK_INPUT_PROC_CONTINUE;
    }

    uint8_t contact_id =
        (param1 < TB_TOUCHPAD_CONTACTS) ? (uint8_t)param1 : (TB_TOUCHPAD_CONTACTS - 1);
    int32_t gain = (param2 > 0) ? (int32_t)param2 : 1;

    switch (event->code) {
    case INPUT_REL_X:
        runtime.x[contact_id] += ((int32_t)event->value) * gain;
        break;
    case INPUT_REL_Y:
        runtime.y[contact_id] += ((int32_t)event->value) * gain;
        break;
    default:
        return ZMK_INPUT_PROC_CONTINUE;
    }

    if (event->sync) {
        emit_touchpad_report(contact_id);
    }

    // We handled this event; suppress default mouse processing.
    return ZMK_INPUT_PROC_STOP;
}

static const struct zmk_input_processor_driver_api tb_touchpad_driver_api = {
    .handle_event = tb_touchpad_handle_event,
};

#define TB_TOUCHPAD_INST(n)                                                                       \
    DEVICE_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                                 \
                          CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &tb_touchpad_driver_api);

DT_INST_FOREACH_STATUS_OKAY(TB_TOUCHPAD_INST)
