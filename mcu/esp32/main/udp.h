#ifndef MAIN_UDP_H_
#define MAIN_UDP_H_
//-------------------------------------------------------------
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

#define DATA_TO_SEND_LEN_VALUES (40) //each value is 2(?)bytes

#define MYSIZE SSIZE_MAX



typedef struct {
    void *pADCData;
} example_queue_element_t;
//-------------------------------------------------------------
void udp_task(void *pvParameters);
//-------------------------------------------------------------
#endif /* MAIN_UDP_H_ */
