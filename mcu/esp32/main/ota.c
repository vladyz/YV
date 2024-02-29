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

#include "esp_ota_ops.h"
#include "esp_partition.h"


#include "ota.h"
#include "gap.h"
//#include "gatt_svr.h"

static const char *TAG= "YV OTA";

//--------------------------------------------------------------------------------------------------------
bool run_diagnostics() 
{
    // do some diagnostics
    return true;
}
//--------------------------------------------------------------------------------------------------------
void yv_ota_check_firmware(void)
{
    // check which partition is running
    const esp_partition_t *partition = esp_ota_get_running_partition();

    switch (partition->address) 
    {
        //case MEM_OFFSET_FACTORY:
        //    ESP_LOGI(TAG, "Running partition: factory");
        //   break;
        case MEM_OFFSET_OTA0:
            ESP_LOGI(TAG, "Running partition: ota_0");
            break;
        case MEM_OFFSET_OTA1:
            ESP_LOGI(TAG, "Running partition: ota_1");
            break;

        default:
            ESP_LOGE(TAG, "Running partition: unknown");
            break;
    }

    // check if an OTA has been done, if so run diagnostics
    esp_ota_img_states_t ota_state;

    if (esp_ota_get_state_partition(partition, &ota_state) == ESP_OK) 
    {
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) 
        {
            ESP_LOGI(TAG, "An OTA update has been detected.");
            if (run_diagnostics()) 
            {
                ESP_LOGI(TAG,
                "Diagnostics completed successfully! Continuing execution.");
                esp_ota_mark_app_valid_cancel_rollback();
            } 
            else
            {
                ESP_LOGE(TAG,
                    "Diagnostics failed! Start rollback to the previous version.");
                esp_ota_mark_app_invalid_rollback_and_reboot();
            }
        }
    }
}
 
