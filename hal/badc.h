#ifndef BADC_H_
#define BADC_H_

/**
 * @brief BADC channel enumeration.
 */
enum badc_chan_t {
	_BADC_CH_INR_AVSS =  0,
	_BADC_CH_PC6_ADC1 =  1,
	_BADC_CH_PB2_ADC2 =  2,
	_BADC_CH_PD6_ADC3 =  3,
	_BADC_CH_PC7_ADC4 =  4,
	_BADC_CH_PC8_ADC5 =  5,
	_BADC_CH_PB5_ADC6 =  6,
	_BADC_CH_PB6_ADC7 =  7,
	_BADC_CH_PD0_ADC8 =  8,
	_BADC_CH_PD3_ADC9 =  9,
	_BADC_CH_INR_V055 = 10,
	_BADC_CH_INR_V1P1 = 11,
	_BADC_CH_INR_V1P2 = 12,
	_BADC_CH_INR_TJ_L = 13,
	_BADC_CH_INR_TJ_H = 14,
	_BADC_CH_PGA_ISNS = 15,
};

/**
 * @brief  Initialize the configuration of the hardware ADC module.
 * 
 * This function configures the ADC's sample average count,
 * sample delay,sample clock,and reference voltage,and enables the ADC module.
 * Ensure that the ADC will function properly in subsequent operations.
 * 
 * @param  None.
 * @return None.
 */
void hal_badc_init(void);

/**
 * @brief  Measures the analog voltage value of the specified channel.
 * 
 * This function takes a channel as an input parameter and 
 * averages six measurements of the analog signal of that channel.
 * It converts the raw measurement to a voltage value based on a reference
 * voltage. The result is scaled to a 12-bit resolution.
 *
 * @param  channel The specified channel to measure. The channel should be
 *         one of the defined enumeration values of type `badc_chan_t`.
 * @return Returns the measured voltage value in millivolts (mV).
 */
uint16_t hal_badc_meas(enum badc_chan_t channel);

/**
 * @brief  Update BADC internal ISNS channel offset.
 *
 * This function update internal ISNS channel DC bias.
 * please call this function before EPWM start driver power stage.
 *
 * @param  None.
 * @return None.
 */
void hal_badc_isns_chan_offest_update(void);

#endif /* BADC_H_ */
