#ifndef _SIMPLEADC_H
#define _SIMPLEADC_H

#include <esp_adc/adc_oneshot.h>

enum type
{
    ADC_READ_RAW,
    ADC_READ_MV,
};

class SimpleADC
{
public:
    SimpleADC();
    ~SimpleADC();
    bool setup(int io_num, adc_bitwidth_t width = ADC_BITWIDTH_12);
    int read(type mode);

private:
    adc_oneshot_unit_handle_t _handle;
    adc_unit_t _unit;
    adc_channel_t _channel;
    adc_cali_handle_t _cali_handle;

    void releaseADCHandle();
};

#endif // _SIMPLEADC_H