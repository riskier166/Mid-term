#include "SimpleADC.h"

static adc_oneshot_unit_handle_t adc1_handle = NULL;
static adc_oneshot_unit_handle_t adc2_handle = NULL;
static unsigned int counter_adc1 = 0;
static unsigned int counter_adc2 = 0;

SimpleADC::SimpleADC()
{
    _handle = NULL;
    _cali_handle = NULL;
}

SimpleADC::~SimpleADC()
{
    releaseADCHandle();
}

bool SimpleADC::setup(int io_num, adc_bitwidth_t width)
{
    if(_handle)
        return false;

    if(adc_oneshot_io_to_channel(io_num, &_unit, &_channel) != ESP_OK)
        return false;

    adc_oneshot_unit_init_cfg_t init_config;
    init_config.unit_id = _unit;
    init_config.ulp_mode = ADC_ULP_MODE_DISABLE;
    init_config.clk_src = ADC_RTC_CLK_SRC_DEFAULT;
    if (_unit == ADC_UNIT_1) {
        if (adc1_handle == NULL) {
            if(adc_oneshot_new_unit(&init_config, &adc1_handle) != ESP_OK)
                return false;
        }
        
        if(adc1_handle) {
            _handle = adc1_handle;
            counter_adc1 ++;
        }
    }
    
    if (_unit == ADC_UNIT_2) {
        if (adc2_handle == NULL) {
            if(adc_oneshot_new_unit(&init_config, &adc2_handle) != ESP_OK)
                return false;
        }
        
        if(adc2_handle) {
            _handle = adc2_handle;
            counter_adc2 ++;
        }
    }

    if(!_handle)
        return false;

    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = (adc_bitwidth_t)width,
    };
    if(adc_oneshot_config_channel(_handle, _channel, &config) != ESP_OK)
        return false;

    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = _unit,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = (adc_bitwidth_t)width,
        .default_vref = 1100,
    };
    if(adc_cali_create_scheme_line_fitting(&cali_config, &_cali_handle) != ESP_OK)
        return false;
    
    return true;
}

int SimpleADC::read(type mode)
{
    int reading;
    if (mode == ADC_READ_RAW)
        adc_oneshot_read(_handle, _channel, &reading);
    else
        adc_oneshot_get_calibrated_result(_handle, _cali_handle, _channel, &reading);
    return reading;
}

void SimpleADC::releaseADCHandle()
{
    if (_handle != NULL) {
        if (_unit == ADC_UNIT_1) {
            counter_adc1 --;

            if(counter_adc1 == 0) {
                adc_oneshot_del_unit(adc1_handle);
                adc1_handle = NULL;
            }
        }

        if (_unit == ADC_UNIT_2) {
            counter_adc2 --;

            if(counter_adc2 == 0) {
                adc_oneshot_del_unit(adc2_handle);
                adc2_handle = NULL;
            }
        }

        _handle = NULL;
    }

    if (_cali_handle) {
        adc_cali_delete_scheme_line_fitting(_cali_handle);
        _cali_handle = NULL;
    }
}
