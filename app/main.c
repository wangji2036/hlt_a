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
#include "nu6805.h"
#include "i2cm.h"
#include "_fml.h"
#include "_wpc.h"
#include "wpc_ping.h"
#include "fm1210.h"
#include "t91206.h"
//#include "usb_pd.h"
#include "tcpm.h"
#include "port_manager.h"
#include "usb_qc.h"
#include "buckboost.h"
#include"sleep.h"
#include"bsp.h"
#include "bat.h"
#include "bat_record.h"
#include "config.h"

#if SUPPORT_MAIN_LOG
	#define main_printk 	printk
#else
	#define main_printk(...)
#endif

uint32_t rrlen;

extern uint8_t array_digest[];
extern uint8_t adt_data_recv_buf[18];
extern uint8_t cert_chain[];

extern void tc_init(void);
extern void tcpm_init(void);
#include "wpc_5_xfer_4_dstrm.h"
int main(void)
{
	if(gd->power_on_magic = 0xaaaa)
	{
		TCPC->CCB_CTRL.BITS.CC_DB_RD_DIS = 1; // enable cc block
		TCPC->CCB_CTRL.BITS.CC_BLOCK_DIS = 1; // enable cc block
	}
	
	VIC_vModuleDisable();

	hal_wdt_init();

	RST_vCheck();

	ap_data_init();
	gd_data_init();
	lib_para_init();// do not delete.
	fml_bsp_init();
	main_printk("\r\n [D1-] post-bsp");
	apl_gui_init();
	main_printk("\r\n [D2] post-gui");

	gd->adp.adp_type = EADP_TYPE_IDUNKNOWN;
	main_printk("\r\n [D3] pre-nu103x");
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
	main_printk("\r\n [D4] post-nu103x");
	hal_wdt_feed();
	main_printk("\r\n [D5] post-wdt");
#if(BUCKBOOST_USED_NU6805 == 1)
	/* 等待 NU6805 稳定 500ms, 每 100ms 做一次 I2C dummy read 保持 SCL 活跃,
	 * 防止复位 IC 因 250ms 无 I2C 活动而拉 RESET 导致冷启动 */
	for (uint8_t i = 0; i < 5; i++) {
		delay_1ms(100);
		hal_wdt_feed();
		uint8_t dummy;
		hal_i2cm_read_one_byte(NU6805_I2C_DEV_ADDR, 0x00, &dummy);
	}
#endif
//	WPC_vInit();

	main_printk("\r\n ap_t size-> %d", sizeof(struct ap_t));
	main_printk("\r\n base_q [%d]", ap->q_factor_base_value);
	main_printk("\r\n base_fre [%d]", ap->fs_base_value);
	main_printk("\r\n gd_t size-> %d %08x", sizeof(struct gd_t), &gd->pid_perd);
	main_printk("\r\n -->NU%d-%02d", SYS->PID_INFO.BITS.PID, SYS->PID_INFO.BITS.VER);
    main_printk("system state---> %x",SYS->OPR_STAT.WORD);
	//SLP_vNormalToSleep();

	if (ap->auth_seic_type == 1)
	{
		t91206_init();
		delay_1ms(100);
		hal_wdt_feed();
//		t91206_get_qi_id(adt_data_recv_buf);
		t91206_read_cert_hash(array_digest + 1);
		t91206_read_se_cert(cert_chain, &rrlen);
		t91206_get_qi_id(adt_data_recv_buf);
	}
	else
	{
		fm1210_init();
		delay_1ms(100);
		hal_wdt_feed();
		fm1210_get_qi_id(adt_data_recv_buf);
		fm1210_read_cert_hash(array_digest + 1);
		fm1210_read_se_cert(cert_chain + 2 + 32 + 328, &rrlen);//TODO: mfr cert len 329 need outside config, using sizeof arr
	}
	hal_wdt_feed();
	fml_adp_init();

#if CONFIG_NEW_CCC_LOG_ENABLE
	battery_record_init();
#endif
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
