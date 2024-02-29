#ifndef __I2S_H
#define __I2S_H


/* Example configurations */
#define EXAMPLE_RECV_BUF_SIZE   (2880)
#define EXAMPLE_SAMPLE_RATE8k     (8000)
#define EXAMPLE_MCLK_MULTIPLE384   (384) // If not using 24-bit data width, 256 should be enough
#define EXAMPLE_MCLK_FREQ_HZ16x384    (EXAMPLE_SAMPLE_RATE8k * EXAMPLE_MCLK_MULTIPLE384)
//#define EXAMPLE_VOICE_VOLUME    CONFIG_EXAMPLE_VOICE_VOLUME


    
#define I2S_QUEUE_SIZE_BYTES    (sizeof(i2s_event_data_t))

#define I2S_TASK_PRIORITY       (5)
#define I2S_TASK_STACK_SIZE     (8192)


int yv_i2s_init(void);

#endif /*__I2S_H*/
