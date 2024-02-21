#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "esp_timer.h"
//#include "esp_adc/adc_oneshot.h"

//#include "lwip/err.h"
//#include "lwip/sys.h"


#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "driver/gptimer.h"

#include "i2s.h"
#include "adc.h"
#include "wifi.h"
#include "udp.h"
#include "ota.h"
#include "nvs.h"


#include "esp_check.h"


static const char *TAG = "YV Main";



void app_main(void) {
    //non volatile flash init
    yv_nvs_init();
    //
    yv_ota_init();

 

   while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

}

//--------------------------------------------------------------------------------------------------------
// void app_main(void)
// {
//     //wifi_ap_record_t info;

//     //Initialize NVS
//     esp_err_t ret = nvs_flash_init();
//     if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
//     {
//         ESP_ERROR_CHECK(nvs_flash_erase());
//         ret = nvs_flash_init();
//     }
//     ESP_ERROR_CHECK(ret);

 
//     // queue_i2s_size = xQueueCreate(1, I2S_QUEUE_SIZE_BYTES);
//     // if (!queue_i2s_size) 
//     // {
//     //     ESP_LOGE(TAG, "Creating i2s queue failed");
//     //     return;
//     // }

//     // queue_i2s_to_udp = xQueueCreate(1, sizeof(i2s_event_data_t ));
//     // if (!queue_i2s_to_udp) 
//     // {
//     //     ESP_LOGE(TAG, "Creating i2s queue failed");
//     //     return;
//     // }


//     // wifi_init_sta(); 
//     // //YV_TimerInit();

//     // /* Initialize i2s peripheral */
//     // if (i2s_driver_init() != ESP_OK) {
//     //     ESP_LOGE(TAG, "i2s driver init failed");
//     //     abort();
//     // } else {
//     //     ESP_LOGI(TAG, "i2s driver init success");
//     // }   


//     // xTaskCreate(udp_task, "udp_task", 4096, queue_i2s_to_udp, 4, NULL);
//     // xTaskCreate(vTask_ReadDataFromI2S, "vTask_ReadDataFromI2S", 8192, NULL, 5, &xTask_ReadDataFromI2S); 

//     while (1)
//     {
//         // //ESP_LOGI(TAG, "Heap free size:%d", xPortGetFreeHeapSize());
//         // ret = esp_wifi_sta_get_ap_info(&info);
//         // //ESP_LOGI(TAG, "esp_wifi_sta_get_ap_info: 0x%04x", ret);
//         // if(ret==0) 
//         // {
//         //     ESP_LOGI(TAG, "SSID: %s", info.ssid);
//         // }
//         // else 
//         // {
//         //     wifi_init_sta();
//         // }
//         vTaskDelay(pdMS_TO_TICKS(2000));
//         ESP_LOGI(TAG, "version");
//     }
//     ESP_LOGI(TAG, "Main exit");
// }


// //----------------------------------------------------------------------------------------------------------------------------------
// void app_main(void)
// {
//     wifi_ap_record_t info;

//     //Initialize NVS
//     esp_err_t ret = nvs_flash_init();
//     if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
//     {
//         ESP_ERROR_CHECK(nvs_flash_erase());
//         ret = nvs_flash_init();
//     }
//     ESP_ERROR_CHECK(ret);

//     //queue_adc_raw = xQueueCreate(1, ADC_DATA_SIZE_BYTES);
//     //if (!queue_adc_raw) {
//     //    ESP_LOGE(TAG, "Creating queue failed");
//     //    return;
//     //}

//     queue_proc_data = xQueueCreate(1, ADC_DATA_SIZE_BYTES);
//     if (!queue_proc_data) {
//         ESP_LOGE(TAG, "Creating queue failed");
//         return;
//     }
//     //create semaphore for adc reading and reloading out of the timer ISR
//     TimerToADC_Semaphore = xSemaphoreCreateBinary();
//     ADCToUDP_Semaphore = xSemaphoreCreateBinary();

//     //create ADC Reload Task with high priority 5
//     ESP_LOGI(TAG, "Create ADC Read_Reload Task");
//     xTaskCreate( vTask_ADCReadReload, "Task ADCReadReload", 2*configMINIMAL_STACK_SIZE, NULL, 5, &xTask_ADCReadReloadHandle ); 
    
//     YV_TimerInit();
//     YV_ADCInit();
//     wifi_init_sta();

//     xTaskCreate(udp_task, "udp_task", 4096, queue_proc_data, 4, NULL);
      
//     while (1)
//     {
//         //ESP_LOGI(TAG, "Heap free size:%d", xPortGetFreeHeapSize());
//         ret = esp_wifi_sta_get_ap_info(&info);
//         //ESP_LOGI(TAG, "esp_wifi_sta_get_ap_info: 0x%04x", ret);
//         if(ret==0) 
//         {
//             //ESP_LOGI(TAG, "SSID: %s", info.ssid);
//         }
//         else 
//         {
//             wifi_init_sta();
//         }
//         vTaskDelay(pdMS_TO_TICKS(2000));
//     }
//     // ESP_LOGI(TAG, "Stop timer");
//     // ESP_ERROR_CHECK(gptimer_stop(gptimer));

//     // ESP_LOGI(TAG, "Disable timer");
//     // ESP_ERROR_CHECK(gptimer_disable(gptimer));
    
//     // ESP_LOGI(TAG, "Delete timer");
//     // ESP_ERROR_CHECK(gptimer_del_timer(gptimer));

//     // vQueueDelete(queue);
// }