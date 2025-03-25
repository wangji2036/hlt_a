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
#include "t91206.h"
#include "usb_pd.h"
#include "tcpm.h"
#include "port_manager.h"
#include "usb_qc.h"
#include "buckboost.h"
#include"sleep.h"
#include"bsp.h"

uint32_t rrlen;

extern uint8_t array_digest[];
extern uint8_t adt_data_recv_buf[18];
extern uint8_t cert_chain[];

extern void tc_init(void);
extern void tcpm_init(void);
#include "wpc_5_xfer_4_dstrm.h"
int main(void)
{
	RST_vCheck();

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
#if(BUCKBOOST_USED_SW7201 == 1)
	delay_1ms(500);
#endif
//	WPC_vInit();

	printk("\r\n ap_t size-> %d", sizeof(struct ap_t));
	printk("\r\n base_q [%d]", ap->q_factor_base_value);
	printk("\r\n base_fre [%d]", ap->fs_base_value);
	printk("\r\n gd_t size-> %d %08x", sizeof(struct gd_t), &gd->pid_perd);
	printk("\r\n -->NU%d-%02d", SYS->PID_INFO.BITS.PID, SYS->PID_INFO.BITS.VER);
    printk("system state---> %x",SYS->OPR_STAT.WORD);
	//SLP_vNormalToSleep();
	if (ap->auth_seic_type == 1)
	{
		t91206_init();
		delay_1ms(100);
//		t91206_get_qi_id(adt_data_recv_buf);
		t91206_read_cert_hash(array_digest + 1);
		t91206_read_se_cert(cert_chain, &rrlen);
		t91206_get_qi_id(adt_data_recv_buf);
	}
	else
	{
		fm1210_init();
		delay_1ms(100);
		fm1210_get_qi_id(adt_data_recv_buf);
		fm1210_read_cert_hash(array_digest + 1);
		fm1210_read_se_cert(cert_chain + 2 + 32 + 328, &rrlen);//TODO: mfr cert len 328 need outside config, using sizeof arr
	}

	fml_adp_init();

	osal_init();
	apl_task_init();

	buckboost_task_init();
	tcpm_task_init();
	usb_dpdm_task_init();


	fml_task_init();
#if(CONFIG_WPC_SUPPORT == 1)
	wpc_task_init();
#endif
	port_manager_task_init();


	osal_start_system();

	return 0;
}
