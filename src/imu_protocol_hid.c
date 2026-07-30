#include "imu_protocol.h"
#include "device_imu.h"

#include "imu_hid.h"

#include <stdlib.h>
#include <string.h>

static bool hid_open_impl(device_imu_type* device, const struct imu_hid_info* info) {
    imu_hid_device* hid = (imu_hid_device*) malloc(sizeof(imu_hid_device));
    if (!hid) {
        return false;
    }

    memset(hid, 0, sizeof(imu_hid_device));

    if (!imu_hid_open(hid, info->product_id, info->interface_number, info->path)) {
        free(hid);
        return false;
    }

    device->handle = hid;
    device->max_payload_size = hid->max_payload_size;
    return true;
}

static void hid_close_impl(device_imu_type* device) {
    imu_hid_device* hid = (imu_hid_device*) device->handle;
    if (hid) {
        imu_hid_close(hid);
        free(hid);
        device->handle = NULL;
    }
}

static bool hid_start_stream(device_imu_type* dev) {
    return imu_hid_start_stream((imu_hid_device*) dev->handle);
}

static bool hid_stop_stream(device_imu_type* dev) {
    return imu_hid_stop_stream((imu_hid_device*) dev->handle);
}

static bool hid_get_static_id(device_imu_type* dev, uint32_t* out_id) {
    return imu_hid_get_static_id((imu_hid_device*) dev->handle, out_id);
}

static bool hid_load_calibration_json(device_imu_type* dev, uint32_t* len, char** data) {
    return imu_hid_load_calibration_json((imu_hid_device*) dev->handle, len, data);
}

static int hid_next_sample(device_imu_type* device, struct imu_sample* out, int timeout_ms) {
    imu_hid_sample s;
    int n = imu_hid_next_sample((imu_hid_device*) device->handle, &s, timeout_ms);
    if (n <= 0) {
        return n;
    }

    out->gx = s.gx; out->gy = s.gy; out->gz = s.gz;
    out->ax = s.ax; out->ay = s.ay; out->az = s.az;
    out->mx = s.mx; out->my = s.my; out->mz = s.mz;
    out->temperature_c = s.temperature_c;
    out->timestamp_ns = s.timestamp_ns;
    out->flags = s.flags;
    return n;
}

const imu_protocol imu_protocol_hid = {
    .open = hid_open_impl,
    .close = hid_close_impl,
    .start_stream = hid_start_stream,
    .stop_stream = hid_stop_stream,
    .get_static_id = hid_get_static_id,
    .load_calibration_json = hid_load_calibration_json,
    .next_sample = hid_next_sample,
};
