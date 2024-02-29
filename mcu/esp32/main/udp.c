#include "udp.h"
#include "i2s.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "driver/i2s_std.h"
#include "freertos/semphr.h"

//-------------------------------------------------------------
static const char *TAG = "udp";

//"192.168.1.16"
#define CONFIG_SERVER_IP  ("192.168.0.105")
#define CONFIG_CLIENT_PORT (4444)
#define CONFIG_SERVER_PORT  (3333)

QueueHandle_t queue_from_i2s = NULL;

//-------------------------------------------------------------
void udp_task(void *pvParameters)
{
  //TickType_t xLastWakeTime;
  //SemaphoreHandle_t semaph = (SemaphoreHandle_t * )pvParameters;
  QueueHandle_t queue = (QueueHandle_t)pvParameters;
  //size_t queue_elem;
  i2s_event_data_t Buff;
  //size_t evBuff;

  int sockfd;
  struct sockaddr_in servaddr, cliaddr;
  ESP_LOGI(TAG, "Create socket...\n");
 // if ( (sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0 ) 
  if ( (sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 )

  {
    ESP_LOGE(TAG, "socket not created\n");
    vTaskDelete(NULL);
  }
  memset(&servaddr, 0, sizeof(servaddr));
  memset(&cliaddr, 0, sizeof(cliaddr));
  //config client
  cliaddr.sin_family    = AF_INET; // IPv4
  cliaddr.sin_addr.s_addr = INADDR_ANY;
  cliaddr.sin_port = htons(CONFIG_CLIENT_PORT);

  
  //������ ����� � ������� �������
  // if (bind(sockfd, (const struct sockaddr *)&cliaddr,  sizeof(struct sockaddr_in)) < 0 )
  // {
  //   ESP_LOGE(TAG, "socket not binded\n");
  //   vTaskDelete(NULL);
  // }
  // ESP_LOGI(TAG, "socket was binded\n");
  //config server
  servaddr.sin_family    = AF_INET; // IPv4
  servaddr.sin_addr.s_addr = inet_addr(CONFIG_SERVER_IP);
  servaddr.sin_port = htons(CONFIG_SERVER_PORT);
  ESP_LOGI(TAG, "connecting...");

  int rc = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in));
  ESP_LOGD(TAG, "connect rc: %d", rc);

  //xLastWakeTime = xTaskGetTickCount();

  // uint8_t my_data[2000];
  // memset(my_data, 0xAA, 2000);
  for(short i=0;;i++)
  {
    // if (xSemaphoreTake( semaph, pdMS_TO_TICKS(2000) ) == pdTRUE)
    // {

    //    send(sockfd, my_data, 2000,0);
    //   printf("Send to PC %d\n", i);
    // }
    // else
    // {
    //   printf("tcp timeout\n"); 
    // }

   
     if (xQueueReceive(queue, &Buff, pdMS_TO_TICKS(2000))) 
        {
          //ESP_LOGI(TAG, "Element catched from queue is %llu", queue_elem.event_count);
          //printf("Received in udp task %i \n",queue_elem.event_count );
          //printf("Buff is %d \n", Buff);
          //printf("Pointer Val%d \n", (int)Buff);
          //Buff2 = *Buff; 
          //printf("Pointer points to %d \n", Buff2);


          //sendto(sockfd, Buff.data, Buff.size ,  0, (struct sockaddr*) &servaddr,  sizeof(servaddr)); //4*DATA_TO_SEND_LEN_VALUES
          send(sockfd, Buff.data, Buff.size ,0);
          //printf("TCP: buff size if %d\n", Buff.size);
        }
        else 
        {
           //ESP_LOGW(TAG, "Element catch timeout");
            printf("Timeout\n");
        }

    //vTaskDelayUntil( &xLastWakeTime, ( 1000 / portTICK_PERIOD_MS ) ); //portTICK_RATE_MS ) );
  }
  shutdown(sockfd, 0);
  close(sockfd);
  vTaskDelete(NULL);
}
//-------------------------------------------------------------
void yv_tcp_init(void)
{
  queue_from_i2s = xQueueCreate(1, sizeof(i2s_event_data_t ));
  if (!queue_from_i2s) 
  {
    ESP_LOGE(TAG, "Creating i2s queue failed");
    return ;//ESP_FAIL;
  }
  xTaskCreate(udp_task, "udp_task", UDP_TASK_STACK_SIZE, queue_from_i2s, UDP_TASK_PRIORITY, NULL);
}
//-------------------------------------------------------------
void yv_tcp_queue_write(void *pData, long unsigned int size)
{
  i2s_event_data_t Buff;
  Buff.size = size;
  Buff.data = pData;
  xQueueSend(queue_from_i2s, &Buff, pdMS_TO_TICKS(2000));
}