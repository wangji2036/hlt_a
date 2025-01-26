#include "regdef.h"
#include "printk.h"
#include "config.h"
#include "g_data.h"
#include "usb_pd.h"
#include "debug.h"
#include "adp.h"
#include "tcpm.h"

void fml_adp_type_set(enum adp_type_t adp_type, uint16_t volt_min, uint16_t volt_max, uint16_t pwr_high)
{
	if (gd->adp.adp_type != adp_type || gd->adp.volt_min != volt_min || gd->adp.volt_max != volt_max || gd->adp.pwr_high != pwr_high)
	{
		gd->adp_type_upd = 1;
	}
	gd->adp.adp_type = adp_type;
	gd->adp.volt_min = volt_min;
	gd->adp.volt_max = volt_max;
	gd->adp.pwr_high = pwr_high;
	printk("\r\n ADP-> %02X %d %d %d", gd->adp.adp_type, gd->adp.volt_min, gd->adp.volt_max, gd->adp.pwr_high);
	osal_clear_event(USB_TASK,TCPM_EVT_QI_SET_VOLT);
}

void fml_adp_init(void)
{
	//if (gd->adp.adp_type == EADP_TYPE_IDUNKNOWN)
//	{
//		gd->adp.adp_type = EADP_TYPE_IDUNKNOWN;
//
//		gd->vbus = 9000;//hal_badc_meas(_BADC_CH_PB6_ADC7);
//		if (gd->vbus > 11000)
//		{
//			fml_adp_type_set(EADP_TYPE_DCSRC_12V, 12000, 19500, 15 * 2);
//		}
//		else if (gd->vbus > 8000)
//		{
//			fml_adp_type_set(EADP_TYPE_DCSRC_09V,  9000, 19500, 15 * 2);
//		}
//		else
//		{
//			fml_adp_type_set(EADP_TYPE_DCSRC_05V,  9000, 11500,  5 * 2);
//			ap->vbus_uvp_thd = 4000;
//		}
//	}
//

	// 15w, set buck-boost to voltage adjust mode,

	fml_adp_type_set(EADP_TYPE_POWERBANK_WIRELESS_ONLY,  9000, 19500, 15 * 2);

//	fml_adp_type_set(EADP_TYPE_POWERBANK_09V,  9000, 9000, 15);
	//fml_adp_type_set(EADP_TYPE_POWERBANK_05V,  5000, 5000, 10);
	ap->vbus_uvp_thd = 4000;
//	else
//	{
//		if (usb_pd_9v_flag)
//		{
//			USBPD_vSetVolt(9000);
//		}
//	}
//
	printk("\r\n ADP-> %02X %d %d %d", gd->adp.adp_type, gd->adp.volt_min, gd->adp.volt_max, gd->adp.pwr_high);

	//fml_adp_volt_set(9000);
}

void fml_adp_update(void)
{

}

void fml_adp_volt_set(uint16_t volt)
{
	printk("adp[%d] = %d \n",gd->adp.adp_type,volt);
	switch (gd->adp.adp_type)
	{
		case EADP_TYPE_PD2P0_05V:
		case EADP_TYPE_PD3P0_10W:
		case EADP_TYPE_PD3P0_20W:
		case EADP_TYPE_PD3P0_30W:
		case EADP_TYPE_PD3P0_50W:
			break;
		case EADP_TYPE_QC3P0_12V:
		case EADP_TYPE_QC3P0_20V:
			break;
		case EADP_TYPE_DCSRC_05V:
		case EADP_TYPE_QC2P0_09V:
		case EADP_TYPE_DCSRC_09V:
		case EADP_TYPE_DCSRC_12V:
		case EADP_TYPE_PD2P0_09V:
		case EADP_TYPE_PD2P0_12V:
		case EADP_TYPE_POWERBANK_05V:
		case EADP_TYPE_POWERBANK_09V:
			break;
		case EADP_TYPE_POWERBANK_WIRELESS_ONLY:
		case EADP_TYPE_POWERBANK_PPS:
			qi_volt = volt;
			osal_set_event(USB_TASK,TCPM_EVT_QI_SET_VOLT);
			break;

			//if(wpc_mode == TCPM_WPC_WORK_BOOST)
			//	hal_tcpc_pd_set_bus_iv(3,volt,3000,0,0);

			//else
			//if((g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE) && g_tc[0].usb_tc_state ==  TC_DRP_TOGGLE && g_tc[1].usb_tc_state ==  TC_DRP_TOGGLE)
			//	hal_tcpc_pd_set_bus_iv(3,volt,3000,0,0);
//			{
//				uint16_t tmp_duty;
//				tmp_duty = (20091 - volt) * 100 / 1263;
//				if (tmp_duty > 900) tmp_duty = 900;
//				if (tmp_duty <   1) tmp_duty =   1;
//				if (SYS->PID_INFO.BITS.PID == NU17111)
//				{
//					hal_bpwm_update(BPWM3, BPWM3->PWM_CTRL.BITS.PERD + 1, tmp_duty);
//				}
//				else
//				{
//					hal_bpwm_update(BPWM8, BPWM8->PWM_CTRL.BITS.PERD + 1, tmp_duty);//TODO:did the boost volt regulation test?
//				}
//			}
			break;
		default:
			break;
	}
}
