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
#include "ntc.h"
#include "_fml.h"

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
			wpc_printk("\r\n ntc_typec=%d,ntc_wpc=%d,bat_temp=%d,bat_res=%d",gd->sys_infos.ntc_temp_typec,gd->sys_infos.ntc_temp_wpc,ntc_to_temp(g_buckboost.adc_tbat1),g_buckboost.adc_tbat1);
//			fml_tntc_otp_check(gd->sys_infos.ntc_temp);
			wpc_power_handle(gd->sys_infos.ntc_temp_wpc, ntc_to_temp(g_buckboost.adc_tbat1));
		//	fml_tntc_utp_check(gd->sys_infos.ntc_temp);
			fml_tdie_otp_check(gd->sys_infos.die_temp);
			fml_tdie_utp_check(gd->sys_infos.die_temp);
#if CONFIG_NEW_CCC_LOG_ENABLE
			/* 按键期间跳过异常记录和过压禁用检测：
			 * 硬件 bug — 按键按下时 ADC 采样受干扰，读数偏高，
			 * 会误触发过压异常记录或 OV_FORBID 永久禁用。
			 * _KEY_LEVEL == 0 表示按键正在被物理按下（低有效，实时 GPIO 电平）。*/
			if (_KEY_LEVEL != 0) {
				battery_record_periodic_check();
				fml_bat_ov_forbid_check();
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
					wpc_printk(" [EPP_OVP_TEST:%d,%d] ", pre_isns, gd->isns);
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
		            uint16_t temp = u16IsnsTmp[j];
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
				// gd->dig_ping_perd = 144000000 / 120000;
				// hal_epwm_pwm_start(EPWM1, gd->pid_perd, gd->pid_duty, gd->pid_phas);
				wpc_printk("\r\n------>!!!!!%d,%d",Tmp_max,gd->isns);
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
		/* Read-Modify-Write: preserve cycle count, OV forbid, Vref at offset+16/+20/+24 */
		uint32_t cfg[7];
		for (uint8_t i = 0; i < 7; i++)
			cfg[i] = *(uint32_t *)(AP_CFG_ROM_ADDR_BASE + i * 4);
		hal_fmc_erase_page(AP_CFG_ROM_ADDR_BASE);

		u32Tmp = switch_big_little_endian(gd->tx_infos.q_fact_air);
		hal_fmc_write_word(AP_CFG_ROM_ADDR_BASE, u32Tmp);

		u32Tmp = switch_big_little_endian(gd->tx_infos.f_self_air);
		hal_fmc_write_word((AP_CFG_ROM_ADDR_BASE + 4), u32Tmp);

		for (uint8_t i = 2; i < 7; i++)
			hal_fmc_write_word(AP_CFG_ROM_ADDR_BASE + i * 4, switch_big_little_endian(cfg[i]));
	}
}
