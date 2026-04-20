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
 * PB6=BADC7：BAT2+ 分压采样。硬件 2M（BAT2+→）+ 1M（→GND），
 * V_pb6 = BAT2+ × 1/(2+1) = BAT2+/3 → BAT2+_mV = hal_badc_meas(PB6) × 3
 */
#define BADC_PB6_BAT2P_DIV_RATIO  3u

/**
 * PC7=BADC4：Cell2 分压采样（移植自 0306 平台）。硬件 2M（Cell2+→）+ 1M（→GND），
 * V_pc7 = Cell2 × 1/(2+1) = Cell2/3 → Cell2_mV = hal_badc_meas(PC7) × 3
 */
#define BADC_PC7_CELL2_DIV_RATIO  3u

/**
 * PD3=BADC9：VBAT- 电池负压采样。硬件 1M（→VDD）+ 2M（→VBAT-），
 * V_pd3 = (VDD×2 + VBAT-×1) / 3 → VBAT-_mV = hal_badc_meas(PD3) × 3 - VDD × 2
 * 注意：若 3.3V 被拉低，V_pd3 异常偏低，应保持上次有效值。
 */
#define BADC_PD3_VBATN_MIN_MV     500u    /* ADC 原始值低于此阈值视为 3.3V 掉电 */
#define BADC_PD3_VBATN_EMA_SHIFT  3u      /* EMA 滤波系数 1/2^N，N=3 即 1/8，越大越平滑 */

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
/**
 * @brief  获取芯片实际 VDD 电压（mV），基于内部 1.2V 参考校准。
 * @return 实际 VDD 电压，单位 mV（典型值 ~3300，各芯片有偏差）。
 */
uint16_t hal_badc_get_vdd_mv(void);

void hal_badc_isns_chan_offest_update(void);

#endif /* BADC_H_ */
