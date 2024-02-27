#pragma once

#include "esp_ota_ops.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#define LOG_TAG_GATT_SVR "gatt_svr"
#define REBOOT_DEEP_SLEEP_TIMEOUT 500
#define GATT_DEVICE_INFO_UUID 0x180A
#define GATT_MANUFACTURER_NAME_UUID 0x2A29
#define GATT_MODEL_NUMBER_UUID 0x2A24

#define GATT_DATA_ARR_MAX_LEN_BYTES (16)

typedef enum {
  SVR_CHR_OTA_CONTROL_NOP,
  SVR_CHR_OTA_CONTROL_REQUEST,
  SVR_CHR_OTA_CONTROL_REQUEST_ACK,
  SVR_CHR_OTA_CONTROL_REQUEST_NAK,
  SVR_CHR_OTA_CONTROL_DONE,
  SVR_CHR_OTA_CONTROL_DONE_ACK,
  SVR_CHR_OTA_CONTROL_DONE_NAK,
} svr_chr_ota_control_val_t;

// service: OTA Service
// d6f1d96d-594c-4c53-b1c6-244a1dfde6d8
static const ble_uuid128_t gatt_svr_svc_ota_uuid =
    BLE_UUID128_INIT(0xd8, 0xe6, 0xfd, 0x1d, 0x4a, 024, 0xc6, 0xb1, 0x53, 0x4c,
                     0x4c, 0x59, 0x6d, 0xd9, 0xf1, 0xd6);

// characteristic: OTA Control
// 7ad671aa-21c0-46a4-b722-270e3ae3d830
static const ble_uuid128_t gatt_svr_chr_ota_control_uuid =
    BLE_UUID128_INIT(0x30, 0xd8, 0xe3, 0x3a, 0x0e, 0x27, 0x22, 0xb7, 0xa4, 0x46,
                     0xc0, 0x21, 0xaa, 0x71, 0xd6, 0x7a);

// characteristic: OTA Data
// 23408888-1f40-4cd8-9b89-ca8d45f8a5b0
static const ble_uuid128_t gatt_svr_chr_ota_data_uuid =
    BLE_UUID128_INIT(0xb0, 0xa5, 0xf8, 0x45, 0x8d, 0xca, 0x89, 0x9b, 0xd8, 0x4c,
                     0x40, 0x1f, 0x88, 0x88, 0x40, 0x23);


// service: WiFi Service
//bee983b1-1ea5-4c80-874b-2ba23ffd3770
static const ble_uuid128_t gatt_svr_svc_wifi_uuid =
    BLE_UUID128_INIT(0x70, 0x37, 0xfd, 0x3f, 0xa2, 0x2b, 0x4b, 0x87, 0x80, 0x4c,
                      0xa5, 0x1e, 0xb1, 0x83, 0xe9, 0xbe);

// characteristic: WiFi data (SSID)
// cc08d0bd-9044-4cd2-85f5-a488bd4412c8
static const ble_uuid128_t gatt_svr_chr_wifi_ssid_data_uuid =
    BLE_UUID128_INIT(0xc8, 0x12, 0x44, 0xbd, 0x88, 0xa4, 0xf5, 0x85, 0xd2, 0x4c,
                     0x44, 0x90, 0xbd, 0xd0, 0x08, 0xcc);

// characteristic: WiFi data (Pass)
// e10334f1-22f8-4b97-bcc6-ebd39a6f8e49
static const ble_uuid128_t gatt_svr_chr_wifi_pass_data_uuid =
    BLE_UUID128_INIT(0x49, 0x8e, 0x6f, 0x9a, 0xd3, 0xeb, 0xc6, 0xbc, 0x97, 0x4b,
                     0xf8, 0x22, 0xf1, 0x34, 0x03, 0xe1);

// characteristic: firmware version
// 3ae1a84d-ddd3-4ef1-922a-70921cc5a914
static const ble_uuid128_t gatt_svr_chr_firmware_ver_uuid =
    BLE_UUID128_INIT(0x14, 0xa9, 0xc5, 0x1c, 0x92, 0x70, 0x2a, 0x92, 0xf1, 0x4e,
                     0xd3, 0xdd, 0x4d, 0xa8, 0xe1, 0x3a);
                                         

void gatt_svr_init();
