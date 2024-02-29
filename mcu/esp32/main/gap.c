#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "driver/gptimer.h"
#include "esp_check.h"
#include "esp_err.h"

#include "gap.h"
#include "gatt_svr.h"

uint8_t addr_type;

static const char *TAG = "YV GAP";
static uint8_t ble_mode;

TaskHandle_t ble_deinit_timer_task_handle = NULL;
SemaphoreHandle_t ble_deinit_semaphore = NULL;
TickType_t ble_deinit_timeout_value = BLE_DEINIT_TIMEOUT_VALUE_MS;

int gap_event_handler(struct ble_gap_event *event, void *arg);
void reset_ble_deinit_timer(void);


void advertise() {
  struct ble_gap_adv_params adv_params;
  struct ble_hs_adv_fields fields;
  int rc;

  memset(&fields, 0, sizeof(fields));

  // flags: discoverability + BLE only
  fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

  // include power levels
  fields.tx_pwr_lvl_is_present = 1;
  fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

  // include device name
  fields.name = (uint8_t *)device_name;
  fields.name_len = strlen(device_name);
  fields.name_is_complete = 1;

  rc = ble_gap_adv_set_fields(&fields);
  if (rc != 0) {
    ESP_LOGE(LOG_TAG_GAP, "Error setting advertisement data: rc=%d", rc);
    return;
  }

  // start advertising
  memset(&adv_params, 0, sizeof(adv_params));
  adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
  adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
  rc = ble_gap_adv_start(addr_type, NULL, BLE_HS_FOREVER, &adv_params,
                         gap_event_handler, NULL);
  if (rc != 0) {
    ESP_LOGE(LOG_TAG_GAP, "Error enabling advertisement data: rc=%d", rc);
    return;
  }
}

void reset_cb(int reason) {
  ESP_LOGE(LOG_TAG_GAP, "BLE reset: reason = %d", reason);
}

void sync_cb(void) {
  // determine best adress type
  ble_hs_id_infer_auto(0, &addr_type);

  // start avertising
  advertise();
}

int gap_event_handler(struct ble_gap_event *event, void *arg) {
  switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
      // A new connection was established or a connection attempt failed
      ESP_LOGI(LOG_TAG_GAP, "GAP: Connection %s: status=%d",
               event->connect.status == 0 ? "established" : "failed",
               event->connect.status);
      reset_ble_deinit_timer();
      break;

    case BLE_GAP_EVENT_DISCONNECT:
      ESP_LOGI(LOG_TAG_GAP, "GAP: Disconnect: reason=%d\n",
               event->disconnect.reason);

      // Connection terminated; resume advertising
      advertise();
      break;

    case BLE_GAP_EVENT_ADV_COMPLETE:
      ESP_LOGI(LOG_TAG_GAP, "GAP: adv complete");
      advertise();
      break;

    case BLE_GAP_EVENT_SUBSCRIBE:
      ESP_LOGI(LOG_TAG_GAP, "GAP: Subscribe: conn_handle=%d",
               event->connect.conn_handle);
      break;

    case BLE_GAP_EVENT_MTU:
      ESP_LOGI(LOG_TAG_GAP, "GAP: MTU update: conn_handle=%d, mtu=%d",
               event->mtu.conn_handle, event->mtu.value);
      break;
  }

  return 0;
}

void host_task(void *param) {
  // returns only when nimble_port_stop() is executed
  nimble_port_run();
  nimble_port_freertos_deinit();
}
//--------------------------------------------------------------------------------------------------------
void ble_deinit_timer_task(void)
{
    int flag=0;
    while (1)
    {
        if (xSemaphoreTake(ble_deinit_semaphore, pdMS_TO_TICKS(BLE_DEINIT_TIMEOUT_VALUE_MS)) == pdTRUE)
        {
            ESP_LOGI(TAG, "BLE timeout not expired!...");
            ble_mode = BLE_MODE_ON;
            break;   
        } 
        ESP_LOGI(TAG, "BLE timeout expired!...");
        yv_ble_deinit();   
        ble_mode = BLE_MODE_OFF;
        break;      
    }
    vTaskDelete( ble_deinit_timer_task_handle );
    ble_deinit_timer_task_handle = NULL;
}
//--------------------------------------------------------------------------------------------------------
void reset_ble_deinit_timer(void)
{
    if (ble_deinit_timer_task_handle != NULL)
    {
      xSemaphoreGive(ble_deinit_semaphore);   
      ESP_LOGI(TAG, "Reset ble deinit timer");
    }
}
//--------------------------------------------------------------------------------------------------------
void start_ble_deinit_timer(void)
{ 
    //powerdown_timer_queue = xQueueCreate( POWERDOWN_TIMER_QUEUE_SIZE, POWERDOWN_TIMER_QUEUE_ELEM_SIZE );  
    //ble_deinit_timeout_value = BLE_DEINIT_TIMEOUT_VALUE_MS;
    //ESP_LOGI(TAG, "ble_deinit_timeout_value is %d", ble_deinit_timeout_value);
    ble_deinit_semaphore = xSemaphoreCreateBinary();
    xTaskCreate(ble_deinit_timer_task, "ble_deinit_timer_task", BLE_DEINIT_TIMER_TASK_STACK_SIZE, 
    NULL, BLE_DEINIT_TIMER_TASK_PRIORITY, &ble_deinit_timer_task_handle);
    //ESP_LOGI(TAG, "Start ble deinit timer");
}
//--------------------------------------------------------------------------------------------------------
void yv_ble_init(void)
{
  start_ble_deinit_timer();
  // register sync and reset callbacks
  ble_hs_cfg.sync_cb = sync_cb;
  ble_hs_cfg.reset_cb = reset_cb;
  // initialize service table
  ble_svc_gap_init();
  // here was gatt init
  // set device name and start host task
  ble_svc_gap_device_name_set(device_name);
  nimble_port_freertos_init(host_task);
  
}
//--------------------------------------------------------------------------------------------------------
void yv_ble_deinit(void)
{
    //nimble_port_stop();
    ESP_LOGI(TAG, "BLE deinit!");
    nimble_port_stop();
    nimble_port_deinit();
}
//--------------------------------------------------------------------------------------------------------
uint8_t yv_ble_get_mode(void)
{
    return ble_mode;
}
//--------------------------------------------------------------------------------------------------------
