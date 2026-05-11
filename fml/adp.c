#include "regdef.h"
#include "printk.h"
#include "config.h"
#include "g_data.h"
//#include "usb_pd.h"
#include "debug.h"
#include "adp.h"
#include "tcpm.h"
#include "_wpc.h"
#include "config.h"

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
	wpc_printk("\r\n ADP-> %02X %d %d %d", gd->adp.adp_type, gd->adp.volt_min, gd->adp.volt_max, gd->adp.pwr_high);
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
#if ONLY7_5W_ENALBE
	fml_adp_type_set(EADP_TYPE_POWERBANK_WIRELESS_ONLY,  5000, 13000, 10 * 2);
#else
	fml_adp_type_set(EADP_TYPE_POWERBANK_WIRELESS_ONLY,  5000, 19500, 15 * 2);
#endif
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
	wpc_printk("\r\n ADP-> %02X %d %d %d", gd->adp.adp_type, gd->adp.volt_min, gd->adp.volt_max, gd->adp.pwr_high);

	//fml_adp_volt_set(9000);
}

void fml_adp_update(void)
{

}

void fml_adp_volt_set(uint16_t volt)
{
	wpc_printk("adp[%x] = %d \n",gd->adp.adp_type,volt);
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
		default:
			break;
	}
}
