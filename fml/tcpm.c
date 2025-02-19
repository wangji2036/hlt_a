#include "regdef.h"
#include "printk.h"
#include "delay.h"
#include "osal.h"
#include "tcpm.h"
#include "pd.h"
#include "typec.h"
#include "buckboost.h"
#include "usb_pd.h"
#include "adp.h"
#include "_wpc.h"
#include "g_data.h"
#include "pid.h"
#include "usb_qc.h"
#include "port_manager.h"

uint16_t port_vbus = 5000;
uint16_t qi_volt = 5000;
uint16_t port_defualt_voltage = 5000;
uint8_t wpc_mode = TCPM_WPC_WORK_BOOST;
uint8_t wpc_mode_pre = TCPM_WPC_WORK_BOOST;
uint8_t tcpm_qi_work_delay = 0;

static uint8_t usba_state = 0;
static uint8_t usba_cnt = 0;
uint8_t qi_state = 0;
static uint8_t qi_cnt = 0;


#define _UI_PIN1_PORT     GPA
#define _UI_PIN2_PORT     GPC
#define _UI_PIN1_PINx     PIN4
#define _UI_PIN2_PINx     PIN5

//#define TEST_PIN
#ifdef TEST_PIN
void test_pin2_out(bool status)
{
	_UI_PIN2_PORT->I_EN.BITS._UI_PIN2_PINx = 0;
	_UI_PIN2_PORT->DOUT.BITS._UI_PIN2_PINx = status;
	_UI_PIN2_PORT-> O_EN.BITS._UI_PIN2_PINx = 1;
}

void test_pin1_out(bool status)
{
	_UI_PIN1_PORT->I_EN.BITS._UI_PIN1_PINx = 0;
	_UI_PIN1_PORT->DOUT.BITS._UI_PIN1_PINx = status;
	_UI_PIN1_PORT-> O_EN.BITS._UI_PIN1_PINx = 1;
}
#endif

void tcpm_task_init(void)
{
	osal_task_handler_reg(USB_TASK, tcpm_task_event_handler);
	osal_start_timerEx(USB_TC_PD_TIMER, 1, 1, USB_TASK, TCPM_EVT_TIME_PERIOD);
	usb_tc_init();
	usb_pd_init();
#ifdef TEST_PIN
	test_pin1_out(1);
	test_pin2_out(0);
#endif
	//fml_adp_type_set(EADP_TYPE_DCSRC_09V,  9000, 19500, 15 * 2);
}


void tcpm_tc_set_state(struct tc_s * tc,enum usb_tc_state_e tc_state,enum usb_tc_substate_e tc_substate)
{
	tc->usb_tc_state = tc_state;
	tc->usb_tc_substate = tc_substate;
}

void tcpm_stop_wpc(uint8_t delay_ping_unit)
{
	tcpm_qi_work_delay = delay_ping_unit;
	wpc_stop_to_idle(ESYS_ERR_CODE_TYPEC_CHANGE);
	printk("wpc stop = %d\n",delay_ping_unit);
}

void tcpm_disable_usba_detect(void)
{
	if(usba_state == 0)
	{
		g_buckboost.usba_dectet_en = buckboost_ops.en_a2_detect(false);
#ifdef TEST_PIN
		test_pin1_out(0);
#endif
		usba_cnt = 0;
	}
}

void tcpm_set_port_sdp(uint8_t tc_index)
{
	if(tc_index == 0) 		DPDM->SOURCE_CTRL.BITS.PORT1_CTRL = 0;
	else if(tc_index == 1) 	DPDM->SOURCE_CTRL.BITS.PORT3_CTRL = 0;
	else if(tc_index == 2) 	DPDM->SOURCE_CTRL.BITS.PORT2_CTRL = 0;

	hal_tcpc_set_cc(tc_index,TYPEC_CC_RP_DEF);

	printk("PORT[%d] set sdp\n",tc_index);
}


void tcpm_update_wpc_work_mode(enum wpc_work_mode mode)
{
	uint32_t source_pdo;

	wpc_mode = mode;
	switch(mode)
	{
		case TCPM_WPC_WORK_FIX5V:
			fml_adp_type_set(EADP_TYPE_POWERBANK_05V,  5000, 5000, 5 * 2);
		    pid_set_volt_limit(gd->adp.volt_max, gd->adp.volt_min, gd->adp.volt_min);
			break;
		case TCPM_WPC_WORK_ADP_FIX:
			fml_adp_type_set(EADP_TYPE_POWERBANK_09V,  9000, 9000, 10 * 2);
			pid_set_volt_limit(gd->adp.volt_max, gd->adp.volt_min, gd->adp.volt_min);
			break;
		case TCPM_WPC_WORK_BOOST:
		#if(BUCKBOOST_USED_NU6801 == 1)
			fml_adp_type_set(EADP_TYPE_POWERBANK_WIRELESS_ONLY,  9000, 16500, 15 * 2);
		#else
			fml_adp_type_set(EADP_TYPE_POWERBANK_WIRELESS_ONLY,  9000, 19500, 15 * 2);
		#endif
			pid_set_volt_limit(gd->adp.volt_max, gd->adp.volt_min, gd->adp.volt_min);
			//printk("\r\n adapter updated! BOOST");
			break;
		case TCPM_WPC_WORK_PD_PPS:
			source_pdo = (uint32_t)g_usb_pd_s.snk_rx_source_cap[rdo_index(g_usb_pd_s.snk_rdo) - 1];
			fml_adp_type_set(EADP_TYPE_POWERBANK_PPS,  9000, pdo_pps_apdo_max_voltage(source_pdo), 15 * 2);
			pid_set_volt_limit(gd->adp.volt_max, gd->adp.volt_min, gd->adp.volt_min);
			//printk("\r\n adapter updated! PPS");
			break;
		case TCPM_WPC_WORK_DISABLE:
			break;
	}
	printk("wpc_mode= %d\n",mode);
}



void tcpm_task_event_handler(uint32_t event)
{
	switch (event)
	{
		case TCPM_EVT_TIME_PERIOD:
			usb_pdevt_run();
			usb_pd_run();
			usb_tc_run();
			break;
		case TCPM_EVT_USBA_SCAN:
			if(g_buckboost.usba_state && g_buckboost.usba_dectet_en)
			{
#ifdef TEST_PIN
				test_pin2_out(1);
#endif
				usba_cnt = 0;
				printk("qi_state= %d usba_state =%d wpc_mode=%d \n",qi_state,usba_state,wpc_mode);
				if(usba_state == 0)
				{
					port_manager_set_event(PORT2_EVENT_TRY_CONNECT);
					usba_state = 1;
				}
			}

			if(usba_state)
			{
#if(BUCKBOOST_USED_SW7201 == 1)
				if(g_buckboost.adc_ibus >= -100 && g_buckboost.adc_ibus <= 0 )
				{
					usba_cnt++;
					if(usba_cnt >= 50)
					{
						usba_cnt = 0;
						usba_state = 0;
#ifdef TEST_PIN
						test_pin2_out(0);
#endif
						port_manager_set_event(PORT2_EVENT_UNCONNECT);
						printk("qi_state= %d usba_state =%d wpc_mode=%d \n",qi_state,usba_state,wpc_mode);
					}
				}
				else
					usba_cnt = 0;

#elif(BUCKBOOST_USED_NU6801 == 1)
				if(g_buckboost.adc_iac1  < 20 )
				{
					usba_cnt++;
					if(usba_cnt >= 50)
					{
						usba_cnt = 0;
						usba_state = 0;
#ifdef TEST_PIN
						test_pin2_out(0);
#endif
						port_manager_set_event(PORT2_EVENT_UNCONNECT);
						printk("qi_state= %d usba_state =%d wpc_mode=%d \n",qi_state,usba_state,wpc_mode);
					}
				}
				else
					usba_cnt = 0;
#endif
				//printk("usba_state =%d wpc_mode=%d \n",qi_state,usba_state,wpc_mode);
			}

			if(qi_state == 1 && gd->ptx_protocol_phase <= WPC_PHASE_PING)
			{
				qi_cnt++;
				if(qi_cnt >= 100)
				{
					qi_state = 0;
					qi_cnt = 0;
					printk("qi_state= %d usba_state =%d wpc_mode=%d \n",qi_state,usba_state,wpc_mode);
					port_manager_set_event(PORT3_EVENT_UNCONNECT);
				}
			}
			else
				qi_cnt = 0;
			//printk("qi_state= %d usba_state =%d wpc_mode=%d \n",qi_state,usba_state,wpc_mode);
			break;

		case TCPM_EVT_USBA_REDETECT:
			if(usba_state == 0)
			{
				osal_start_timerEx(TCPM_USB_A_TIMER, 300, 0, USB_TASK, TCPM_EVT_USBA_DETEN);
				buckboost_ops.usb_a_dischg_en(true);
			}
			break;
		case TCPM_EVT_USBA_DETEN:
			buckboost_ops.usb_a_dischg_en(false);
			g_buckboost.usba_dectet_en = buckboost_ops.en_a2_detect(true);
			buckboost_ops.vbus_dischg_en(true);
#ifdef TEST_PIN
			test_pin1_out(1);
#endif
			buckboost_ops.vbus_dischg_en(false);
			printk("enable A det\n");
			break;
		case TCPM_EVT_QI_SET_VOLT:
			if(wpc_mode == TCPM_WPC_WORK_BOOST)
			{
				hal_tcpc_pd_set_bus_iv(WPC_INDEX,qi_volt,3500,0,0);

			}
			else if(wpc_mode == TCPM_WPC_WORK_PD_PPS)
			{
				uint32_t source_pdo = (uint32_t)g_usb_pd_s.snk_rx_source_cap[g_usb_pd_s.snk_rx_pdo_n - 1];
				usb_pd_requsrt_voltage(g_usb_pd_s.snk_rx_pdo_n,qi_volt,pdo_pps_apdo_max_current(source_pdo));
			}
			printk("wpc[%d] set volt = %d\n",wpc_mode,qi_volt);
			break;
		case TCPM_EVT_QI_WORK:
			if(qi_state == 0)   //无线充接入事件发生
			{
				qi_state = 1;
				port_manager_set_event(PORT3_EVENT_TRY_CONNECT);
				printk("qi_state= %d usba_state =%d wpc_mode=%d \n",qi_state,usba_state,wpc_mode);
			}
			break;
		default:
			break;
	}
}
