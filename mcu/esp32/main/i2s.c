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

#include "driver/i2s_std.h"
#include "driver/i2s_common.h"


#include "i2s.h"
#include "platform_config.h"
#include "udp.h"

static const char *TAG = "YV I2S";

QueueHandle_t queue_i2s_size = NULL;
//QueueHandle_t queue_i2s_to_udp = NULL;

i2s_chan_handle_t tx_handle = NULL;
i2s_chan_handle_t rx_handle = NULL;

TaskHandle_t xTask_ReadDataFromI2S = NULL;

//callback function for i2s_rx_transfer_complete_cb. This function is pending within I2S Rx ISR------------------------
static bool IRAM_ATTR I2S_on_rcv_cb(i2s_chan_handle_t handle, i2s_event_data_t *event, void *user_ctx)
{
    BaseType_t high_task_awoken = pdFALSE;
    i2s_event_data_t I2SCurrEventData = *event;
    
    xQueueSendFromISR(queue_i2s_size, &I2SCurrEventData , &high_task_awoken);
    // return whether we need to yield at the end of ISR
    return (high_task_awoken == pdTRUE);
}
//---------------------------------------------------------------------------------------------------------------------
void vTask_ReadDataFromI2S(void *user_data)
{
    int *mic_data = malloc(EXAMPLE_RECV_BUF_SIZE);
    i2s_event_data_t I2SCurrEventData;
    esp_err_t ret = ESP_OK;
    size_t bytes_read = 0;

    while (1)
    {
        if (xQueueReceive(queue_i2s_size, &I2SCurrEventData, pdMS_TO_TICKS(2000))) 
        {   
            memset(mic_data, 0, EXAMPLE_RECV_BUF_SIZE);
            // Read sample data from mic 
            ret = i2s_channel_read(rx_handle, mic_data, I2SCurrEventData.size, &bytes_read, 1000); //EXAMPLE_RECV_BUF_SIZE

            if (ret != ESP_OK) 
            {
                ESP_LOGE(TAG, "I2S read failed ESP_ERR_TIMEOUT");
                abort();
            }
            //I2SCurrEventData.size = bytes_read;
            //I2SCurrEventData.data = mic_data;
            //xQueueSend(queue_i2s_to_udp, &I2SCurrEventData , pdMS_TO_TICKS(2000));
            //ESP_LOGI(TAG, "I2S bytes readen: %d", bytes_read);
            yv_tcp_queue_write(mic_data, bytes_read); 
        }
    }
    vTaskDelete(xTask_ReadDataFromI2S );    
}
//--------------------------------------------------------------------------------------------------------
esp_err_t yv_i2s_sync_init(void)
{
    //init sync objects
    queue_i2s_size = xQueueCreate(1, I2S_QUEUE_SIZE_BYTES);
    if (!queue_i2s_size) 
    {
        ESP_LOGE(TAG, "Creating i2s queue failed");
        return ESP_FAIL;
    } 

    // queue_i2s_to_udp = xQueueCreate(1, sizeof(i2s_event_data_t ));
    // if (!queue_i2s_to_udp) 
    // {
    //     ESP_LOGE(TAG, "Creating i2s queue failed");
    //     return ESP_FAIL;
    // }
    return ESP_OK;
}
//--------------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------------
esp_err_t yv_i2s_init(void)
{
    i2s_chan_config_t chan_cfg = {
        .id = I2S_NUM,                 //!< I2S port id 
        .role = I2S_ROLE_MASTER,    //! < I2S role, I2S_ROLE_MASTER or I2S_ROLE_SLAVE 
        .dma_desc_num = 6, //?       //!< I2S DMA buffer number, it is also the number of DMA descriptor 
        .dma_frame_num = 180,      //!< I2S frame number in one DMA buffer. One frame means one-time sample data in all slots, it should be the multiple of '3' when the data bit width is 24.                                  
        .auto_clear = true,     // Auto clear the legacy data in the DMA buffer                                
    };
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_handle, &rx_handle));

    i2s_std_config_t std_cfg = {
        .clk_cfg = {
            .sample_rate_hz = EXAMPLE_SAMPLE_RATE8k,
            .clk_src = SOC_MOD_CLK_APLL, 
            .mclk_multiple = EXAMPLE_MCLK_MULTIPLE384, //I2S_STD_CLK_DEFAULT_CONFIG(EXAMPLE_SAMPLE_RATE),
        },
        .slot_cfg = {
                .data_bit_width = I2S_DATA_BIT_WIDTH_24BIT, 
                .slot_bit_width = I2S_SLOT_BIT_WIDTH_24BIT, 
                .slot_mode = I2S_SLOT_MODE_STEREO, 
                .slot_mask = I2S_STD_SLOT_BOTH, 
                .ws_width = I2S_DATA_BIT_WIDTH_24BIT, 
                .ws_pol = false, 
                .bit_shift = true, //Set to enable bit shift in Philips mode
                .msb_right = true, //Set to place right channel data at the MSB in the FIFO 
                },
        .gpio_cfg = {
            .mclk = I2S_MCK_IO,
            .bclk = I2S_BCK_IO,
            .ws = I2S_WS_IO,
            .dout = I2S_DO_IO,
            .din = I2S_DI_IO,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },       
    };

    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_handle, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_handle, &std_cfg));

    //set a new callback function for i2s rcv data interrupt
    i2s_event_callbacks_t cbs = {
        .on_recv = I2S_on_rcv_cb,
    }; 

    ESP_ERROR_CHECK(i2s_channel_register_event_callback(rx_handle, &cbs, NULL)); //void *user_data);

    yv_i2s_sync_init();
    
    ESP_ERROR_CHECK(i2s_channel_enable(tx_handle));
    ESP_ERROR_CHECK(i2s_channel_enable(rx_handle));

    //run i2s task
    xTaskCreate(vTask_ReadDataFromI2S, "vTask_ReadDataFromI2S", I2S_TASK_STACK_SIZE, NULL, I2S_TASK_PRIORITY, &xTask_ReadDataFromI2S);
     return ESP_OK;
}
//--------------------------------------------------------------------------------------------------------