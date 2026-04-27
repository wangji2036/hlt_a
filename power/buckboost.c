#include "regdef.h"
#include "buckboost.h"
#include "nu6805.h"
#include "nu6801.h"
#include "printk.h"
#include "tcpm.h"
#include "pdlib.h"
#include "port_manager.h"
#include "config.h"
#include "config.h"
#include "tcpm.h"
#include "g_data.h"
#include "led.h"
#include "bat.h"
#include "ntc.h"
#include "typec.h"
#include "dpdm.h"

uint8_t buckboost_protection_flag = false;
uint8_t zero_soc_cnt =0;
static bool adc_protect_flag = false;
#if(BUCKBOOST_USED_NU6801 == 1)
static bool adc_err_flag = 0;
#endif
#if(BUCKBOOST_USED_NU6805 == 1)
extern uint8_t bat_cell_num;
#endif
struct buckboost_s  g_buckboost;
uint16_t g_vref_mv = VREF_DEFAULT_MV;
uint8_t g_vref_cal_delay = 0;
int16_t ibus_to_ibat(int16_t ibus,int16_t vbus,int16_t vbat)
{
	int16_t k,b;
	int32_t actual_effiency;
	int32_t temp_ibat;
	// x1 =5,y1= 970; x2 = 9, y2= 950;
	// xielv k ----- (y2-y1)/(x2-x1) , so k = (950-970)/(9-5) = -5,  k  used as * 100
	// jieju ------y1=kx1+b, b= y1-kx1, so  b = 970 - (-5)*5 = 995
	// y= (k * x) + b; effiency, used as *1000
	if(ibus<0)// buck mode
	{
		if(vbus> 15000)
		{
			k= -375;
			b = 981;
		}
		else if(vbus> 12000)
		{
			k = -300;
			b = 1000;
		}
		else if(vbus> 9000)
		{
			 k = -333;
			 b = 980;
		}
		else // <9v
		{
			k = -500;
			b = 995;
		}
	}
	else // boost mode
	{

		if(vbus> 15000)
		{
            k= -375;
            b = 981;
		}
		else if(vbus> 12000)
		{
            k = -300;
            b = 1000;
		}
		else if(vbus> 9000)
		{
			 k = -333;
			 b = 980;
		}
		else // <9v
		{
			k = -500;
			b = 995;
		}
	}
	//bb_printk(" \r\nvbus = %d vbat= %d k= %d b= %d\n",vbus,vbat,k,b);
	actual_effiency = (k*(vbus/100))/1000 +b;
	//bb_printk("\r\n effi = %d \n",actual_effiency);
//	temp_ibat = ((actual_effiency*((vbus*ibus) /1000))/vbat);

	if(ibus>=0)
	{
	    temp_ibat = ((actual_effiency*((vbus*ibus) /1000))/vbat);
	}
	else
	{
		temp_ibat = (((vbus*ibus*10) /actual_effiency) *100)/vbat;
	}
    return (int16_t)temp_ibat;
}
void buckboost_set_bus_iv(uint16_t voltage,uint16_t current,uint16_t wait, uint16_t delay)
{

	bb_printk("OUT = %d %d\n",voltage,current);

	if(voltage != g_buckboost.buckboost_out_voltage)
	{
		osal_start_timerEx(BUCKBOOST_VBUS_DISG_TIMER, 300, 0, BUCKBOOST_TASK, BUCKBOOST_EVT_DUMMYLOADOVER);
		buckboost_ops.vbus_dischg_en(true);
		buckboost_ops.set_ovp(20000);
		bb_printk("vbus disg start\n");
	}

	osal_stop_timerEx(BUCKBOOST_REGULATOR_TIMER);
	g_buckboost.out_voltage_wait = wait;
	g_buckboost.out_voltage_delay = delay;
	g_buckboost.buckboost_out_voltage = voltage;
	g_buckboost.buckboost_out_current = current;
	g_buckboost.regulator_state = 0;
	osal_set_event(BUCKBOOST_TASK,BUCKBOOST_EVT_SET_DISCHG_VBUS_VOLT);
}

void buckboost_set_charge_current(uint16_t ibat,uint16_t ibus)
{
	g_buckboost.chager_ibus_limit = ibus;
	g_buckboost.chager_ibat_limit = ibat;

//	g_buckboost.woke_mode = BUCKBOOST_SHUTDOWM_MODE;
//	buckboost_ops.set_work_mode(BUCKBOOST_SHUTDOWM_MODE);
//	g_buckboost.woke_mode = BUCKBOOST_CHAGER_MODE;
//	buckboost_ops.set_work_mode(g_buckboost.woke_mode);
	buckboost_ops.set_chager_ibus_limit(300);
	buckboost_ops.set_chager_ibat_limit(ibat);
	g_buckboost.chager_ibus_value = 300;
	g_buckboost.chager_ibus_start = 1;
	bb_printk("Charging = [%d %d]!\n",ibat,ibus);
}

void buckboost_set_work_mode(enum buckboost_mode mode)
{
	g_buckboost.woke_mode = mode;

	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
	{
		g_buckboost.chager_ibat_limit = 300;
		g_buckboost.chager_ibus_limit = 300;
		buckboost_ops.set_chager_ibat_limit(g_buckboost.chager_ibat_limit);
		buckboost_ops.set_chager_ibus_limit(g_buckboost.chager_ibus_limit);
	}

	g_buckboost.chager_ibus_start = 0;
	g_buckboost.chager_ibus_value = 200;
	buckboost_ops.set_work_mode(g_buckboost.woke_mode);

#if(BUCKBOOST_USED_NU6805 == 1)
	{
		uint8_t reg_mode = 0, reg_status = 0;
		hal_i2cm_read_one_byte(NU6805_I2C_DEV_ADDR, REG_Mode_Control, &reg_mode);
		hal_i2cm_read_one_byte(NU6805_I2C_DEV_ADDR, REG_System_Status, &reg_status);
		bb_printk("\r\n[BB_MODE] set=%d reg_mode=0x%02x sys_status=0x%02x", mode, reg_mode, reg_status);
	}
#endif
}


void buckboost_set_typeca_gate_en(bool en)
{
	g_buckboost.set_typeca_gate_en = en;
	buckboost_ops.typca_gate_en(g_buckboost.set_typeca_gate_en);
}

void buckboost_set_typecb_gate_en(bool en)
{
	g_buckboost.set_typecb_gate_en = en;
	buckboost_ops.typcb_gate_en(g_buckboost.set_typecb_gate_en);
}

void buckboost_set_usb_a_gate_en(bool en)
{
	g_buckboost.set_usb_a_gate_en = en;
	buckboost_ops.usb_a_gate_en(g_buckboost.set_usb_a_gate_en);
}

bool buckboost_regulator_done(void)
{
	return g_buckboost.regulator_state;
}

void buckboost_task_init(void)
{
	osal_mem_set(&g_buckboost,0,sizeof(struct buckboost_s));
	osal_task_handler_reg(BUCKBOOST_TASK, buckboost_task_event_handler);
	osal_start_timerEx(BUCKBOOST_PERIOD_TIMER, BUCKBOOST_TIME_PERIOD, BUCKBOOST_TIME_PERIOD, BUCKBOOST_TASK, BUCKBOOST_EVT_TIME_PERIOD);
	osal_start_timerEx(BUCKBOOST_VBUS_TIMER, BUCKBOOST_VBUS_PERIOD, BUCKBOOST_VBUS_PERIOD, BUCKBOOST_TASK, BUCKBOOST_EVT_VBUS_PERIOD);
	osal_start_timerEx(BUCKBOOST_CHAGER_TIMER, BUCKBOOST_CHAG_PERIOD, BUCKBOOST_CHAG_PERIOD, BUCKBOOST_TASK, BUCKBOOST_EVT_CHAG_PERIOD);
	buckboost_ops.init();

	g_buckboost.adc_vbat = buckboost_ops.get_bat_voltage();
	g_buckboost.adc_ibat = buckboost_ops.get_bat_current();
	g_buckboost.adc_ibus = buckboost_ops.get_bus_current();
	g_buckboost.adc_vbus = buckboost_ops.get_bus_voltage();
#if(CONFIG_USBA_SUPPORT == 1)
	g_buckboost.usba_dectet_en = buckboost_ops.en_a2_detect(true);
#endif
}



void buckboost_protection_handle(void)
{
#if (CONFIG_TRIPLE_CLICK_COMM_ENABLE == 1)
	if (gd->usb_comm_activated) return;
#endif

	static uint8_t cnt = 0;
#if(BUCKBOOST_USED_NU6805 == 1)
	static uint8_t nu6805_ocp_cnt = 0;
	#define NU6805_OCP_LOCK_COUNT 4
#endif

#if(BUCKBOOST_USED_NU6805 == 1)
	#define VBUS_FUALT_VBUS_OCP			BIT(1)
	#define VBUS_FUALT_VBUS_SCP			BIT(2)
	#define VBUS_FUALT_VBAT_UVP			BIT(3)
	#define VBUS_FUALT_VBAT_OVP			BIT(4)
	#define VBUS_FUALT_VBUS_OVP			BIT(5)
	#define VBUS_FAULT_VBUS_NTC         BIT(6)
	#define PPS_UV						BIT(9)
	#define NTC_PCT						BIT(10)
	#define VBUS_SOFT_PROTECT			BIT(13)
    #define VBUS_FAULT_VBUS_UVP			BIT(14)
#elif(BUCKBOOST_USED_NU6801 == 1)
	#define URB_DET						BIT(0)
	#define BST_UV_FLAG					BIT(1)
	#define VBAT_OV_FLAG				BIT(2)
	#define VBAT_LOW_FLAG				BIT(3)
	#define VBUS_OV_FLAG				BIT(4)
	#define VBUS_REL_LOW_FLAG			BIT(5)
	#define VBUS_UV_FLAG				BIT(6)
	#define HFET_OCP					BIT(7)

	#define DIS_VBAT_LOW				BIT(8)
	#define PPS_UV						BIT(9)
	#define NTC_PCT						BIT(10)
	#define ADC_ERR						BIT(11)
	#define SWITCH_ERR					BIT(12)
	#define VBUS_SOFT_PROTECT			BIT(13)

#endif

	#define NU6805_VBUS_OVP_TH					21500
	#define NU6801_VBUS_OVP_TH					20000




	uint16_t status = 0;

	status = buckboost_ops.get_protect_status();
	
	bb_printk("Flaut State = 0x%x\n",status);
	bb_printk("vbus = %d\n",g_buckboost.adc_vbus);
	bb_printk("\r\n[BB] mode=%d gate[a=%d b=%d] ov_f=%d uv_f=%d bypass=%d ibat=%d ibus=%d vbat=%d ilim[%d %d] soc=%d",
		g_buckboost.woke_mode,
		g_buckboost.set_typeca_gate_en,
		g_buckboost.set_typecb_gate_en,
		gd->bat_ov_forbid_flag,
		gd->bat_uv_forbid_flag,
		gd->forbid_bypass_flag,
		g_buckboost.adc_ibat,
		g_buckboost.adc_ibus,
		g_buckboost.adc_vbat,
		g_buckboost.chager_ibat_limit,
		g_buckboost.chager_ibus_limit,
		gd->real_soc_show);
#if(BUCKBOOST_USED_NU6805 == 1)
	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE) {
		uint8_t reg_mode=0, reg_ibat=0, reg_ibus=0, reg_cv_h=0, reg_cv_l=0, reg_set1=0, reg_sys=0;
		hal_i2cm_read_one_byte(NU6805_I2C_DEV_ADDR, REG_Mode_Control, &reg_mode);
		hal_i2cm_read_one_byte(NU6805_I2C_DEV_ADDR, REG_Charger_Ibat_Limit, &reg_ibat);
		hal_i2cm_read_one_byte(NU6805_I2C_DEV_ADDR, REG_Charger_Ibus_Limit, &reg_ibus);
		hal_i2cm_read_one_byte(NU6805_I2C_DEV_ADDR, REG_Charger_VbatVol_High, &reg_cv_h);
		hal_i2cm_read_one_byte(NU6805_I2C_DEV_ADDR, REG_Charger_VbatVol_Low, &reg_cv_l);
		hal_i2cm_read_one_byte(NU6805_I2C_DEV_ADDR, REG_Charger_Setting1, &reg_set1);
		hal_i2cm_read_one_byte(NU6805_I2C_DEV_ADDR, REG_System_Status, &reg_sys);
		bb_printk("\r\n[NU6805] mode=0x%02x ibat=0x%02x ibus=0x%02x cv[%02x:%02x] set1=0x%02x sys=0x%02x",
			reg_mode, reg_ibat, reg_ibus, reg_cv_h, reg_cv_l, reg_set1, reg_sys);
	}
	 if(g_buckboost.adc_vbus > g_buckboost.ovp_value&&g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE) status |= VBUS_FUALT_VBUS_OVP;
	if(g_buckboost.adc_vbus <= 4582 && g_buckboost.adc_ibus == 0 && g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
	{
		status |= VBUS_FAULT_VBUS_UVP;
	}
	if(g_buckboost.adc_vbat < 6300)// ||  zero_soc_cnt >240)// && g_buckboost.woke_mode != BUCKBOOST_CHAGER_MODE)// && !g_tc[TYPEC_PORT_A].is_deadbattery)
	{
		bb_printk("\r\n [BAT_DEAD] adc_vbat=%d bat_dead_flag=%d", g_buckboost.adc_vbat, gd->bat_dead_flag);    
        cnt++;
		if(cnt >= 10)
		{
			//if(g_buckboost.woke_mode != BUCKBOOST_CHAGER_MODE)

			if (g_port.port_state[0] != PORT_STATE_SINK) //if(g_buckboost.woke_mode != BUCKBOOST_CHAGER_MODE)
			{
				status |= VBUS_FUALT_VBAT_UVP;
				gd->bat_dead_flag = 1;
				bb_printk("\r\n [BAT_DEAD] adc_vbat=%d bat_dead_flag=%d", g_buckboost.adc_vbat, gd->bat_dead_flag);
			}
			else
			{
				gd->bat_dead_flag = 0;
				bb_printk("\r\n [BAT_DEAD] adc_vbat=%d bat_dead_flag=%d (sink,cleared)", g_buckboost.adc_vbat, gd->bat_dead_flag);
				if(pdlib_get_deadbat() == 0)
				{
					port_manager_set_event(PORT_EVENT_RESET_CHARGE);
				}
			}
			pdlib_set_deadbat(true);
			cnt = 0;
		}
	}
	else
	{
		cnt = 0;
        gd->bat_dead_flag = 0;
	}
	 //----1014

	if(status&VBUS_FUALT_VBAT_UVP)
	{
	    if(g_port.port_state[0] != PORT_STATE_SOURCE && g_port.port_state[3] != PORT_STATE_SOURCE) status &= ~VBUS_FUALT_VBAT_UVP;
		//if(g_tc[0].usb_tc_state == TC_SNK_Attached) status &= ~VBUS_FUALT_VBAT_UVP;
	}

		//---1014      //
	if(adc_protect_flag)
	{
		adc_protect_flag = false;
		status |= VBUS_SOFT_PROTECT;
	}
#if(BUCKBOOST_USED_NU6805 == 1)
	if(status & VBUS_FUALT_VBUS_OCP)
	{
		if(nu6805_ocp_cnt < NU6805_OCP_LOCK_COUNT) nu6805_ocp_cnt++;
		if(nu6805_ocp_cnt < NU6805_OCP_LOCK_COUNT)
		{
			bb_printk("ocp debounce %d/%d\n", nu6805_ocp_cnt, NU6805_OCP_LOCK_COUNT);
			status &= ~VBUS_FUALT_VBUS_OCP;
		}
	}
	else
	{
		nu6805_ocp_cnt = 0;
	}
#endif
	if(status&0x2006) gd->typec_scp = 1;
	if(status & VBUS_FUALT_VBUS_OVP) gd->vbus_ovp = 1;
	if(!gd->led_fault&&((status & 0x4060)||bat_ntc_stop_chrg_flag||(gd->typec_charge_ntc_lock&&g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)||gd->vbus_ovp))
	{
		gd->led_fault = 1;
	}
	if(!gd->led_fault1&&(gd->typec_scp||gd->bat_ntc_lock_flag||gd->wirless_ntc_lock||gd->typec_ntc_lock||bat_ntc_dual_dischg_lock))
	{
		gd->led_fault1 = 1;
	}
#if(CONFIG_USE_NTC_FOR_CHAGER == 1)
	if(gd->typec_ntc_lock||gd->bat_ntc_lock_flag||bat_ntc_dual_dischg_lock) status|=VBUS_FAULT_VBUS_NTC;
	printk("VBUS_FAULT_VBUS_NTC\n");
#endif
#endif
	if(gd->led_fault&&!(status&0x6060)&&!bat_ntc_stop_chrg_flag&&!(gd->typec_charge_ntc_lock&&g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)&&!gd->vbus_ovp)
	{
		gd->led_fault = 0;
	}
	if(gd->led_fault1&&!gd->bat_ntc_lock_flag&&!gd->wirless_ntc_lock&&!gd->typec_ntc_lock&&!gd->typec_scp&&!bat_ntc_dual_dischg_lock)
	{
		gd->led_fault1 = 0;
	}
	//bb_printk("\r\ngd->led_fault=%d\r\n",gd->led_fault);
	//bb_printk("ssss=%d\r\n",status & 0x4060);
	gd->fault_status = status;
	if(status != 0)
	{
#if(BUCKBOOST_USED_NU6805 == 1)
		if(status & (VBUS_FUALT_VBUS_SCP | VBUS_FUALT_VBUS_OVP | VBUS_FUALT_VBUS_OCP | VBUS_FUALT_VBAT_UVP | VBUS_SOFT_PROTECT | NTC_PCT |VBUS_FAULT_VBUS_NTC))
		{
			nu6805_ocp_cnt = 0;
			bb_printk("protect lock =0x%x\n",status);

			//bb_printk("vbus = %d\n",g_buckboost.adc_vbus);

			if(status & VBUS_FAULT_VBUS_NTC)
			{
				bb_printk("\r\n[VBUS_NTC] VBUS_FAULT_VBUS_NTC triggered!");
				bb_printk("\r\n[VBUS_NTC] typec_ntc_lock=%d, bat_ntc_lock_flag=%d, dual_dischg_lock=%d", gd->typec_ntc_lock, gd->bat_ntc_lock_flag, bat_ntc_dual_dischg_lock);
				bb_printk("\r\n[VBUS_NTC] typec_ntc_temp=%d, bat_temp=%d, wpc_ntc_temp=%d", 
					gd->sys_infos.ntc_temp_typec, g_buckboost.batTemp, gd->sys_infos.ntc_temp_wpc);
			}

			if(status & (VBUS_FUALT_VBUS_SCP | VBUS_FUALT_VBUS_OVP | VBUS_FUALT_VBUS_OCP | VBUS_FUALT_VBAT_UVP | VBUS_SOFT_PROTECT |VBUS_FAULT_VBUS_NTC))
			{
				//lock
				if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE) pdlib_disable_typec(PORT0_INDEX);
				if(g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE) pdlib_disable_typec(PORT1_INDEX);
			}
			else  //ntc
			{
				g_port.port_state[PORT0_INDEX] = PORT_STATE_NONE;
				g_port.port_state[PORT1_INDEX] = PORT_STATE_NONE;
				g_port.port_state[PORT2_INDEX] = PORT_STATE_NONE;
				g_port.port_state[PORT3_INDEX] = PORT_STATE_NONE;
				pdlib_disable_typec(PORT0_INDEX);
				pdlib_disable_typec(PORT1_INDEX);
			}

			hal_tcpc_set_gate_en(PORT0_INDEX,false);
			hal_tcpc_set_gate_en(PORT1_INDEX,false);
			hal_tcpc_set_gate_en(PORT2_INDEX,false);
			buckboost_set_bus_iv(5000,3300,0,0);
			pdlib_clear_typec_prswap(PORT0_INDEX);
			pdlib_clear_typec_prswap(PORT1_INDEX);
			pdlib_disable_usbpd();
			tcpm_stop_wpc(WPC_DELAY);
			qi_state = 0;
			gd->sigle_clicked = 0;
			tcpm_update_wpc_work_mode(TCPM_WPC_WORK_DISABLE);
			tcpm_disable_usba_detect();
			buckboost_ops.init();
			buckboost_protection_flag = 1;
		}
#elif(BUCKBOOST_USED_NU6801 == 1)
		//static bool protection_lock = false;
		if(status & (BST_UV_FLAG | URB_DET  | VBAT_OV_FLAG | VBUS_OV_FLAG | HFET_OCP | VBUS_UV_FLAG  | DIS_VBAT_LOW  | PPS_UV | NTC_PCT | ADC_ERR | VBUS_SOFT_PROTECT))
		{
			bb_printk("protect lock =0x%x\n",status);

			if(status & (URB_DET  | VBAT_OV_FLAG | VBUS_OV_FLAG | HFET_OCP | VBUS_UV_FLAG  | DIS_VBAT_LOW | PPS_UV | VBUS_SOFT_PROTECT | DIS_VBAT_LOW))
			{
				//lock
				if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE) pdlib_disable_typec(PORT0_INDEX);
				if(g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE) pdlib_disable_typec(PORT1_INDEX);
			}
			else  //ntc
			{
				g_port.port_state[PORT0_INDEX] = PORT_STATE_NONE;
				g_port.port_state[PORT1_INDEX] = PORT_STATE_NONE;
				g_port.port_state[PORT2_INDEX] = PORT_STATE_NONE;
				g_port.port_state[PORT3_INDEX] = PORT_STATE_NONE;
				pdlib_disable_typec(PORT0_INDEX);
				pdlib_disable_typec(PORT1_INDEX);
			}

			hal_tcpc_set_gate_en(PORT0_INDEX,false);
			hal_tcpc_set_gate_en(PORT1_INDEX,false);
			hal_tcpc_set_gate_en(PORT2_INDEX,false);
			buckboost_set_bus_iv(5000,3300,0,0);
			pdlib_clear_typec_prswap(PORT0_INDEX);
			pdlib_clear_typec_prswap(PORT1_INDEX);
			pdlib_disable_usbpd();
			tcpm_stop_wpc(WPC_DELAY);
			qi_state = 0;
			adc_err_flag = 0;
			tcpm_update_wpc_work_mode(TCPM_WPC_WORK_DISABLE);
			tcpm_disable_usba_detect();
			if(status & ADC_ERR) buckboost_ops.init();

			buckboost_protection_flag = 1;
		}
#endif
	}
	else
	{
#if(BUCKBOOST_USED_NU6805 == 1)
		nu6805_ocp_cnt = 0;
#endif
		if(buckboost_protection_flag && g_port.port_state[PORT0_INDEX] ==PORT_STATE_NONE &&g_port.port_state[PORT1_INDEX] ==PORT_STATE_NONE)
		{
			buckboost_protection_flag = 0;
			pdlib_restart_typec(PORT0_INDEX);
			pdlib_restart_typec(PORT1_INDEX);
			osal_set_event(USB_TASK, TCPM_EVT_USBA_REDETECT);
			tcpm_stop_wpc(WPC_DELAY);
			tcpm_update_wpc_work_mode(TCPM_WPC_WORK_BOOST);
			bb_printk("protect unlock\n");
			//buckboost_ops.init();
			buckboost_set_work_mode(BUCKBOOST_DISCHG_MODE);
			buckboost_set_bus_iv(5000,3300,0,0);
		}
	}

	g_buckboost.protect_status = status;
}

void buckboost_fault_restore(void)
{
	buckboost_protection_flag = 0;
	pdlib_restart_typec(PORT0_INDEX);
	pdlib_restart_typec(PORT1_INDEX);
	osal_set_event(USB_TASK, TCPM_EVT_USBA_REDETECT);
	tcpm_stop_wpc(WPC_DELAY);
	tcpm_update_wpc_work_mode(TCPM_WPC_WORK_BOOST);
	bb_printk("protect unlock\n");
	buckboost_ops.init();
	buckboost_set_work_mode(BUCKBOOST_DISCHG_MODE);
	buckboost_set_bus_iv(5000,3300,0,0);
}

#if (CONFIG_TRIPLE_CLICK_COMM_ENABLE == 1)
void usb_comm_lock(void)
{
	g_port.port_state[PORT0_INDEX] = PORT_STATE_NONE;
	g_port.port_state[PORT1_INDEX] = PORT_STATE_NONE;
	g_port.port_state[PORT2_INDEX] = PORT_STATE_NONE;
	g_port.port_state[PORT3_INDEX] = PORT_STATE_NONE;

	hal_tcpc_set_gate_en(PORT0_INDEX, false);
	hal_tcpc_set_gate_en(PORT1_INDEX, false);
	hal_tcpc_set_gate_en(PORT2_INDEX, false);

	buckboost_set_bus_iv(5000, 3300, 0, 0);

	pdlib_clear_typec_prswap(PORT0_INDEX);
	pdlib_clear_typec_prswap(PORT1_INDEX);

	pdlib_disable_usbpd();

	tcpm_stop_wpc(WPC_DELAY);
	tcpm_update_wpc_work_mode(TCPM_WPC_WORK_DISABLE);

	tcpm_disable_usba_detect();

	buckboost_ops.init();

#if (CONFIG_USB_COM_FORCE_SINK == 1)
	hal_tcpc_set_source_mode(BUCKBOOST_SHUTDOWM_MODE);
	DPDM->SOURCE_CTRL.BITS.PORT1_CTRL = 0;
	DPDM->SOURCE_CTRL.BITS.PORT2_CTRL = 0;
	DPDM->SOURCE_CTRL.BITS.PORT3_CTRL = 0;
	DPDM->SOURCE_CTRL.BITS.EN_SRC_PROTOCOL = 0;
	usb_dpdm_port0_switch(false);
	usb_dpdm_port1_switch(false);
	/* Force SNK directly instead of DRP toggle */
	{
		extern struct tc_s g_tc[];
		bb_printk("[LOCK] pre: CCA_ROLE=0x%x light=%d\n", TCPC->CCA_ROLE.WORD, gd->tc0_lighting_mode);
		hal_tcpc_set_cc(PORT0_INDEX, TYPEC_CC_RD);
		bb_printk("[LOCK] post: CCA_ROLE=0x%x\n", TCPC->CCA_ROLE.WORD);
		usb_tc_set_state(&g_tc[PORT0_INDEX], TC_SNK_Unattached, enter_state);
		g_tc[PORT0_INDEX].typec_delay_ms = 0x00;
		bb_printk("[DIAG] Force SNK on Port0 (was DRP)\n");
	}
	bb_printk("USB comm lock: force SINK on Port0 for WB7720\n");
#else
	bb_printk("USB comm lock: all charge/discharge stopped\n");
#endif
}

void usb_comm_unlock(void)
{
	buckboost_protection_flag = 0;

	usb_dpdm_port0_switch(true);               // Restore DPDM mode

	pdlib_restart_typec(PORT0_INDEX);
	pdlib_restart_typec(PORT1_INDEX);

	osal_set_event(USB_TASK, TCPM_EVT_USBA_REDETECT);

	tcpm_stop_wpc(WPC_DELAY);
	tcpm_update_wpc_work_mode(TCPM_WPC_WORK_BOOST);

	buckboost_ops.init();
	buckboost_set_work_mode(BUCKBOOST_DISCHG_MODE);
	buckboost_set_bus_iv(5000, 3300, 0, 0);

	bb_printk("USB comm unlock: charge/discharge restored\n");
}
#endif /* CONFIG_TRIPLE_CLICK_COMM_ENABLE */

void buckboost_ir_drop_handle(void)
{

	uint16_t ir_drop = 0;
	static uint8_t cnt_delay = 0;
	if(g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE && wpc_mode != TCPM_WPC_WORK_BOOST && !pdlib_is_pps_source())
	{
		ir_drop = -g_buckboost.adc_ibus * 100 / 1000 ;    //1A +100mV
		ir_drop = ir_drop / 20 * 20;
		if(ir_drop >= 150) ir_drop = 150;
		if(ir_drop != g_buckboost.ir_drop)
		{
			cnt_delay++;
			if(cnt_delay >= 5)
			{
				g_buckboost.ir_drop = ir_drop;
				bb_printk("ir drop = %d\n",g_buckboost.ir_drop);
				buckboost_ops.set_out(g_buckboost.buckboost_out_voltage + g_buckboost.ir_drop,g_buckboost.buckboost_out_current_actual);
			}
		}
		else
		{
			cnt_delay = 0;
		}
	}
	else
	{
		cnt_delay = 0;
		g_buckboost.ir_drop = 0;;
	}
}

void buckboost_task_event_handler(uint32_t event)
{
	static uint8_t get_info_step = 0;
	static uint8_t adc_protect_cnt = 0;
	uint16_t out_ibus = 0;
#if(BUCKBOOST_USED_NU6801 == 1)
	uint32_t row;
	uint32_t vref;
	bool is_220uA = false;
	extern uint16_t ntc1_v;
	extern uint8_t ntc1_level;
	extern uint16_t ntc2_v;
	extern uint8_t ntc2_level;
	uint32_t adc;
	uint8_t read_r;
#endif
	switch (event)
	{
		case BUCKBOOST_EVT_TIME_PERIOD:
			if(get_info_step == 0)
			{
			#if(BUCKBOOST_USED_NU6801 == 1 && CONFIG_USE_NTC_FOR_CHAGER == 1)
				hal_nu6801_buckboost_set_adc_channel(NU6801_ADC_RNTC1);
			#else
				//bb_printk("Rntc = %d\n",buckboost_ops.get_bat_temperature());
				g_buckboost.adc_tbat1 = buckboost_ops.get_bat_temperature()/100;
				buckboost_ntc_handle();
			#endif
			}
			else if(get_info_step == 1)
			{
			#if(CONFIG_USBA_SUPPORT == 1)
				g_buckboost.usba_state =  buckboost_ops.get_a2_state();
			#endif
			#if(BUCKBOOST_USED_NU6805 == 1)
				g_buckboost.adc_ibat = buckboost_ops.get_bat_current();
			#else
				hal_nu6801_buckboost_set_adc_channel(NU6801_ADC_IBAT);
			#endif
			#if(BUCKBOOST_USED_NU6805 == 1)
				if(g_buckboost.adc_vbat < BAT_DEAD_BATTER_V)
				{
					pdlib_set_deadbat(true);
				}
				else
			#endif

			#if(BUCKBOOST_USED_NU6801 == 1)
				if(g_buckboost.adc_vbat > BAT_ACTIVE_RBATTER_V && !nu6801_dead_bat)
			#else
				if(g_buckboost.adc_vbat > BAT_ACTIVE_RBATTER_V)
			#endif
				{

					if(pdlib_get_deadbat())
					{
						pdlib_set_deadbat(false);
						port_manager_set_event(PORT_EVENT_RESET_CHARGE);
						bb_printk("%s\n",__func__);
					}
				}
				osal_set_event(USB_TASK,TCPM_EVT_USBA_SCAN);

			}
			else if(get_info_step == 2)
			{
			#if(BUCKBOOST_USED_NU6805 == 1)
				g_buckboost.adc_ibus = buckboost_ops.get_bus_current();
			#else
				hal_nu6801_buckboost_set_adc_channel(NU6801_ADC_IBUS);
			#endif
				buckboost_protection_handle();

			#if(BUCKBOOST_USED_NU6805 == 1)
				if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
				{
					if(hal_nu6805_buckboost_is_charge_full())
					{
						g_buckboost.bat_full_flag = 1;
						bb_printk("bat full\n");
					}
					if(g_buckboost.bat_full_flag && g_buckboost.adc_vbat < 4000 * bat_cell_num)
					{
						g_buckboost.bat_full_flag = 0;
						bb_printk("bat full flag cleared by vbat drop\n");
					}
				}
				else
				{
					g_buckboost.bat_full_flag = 0;
				}
			#endif

			#if(BUCKBOOST_USED_NU6801 == 1)
				if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
				{
					uint8_t flag = buckboost_ops.get_charge_flag();

					bb_printk("charge flag = 0x%x\n",flag);

					if(flag & 0x02) g_buckboost.bat_full_flag = 1;
					if((g_buckboost.bat_full_flag && g_buckboost.adc_vbat < 4000) || (flag & 0x01 ))
					{
						g_buckboost.bat_full_flag = 0;
						buckboost_ops.set_work_mode(BUCKBOOST_CHAGER_MODE);
//						port_manager_set_event(PORT_EVENT_RESET_CHARGE);
//						bb_printk("------------------------- rechage \n");
					}
				}
				else
				{
					g_buckboost.bat_full_flag = 0;
				}

			#endif
			}
			else if(get_info_step == 3)
			{

			#if(BUCKBOOST_USED_NU6801 == 1)
				hal_nu6801_deadbat_patch();
			#endif
			#if(BUCKBOOST_USED_NU6805 == 1)
				buckboost_ir_drop_handle();
				g_buckboost.adc_vbat = buckboost_ops.get_bat_voltage();
				uint16_t pd3_adc_mv = hal_badc_meas(_BADC_CH_PD3_ADC9);

				if (g_vref_cal_delay > 0) {
					g_vref_cal_delay--;
					if (g_vref_cal_delay == 0) {
						g_vref_mv = 3 * pd3_adc_mv / 2;
						bb_printk("\r\n[VREF_CAL] pd3=%d Vref=%dmV, saving to Flash", pd3_adc_mv, g_vref_mv);
						cycle_count_save_to_flash();
					}
				}

				/* ========== 6805 Vcell ADC + 跨周期 3 点中值滤波 ==========
				 * 保留最近 3 次调用 (~150ms 间隔) 的计算结果，取中值输出。
				 * 偶发毛刺在 3 次中最多命中 1 次，被中值自动剔除。
				 * 真实变化连续 2 次即可跟上，最大延迟 1 周期。 */
				#define MEDIAN3(a,b,c) ((a)>(b) ? ((b)>(c)?(b):((a)>(c)?(c):(a))) \
				                               : ((a)>(c)?(a):((b)>(c)?(c):(b))))

				static uint16_t c1_hist[3], c2_hist[3];
				static uint8_t hist_idx = 0;
				static uint8_t hist_cnt = 0;  /* 已填入的采样数 0~3 */

				g_buckboost.adc_Packnegative = (int16_t)(3 * pd3_adc_mv - g_vref_mv * 2);

				uint16_t pc7_adc_mv = hal_badc_meas(_BADC_CH_PC7_ADC4);
				int16_t vcell1_raw = 3 * pc7_adc_mv - g_buckboost.adc_Packnegative;
				c1_hist[hist_idx] = (vcell1_raw > 0) ? (uint16_t)vcell1_raw : 0;

				uint16_t pb6_adc_mv = hal_badc_meas(_BADC_CH_PB6_ADC7);
				int16_t vcell2_raw = 3 * pb6_adc_mv - g_buckboost.adc_Packnegative - c1_hist[hist_idx];
				c2_hist[hist_idx] = (vcell2_raw > 0) ? (uint16_t)vcell2_raw : 0;

				hist_idx = (hist_idx + 1) % 3;
				if (hist_cnt < 3) hist_cnt++;

				if (hist_cnt >= 3) {
					g_buckboost.adc_vcell1 = MEDIAN3(c1_hist[0], c1_hist[1], c1_hist[2]);
					g_buckboost.adc_vcell2 = MEDIAN3(c2_hist[0], c2_hist[1], c2_hist[2]);
				} else {
					/* 未填满：直接用最新值 */
					g_buckboost.adc_vcell1 = c1_hist[hist_idx ? hist_idx - 1 : 2];
					g_buckboost.adc_vcell2 = c2_hist[hist_idx ? hist_idx - 1 : 2];
				}

				bb_printk("\nvcell1=%d [%d,%d,%d] vcell2=%d [%d,%d,%d] Vref=%d total=%d\n",
				       g_buckboost.adc_vcell1, c1_hist[0], c1_hist[1], c1_hist[2],
				       g_buckboost.adc_vcell2, c2_hist[0], c2_hist[1], c2_hist[2],
				       g_vref_mv,g_buckboost.adc_vcell1 + g_buckboost.adc_vcell2);

			#else
				buckboost_ir_drop_handle();
				hal_nu6801_buckboost_set_adc_channel(NU6801_ADC_VBAT);
			#endif
			}
			else if(get_info_step == 4)
			{
				//hal_nu6801_buckboost_get_main_state();
			#if(BUCKBOOST_USED_NU6801 == 1)
				hal_nu6801_get_charge_state();
				hal_nu6801_buckboost_set_adc_channel(NU6801_ADC_VREF);
			#endif
			}
			else if(get_info_step == 5)
			{
			#if(BUCKBOOST_USED_NU6801 == 1 && CONFIG_USE_NTC_FOR_CHAGER == 1)
				hal_nu6801_buckboost_set_adc_channel(NU6801_ADC_RNTC2);
			#endif
			}
			else if(get_info_step == 6)
			{
			#if(BUCKBOOST_USED_NU6801 == 1)
				hal_nu6801_buckboost_set_adc_channel(NU6801_ADC_IAC2);
			#endif
			}
			else if(get_info_step == 7)
			{
			#if(BUCKBOOST_USED_NU6801 == 1)
				hal_nu6801_buckboost_set_adc_channel(NU6801_ADC_IAC1);
			#endif
			}
			if(get_info_step ++ > 7) get_info_step = 0;
			break;
		case BUCKBOOST_EVT_VBUS_PERIOD:
		#if(BUCKBOOST_USED_NU6805 == 1)
			g_buckboost.adc_vbus = buckboost_ops.get_bus_voltage();
		#else
			hal_nu6801_buckboost_set_adc_channel(NU6801_ADC_VBUS);
		#endif


			//if(g_usb_pd_s.is_in_pps && g_usb_pd_s.explicit_contract)
			if(g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE && (g_port.port_state[0] == PORT_STATE_SOURCE ||g_port.port_state[1] == PORT_STATE_SOURCE))
			{
				g_buckboost.ibus_cc_flag =  buckboost_ops.is_ibus_loop();
				{
					static uint8_t src_log_cnt = 0;
					if(++src_log_cnt >= 50) { /* ~1s @ 20ms period */
						src_log_cnt = 0;
						bb_printk("SRC VBUS set=%d adc=%d ir=%d\n",
							g_buckboost.buckboost_out_voltage, g_buckboost.adc_vbus, g_buckboost.ir_drop);
					}
				}
				if(
						//g_buckboost.adc_vbus < g_buckboost.buckboost_out_voltage * 80 / 100 ||
						g_buckboost.adc_vbus > g_buckboost.buckboost_out_voltage * 115 / 100)
				{
					bb_printk("adc vbus = %d\n",g_buckboost.adc_vbus);

					adc_protect_cnt++;
					if(adc_protect_cnt >= 50)
					{
						adc_protect_cnt = 0;
						adc_protect_flag = true;
						bb_printk("adc uvp or ovp [%d %d]\n",g_buckboost.adc_vbus,g_buckboost.buckboost_out_voltage);
					}
				}
				else
				{
					adc_protect_cnt = 0;
					adc_protect_flag = 0;
				}
			}
			else
			{
				adc_protect_cnt = 0;
				adc_protect_flag = false;
			}

			break;
		case BUCKBOOST_EVT_CHAG_PERIOD:
			if(g_buckboost.chager_ibus_start)
			{
				if(g_buckboost.chager_ibus_value < g_buckboost.chager_ibus_limit)
				{
					g_buckboost.chager_ibus_value += 100;
					if(g_buckboost.chager_ibus_value > g_buckboost.chager_ibus_limit) g_buckboost.chager_ibus_value = g_buckboost.chager_ibus_limit;
					buckboost_ops.set_chager_ibus_limit(g_buckboost.chager_ibus_value);
				}
				else
					g_buckboost.chager_ibus_start = 0;
				bb_printk("\r\n[RAMP] ibus_start=%d ibus_val=%d ilim[%d %d]",
					g_buckboost.chager_ibus_start,
					g_buckboost.chager_ibus_value,
					g_buckboost.chager_ibat_limit,
					g_buckboost.chager_ibus_limit);
			}
			break;
		case BUCKBOOST_EVT_SWITCH_WORK_MODE:  //
			break;
		case BUCKBOOST_EVT_SET_DISCHG_VBUS_VOLT:

			g_buckboost.regulator_state = 0;
			osal_start_timerEx(BUCKBOOST_REGULATOR_TIMER, g_buckboost.out_voltage_wait, 0, BUCKBOOST_TASK, BUCKBOOST_EVT_REGULATOR_WAITDONE);
			break;
		case BUCKBOOST_EVT_REGULATOR_WAITDONE:
			out_ibus = g_buckboost.buckboost_out_current;
			g_buckboost.buckboost_out_current_actual = out_ibus;
			buckboost_ops.set_out(g_buckboost.buckboost_out_voltage + g_buckboost.ir_drop,out_ibus);
			osal_start_timerEx(BUCKBOOST_REGULATOR_TIMER, g_buckboost.out_voltage_delay + 10, 0, BUCKBOOST_TASK, BUCKBOOST_EVT_REGULATOR_DELAYDONE);
			break;
		case BUCKBOOST_EVT_REGULATOR_DELAYDONE:
			g_buckboost.regulator_state = 1;
			g_buckboost.out_voltage_wait = 0;
			g_buckboost.out_voltage_delay = 0;

			osal_stop_timerEx(BUCKBOOST_REGULATOR_TIMER);
			break;
		case BUCKBOOST_EVT_SET_CHARGER_CURRENT:
			break;
		case BUCKBOOST_EVT_SET_TYPECA_GATE_EN:
			break;
		case BUCKBOOST_EVT_SET_TYPECB_GATE_EN:
			break;
		case BUCKBOOST_EVT_SET_USB_A_GATE_EN:
			break;
		case BUCKBOOST_EVT_SET_TYPECA_DUMMYLOAD_EN:
			buckboost_ops.typca_dischg_en(true);
			break;
		case BUCKBOOST_EVT_SET_TYPECB_DUMMYLOAD_EN:
			buckboost_ops.typcb_dischg_en(true);
			break;
		case BUCKBOOST_EVT_SET_TYPECA_DUMMYLOAD_DIS:
			buckboost_ops.typca_dischg_en(false);
			break;
		case BUCKBOOST_EVT_SET_TYPECB_DUMMYLOAD_DIS:
			buckboost_ops.typcb_dischg_en(false);
			break;
		case BUCKBOOST_EVT_DUMMYLOADOVER:
			buckboost_ops.vbus_dischg_en(false);
			#if(BUCKBOOST_USED_NU6801 == 1)
				if(g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE) buckboost_ops.set_ovp(g_buckboost.buckboost_out_voltage);
			#endif
			bb_printk("vbus disg end\n");
			break;
		case BUCKBOOST_EVT_ADC_PERIOD:
		#if(BUCKBOOST_USED_NU6801 == 1)

			switch(nu6801_adc_chennel)
			{
				case NU6801_ADC_VBAT:
					row = (uint32_t) hal_badc_meas(_BADC_CH_PD3_ADC9);
					uint32_t vbat = row* 120  * 25 / nu6801_vref;
					g_buckboost.adc_vbat = vbat;
					bb_printk("adc_vbat = %d\n",g_buckboost.adc_vbat);
					break;
				case NU6801_ADC_IBAT:
                  #if 0
					g_buckboost.adc_ibat = ibus_to_ibat(g_buckboost.adc_ibus,g_buckboost.adc_vbus,g_buckboost.adc_vbat);
                  #else
					row = (uint32_t) hal_badc_meas(_BADC_CH_PD3_ADC9);
					uint32_t ibat = 0;
					if(g_buckboost.ibat_level == 0)
					{
						ibat = row* 120  * 100 / nu6801_vref; // 1/125k
						if(ibat < 6000 )
						{
							g_buckboost.ibat_level = 1;
							break;
						}
					}
					else  //level == 1
					{
						ibat = row* 120  * 10 * 4 / nu6801_vref; // 1/50k
					}
					//uint32_t ibat = row* 120  * 100 / nu6801_vref;

					ibat = ibat + 90;
					if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
						g_buckboost.adc_ibat = ibat;
					else
						g_buckboost.adc_ibat = -ibat;
					//bb_printk("adc_ibat = %d\n",g_buckboost.adc_ibat);
					bb_printk("\r\n adc_ibat = %d row:%d 6801_vref:%d ibat_level:%d  \r\n",g_buckboost.adc_ibat,row,nu6801_vref,g_buckboost.ibat_level);
					g_buckboost.ibat_level = 0;
                 #endif
					break;
				case NU6801_ADC_VBUS:
					row = (uint32_t) hal_badc_meas(_BADC_CH_PD3_ADC9);
					uint32_t vbus = row* 120  * 100 / nu6801_vref;
					g_buckboost.adc_vbus = vbus;
					//bb_printk("adc_vbus = %d\n",g_buckboost.adc_vbus);
					break;
				case NU6801_ADC_IBUS:
					row = (uint32_t) hal_badc_meas(_BADC_CH_PD3_ADC9);
					uint32_t ibus = row* 120  * 25 / nu6801_vref;
					if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
						g_buckboost.adc_ibus = ibus;
					else
						g_buckboost.adc_ibus = -ibus;
					//bb_printk("adc_ibus = %d\n",g_buckboost.adc_ibus);
					break;
				case NU6801_ADC_IAC1:
					row = (uint32_t) hal_badc_meas(_BADC_CH_PD3_ADC9);
					uint32_t iac1 = row* 1200* 4 / CONFIG_TYPEC_MOS_R  / nu6801_vref;
					g_buckboost.adc_iac1 = iac1;
					bb_printk("adc_iac1 [%d %d]\n",g_buckboost.adc_iac1,row);
					break;
				case NU6801_ADC_IAC2:
					row = (uint32_t) hal_badc_meas(_BADC_CH_PD3_ADC9);
					uint32_t iac2 = row* 1200* 4 / CONFIG_TYPEC_MOS_R  / nu6801_vref;
					g_buckboost.adc_iac2 = iac2;
					bb_printk("adc_iac2 [%d %d]\n",g_buckboost.adc_iac2,row);
					break;
				case NU6801_ADC_RNTC1:
					row = (uint32_t) hal_badc_meas(_BADC_CH_PD3_ADC9);
					ntc1_v = row;
					if(ntc1_level == 0) row = row * 2 / 5 / 5;
					else if(ntc1_level == 1) row = row * 2 / 5;

					adc = row* 120 * 10  / nu6801_vref;
					//ntc1_v_switch = adc;
					hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_TEMP_STAT,&read_r);
					if(read_r & 0x04) is_220uA = true;
					else is_220uA = false;
					if(is_220uA) adc = adc / 22; //220uA
					else adc = adc / 2; //220uA
					g_buckboost.adc_tbat1 = adc;
					bb_printk("RNTC1 [%d %d] ntc_v = %d rntc = %d \n",is_220uA,ntc1_level,ntc1_v,g_buckboost.adc_tbat1);
				#if(CONFIG_USE_NTC_FOR_CHAGER == 1)
					buckboost_ntc_handle();
					//hal_nu6801_buckboost_switch_isrc();
				#endif
					break;
				case NU6801_ADC_RNTC2:
					row = (uint32_t) hal_badc_meas(_BADC_CH_PD3_ADC9);
					ntc2_v = row;
					if(ntc2_level == 0) row = row * 2 / 5 / 5;
					else if(ntc2_level == 1) row = row * 2 / 5;

					adc = row* 120 * 10  / nu6801_vref;
					//ntc1_v_switch = adc;

					hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_TEMP_STAT,&read_r);
					if(read_r & 0x04) is_220uA = true;
					else is_220uA = false;
					if(is_220uA) adc = adc / 22; //220uA
					else adc = adc / 2; //220uA
					g_buckboost.adc_tbat2 = adc;
					bb_printk("RNTC2 [%d %d] ntc_v = %d rntc = %d \n",is_220uA,ntc2_level,ntc2_v,g_buckboost.adc_tbat2);
				#if(CONFIG_USE_NTC_FOR_CHAGER == 1)
					//buckboost_ntc_handle();
					//hal_nu6801_buckboost_switch_isrc();
				#endif
					break;
				case NU6801_ADC_VREF:

					vref  = hal_badc_meas(_BADC_CH_PD3_ADC9);

					if(vref < 1500)
					{
						bb_printk("adc_vref = %d\n",nu6801_vref);
						uint8_t read_0x10,read_0x11,read_0x06,read_0x00;
						hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL,&read_0x11);
						hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_MISC_CTRL,&read_0x10);
						hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_BUBO_FAULT_FLAG,&read_0x06);
						hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_INT_FLAG,&read_0x00);
						adc_err_flag = 1;
						bb_printk("adc_err [0x00]=0x%x [0x06]=0x%x [0x10]=0x%x [0x11]=0x%x\n",read_0x00,read_0x06,read_0x10,read_0x11);
					}
					else
					{
						nu6801_vref = vref;
					}
					break;
			}
		#endif
			break;
		default:
			break;
	}

}

#if(BUCKBOOST_USED_NU6805 == 1)
const struct buckboost_operations buckboost_ops =
{
	.init = 					hal_nu6805_buckboost_init,
	.set_work_mode = 			hal_nu6805_buckboost_set_mode,
	.set_out = 					hal_nu6805_buckboost_set_busiv,
	.typca_gate_en = 			hal_nu6805_buckboost_typeca_gate_en,
	.typcb_gate_en = 			hal_nu6805_buckboost_typecb_gate_en,
	.usb_a_gate_en = 			hal_nu6805_buckboost_usb_a_gate_en,
	.set_chager_ibus_limit = 	hal_nu6805_buckboost_charge_ibus_limit,
	.set_chager_ibat_limit = 	hal_nu6805_buckboost_charge_ibat_limit,
	.get_bus_current = 			hal_nu6805_buckboost_get_bus_current,
	.get_bat_current =  		hal_nu6805_buckboost_get_bat_current,
	.get_bat_voltage =  		hal_nu6805_buckboost_get_bat_voltage,
	.get_bus_voltage =  		hal_nu6805_buckboost_get_bus_voltage,
	.get_a2_state    = 			hal_nu6805_buckboost_get_a2_state,
	.en_a2_detect  = 			hal_nu6805_buckboost_a2_detect_enable,
	.get_bat_temperature =      hal_nu6805_buckboost_get_bat_temperature,
	.typca_dischg_en = 			hal_nu6805_buckboost_typeca_dischg,
	.typcb_dischg_en = 			hal_nu6805_buckboost_typecb_dischg,
	.usb_a_dischg_en = 			hal_nu6805_buckboost_usb_a_dischg,
	.vbus_dischg_en = 			hal_nu6805_buckboost_vbus_dischg,
	.get_protect_status = 		hal_nu6805_buckboost_get_protect,
	.is_ibus_loop = 			hal_nu6805_buckboost_is_ibus_loop,
	.set_ovp = 					hal_nu6805_buckboost_set_ovp,
};
#elif(BUCKBOOST_USED_NU6801 == 1)

const struct buckboost_operations buckboost_ops =
{
	.init = 					hal_nu6801_buckboost_init,
	.set_work_mode = 			hal_nu6801_buckboost_set_mode,
	.set_out = 					hal_nu6801_buckboost_set_busiv,
	.typca_gate_en = 			hal_nu6801_buckboost_typeca_gate_en,
	.typcb_gate_en = 			hal_nu6801_buckboost_typecb_gate_en,
	.usb_a_gate_en = 			hal_nu6801_buckboost_usb_a_gate_en,
	.set_chager_ibus_limit = 	hal_nu6801_buckboost_charge_ibus_limit,
	.set_chager_ibat_limit = 	hal_nu6801_buckboost_charge_ibat_limit,
	.get_bus_current = 			hal_nu6801_buckboost_get_bus_current,
	.get_bat_current =  		hal_nu6801_buckboost_get_bat_current,
	.get_bat_voltage =  		hal_nu6801_buckboost_get_bat_voltage,
	.get_bus_voltage =  		hal_nu6801_buckboost_get_bus_voltage,
	.get_a2_state    = 			hal_nu6801_buckboost_get_usba_state,
	.en_a2_detect  = 			hal_nu6801_buckboost_usba_detect_enable,
	.get_bat_temperature =      hal_nu6801_buckboost_get_bat_temperature,
	.typca_dischg_en = 			hal_nu6801_buckboost_typeca_dischg,
	.typcb_dischg_en = 			hal_nu6801_buckboost_typecb_dischg,
	.usb_a_dischg_en = 			hal_nu6801_buckboost_usb_a_dischg,
	.vbus_dischg_en = 			hal_nu6801_buckboost_vbus_dischg,
	.get_protect_status = 		hal_nu6801_buckboost_get_protect,
	.is_ibus_loop = 			hal_nu6801_buckboost_is_ibus_loop,
	.set_ovp = 					hal_nu6801_buckboost_set_ovp,
#if(BUCKBOOST_USED_NU6801 == 1)
	.get_typeca_vbus_present = 	hal_nu6801_buckboost_typeca_vbus_present,
	.get_typecb_vbus_present = 	hal_nu6801_buckboost_typecb_vbus_present,
	.get_charge_flag = 			hal_nu6801_buckboost_get_charge_flag,
	.get_adc_iac1 = 			hal_nu6801_buckboost_get_iac1,
#endif
};

#endif


