#include "mcu_protocol.h"
#include "device_mcu.h"
#include "xreal_air_devices.h"

#include "mcu_hid.h"

#include <stdlib.h>
#include <string.h>

static bool hid_open_impl(device_mcu_type* device, const struct mcu_hid_info* info) {
    mcu_hid_device* hid = (mcu_hid_device*) malloc(sizeof(mcu_hid_device));
    if (!hid) {
        return false;
    }

    memset(hid, 0, sizeof(mcu_hid_device));

    if (!mcu_hid_open(hid, info->product_id, info->interface_number, info->path)) {
        free(hid);
        return false;
    }

    device->handle = hid;

    device->activated = hid->activated;
    memcpy(device->mcu_app_fw_version, hid->mcu_app_fw_version, sizeof(device->mcu_app_fw_version));
    memcpy(device->dp_fw_version, hid->dp_fw_version, sizeof(device->dp_fw_version));
    memcpy(device->dsp_fw_version, hid->dsp_fw_version, sizeof(device->dsp_fw_version));
    device->active = hid->active;
    device->brightness = hid->brightness;
    device->disp_mode = hid->disp_mode;
    device->blend_state = hid->blend_state;
    device->control_mode = hid->control_mode;
    return true;
}

static void hid_close_impl(device_mcu_type* device) {
    mcu_hid_device* hid = (mcu_hid_device*) device->handle;
    if (hid) {
        mcu_hid_close(hid);
        free(hid);
        device->handle = NULL;
    }
}

static int hid_read_next(device_mcu_type* device, struct mcu_event* out, int timeout_ms) {
    mcu_hid_device* hid = (mcu_hid_device*) device->handle;

    mcu_hid_event e;
    memset(&e, 0, sizeof(e));

    int n = mcu_hid_read_next(hid, &e, timeout_ms);
    if (n <= 0) {
        return n;
    }

    out->timestamp = e.timestamp;
    out->event = e.event;
    out->brightness = e.brightness;
    strncpy(out->msg, e.msg, sizeof(out->msg) - 1);
    out->msg[sizeof(out->msg) - 1] = '\0';

    device->active = hid->active;
    device->brightness = hid->brightness;
    device->disp_mode = hid->disp_mode;
    device->blend_state = hid->blend_state;
    device->control_mode = hid->control_mode;
    return n;
}

const mcu_protocol mcu_protocol_hid = {
    .open = hid_open_impl,
    .close = hid_close_impl,
    .read_next = hid_read_next,
};

device_mcu_error_type device_mcu_poll_display_mode(device_mcu_type* device) {
    if ((!device) || (!device->handle)) {
        return DEVICE_MCU_ERROR_NO_HANDLE;
    }

    mcu_hid_device* hid = (mcu_hid_device*) device->handle;
    if (!mcu_hid_poll_display_mode(hid)) {
        return DEVICE_MCU_ERROR_PAYLOAD_FAILED;
    }

    device->disp_mode = hid->disp_mode;
    return DEVICE_MCU_ERROR_NO_ERROR;
}

device_mcu_error_type device_mcu_update_display_mode(device_mcu_type* device) {
    if ((!device) || (!device->handle)) {
        return DEVICE_MCU_ERROR_NO_HANDLE;
    }

    mcu_hid_device* hid = (mcu_hid_device*) device->handle;
    hid->disp_mode = device->disp_mode;
    if (!mcu_hid_update_display_mode(hid)) {
        return DEVICE_MCU_ERROR_PAYLOAD_FAILED;
    }

    return DEVICE_MCU_ERROR_NO_ERROR;
}

device_mcu_error_type device_mcu_update_firmware(device_mcu_type* device, const char* path) {
    if ((!device) || (!device->handle)) {
        return DEVICE_MCU_ERROR_NO_HANDLE;
    }

    mcu_hid_device* hid = (mcu_hid_device*) device->handle;
    return mcu_hid_update_firmware(hid, path)? DEVICE_MCU_ERROR_NO_ERROR : DEVICE_MCU_ERROR_PAYLOAD_FAILED;
}
