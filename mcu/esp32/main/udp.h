#ifndef MAIN_UDP_H_
#define MAIN_UDP_H_
//-------------------------------------------------------------


#define DATA_TO_SEND_LEN_VALUES (40) //each value is 2(?)bytes

#define MYSIZE SSIZE_MAX

#define UDP_TASK_STACK_SIZE (4096)
#define UDP_TASK_PRIORITY (4)



// typedef struct {
//     void *pADCData;
// } example_queue_element_t;
//-------------------------------------------------------------
//void udp_task(void *pvParameters);

void yv_tcp_init(void);
void yv_tcp_queue_write(void *pData, long unsigned int size);
//-------------------------------------------------------------
#endif /* MAIN_UDP_H_ */
