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

#include "nvs_flash.h"

#include "yv_nvs.h"

//--------------------------------------------------------------------------------------------------------
void yv_nvs_init(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}
//--------------------------------------------------------------------------------------------------------
esp_err_t yv_nvs_read(uint8_t *pData, uint8_t DataLen, char *NVSKey)
{       
    nvs_handle_t my_handle;
    esp_err_t err;
    // Open
    printf("\n");
    printf("Opening Non-Volatile Storage (NVS) handle for read... ");
    err = nvs_open(USERDATA_STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return err;

    // Read run time blob
    // obtain required memory space to store blob being read from NVS
    err = nvs_get_blob(my_handle, NVSKey, pData, &DataLen);
    if (err != ESP_OK) return err;
    for (int i = 0; i < DataLen; i++) 
    {
        printf("%d: %" PRIu8 "\n", i + 1, *(pData+i));
    }
    // Close
    nvs_close(my_handle);
    printf("\n");
    printf("Read from Non-Volatile Storage (NVS) was succesful... ");
    printf("\n");
    return ESP_OK;
}
//--------------------------------------------------------------------------------------------------------
esp_err_t yv_nvs_write(uint8_t *pData, uint8_t DataLen, char *NVSKey)
{ 
    nvs_handle_t my_handle;
    esp_err_t err;

    // Open
    printf("\n");
    printf("Opening Non-Volatile Storage (NVS) handle for write... ");
    err = nvs_open(USERDATA_STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return err;

    // Read the size of memory space required for blob
    // size_t required_size = 0;  // value will default to 0, if not set yet in NVS
    // err = nvs_get_blob(my_handle, "run_time", NULL, &required_size);
    // if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;

    // Read previously saved blob if available
    // uint8_t* run_time = malloc(required_size + DataLen);
    // if (required_size > 0) {
    //     err = nvs_get_blob(my_handle, "run_time", run_time, &required_size);
    //     if (err != ESP_OK) {
    //         free(run_time);
    //         return err;
    //     }
    // }

    // Write value including previously saved blob if available
    //required_size = DataLen;
    //run_time[required_size / sizeof(uint8_t) - 1] = xTaskGetTickCount() * portTICK_PERIOD_MS;
    err = nvs_set_blob(my_handle, NVSKey, pData, DataLen);

    if (err != ESP_OK) return err;

    // Commit
    err = nvs_commit(my_handle);
    if (err != ESP_OK) return err;

    // Close
    nvs_close(my_handle);
    printf("\n");
    printf("Write to Non-Volatile Storage (NVS) was succesful... ");
    printf("\n");
    return ESP_OK;
}
//--------------------------------------------------------------------------------------------------------
int yv_nvs_get_size(char *Key)
{ 
    //size_t NVSGetSize(nvs_handle_t nvs, const char* key) {
    esp_err_t err;
    size_t valueSize = 0;   
    nvs_handle_t my_handle;   
    // Open
    printf("\n");
    printf("Opening Non-Volatile Storage (NVS) handle... \n");
    err = nvs_open(USERDATA_STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return err;

    if((err = nvs_get_str(my_handle, Key, NULL, &valueSize)) != ESP_OK) 
    {
        if(err == ESP_ERR_NVS_NOT_FOUND) {
            // Not found, no error
            return 0;
        } else 
        {
            // Actual error, log & return 0
            printf("Failed to get size of NVS key %s: %s\r\n", Key, esp_err_to_name(err));
            return 0;
        }
    }
    
    // Close
    nvs_close(my_handle);
    return valueSize;
}
