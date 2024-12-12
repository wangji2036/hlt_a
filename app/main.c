#include "regdef.h"
#include "printk.h"
#include "delay.h"
#include "app.h"
#include "bsp.h"
#include "isr.h"
#include "gui.h"
#include "prot.h"
#include "epwm.h"
#include "fsk.h"
#include "qdt.h"
#include "ask.h"
#include "led.h"
#include "g_data.h"
#include "osal.h"
#include "_fml.h"
#include "_wpc.h"
#include "wpc_ping.h"
#include "fm1210.h"
#include "usb_pd.h"
#include "tcpm.h"
#include "usb_qc.h"

uint16_t rrlen;

extern uint8_t array_digest[];
extern uint8_t adt_data_recv_buf[18];
extern uint8_t cert_chain[];

extern void tc_init(void);
extern void tcpm_init(void);

int main(void)
{
	ap_data_init();
	gd_data_init();

	fml_bsp_init();
	apl_gui_init();

	gd->adp.adp_type = EADP_TYPE_IDUNKNOWN;
//	uint32_t timeout = 0;
//	USBPD_vInit();
//	while (1)
//	{
//		USBPD_vStateMachine();
//		delay_1ms(1);
//		if (++timeout > 1000)
//		{
//			break;
//		}
//	}
//
//	fml_usbqc_init();

	fml_nu103x_por_init();

	delay_1ms(500);
//	WPC_vInit();

	printk("\r\n ap_t size-> %d", sizeof(struct ap_t));
	printk("\r\n base_q [%d]", ap->q_factor_base_value);
	printk("\r\n base_fre [%d]", ap->fs_base_value);
	printk("\r\n gd_t size-> %d %08x", sizeof(struct gd_t), &gd->pid_perd);
	fm1210_init();

	printk("\r\n -->NU%d-%02d UID->%08X", SYS->PID_INFO.BITS.PID, SYS->PID_INFO.BITS.VER, SYS->UID_INFO.BITS.UID);

	delay_1ms(100);
	fm1210_get_qi_id(adt_data_recv_buf);
	fm1210_read_cert_hash(array_digest + 1);
	fm1210_read_se_cert(cert_chain + 2 + 32 + 328, &rrlen);//TODO: mfr cert len 328 need outside config, using sizeof arr

//	fm1210_get_cert_chain((uint8_t *)wpc_cert_hash, (uint8_t *)manufacturer_cert, sizeof(manufacturer_cert), rrbuf, &rrlen);
//	fm1210_get_tbs_auth(rrbuf);

	fml_adp_init();

	osal_init();
	apl_task_init();
	tcpm_task_init();
	usb_dpdm_task_init();
	buckboost_task_init();


	fml_task_init();
	//wpc_task_init();



	osal_start_system();

	return 0;
}
