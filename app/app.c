#include "regdef.h"
#include "printk.h"
#include "g_data.h"
#include "config.h"
#include "delay.h"
#include "debug.h"
#include "prot.h"
#include "_wpc.h"
#include "led.h"
#include "gui.h"
#include "app.h"
#include "ask.h"
#include "bat_record.h"

#define T_APP_250ms_POLL    250
#define T_APP_100ms_POLL    100
#define T_APP_010ms_POLL     10

static uint16_t u16Isns[5] = {0};
static uint16_t u16IsnsTmp[5] = {0};
static uint8_t indexIsns = 0;

uint16_t pre_isns;

void apl_task_init(void)
{
//	fml_tntc_otp_init();
	fml_tntc_utp_init();
	fml_tdie_otp_init();
	fml_tdie_utp_init();
	fml_isns_ocp_init();
	fml_vbus_ovp_init();
	fml_vbus_uvp_init();
	fml_vbus_dpl_init();
	fml_vpwr_ovp_init();
	fml_pout_opp_init();

	led_init();
	//initKey();
	osal_task_handler_reg(APL_TASK, apl_task_event_handler);
	osal_start_timerEx(APP_250ms_TIMER, 0, T_APP_250ms_POLL, APL_TASK, APL_EVT_250ms_POLL);
	osal_start_timerEx(APP_100ms_TIMER, 0, T_APP_100ms_POLL, APL_TASK, APL_EVT_100ms_POLL);
	osal_start_timerEx(APP_010ms_TIMER, 0, T_APP_010ms_POLL, APL_TASK, APL_EVT_010ms_POLL);
}

void apl_task_event_handler(uint32_t event)
{
	switch (event)
	{
		case APL_EVT_250ms_POLL://250ms
			ui_update();
			extern uint16_t cell2_voltage;
			{
				/* PB6=BADC7: BAT2+ 总电压（2M+1M 分压 ×3） */
				uint16_t pb6_adc_mv = hal_badc_meas(_BADC_CH_PB6_ADC7);
				uint16_t bat2_plus  = (uint16_t)(pb6_adc_mv * BADC_PB6_BAT2P_DIV_RATIO);

				/* PC7=BADC4: Cell2 分压采样（2M+1M ×3） */
				uint16_t pc7_adc_mv = hal_badc_meas(_BADC_CH_PC7_ADC4);
				uint16_t pc7_mv     = (uint16_t)(pc7_adc_mv * BADC_PC7_CELL2_DIV_RATIO);

				/* PD3=BADC9: VBAT- 负压（1M→VDD + 2M→VBAT-）
				 * V_pd3 = (VDD×2 + VBAT-) / 3, VBAT- = pd3_adc × 3 - VDD × 2
				 * 若 3.3V 被拉低 → pd3 读数异常偏低，保持上次有效值 */
				static int16_t vbat_minus_last = 0;
				uint16_t pd3_adc_mv = hal_badc_meas(_BADC_CH_PD3_ADC9);
				if (pd3_adc_mv >= BADC_PD3_VBATN_MIN_MV) {
					uint16_t vdd_mv = hal_badc_get_vdd_mv();
					int16_t raw = (int16_t)(pd3_adc_mv * 3) - (int16_t)(vdd_mv * 2);
					/* EMA 滤波：filtered += (raw - filtered) >> N */
					vbat_minus_last += (raw - vbat_minus_last) >> BADC_PD3_VBATN_EMA_SHIFT;
				}

				uint16_t total_vbat = (uint16_t)((int16_t)bat2_plus - vbat_minus_last);
				cell2_voltage = (uint16_t)((int16_t)pc7_mv - vbat_minus_last);
				printk("PB6=%u PC7=%u PD3=%u VDD=%u PB6_V=%u PC7_V=%u total=%u cell2=%u vbat-=%d NU6805=%d\n",
				       pb6_adc_mv, pc7_adc_mv, pd3_adc_mv, hal_badc_get_vdd_mv(),
				       bat2_plus, pc7_mv, total_vbat, cell2_voltage, vbat_minus_last, g_buckboost.adc_vbat);
			}


#if 1
			if (gd->atl_test_tpr1c_coil_flag != 1)
			{
				if (gd->ptx_protocol_phase >= WPC_PHASE_XFER)
				{
					if (gd->dmo1_phase == _NU103x_DM_PHASE_DIG_PING || gd->dmo2_phase == _NU103x_DM_PHASE_DIG_PING)
					{
						if (gd->vpwr * gd->isns / 1000 > 1500)
						{
							if (gd->dmo1_phase != _NU103x_DM_PHASE_HI_POWER)
							{
								fml_nu103x_dmo1_param_set(_1030_CFG_DMO1_DDM_SRC_IAVG, _1030_CFG_DMO1_DDM_GAIN_MODE_FIXD, _1030_CFG_DMO1_DDM_FIXED_GAIN_X60);
								gd->dmo1_phase = _NU103x_DM_PHASE_HI_POWER;
							}

							if (gd->dmo2_phase != _NU103x_DM_PHASE_HI_POWER)
							{
								fml_nu103x_dmo2_param_set(_1030_CFG_DMO2_DDM_SRC_VCAP, _1030_CFG_DMO2_DDM_GAIN_MODE_FIXD, _1030_CFG_DMO2_DDM_FIXED_GAIN_X60, _1030_CFG_DMO2_VCAP_RATIO_K3);
								gd->dmo2_phase = _NU103x_DM_PHASE_HI_POWER;
							}
						}
						else
						{
							if (gd->dmo1_phase != _NU103x_DM_PHASE_LO_POWER)
							{
								fml_nu103x_dmo1_param_set(_1030_CFG_DMO1_DDM_SRC_IAVG, _1030_CFG_DMO1_DDM_GAIN_MODE_FIXD, _1030_CFG_DMO1_DDM_FIXED_GAIN_X60);
								gd->dmo1_phase = _NU103x_DM_PHASE_LO_POWER;
							}

							if (gd->dmo2_phase != _NU103x_DM_PHASE_LO_POWER)
							{
								fml_nu103x_dmo2_param_set(_1030_CFG_DMO2_DDM_SRC_VCAP, _1030_CFG_DMO2_DDM_GAIN_MODE_FIXD, _1030_CFG_DMO2_DDM_FIXED_GAIN_X60, _1030_CFG_DMO2_VCAP_RATIO_K2);
								gd->dmo2_phase = _NU103x_DM_PHASE_LO_POWER;
							}
						}
					}
					else
					{
						if (gd->vpwr * gd->isns / 1000 > 1500)
						{
							gd->dmo1_phase = _NU103x_DM_PHASE_HI_POWER;
							gd->dmo2_phase = _NU103x_DM_PHASE_HI_POWER;
						}
						else
						{
							gd->dmo1_phase = _NU103x_DM_PHASE_LO_POWER;
							gd->dmo2_phase = _NU103x_DM_PHASE_LO_POWER;
						}
					}
				}
			}
#endif
			break;
		case APL_EVT_100ms_POLL:
//			gd->sys_infos.ntc_temp = fml_ntc_temp_get();
			gd->sys_infos.ntc_temp_typec = fml_ntc_temp_get_typec();
			gd->sys_infos.ntc_temp_wpc = fml_ntc_temp_get_wpc();
//			gd->sys_infos.die_temp = fml_die_temp_get();
			// printk("\r\n ntc_typec --> %d,ntc_wpc --> %d",gd->sys_infos.ntc_temp_typec,gd->sys_infos.ntc_temp_wpc);
//			fml_tntc_otp_check(gd->sys_infos.ntc_temp);
			fml_tntc_otp_limit_power(gd->sys_infos.ntc_temp_wpc);
		//	fml_tntc_utp_check(gd->sys_infos.ntc_temp);
			fml_tdie_otp_check(gd->sys_infos.die_temp);
			fml_tdie_utp_check(gd->sys_infos.die_temp);
#if CONFIG_NEW_CCC_LOG_ENABLE
			battery_record_periodic_check();
			fml_bat_ov_forbid_check();
			fml_bat_uv_forbid_check();

			/* Cycle count: cumulative charge integration (standard definition)
			 * 1 cycle = total charge-in reaches CONFIG_BATTERY_CAPACITY_MAH
			 * Accumulates charging current × time each 100ms tick.
			 * Unit: mA per 100ms tick. Threshold: capacity_mAh × 36000 ticks/hour */
			{
				static uint32_t charge_accum = 0;
				#define CYCLE_CHARGE_THRESHOLD  ((uint32_t)CONFIG_BATTERY_CAPACITY_MAH * 36000UL)
				if (g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE && g_buckboost.adc_ibat > 0) {
					charge_accum += (uint32_t)g_buckboost.adc_ibat;
					if (charge_accum >= CYCLE_CHARGE_THRESHOLD) {
						charge_accum -= CYCLE_CHARGE_THRESHOLD;
						SET_CYCLE_COUNT(gd, GET_CYCLE_COUNT(gd) + 1);
#if CYCLE_COUNT_FLASH_PERSIST
						cycle_count_save_to_flash();
#endif
					}
				}
			}
#endif
			break;
		case APL_EVT_010ms_POLL:
			//detectSingleKey();
			gd->isns = hal_badc_meas(_BADC_CH_PD6_ADC3);

			if (gd->isns < 150 && pre_isns > 500)
			{
				if (gd->ptx_protocol_phase == WPC_PHASE_XFER && gd->rx_infos.power_profile_mode == EPP)
				{
					gd->pid_perd = gd->pid_limit.perd_lim_lo;
					gd->pid_duty = gd->pid_limit.duty_lim_lo;
					hal_epwm_pwm_update(EPWM1, gd->pid_perd, gd->pid_duty, gd->pid_phas);
					gd->pid_volt = gd->pid_limit.volt_lim_lo;
					fml_adp_volt_set(gd->pid_volt);
					printk(" [EPP_OVP_TEST:%d,%d] ", pre_isns, gd->isns);
				}
			}
			pre_isns = gd->isns;


			u16Isns[indexIsns++] = gd->isns;
			if (indexIsns >= 5)
			{
				indexIsns = 0;
			}

			uint8_t i;
			for (i=0; i<5; i++)
			{
				u16IsnsTmp[i] = u16Isns[i];
			}

		    for (uint8_t i = 0; i < 4; i++)
		    {
		        for (uint8_t j = 0; j < (4 - i); j++)
		        {
		            if (u16IsnsTmp[j] < u16IsnsTmp[j + 1])
		            {
		            	uint8_t temp = u16IsnsTmp[j];
		            	u16IsnsTmp[j] = u16IsnsTmp[j + 1];
		            	u16IsnsTmp[j + 1] = temp;
		            }
		        }
		    }

		    uint16_t Tmp_max = u16IsnsTmp[0] - u16IsnsTmp[4];

			if ((Tmp_max > 500) && (gd->isns < 400) && (u16IsnsTmp[0] > 700))
			{
//				if (gd->dig_ping_perd == 144000000/360000)
//				{
//					gd->pid_duty = 500;
//					gd->pid_phas = 50;
//				}
//				else
//				{
//					gd->pid_duty = 125;
//					gd->pid_phas = 0;
//				}
//				hal_epwm_pwm_start(EPWM1, gd->pid_perd, gd->pid_duty, gd->pid_phas);
//
//				gd->pid_volt = 11000;
//				fml_adp_volt_set(gd->pid_volt);

//				wpc_stop_to_idle(ESYS_ERR_CODE_XFER_PHASE_CEP_TIMEOUT);

				printk("\r\n------>!!!!!%d,%d",Tmp_max,gd->isns);
			}
			gd->vpwr = g_buckboost.adc_vbus;
			//gd->vpwr = hal_badc_meas(_BADC_CH_PD0_ADC8);
			//gd->tx_power = gd->isns * gd->vpwr / 1000;//TODO
			gd->vbus = 9000;//hal_badc_meas(_BADC_CH_PB6_ADC7);
			fml_isns_ocp_check(gd->isns);
			fml_vpwr_ovp_check(gd->vpwr);
			fml_vbus_ovp_check(gd->vbus);
			fml_vbus_uvp_check(gd->vbus);
			fml_vbus_dpl_check(gd->vbus);
			fml_pout_opp_check(gd->vpwr, gd->isns);
			if (gd->ptx_protocol_phase == WPC_PHASE_XFER)
			{
				fml_ask_decode_check();
			}

			iic_read_info_sync();
			iic_write_info_sync();

			break;
		default:
			break;
	}
}




#define _UART_RX_BUFF_SIZE_MAX    140
#define _UART_TX_BUFF_SIZE_MAX    140

volatile uint8_t rxdata_received_flag;

static uint8_t m_u8RxDataIndex;
static uint8_t m_u8RxData_Prev;
static uint8_t m_arr_u8RxDataBuff_Orig[_UART_RX_BUFF_SIZE_MAX];
static uint8_t m_arr_u8RxDataBuff_Copy[_UART_RX_BUFF_SIZE_MAX];
static uint8_t m_arr_u8TxDataBuff[_UART_TX_BUFF_SIZE_MAX];

void APP_vInit(void)
{
	rxdata_received_flag = 0;
	m_u8RxDataIndex = 0;
	m_u8RxData_Prev = 0;
	hal_uart_reg_int_cb(UART1, APP_vUART1_RxIntHandler);
}

void APP_vUART1_RxIntHandler(void)
{
	uint8_t tmp = UART1->DAT_BUFF.RXDB.DATA;

	if (m_u8RxData_Prev == 0x55 && tmp == '#')
	{
		m_u8RxDataIndex = 0;
		m_arr_u8RxDataBuff_Orig[m_u8RxDataIndex++] = tmp;
	}
	else if (m_u8RxData_Prev == '.' && tmp == 0x55)
	{
		for (int i=0; i<m_u8RxDataIndex; i++)
		{
			m_arr_u8RxDataBuff_Copy[i] = m_arr_u8RxDataBuff_Orig[i];
		}
		tmp = 0;
		m_u8RxDataIndex = 0;
		rxdata_received_flag = 1;
	}
	else
	{
		if (m_u8RxDataIndex < _UART_RX_BUFF_SIZE_MAX)
		{
			m_arr_u8RxDataBuff_Orig[m_u8RxDataIndex++] = tmp;
		}
	}

	m_u8RxData_Prev = tmp;
}

unsigned char char2hex(unsigned char ascii)
{
	uint8_t hex;
	if (ascii >= 0x30 && ascii <= 0x39)
	{
		hex = ascii - 0x30;
	}
	else if (ascii >= 41 && ascii <= 0x46)
	{
		hex = ascii - 0x37;
	}
	else if (ascii >= 0x61 && ascii <= 0x66)
	{
		hex = ascii - 0x57;
	}
	else
	{
		hex = 0;
	}
	return hex;
}

unsigned char hex2char(unsigned char hex)
{
	uint8_t ascii;

	if (hex >=0 && hex <= 9)
	{
		ascii = hex + 0x30;
	}
	else if (hex >= 10 && hex <= 15)
	{
		ascii = hex + 0x37;
	}
	else
	{
		ascii = 0xFF;
	}
	return ascii;
}

static uint8_t m_u8SerialNo;
void app_send_data(uint8_t cmd, uint8_t len, uint8_t *buff)
{
	uint8_t buff_len;
	if (len > 64) len = 64;
	if (m_u8SerialNo++ > 0x6A) m_u8SerialNo = 0x61;

	m_arr_u8TxDataBuff[0] = 0x55;
	m_arr_u8TxDataBuff[1] = 0x55;
	m_arr_u8TxDataBuff[2] = 0x23;
	m_arr_u8TxDataBuff[3] = m_u8SerialNo;
	m_arr_u8TxDataBuff[4] = cmd;//0x7A-data, 0x77-ack;

	m_arr_u8TxDataBuff[5] = hex2char(((len*2)>>4) & 0x0F);
	m_arr_u8TxDataBuff[6] = hex2char(((len*2)>>0) & 0x0F);

	for (uint8_t i=0; i<len; i++)
	{
		m_arr_u8TxDataBuff[7 + 2*i + 0] = hex2char((buff[i]>>4) & 0x0F);
		m_arr_u8TxDataBuff[7 + 2*i + 1] = hex2char((buff[i]>>0) & 0x0F);
	}

	uint8_t u8Checksum = 0;
	for (uint8_t i=2; i<7+len*2; i++)
	{
		u8Checksum += m_arr_u8TxDataBuff[i];
	}

	m_arr_u8TxDataBuff[7 + 2*len + 0] = hex2char((u8Checksum>>4) & 0x0F);
	m_arr_u8TxDataBuff[7 + 2*len + 1] = hex2char((u8Checksum>>0) & 0x0F);
	m_arr_u8TxDataBuff[7 + 2*len + 2] = 0x2E;
	m_arr_u8TxDataBuff[7 + 2*len + 3] = 0x55;
	m_arr_u8TxDataBuff[7 + 2*len + 4] = 0x55;

	buff_len = 7 + len * 2 + 5;

	for (int i=0; i<buff_len; i++)
	{
		hal_uart_putc(UART1, m_arr_u8TxDataBuff[i]);
	}
}

void APP_vHandler(void)
{
	if (rxdata_received_flag == 1)
	{
		rxdata_received_flag = 0;
		uint8_t tmp_buff[4] = { 0x01, 0x02, 0x03, 0x04, };
		app_send_data(0x7A, 4, tmp_buff);
	}
}

uint32_t switch_big_little_endian(uint32_t Value)
{
	uint32_t u32Tmp;
	uint8_t B3,B2,B1,B0;

	B3 = (Value & 0xFF000000) >> 24;
	B2 = (Value & 0x00FF0000) >> 16;
	B1 = (Value & 0x0000FF00) >> 8;
	B0 =  Value & 0x000000FF;
	u32Tmp = (B0 << 24) | (B1 << 16) | (B2 << 8) | B3;

	return u32Tmp;
}

void jig_store_Q_value_process(struct com_prx_ask_pkt_t *com_pkt)
{
	uint32_t u32Tmp;

	if ((0x12 == com_pkt->msg.prop.data[0]) && (0x34 == com_pkt->msg.prop.data[1]))
	{
		hal_fmc_erase_page(AP_CFG_ROM_ADDR_BASE);

		u32Tmp = switch_big_little_endian(gd->tx_infos.q_fact_air);
		hal_fmc_write_word(AP_CFG_ROM_ADDR_BASE, u32Tmp);

		u32Tmp = switch_big_little_endian(gd->tx_infos.f_self_air);
		hal_fmc_write_word((AP_CFG_ROM_ADDR_BASE + 4), u32Tmp);
	}
}
