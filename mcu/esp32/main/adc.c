#include <inttypes.h>
#include <stdio.h>
#include "adc.h"

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "driver/gptimer.h"
#include "esp_check.h"

#include "esp_adc/adc_oneshot.h"



SemaphoreHandle_t TimerToADC_Semaphore = NULL;
QueueHandle_t queue_adc_raw = NULL;
QueueHandle_t queue_proc_data = NULL;

adc_oneshot_unit_handle_t adc1_handle = NULL;
adc_cali_handle_t adc1_cali_chan0_handle = NULL;
TaskHandle_t xTask_ADCReadReloadHandle = NULL;
SemaphoreHandle_t ADCToUDP_Semaphore = NULL;

//int data_proc[DATA_TO_SEND_LEN_VALUES*2];
uint16_t data_idx=0;

static const char *TAG = "YV ADC";

//callback of the timer interrupt. This code is pending within the timer ISR--------------------------------------------------------
static bool IRAM_ATTR example_timer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
    BaseType_t high_task_awoken = pdFALSE;
    
    xSemaphoreGiveFromISR( TimerToADC_Semaphore, &high_task_awoken );
    //xSemaphoreGiveFromISR( ReadDataFromI2SToUDP_Semaphore, &high_task_awoken );
    
    // return whether we need to yield at the end of ISR
    return (high_task_awoken == pdTRUE);
}
//init hardware timer---------------------------------------------------------------------------------------------------------------
void yv_adc_timer_init(void)
{
    //create and init timer
    ESP_LOGI(TAG, "Create timer handle");
    gptimer_handle_t gptimer = NULL;
    gptimer_config_t timer_config = 
    {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1MHz, 1 tick=1us
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));  

    // set a new callback function for timer alarm interrupt
    gptimer_event_callbacks_t cbs = 
    {
        .on_alarm = example_timer_on_alarm_cb,
    }; 
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, queue_adc_raw));
    ESP_LOGI(TAG, "Enable timer");
    ESP_ERROR_CHECK(gptimer_enable(gptimer));

    ESP_LOGI(TAG, "Start timer, auto-reload at alarm event");
    gptimer_alarm_config_t alarm_config2 = 
    {
        .reload_count = 0,
        .alarm_count = 2500, // period = 2.5ms 
        .flags.auto_reload_on_alarm = true,
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config2));
    ESP_ERROR_CHECK(gptimer_start(gptimer));
}


// //ADC calibration-------------------------------------------------------------------------------------------------------------------
// //adc_cali_schemes.h: ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED  is  1
// static bool YV_ADC_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
// {
//     adc_cali_handle_t handle = NULL;
//     esp_err_t ret = ESP_FAIL;
//     bool calibrated = false;

//     ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
//     adc_cali_line_fitting_config_t cali_config =
//     {
//         .unit_id = unit,
//         .atten = atten,
//         .bitwidth = ADC_BITWIDTH_DEFAULT,
//     };
//     ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
//     if (ret == ESP_OK) 
//     {
//         calibrated = true;
//     }

//     *out_handle = handle;
//     if (ret == ESP_OK) {
//         ESP_LOGI(TAG, "Calibration Success");
//     } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
//         ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
//     } else {
//         ESP_LOGE(TAG, "Invalid arg or no memory");
//     }
//     return calibrated;
// }
// //init hardware ADC-----------------------------------------------------------------------------------------------------------------
// static void YV_ADCInit(void)
// {
//     //-------------ADC1 Init---------------//
//     adc_oneshot_unit_init_cfg_t init_config1 = {
//         .unit_id = ADC_UNIT_1,
//     };
//     ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

//     //-------------ADC1 Config---------------//
//     adc_oneshot_chan_cfg_t config = {
//         .bitwidth = ADC_BITWIDTH_DEFAULT,
//         .atten = EXAMPLE_ADC_ATTEN,
//     };
//     ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, EXAMPLE_ADC1_CHAN0, &config));
//     ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, EXAMPLE_ADC1_CHAN1, &config));

//     //-------------ADC1 Calibration Init---------------//

//     //adc_cali_handle_t adc1_cali_chan1_handle = NULL;
//     bool do_calibration1_chan0 = YV_ADC_calibration_init(ADC_UNIT_1, EXAMPLE_ADC1_CHAN0, EXAMPLE_ADC_ATTEN, &adc1_cali_chan0_handle);
//    // bool do_calibration1_chan1 = example_adc_calibration_init(ADC_UNIT_1, EXAMPLE_ADC1_CHAN1, EXAMPLE_ADC_ATTEN, &adc1_cali_chan1_handle);
// }
// //This task reloads ADC on Queue data ready-----------------------------------------------------------------------------------------
// void vTask_ADCReadReload(void *user_data)
// {
//     example_queue_element_t ele;
//     int *Buff = 0;
//     int Buff2 = 0;
//     int Pointer = 0;
//     int Rcvd=0;
//     while (1)
//     {
//         if (xSemaphoreTake( TimerToADC_Semaphore, pdMS_TO_TICKS(2000) ) == pdPASS)
//         {
//            // Buff = adc_raw[0];
//             adc_oneshot_read(adc1_handle, EXAMPLE_ADC1_CHAN0, &Buff);
//             //other adc channels reading
//             //calibration of value
//             adc_cali_raw_to_voltage(adc1_cali_chan0_handle, Buff, &data_proc[data_idx] );

//             data_idx++;
//             //and now we send to queue_proc_data a pointer to the part of data_proc array with actual data from adc
//             if (data_idx == DATA_TO_SEND_LEN_VALUES)
//             {
//                 //ele.pADCData = &data_proc[0];
//                 Pointer = &data_proc[0]; 

//                 //printf("adc Pointer Buff Val %d \n", (int)Pointer);
//                 //Buff = &Pointer;
//                 //Buff2 = *Buff;
//                 //printf("adc Pointer Buff2 Val %d \n", Buff2);
//                 xQueueSend(queue_proc_data, &Pointer , pdMS_TO_TICKS(2000));
//                 //xQueueReceive(queue_proc_data, &Rcvd, pdMS_TO_TICKS(2000));
//                 //printf("adc Pointer Rcvd Val %d \n", Rcvd);
//             }

//             if (data_idx == 2* DATA_TO_SEND_LEN_VALUES)
//             {
//                 //ele.pADCData = &data_proc[DATA_TO_SEND_LEN_VALUES];
//                 Pointer = &data_proc[DATA_TO_SEND_LEN_VALUES];
//                 xQueueSend(queue_proc_data, &Pointer, pdMS_TO_TICKS(2000)); 
//                 data_idx = 0;  
//             }           
//             //printf("Received %i \n",ele.event_count );
//             //xQueueSend(queue_proc_data, &ele, pdMS_TO_TICKS(2000));
//             //printf("Send to wifi queue \n");
//         }
//         else 
//         {
//             printf("adc read_reload Timeout\n");
//         }
//     }
//     vTaskDelete(xTask_ADCReadReloadHandle );    
// }
