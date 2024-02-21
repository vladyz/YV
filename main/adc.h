#ifndef __ADC_H
#define __ADC_H

#define ADC_DATA_SIZE_BYTES (sizeof(int))
#define EXAMPLE_ADC_UNIT                    ADC_UNIT_1
// #define _EXAMPLE_ADC_UNIT_STR(unit)         #unit
// #define EXAMPLE_ADC_UNIT_STR(unit)          _EXAMPLE_ADC_UNIT_STR(unit)
#define EXAMPLE_ADC_CONV_MODE               ADC_CONV_SINGLE_UNIT_1
#define EXAMPLE_ADC_ATTEN                   ADC_ATTEN_DB_0
#define EXAMPLE_ADC_BIT_WIDTH               SOC_ADC_DIGI_MAX_BITWIDTH

#define EXAMPLE_ADC1_CHAN0          ADC_CHANNEL_4
#define EXAMPLE_ADC1_CHAN1          ADC_CHANNEL_5


void yv_adc_init(void);

#endif /*__ADC_H*/