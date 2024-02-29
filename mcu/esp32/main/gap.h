#pragma once

#include "esp_log.h"
#include "esp_nimble_hci.h"
#include "host/ble_hs.h"
#include "nimble/ble.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"

#define LOG_TAG_GAP "gap"

static const char device_name[] = "esp32";

#define BLE_DEINIT_TIMEOUT_VALUE_MS (7000)
#define BLE_DEINIT_TIMER_TASK_STACK_SIZE (4096)
#define BLE_DEINIT_TIMER_TASK_PRIORITY (2)

#define BLE_MODE_ON (1)
#define BLE_MODE_OFF (0)

void advertise();
void reset_cb(int reason);
void sync_cb(void);
void host_task(void *param);

void yv_ble_init(void);
void yv_ble_deinit(void);
uint8_t yv_ble_get_mode(void);
