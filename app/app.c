#include "regdef.h"
#include "printk.h"
#include "g_data.h"
#include "config.h"
#include "delay.h"
#include "debug.h"
#include "prot.h"
#include "_wpc.h"
#include "led.h"
#include "app.h"
#include "ask.h"

#define T_APP_250ms_POLL    250
#define T_APP_100ms_POLL    100
#define T_APP_010ms_POLL     10

void apl_task_init(void)
{
	fml_tntc_otp_init();
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
#endif
			break;
		case APL_EVT_100ms_POLL:
			gd->sys_infos.ntc_temp = fml_ntc_temp_get();
			gd->sys_infos.die_temp = fml_die_temp_get();
//			fml_tntc_otp_check(gd->sys_infos.ntc_temp);
//			fml_tntc_utp_check(gd->sys_infos.ntc_temp);
//			fml_tdie_otp_check(gd->sys_infos.die_temp);
//			fml_tdie_utp_check(gd->sys_infos.die_temp);
			break;
		case APL_EVT_010ms_POLL:
			gd->isns = hal_badc_meas(_BADC_CH_PD6_ADC3);
			gd->vpwr = hal_badc_meas(_BADC_CH_PD0_ADC8);
			//gd->tx_power = gd->isns * gd->vpwr / 1000;//TODO
			gd->vbus = hal_badc_meas(_BADC_CH_PB6_ADC7);
//			fml_isns_ocp_check(gd->isns);
//			fml_vpwr_ovp_check(gd->vpwr);
//			fml_vbus_ovp_check(gd->vbus);
//			fml_vbus_uvp_check(gd->vbus);
//			fml_vbus_dpl_check(gd->vbus);
//			fml_pout_opp_check(gd->vpwr, gd->isns);
			if (gd->ptx_protocol_phase >= WPC_PHASE_XFER)
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

//must be 4 byte aligned.
const uint8_t m_arr_u8ConstInfoData[4] __attribute__ ((unused, section(".chip"))) = { 0x00, (RELE_DATE>>8)&0xFF, (RELE_DATE>>0)&0xFF, TX_FW_VER, };
