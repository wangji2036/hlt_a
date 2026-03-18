#include "regdef.h"
#include "printk.h"
#include "delay.h"
#include "g_data.h"
#include "osal.h"
#include "_wpc.h"
#include "fsk.h"
#include "pid.h"
#include "pfod.h"
#include "fm1210.h"
#include "pkt_type.h"
#include "wpc_idle.h"
#include "wdt.h"
#include "ecap.h"
#include "usb_qc.h"
#include"i2cm.h"
#include"buckboost.h"
#include"nu6801.h"
#include"nu6805.h"
#include "fmc.h"
#include"qdt.h"
#include"led.h"
#include"nu103x.h"
#include"eadc.h"
#include"fm1210.h"
#include"ask.h"
#include"bpwm.h"
#include "usbpd_config.h"
#include"bsp.h"
#include "config.h"
#include "sleep.h"
#include "tcpm.h"
#include "pdlib.h"
#include "port_manager.h"
#include "_fml.h"
//#include "typec.h"
//uint8_t reset_magic_code;

// ==== 锟斤拷锟斤拷 Victor 2024-06-22 start ====
// PB4锟斤拷锟斤拷锟斤拷锟窖硷拷锟疥定锟斤拷
#define PB4_TOUCH_LEVEL    (GPB->D_IN.BITS.PIN4)
#define PB4_TOUCH_PRESSED  (!PB4_TOUCH_LEVEL)  // 锟斤拷锟斤拷锟斤拷锟斤拷时为锟酵碉拷平

// PC6锟结触锟斤拷锟斤拷锟斤拷锟疥定锟斤拷
#define PC6_KEY_LEVEL      (GPC->D_IN.BITS.PIN6)
#define PC6_KEY_PRESSED    (!PC6_KEY_LEVEL)    // 锟斤拷锟斤拷锟斤拷锟斤拷时为锟酵碉拷平
// ==== 锟斤拷锟斤拷 Victor 2024-06-22 end ====

#define SHIP_MODE_CNT  30
#if ONLY7_5W_ENALBE
static uint16_t sleep_q_68nf_thd = 0;
static uint16_t sleep_f_68nf_thd = 0;
#else
#if CAPACITOR_300_NF
static uint16_t sleep_q_68nf_thd = 158;
static uint16_t sleep_f_68nf_thd = 1340;// sleep F -normal F,
#else
static uint16_t sleep_q_68nf_thd = 166;
static uint16_t sleep_f_68nf_thd = 1317;// sleep F -normal F,
#endif
#endif
void SLP_vNormalToSleep(void)
{
#if (CONFIG_TRIPLE_CLICK_COMM_ENABLE == 1)
	if (gd->usb_comm_activated) {
		printk("[SLEEP] blocked by USB_COM mode\n");
		return;
	}
#endif
	VIC_vModuleDisable();
	sleep_printk("\r\n enter sleep");
	hal_wdt_feed();
	fm1210_sleep();
	SYS->PWR_CTRL.WORD = 0;
	gd->touch_to_weakup = 0;
	gd->rd0_cnt = 0;
	gd->rd1_cnt = 0;
	gd->light0_cnt = 0;
	gd->light1_cnt = 0;
	gd->SOC_SleepTime_s = 0;
	gd->reset_magicode = 0;// magic code,important for sleep Q wake-up.
	gd->sleep_q_times = 0;
#if(CONFIG_TYPECA_SUPPORT == 1)
	if(!(gd->tc0_lighting_mode || gd->bat_dead_flag_with_snk0)) pdlib_tcpc_set_cc(TYPEC_PORT_A,TYPEC_CC_OPEN);
	else pdlib_tcpc_set_cc(TYPEC_PORT_A,TYPEC_CC_RP_DEF);
#endif
#if(CONFIG_TYPECB_SUPPORT == 1)
	if(!(gd->tc1_lighting_mode || gd->bat_dead_flag_with_snk1)) pdlib_tcpc_set_cc(TYPEC_PORT_B,TYPEC_CC_OPEN);
	else pdlib_tcpc_set_cc(TYPEC_PORT_B,TYPEC_CC_RP_DEF);
#endif
#if(BUCKBOOST_USED_NU6801 == 1)
    // enable all 6801 INT
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_INT_MASK,0x80);
	// 6801 sleep function and firmware work-round start
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,0x00);//09
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_BUBO_CTRL,0x09);//09
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VBUS_SET_H,0x1C);//0C
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VBUS_SET_L,0x50);//0D
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_BUBO_CTRL,0x0D);//09
	for(uint8_t i = 0; i < 50; i++)
	{
		hal_wdt_feed();
		if(!_KEY_LEVEL)
		{
			printk("mcu reset\n");
			SYS->RST_CTRL.BITS.MCU_RST = 1;
		}
		delay_1ms(10);
	}
	//delay_1ms(500);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0x65);//
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0x37);//
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0x2D);//
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0xF9);//
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x51,0x29);//
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x51,0xCB);//
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x51,0xE2);//
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x51,0x6A);//

	for(uint8_t i = 0; i < 50; i++)
	{
		hal_wdt_feed();
		if(!_KEY_LEVEL)
		{
			printk("mcu reset\n");
			SYS->RST_CTRL.BITS.MCU_RST = 1;
		}
		delay_1ms(10);
	}
	//delay_1ms(500);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x63,0x01);//
	hal_wdt_feed();
	for(uint8_t i = 0; i < 50; i++)
	{
		hal_wdt_feed();
		if(!_KEY_LEVEL)
		{
			printk("mcu reset\n");
			SYS->RST_CTRL.BITS.MCU_RST = 1;
		}
		delay_1ms(10);
	}
	//delay_1ms(500);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_BUBO_CTRL,0x01);//09

	for(uint8_t i = 0; i < 50; i++)
	{
		hal_wdt_feed();
		if(!_KEY_LEVEL)
		{
			printk("mcu reset\n");
			SYS->RST_CTRL.BITS.MCU_RST = 1;
		}
		delay_1ms(10);
	}
	//delay_1ms(500);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_MISC_CTRL,0x41);//10

	/* 6801 sleep function and firmware work-round end*/
/*#else if (BUCKBOOST_USED_NU6805 ==1)

	hal_i2cm_wirte_one_byte(NU6805_I2C_DEV_ADDR,REG_Indt_Control,0x00);

    hal_i2cm_wirte_one_byte(NU6805_I2C_DEV_ADDR,REG_Indt_Control,0x11);*/
#endif

#if(BUCKBOOST_USED_NU6805 == 1)
	hal_wdt_feed();

	uint8_t read;

	hal_i2cm_read_one_byte(NU6805_I2C_DEV_ADDR,REG_IRQ_EN_1,&read);
    hal_i2cm_wirte_one_byte(NU6805_I2C_DEV_ADDR,REG_IRQ_EN_1,read & (~0x0C));// disable B port.

	hal_i2cm_read_one_byte(NU6805_I2C_DEV_ADDR,REG_discharge_Control,&read);
    hal_i2cm_wirte_one_byte(NU6805_I2C_DEV_ADDR,REG_discharge_Control,read & (~0x0F));
//	uint8_t read;
	hal_i2cm_read_one_byte(NU6805_I2C_DEV_ADDR,REG_Powerpath_Control,&read);
	hal_i2cm_wirte_one_byte(NU6805_I2C_DEV_ADDR,REG_Powerpath_Control,read & (~0x07));
	hal_wdt_feed();

	hal_i2cm_read_one_byte(NU6805_I2C_DEV_ADDR,REG_Mode_Control,&read);
    hal_i2cm_wirte_one_byte(NU6805_I2C_DEV_ADDR,REG_Mode_Control,read & (~0x11));

	hal_i2cm_wirte_one_byte(NU6805_I2C_DEV_ADDR,REG_IRQ_Event1,0xFF);
	hal_i2cm_wirte_one_byte(NU6805_I2C_DEV_ADDR,REG_IRQ_Event2,0xFF);

/*    hal_i2cm_read_one_byte(NU6805_I2C_DEV_ADDR,REG_Indt_Control,&read);
	hal_i2cm_wirte_one_byte(NU6805_I2C_DEV_ADDR,REG_Indt_Control,read & (~0x07));*/
    hal_i2cm_wirte_one_byte(NU6805_I2C_DEV_ADDR,REG_Indt_Control,0x03);
	ubsd_wb7720_sleep();
#endif
    TCPC->CCA_CTRL.WORD = 0;
    TCPC->CCB_CTRL.WORD = 0;
    TCPC->RXD_CTRL.WORD = 0;
    ECAP2->QDT_CTRL.WORD = 0;
	//tcpc wake up start.
	
	//SYS->PWR_CTRL.WORD &= !SYS_PWR_CTRL_TCPC_WKUP_DIS_Pos;
	//CCA
	  //(Enable CC, Disable RDB, Enter low power mode)
#if(CONFIG_TYPECA_SUPPORT == 1)

	if(!(gd->tc0_lighting_mode || gd->bat_dead_flag_with_snk0))
	{
		TCPC->CCA_CTRL.BITS.CC_DB_RD_DIS = 1; // enable cc block
		TCPC->CCA_CTRL.BITS.CC_LPMODE_EN = 1; // enable cc block
		TCPC->CCA_CTRL.BITS.CC_BLOCK_DIS = 0; // enable cc block
		  //(Enable DRP)
		TCPC->CCA_ROLE.BITS.DRP_MODE = 1;
		TCPC->CCA_ROLE.BITS.CC1_ROLE = 1;
		TCPC->CCA_ROLE.BITS.CC2_ROLE = 1;
		TCPC->CCA_CMD_.BITS.CMD_TYPE = 0x99;//(Start DRP)
		sleep_printk("\r\n no with snk sleep");
	}
	else
	{
		SYS->PWR_CTRL.BITS.TCPC_WKUP_DIS = 1;
		sleep_printk("\r\n with snk sleep");
	}

#endif
	//CCB
	  //(Enable CC, Disable RDB, Enter low power mode)
#if(CONFIG_TYPECB_SUPPORT == 1)
	if(!(gd->tc0_lighting_mode || gd->bat_dead_flag_with_snk1))
	{
		TCPC->CCB_CTRL.BITS.CC_DB_RD_DIS = 1;
		TCPC->CCB_CTRL.BITS.CC_LPMODE_EN = 1;
		TCPC->CCB_CTRL.BITS.CC_BLOCK_DIS = 0;
		TCPC->CCB_CTRL.BITS.CC_DCSRC_DRP = 2;   // DRP duty cycle (cleared by WORD=0, must restore)
		TCPC->CCB_CTRL.BITS.CC_T_DRP_SEL = 3;   // DRP toggle period (cleared by WORD=0, must restore)
		  //(Enable DRP)
		TCPC->CCB_ROLE.BITS.RP_VALUE = 0;        // RP_VALUE_DEFAULT for Rp phase
		TCPC->CCB_ROLE.BITS.DRP_MODE = 1;
		TCPC->CCB_ROLE.BITS.CC1_ROLE = 1;
		TCPC->CCB_ROLE.BITS.CC2_ROLE = 0;  // CC2=OPEN: PC8 external pull-up causes false detection
		TCPC->CCB_CMD_.BITS.CMD_TYPE = 0x99;//(Start DRP)
	}
	else
	{
		sleep_printk("\r\n lighting sleep");
		SYS->PWR_CTRL.BITS.TCPC_WKUP_DIS = 1;
	}
#endif
	_SET_ALL_PINS_IN_PUT();
	hal_wdt_feed();

	BADC->CTRL.WORD = 0;
	EADC->CTRL.WORD = 0;
	I2CS->CTRL.WORD = 0;  // Keep I2C Slave enabled during sleep
	BPWM3->GEN_CTRL.WORD = 0;
	BPWM4->GEN_CTRL.WORD = 0;
	BPWM7->GEN_CTRL.WORD = 0;
	BPWM8->GEN_CTRL.WORD = 0;

	EPWM1->PWM_CTRL.WORD = 0;
	EPWM2->PWM_CTRL.WORD = 0;

    TCPC->PHY_CTRL.WORD = 0;
	DPDM->AFC_CTRL.WORD = 0;
	DPDM->HVDCP_CTRL.WORD = 0;

	hal_wdt_feed();

	FMC->FMC_CMD_CTRL.WORD = _FMC_CMD_ALL_CTRL_DISABLE;// off flash
	//How to shut sram?
	ECAP1->GEN_CTRL.WORD =0;//ecap
	ECAP2->GEN_CTRL.WORD =0;//&= ~ECAP_GEN_CTRL_CAP_EN_Msk;//ecap
	ECAP3->GEN_CTRL.WORD =0;//&= ~ECAP_GEN_CTRL_CAP_EN_Msk;//ecap
	ECAP4->GEN_CTRL.WORD =0;//&= ~ECAP_GEN_CTRL_CAP_EN_Msk;//ecap
	ECAP5->GEN_CTRL.WORD =0;//&= ~ECAP_GEN_CTRL_CAP_EN_Msk;//ecap
	DPDM_QC_SINK->BC1P2_INTMSK_CTRL.WORD = 0;
	DPDM_QC_SINK->BC1P2_INTMSK_CTRL.WORD  = 0;
	DPDM->QC_SRC_CTRL.WORD = 0;
	DPDM->SOURCE_CTRL.WORD = 0;
	I2CM->GEN_CTRL.WORD = 0;  // Keep I2C Master enabled during sleep

	SYS->CLK_CTRL.WORD = 0;
	SYS->PRO_CTRL.WORD = 0;
	SYS->CLK_CTRL.BITS.XTAL_EN = 0;// disable XTAL
	SYS->PRO_CTRL.BITS.PVD_EN = 0;// disable PVD
 

	TMR1->GEN_CTRL.WORD = 0;
	TMR2->GEN_CTRL.WORD = 0;
	TMR3->GEN_CTRL.WORD = 0;
	TMR0->GEN_CTRL.WORD = 0;
	TMR0->LOAD_CNT.WORD = 16 * 100 * 1 - 1; //first Q,100ms start.

	if(!(gd->bat_dead_flag_with_snk0 || gd->bat_dead_flag_with_snk1) && gd->bat_dead_flag)
	{
		TMR0->SPL_CTRL.WORD = (_TMR_CLK_SRC_LIRC << TMR_SPL_CTRL_CLK_SRC_Pos) ; //LIRC: 64K
		hal_wdt_stop();
	}
	else
		TMR0->SPL_CTRL.WORD = (_TMR_CLK_SRC_LIRC << TMR_SPL_CTRL_CLK_SRC_Pos) | TMR_SPL_CTRL_WKUP_EN_Msk; //LIRC: 64K
#if(CONFIG_SHIP_MODE_ENABLE_DEBUG ==1)
	if(gd->ship_mode_cnt == SHIP_MODE_CNT)
	{
	    TMR0->SPL_CTRL.WORD = (_TMR_CLK_SRC_LIRC << TMR_SPL_CTRL_CLK_SRC_Pos) ; //LIRC: 64K
	}
#endif
	TMR0->GEN_CTRL.WORD = (2 << TMR_GEN_CTRL_CLK_PSC_Pos) | (_TMR_OP_MODE_ONE_SHOT << TMR_GEN_CTRL_OP_MODE_Pos) | TMR_GEN_CTRL_CNT_EN_Msk; //16K
	UART1->GEN_CTRL.WORD = 0;
	UART1->BRG_CTRL.WORD = 0;
	UART2->GEN_CTRL.WORD = 0;
	UART2->BRG_CTRL.WORD = 0;
	UART1-> SLA_ADEN.WORD = 0;
	UART2-> SLA_ADEN.WORD = 0;
	/* GPIO config*/
	hal_gpio_init_default();// case SW1 HIGH
    // key wake up start
	GPC->I_EN.BITS.PIN6 = 1;
	GPC->O_EN.BITS.PIN6 = 0;
	GPC->MODE.BITS.PIN6 = 0; //00:PC6 01:JTAG_DAT 10:BPWM8 11:RESERVED
	GPC->ITEN.BITS.PIN6 = 1;
	GPC->ITTP.BITS.PIN6 = 0;
	if(gd->ship_mode_cnt == SHIP_MODE_CNT)
	{
		// touch wake up start
		GPB->I_EN.BITS.PIN4 = 1;
		GPB->O_EN.BITS.PIN4 = 0;
		GPB->MODE.BITS.PIN4 = 0; //00:PB4 01:JTAG_DAT 10:BPWM8 11:RESERVED
		GPB->ITEN.BITS.PIN4 = 1;
		GPB->ITTP.BITS.PIN4 = 0; //00:Falling Edge
	}
	else
	{
		// touch wake up
		GPB->I_EN.BITS.PIN4 = 1;
		GPB->O_EN.BITS.PIN4 = 0;
		GPB->MODE.BITS.PIN4 = 0; //00:PB4 01:JTAG_DAT 10:BPWM8 11:RESERVED
		GPB->ITEN.BITS.PIN4 = 1;
		GPB->ITTP.BITS.PIN4 = 0; //00:Falling Edge
	}

#if(BUCKBOOST_USED_NU6801 == 1)
    // charger irq wake up start
	GPD->I_EN.BITS.PIN1 = 1;
	GPD->MODE.BITS.PIN1 = 0; //00:PB4 01:JTAG_DAT 10:BPWM8 11:RESERVED
	GPD->ITEN.BITS.PIN1 = 1;
	GPD->ITTP.BITS.PIN1 = 0;
#endif
	hal_epwm_pwm_stop(EPWM1);
	fml_nu103x_config(_1030_CFG_ALL_RST);
	fml_nu103x_config(_1030_CFG_VDD_V5V_BUCK_DIS);
	fml_nu103x_config(_1030_CFG_LPM_EN_);
	hal_wdt_feed();

	WDT->CTRL.WORD = 0;
	SYS->PWR_CTRL.BITS.SLEEP_MODE_EN = 1;
	SYS->PWR_CTRL.BITS.GPIO_WKUP_DIS = 0;  // [NEW-VICTOR] 锟斤拷锟斤拷确锟斤拷GPIO锟斤拷锟窖癸拷锟斤拷使锟斤拷

}
void SLP_vSleepToSleep(void)
{

	if(gd->SOC_SleepTime_s < 10000) gd->SOC_SleepTime_s++;
	gd->reset_magicode = 0;// magic code,important for sleep Q wake-up.
	//gd->tc0_lighting_mode = 0;
	if(!(gd->bat_dead_flag_with_snk0 || gd->bat_dead_flag_with_snk1))
	{
	#if SLEEPQ_WAKEUP_ENABLE
		if(gd->sleep_q_times <20)
		{
	        TMR0->LOAD_CNT.WORD = 16 * 50 * 1 - 1; // 50ms fast sleep Q to charge the DH2 CAP, work-round
	    }
	    else
	#endif
	    {
	    	if(gd->rd0_cnt == 0 && gd->rd1_cnt == 0 && gd->light0_cnt == 0 && gd->light1_cnt == 0)
				TMR0->LOAD_CNT.WORD = 16 * 800 * 1 - 1; //500ms
			else
				TMR0->LOAD_CNT.WORD = 16 * 127 * 1 - 1; //500ms
    	}

    }
	else
	{
		SYS->PWR_CTRL.BITS.GPIO_WKUP_DIS = 0;
		SYS->PWR_CTRL.BITS.TCPC_WKUP_DIS = 1;
		sleep_printk("\r\n batlow");
		if(gd->rd0_cnt == 0 && gd->rd1_cnt == 0 && gd->light0_cnt == 0 && gd->light1_cnt == 0)
			TMR0->LOAD_CNT.WORD = 16 * 800 * 1 - 1; //500ms
		else
			TMR0->LOAD_CNT.WORD = 16 * 127 * 1 - 1; //500ms
	}
	if(!(gd->bat_dead_flag_with_snk0 || gd->bat_dead_flag_with_snk1) && gd->bat_dead_flag)
	{
			TMR0->SPL_CTRL.WORD = (_TMR_CLK_SRC_LIRC << TMR_SPL_CTRL_CLK_SRC_Pos) ; //LIRC: 64K'
			hal_wdt_stop();
	}
		else
			TMR0->SPL_CTRL.WORD = (_TMR_CLK_SRC_LIRC << TMR_SPL_CTRL_CLK_SRC_Pos) | TMR_SPL_CTRL_WKUP_EN_Msk; //LIRC: 64K
	TMR0->GEN_CTRL.WORD = (2 << TMR_GEN_CTRL_CLK_PSC_Pos) | (_TMR_OP_MODE_ONE_SHOT << TMR_GEN_CTRL_OP_MODE_Pos) | TMR_GEN_CTRL_CNT_EN_Msk; //16K

    SYS->CLK_CTRL.WORD = 0;
	SYS->PRO_CTRL.WORD = 0;
	SYS->CLK_CTRL.BITS.XTAL_EN = 0;// disable XTAL
	SYS->PRO_CTRL.BITS.PVD_EN = 0;// disable PVD
    //SYS->PWR_CTRL.WORD = 0;

    TCPC->PHY_CTRL.WORD = 0;
	DPDM->AFC_CTRL.WORD = 0;
	DPDM->HVDCP_CTRL.WORD = 0;
	UART2->GEN_CTRL.WORD = 0;
	UART2->BRG_CTRL.WORD = 0;
	UART1-> SLA_ADEN.WORD = 0;
	UART2-> SLA_ADEN.WORD = 0;

	sleep_printk("\r\n sleep again  --- sleep time%d",gd->SOC_SleepTime_s);



	hal_wdt_feed();
	hal_epwm_pwm_stop(EPWM1);
	GPC->I_EN.BITS.PIN0 = 1;
	GPC->O_EN.BITS.PIN0 = 0;
	GPC->MODE.BITS.PIN0 = 0; //00:PC0 01:EPWM1 10:RESERVED 11:RESERVED

	/* PC1 */
	GPC->I_EN.BITS.PIN1 = 1;
	GPC->O_EN.BITS.PIN1 = 0;
	GPC->MODE.BITS.PIN1 = 0; //00:PC1 01:EPWM2 10:RESERVED 11:RESERVED

	/* PD4 */
	GPD->I_EN.BITS.PIN4 = 1;
	GPD->MODE.BITS.PIN4 = 0; //00:PD4 01:ECAP1 10:RESERVED 11:RESERVED

	/* PD5 */
	GPD->I_EN.BITS.PIN5 = 1;
	GPD->MODE.BITS.PIN5 = 0; //00:PD5 01:ECAP2 10:RESERVED 11:RESERVED
	delay_1ms(3);

	hal_gpio_init_default();
    // key wake up start
	GPC->I_EN.BITS.PIN6 = 1;
	GPC->O_EN.BITS.PIN6 = 0;
	GPC->MODE.BITS.PIN6 = 0; //00:PC6 01:JTAG_DAT 10:BPWM8 11:RESERVED
	GPC->ITEN.BITS.PIN6 = 1;
	GPC->ITTP.BITS.PIN6 = 0;
	if(gd->ship_mode_cnt == SHIP_MODE_CNT)
	{
		// touch wake up start
		GPB->I_EN.BITS.PIN4 = 1;
		GPB->O_EN.BITS.PIN4 = 0;
		GPB->MODE.BITS.PIN4 = 0; //00:PB4 01:JTAG_DAT 10:BPWM8 11:RESERVED
		GPB->ITEN.BITS.PIN4 = 1;
		GPB->ITTP.BITS.PIN4 = 0; //00:Falling Edge
	}
	else
	{
		// touch wake up
		GPB->I_EN.BITS.PIN4 = 1;
		GPB->O_EN.BITS.PIN4 = 0;
		GPB->MODE.BITS.PIN4 = 0; //00:PB4 01:JTAG_DAT 10:BPWM8 11:RESERVED
		GPB->ITEN.BITS.PIN4 = 1;
		GPB->ITTP.BITS.PIN4 = 0; //00:Falling Edge

	}

#if(BUCKBOOST_USED_NU6801 == 1)
    // charger irq wake up start
	GPD->I_EN.BITS.PIN1 = 1;
	GPD->MODE.BITS.PIN1 = 0; //00:PB4 01:JTAG_DAT 10:BPWM8 11:RESERVED
	GPD->ITEN.BITS.PIN1 = 1;
	GPD->ITTP.BITS.PIN1 = 0;
#endif
#if(BUCKBOOST_USED_NU6805 == 1)
	if(gd->SOC_SleepTime_s >=25)
	{
	    // charger irq wake up start
		GPD->I_EN.BITS.PIN1 = 1;
		GPD->MODE.BITS.PIN1 = 0; //00:PB4 01:JTAG_DAT 10:BPWM8 11:RESERVED
		GPD->ITEN.BITS.PIN1 = 1;
		GPD->ITTP.BITS.PIN1 = 0;
	}
	//if(gd->SOC_SleepTime_s <=25)
	else
	{
		hal_wdt_feed();
		_SET_I2CM_SDA_OUTPUT();
		_SET_I2CM_SCL_OUTPUT();
		uint8_t read;
		hal_i2cm_read_one_byte(NU6805_I2C_DEV_ADDR,REG_discharge_Control,&read);
		hal_i2cm_wirte_one_byte(NU6805_I2C_DEV_ADDR,REG_discharge_Control,read & (~0x0F));
	//	uint8_t read;
		hal_i2cm_read_one_byte(NU6805_I2C_DEV_ADDR,REG_Powerpath_Control,&read);
		hal_i2cm_wirte_one_byte(NU6805_I2C_DEV_ADDR,REG_Powerpath_Control,read & (~0x07));
		hal_wdt_feed();

		hal_i2cm_read_one_byte(NU6805_I2C_DEV_ADDR,REG_Mode_Control,&read);
		hal_i2cm_wirte_one_byte(NU6805_I2C_DEV_ADDR,REG_Mode_Control,read & (~0x11));

		hal_i2cm_wirte_one_byte(NU6805_I2C_DEV_ADDR,REG_IRQ_Event1,0xFF);
		hal_i2cm_wirte_one_byte(NU6805_I2C_DEV_ADDR,REG_IRQ_Event2,0xFF);

	/*    hal_i2cm_read_one_byte(NU6805_I2C_DEV_ADDR,REG_Indt_Control,&read);
		hal_i2cm_wirte_one_byte(NU6805_I2C_DEV_ADDR,REG_Indt_Control,read & (~0x07));*/
		hal_i2cm_wirte_one_byte(NU6805_I2C_DEV_ADDR,REG_Indt_Control,0x03);

	 	ubsd_wb7720_sleep();
	}
#endif
    GPA->PDEN.BITS.PIN0 = 1;
    GPA->MODE.BITS.PIN0 = 0;
    GPA->PDEN.BITS.PIN1 = 1;
    GPA->MODE.BITS.PIN1 = 0;
	fml_nu103x_config(_1030_CFG_ALL_RST);
	fml_nu103x_config(_1030_CFG_VDD_V5V_BUCK_DIS);
	fml_nu103x_config(_1030_CFG_LPM_EN_);

	/* SleepToSleep 期间 I2C 总线活动 (NU6805) 可能通过 SCL 下降沿
	 * 唤醒 WB7720 的 EXTI，导致 WB7720 白跑耗电。
	 * 重新发送 SLEEP CMD 确保 WB7720 回到 STOP 模式。 */
	_SET_I2CM_SDA_OUTPUT();
	_SET_I2CM_SCL_OUTPUT();
	ubsd_wb7720_sleep();

    VIC_vModuleDisable();
    hal_wdt_feed();
	SYS->PWR_CTRL.BITS.SLEEP_MODE_EN = 1;
	SYS->PWR_CTRL.BITS.GPIO_WKUP_DIS = 0;        // [NEW-VICTOR] 锟斤拷锟斤拷确锟斤拷GPIO锟斤拷锟窖癸拷锟斤拷使锟斤拷


}
void SLP_vSleepQToSleep(void)
{

}

void SLP_vSleepQToNormal(void)
{

}
uint8_t sleep_idle_qdt_back_to_normal(void)
{
	return (gd->tx_infos.q_fact + ap->q_factor_reco_value > ap->q_factor_base_value + sleep_q_68nf_thd && gd->tx_infos.q_fact < ap->q_factor_limH_value &&
			gd->tx_infos.f_self + ap->fs_reco_value > ap->fs_base_value + sleep_f_68nf_thd && gd->tx_infos.f_self < ap->fs_limH_value+500);
}
uint8_t SLP_u8SleepModeQDetect(void)
{

	//hal_gpio_init();// needs update, only the q IOs
	//fml_bsp_init();

	GPC->I_EN.BITS.PIN0 = 0;
	GPC->O_EN.BITS.PIN0 = 1;
	GPC->DOUT.BITS.PIN0 = 0;
	GPC->ODEN.BITS.PIN0 = 0;
	GPC->PUEN.BITS.PIN0 = 0;
	GPC->PDEN.BITS.PIN0 = 0;
	GPC->MODE.BITS.PIN0 = 1; //00:PC0 01:EPWM1 10:RESERVED 11:RESERVED

	/* PC1 */
	GPC->I_EN.BITS.PIN1 = 0;
	GPC->O_EN.BITS.PIN1 = 1;
	GPC->DOUT.BITS.PIN1 = 0;
	GPC->ODEN.BITS.PIN1 = 0;
	GPC->PUEN.BITS.PIN1 = 0;
	GPC->PDEN.BITS.PIN1 = 0;
	GPC->MODE.BITS.PIN1 = 1; //00:PC1 01:EPWM2 10:RESERVED 11:RESERVED

	/* PD4 */
	GPD->I_EN.BITS.PIN4 = 1;
	GPD->MODE.BITS.PIN4 = 1; //00:PD4 01:ECAP1 10:RESERVED 11:RESERVED

	/* PD5 */
	GPD->I_EN.BITS.PIN5 = 1;
	GPD->MODE.BITS.PIN5 = 1; //00:PD5 01:ECAP2 10:RESERVED 11:RESERVED

//	hal_sys_init();
	//hal_vic_init();

	fml_nu103x_config(_1030_CFG_ALL_RST);
	fml_nu103x_config(_1030_CFG_LPM_DIS);
//	delay_1ms(5);


	fml_nu103x_por_rst();

	fml_qdt_detect((uint32_t *)&gd->tx_infos.q_fact, (uint32_t *)&gd->tx_infos.f_self);
	fml_nu103x_config(_1030_CFG_ALL_RST);
	fml_nu103x_config(_1030_CFG_LPM_EN_);
	uint8_t u8NeedToNormal = 0;
	if(gd->sleep_q_times <20)
	{
		gd->sleep_q_times ++;// from sleep start, not judge.
		return u8NeedToNormal;
	}
	sleep_printk("\r\n sleep: [%d] [q:%d,%d,%d] [f:%d,%d,%d] ",gd->ptx_idle_phase_status,
			gd->tx_infos.q_fact, ap->q_factor_base_value + sleep_q_68nf_thd , gd->tx_infos.q_fact - (ap->q_factor_base_value+ sleep_q_68nf_thd),
			gd->tx_infos.f_self, ap->fs_base_value+ sleep_f_68nf_thd , gd->tx_infos.f_self - (ap->fs_base_value+sleep_f_68nf_thd));
	switch (gd->ptx_idle_phase_status)
	{
		case WPC_IDLE_STAT_STANDBY:
			if ((gd->tx_infos.q_fact  + ap->q_factor_obj_value + 20  < ap->q_factor_base_value+ sleep_q_68nf_thd ) ||
				(gd->tx_infos.f_self > ap->fs_limL_value && gd->tx_infos.f_self + ap->fs_obj_value < ap->fs_base_value+ sleep_f_68nf_thd ))
			{
				u8NeedToNormal = 1;
			}
			break;
		case WPC_IDLE_STAT_XER_COM:
			if(sleep_idle_qdt_back_to_normal())
			{
				gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
				gd->sleep_qdt_complete_charg_count=0;
			}
			else
			{
				if ((++gd->sleep_qdt_complete_charg_count * 750) > 11 * 60 * 1000) //11min
				{
					gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
				}
			}
			break;
		case WPC_IDLE_STAT_XER_FOD:
		case WPC_IDLE_STAT_QDT_FOD:
		case WPC_IDLE_STAT_LAR_MET:
		case WPC_IDLE_STAT_EPT_ERR:
			if(sleep_idle_qdt_back_to_normal())
			{
				if(++gd->sleep_qdt_fod_rec_count > 3)
				{
					gd->sleep_qdt_fod_rec_count = 0;
					gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
				}
			}
			else gd->sleep_qdt_fod_rec_count = 0;
			break;
		case WPC_IDLE_STAT_EPT_RES:
		case WPC_IDLE_STAT_EPT_REP:
			gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
			break;

		default:
			break;
	}
	sleep_printk("\r\n Q wake-up? [%d]",u8NeedToNormal);
	return u8NeedToNormal;
}
#define FMS_STS_IDLE       0
#define FMS_STS_NORMAL     1
#define FMS_STS_SLEEP      2

#define RST_SRC_IDLE            0
#define RST_SRC_WARMUP_DONE     1
#define RST_SRC_1PTIMER         2
#define RST_SRC_GPIO            3
#define RST_SRC_PROTOCOL        4
uint8_t reset_cnt;

extern uint16_t key_ui_cnt;

/*
 * tc_check_wake() - Software CC polling for sleep wake-up detection
 *
 * Ported from platform code (nu17112_powerbank/app/sleep.c:665-925).
 * Replaces hardware DRP wake-up for Port1 (CCB) where CC2_ROLE=OPEN
 * prevents hardware DRP toggle.
 *
 * 4-step polling:
 *   Step 0: Check Source disconnect (lighting_mode / bat_dead)
 *   Step 1: Check Sink (charger) attach - set CC=Rd, read Rp
 *   Step 2: Check Source (device) attach - set CC=Rp_1.5, read Rd
 *   Step 3: Re-confirm Sink attach - set CC=Rd again
 *
 * Returns bitmask of detected events (0 = no change).
 */
#define TC_WAKE_TC0_OUT		(0x01 << 1)
#define TC_WAKE_TC0_SNK		(0x01 << 2)
#define TC_WAKE_TC0_SRC		(0x01 << 3)
#define TC_WAKE_TC1_OUT		(0x01 << 4)
#define TC_WAKE_TC1_SNK		(0x01 << 5)
#define TC_WAKE_TC1_SRC		(0x01 << 6)

uint8_t tc_check_wake(void)
{
	uint8_t ret = 0;
	uint8_t step = 0;

	extern bool tc_src_is_connected(enum tc_cc_status cc1, enum tc_cc_status cc2);

	while(step < 4)
	{
		if(step == 0)
		{
			/* Step 0: Check Source disconnect for lighting/dead_batt ports */
#if(CONFIG_TYPECA_SUPPORT == 1)
			if(gd->tc0_lighting_mode || gd->bat_dead_flag_with_snk0)
			{
				TCPC->CCA_CTRL.BITS.CC_BLOCK_DIS = 0;
				TCPC->CCA_CTRL.BITS.CC_DB_RD_DIS = 1;
			}
#endif

#if(CONFIG_TYPECB_SUPPORT == 1)
			if(gd->tc1_lighting_mode || gd->bat_dead_flag_with_snk1)
			{
				TCPC->CCB_CTRL.BITS.CC_BLOCK_DIS = 0;
				TCPC->CCB_CTRL.BITS.CC_DB_RD_DIS = 1;
			}
#endif
			if(gd->tc0_lighting_mode || gd->bat_dead_flag_with_snk0 || gd->tc1_lighting_mode || gd->bat_dead_flag_with_snk1)
				delay_1us(2000);

#if(CONFIG_TYPECA_SUPPORT == 1)
			if(gd->tc0_lighting_mode || gd->bat_dead_flag_with_snk0)
			{
				enum tc_cc_status cc1, cc2;
				pdlib_tcpc_get_cc(TYPEC_PORT_A, &cc1, &cc2);
				sleep_printk("\r\n [%d]0cc1:[%d %d 0x%x]\n", step, cc1, cc2, TCPC->CCA_STAT.WORD);
				if (!tc_src_is_connected(cc1, cc2))
				{
					gd->rd0_cnt++;
					sleep_printk("\r\n rd0_cnt = %d\n", gd->rd0_cnt);
					if(gd->rd0_cnt >= 10)
					{
						gd->bat_dead_flag_with_snk0 = 0;
						gd->tc0_lighting_mode = 0;
						ret |= TC_WAKE_TC0_OUT;
					}
				}
				else
				{
					gd->rd0_cnt = 0;
				}
			}
#endif

#if(CONFIG_TYPECB_SUPPORT == 1)
			if(gd->tc1_lighting_mode || gd->bat_dead_flag_with_snk1)
			{
				enum tc_cc_status cc1, cc2;
				pdlib_tcpc_get_cc(TYPEC_PORT_B, &cc1, &cc2);
				sleep_printk("\r\n 1cc0:[%d %d 0x%x]\n", cc1, cc2, TCPC->CCB_STAT.WORD);
				if (!tc_src_is_connected(cc1, cc2))
				{
					gd->rd1_cnt++;
					sleep_printk("\r\n rd1_cnt = %d\n", gd->rd1_cnt);
					if(gd->rd1_cnt >= 10)
					{
						gd->bat_dead_flag_with_snk1 = 0;
						gd->tc1_lighting_mode = 0;
						ret |= TC_WAKE_TC1_OUT;
					}
				}
				else
				{
					gd->rd1_cnt = 0;
				}
			}
#endif
		}

		if(step == 1)
		{
			/* Step 1: Check Sink (charger) attach - set CC=Rd, detect Rp */
#if(CONFIG_TYPECA_SUPPORT == 1)
			if(!(gd->tc0_lighting_mode || gd->bat_dead_flag_with_snk0))
			{
				TCPC->CCA_CTRL.BITS.CC_BLOCK_DIS = 0;
				TCPC->CCA_CTRL.BITS.CC_DB_RD_DIS = 1;
				pdlib_tcpc_set_cc(TYPEC_PORT_A, TYPEC_CC_RD);
			}
#endif

#if(CONFIG_TYPECB_SUPPORT == 1)
			if(!(gd->tc1_lighting_mode || gd->bat_dead_flag_with_snk1))
			{
				TCPC->CCB_CTRL.BITS.CC_BLOCK_DIS = 0;
				TCPC->CCB_CTRL.BITS.CC_DB_RD_DIS = 1;
				pdlib_tcpc_set_cc(TYPEC_PORT_B, TYPEC_CC_RD);
				TCPC->CCB_ROLE.BITS.CC2_ROLE = 0;  // Force CC2=OPEN: PC8 pull-up protection
			}
#endif
			if(!(gd->tc0_lighting_mode || gd->bat_dead_flag_with_snk0) || !(gd->tc1_lighting_mode || gd->bat_dead_flag_with_snk1))
			{
				delay_1us(1700);
			}

#if(CONFIG_TYPECA_SUPPORT == 1)
			if(!(gd->tc0_lighting_mode || gd->bat_dead_flag_with_snk0))
			{
				enum tc_cc_status cc1, cc2;
				pdlib_tcpc_get_cc(TYPEC_PORT_A, &cc1, &cc2);
				if(cc1 != TYPEC_CC_OPEN || cc2 != TYPEC_CC_OPEN)
				{
					sleep_printk("\n tc_wake K1_cc:[%d %d 0x%x]\n", cc1, cc2, TCPC->CCA_STAT.WORD);
					ret |= TC_WAKE_TC0_SNK;
				}
				TCPC->CCA_CTRL.BITS.CC_DB_RD_DIS = 1;
			}
#endif

#if(CONFIG_TYPECB_SUPPORT == 1)
			if(!(gd->tc1_lighting_mode || gd->bat_dead_flag_with_snk1))
			{
				enum tc_cc_status cc1, cc2;
				pdlib_tcpc_get_cc(TYPEC_PORT_B, &cc1, &cc2);
				if(cc1 != TYPEC_CC_OPEN || cc2 != TYPEC_CC_OPEN)
				{
					sleep_printk("\n tc_wake K2_cc:[%d %d 0x%x]\n", cc1, cc2, TCPC->CCB_STAT.WORD);
					ret |= TC_WAKE_TC1_SNK;
				}
				TCPC->CCB_CTRL.BITS.CC_DB_RD_DIS = 1;
			}
#endif
		}

		if(step == 2)
		{
			/* Step 2: Check Source (device) attach - set CC=Rp_1.5, detect Rd */
#if(CONFIG_TYPECA_SUPPORT == 1)
			if(!(gd->tc0_lighting_mode || gd->bat_dead_flag_with_snk0))
			{
				TCPC->CCA_CTRL.BITS.CC_BLOCK_DIS = 0;
				TCPC->CCA_CTRL.BITS.CC_DB_RD_DIS = 1;
				pdlib_tcpc_set_cc(TYPEC_PORT_A, TYPEC_CC_RP_1_5);
			}
#endif

#if(CONFIG_TYPECB_SUPPORT == 1)
			if(!(gd->tc1_lighting_mode || gd->bat_dead_flag_with_snk1))
			{
				TCPC->CCB_CTRL.BITS.CC_BLOCK_DIS = 0;
				TCPC->CCB_CTRL.BITS.CC_DB_RD_DIS = 1;
				pdlib_tcpc_set_cc(TYPEC_PORT_B, TYPEC_CC_RP_1_5);
				TCPC->CCB_ROLE.BITS.CC2_ROLE = 0;  // Force CC2=OPEN: PC8 pull-up protection
			}
#endif
			if(!(gd->tc0_lighting_mode || gd->bat_dead_flag_with_snk0) || !(gd->tc1_lighting_mode || gd->bat_dead_flag_with_snk1))
			{
				delay_1us(1700);
			}

#if(CONFIG_TYPECA_SUPPORT == 1)
			if(!(gd->tc0_lighting_mode || gd->bat_dead_flag_with_snk0))
			{
				enum tc_cc_status cc1, cc2;
				pdlib_tcpc_get_cc(TYPEC_PORT_A, &cc1, &cc2);
				if(cc1 == TYPEC_CC_RD || cc2 == TYPEC_CC_RD)
				{
					sleep_printk("\n tc_wake K3_cc:[%d %d 0x%x]\n", cc1, cc2, TCPC->CCA_STAT.WORD);
					ret |= TC_WAKE_TC0_SRC;
				}
			}
#endif

#if(CONFIG_TYPECB_SUPPORT == 1)
			if(!(gd->tc1_lighting_mode || gd->bat_dead_flag_with_snk1))
			{
				enum tc_cc_status cc1, cc2;
				pdlib_tcpc_get_cc(TYPEC_PORT_B, &cc1, &cc2);
				if(cc1 == TYPEC_CC_RD || cc2 == TYPEC_CC_RD)
				{
					sleep_printk("\n tc_wake K4_cc:[%d %d 0x%x]\n", cc1, cc2, TCPC->CCB_STAT.WORD);
					ret |= TC_WAKE_TC1_SRC;
				}
			}
#endif
		}

		if(step == 3)
		{
			/* Step 3: Re-confirm Sink attach - set CC=Rd again */
#if(CONFIG_TYPECA_SUPPORT == 1)
			if(!(gd->tc0_lighting_mode || gd->bat_dead_flag_with_snk0))
			{
				TCPC->CCA_CTRL.BITS.CC_BLOCK_DIS = 0;
				TCPC->CCA_CTRL.BITS.CC_DB_RD_DIS = 1;
				pdlib_tcpc_set_cc(TYPEC_PORT_A, TYPEC_CC_RD);
			}
#endif

#if(CONFIG_TYPECB_SUPPORT == 1)
			if(!(gd->tc1_lighting_mode || gd->bat_dead_flag_with_snk1))
			{
				TCPC->CCB_CTRL.BITS.CC_BLOCK_DIS = 0;
				TCPC->CCB_CTRL.BITS.CC_DB_RD_DIS = 1;
				pdlib_tcpc_set_cc(TYPEC_PORT_B, TYPEC_CC_RD);
				TCPC->CCB_ROLE.BITS.CC2_ROLE = 0;  // Force CC2=OPEN: PC8 pull-up protection
			}
#endif
			if(!(gd->tc0_lighting_mode || gd->bat_dead_flag_with_snk0) || !(gd->tc1_lighting_mode || gd->bat_dead_flag_with_snk1))
			{
				delay_1us(1700);
			}

#if(CONFIG_TYPECA_SUPPORT == 1)
			if(!(gd->tc0_lighting_mode || gd->bat_dead_flag_with_snk0))
			{
				enum tc_cc_status cc1, cc2;
				pdlib_tcpc_get_cc(TYPEC_PORT_A, &cc1, &cc2);
				if(cc1 != TYPEC_CC_OPEN || cc2 != TYPEC_CC_OPEN)
				{
					sleep_printk("\n tc_wake K5_cc:[%d %d 0x%x]\n", cc1, cc2, TCPC->CCA_STAT.WORD);
					ret |= TC_WAKE_TC0_SNK;
				}
				TCPC->CCA_CTRL.BITS.CC_DB_RD_DIS = 1;
			}
#endif

#if(CONFIG_TYPECB_SUPPORT == 1)
			if(!(gd->tc1_lighting_mode || gd->bat_dead_flag_with_snk1))
			{
				enum tc_cc_status cc1, cc2;
				pdlib_tcpc_get_cc(TYPEC_PORT_B, &cc1, &cc2);
				if(cc1 != TYPEC_CC_OPEN || cc2 != TYPEC_CC_OPEN)
				{
					sleep_printk("\n tc_wake K6_cc:[%d %d 0x%x]\n", cc1, cc2, TCPC->CCB_STAT.WORD);
					ret |= TC_WAKE_TC1_SNK;
				}
				TCPC->CCB_CTRL.BITS.CC_DB_RD_DIS = 1;
			}
#endif
		}

		step++;
	}

	// Restore CCB CC2=OPEN before returning to sleep
	TCPC->CCB_ROLE.BITS.CC2_ROLE = 0;

	return ret;
}

void RST_vCheck(void)
{
	uint32_t tmr_cnt;
#if SUPPORT_SLEEP_LOG
	/* PB7 */
	GPB->I_EN.BITS.PIN7 = 0;
	GPB->O_EN.BITS.PIN7 = 1;
	GPB->DOUT.BITS.PIN7 = 1;
	GPB->ODEN.BITS.PIN7 = 0;
	GPB->PUEN.BITS.PIN7 = 1;
	GPB->PDEN.BITS.PIN7 = 0;
	GPB->MODE.BITS.PIN7 = 1; //00:PB7 01:UART1_TXD 10:RESERVED 11:RESERVED
	hal_uart_init(UART1);
#endif



		sleep_printk("\r\n sleep check");
		sleep_printk(" rst=%d CCB[S=0x%x C=0x%x R=0x%x] FSM=0x%x WK=%d",
			SYS->OPR_STAT.BITS.RST_SRC,
			TCPC->CCB_STAT.WORD,   // CC1(bit1:0) CC2(bit3:2) 实际检测值
			TCPC->CCB_CTRL.WORD,   // CC block/LPMODE 配置
			TCPC->CCB_ROLE.WORD,   // CC1/CC2 role + DRP
			TCPC->FSM_STAT.WORD,   // DRP FSM 状态
			SYS->PWR_CTRL.BITS.TCPC_WKUP_DIS);  // 0=唤醒使能 1=禁用
		gd->idle_to_sleep_cnt = 0;
		switch(SYS->OPR_STAT.BITS.RST_SRC)
		{
			case RST_SRC_1PTIMER:
				tmr_cnt = TMR0->LOAD_CNT.WORD;
				gd->Bat_RTC_Timer +=  tmr_cnt >> 2;
#if CONFIG_NEW_CCC_LOG_ENABLE
				{
					uint32_t sleep_ms = (tmr_cnt >> 4) - 30; // -30ms empirical correction, re-calibrate on hardware
					VIC_vModuleDisable(); // Protect 32-bit read-modify-write from TMR1 ISR
					gd->Bat_RTC_Milliseconds += sleep_ms;
					if (gd->Bat_RTC_Milliseconds >= 1000) {
						uint32_t extra = gd->Bat_RTC_Milliseconds / 1000;
						gd->Bat_RTC_Milliseconds %= 1000;
						gd->Bat_RTC_Seconds += extra;
					}
					VIC_vModuleEnable();
				}
#endif
				//sleep_printk("\r\n sleep check- timer[%d]",gd->reset_magicode);
				if(gd->reset_magicode == 55)
				{
					gd->reset_magicode = 0;
					sleep_printk("\r\n reset go on");
				}
				else if(gd->bat_dead_flag_with_snk0 || gd->bat_dead_flag_with_snk1)
				{
					hal_wdt_feed();
					sleep_printk("\r\n set Rd");
				#if(CONFIG_TYPECA_SUPPORT == 1)
					TCPC->CCA_CTRL.BITS.CC_BLOCK_DIS = 0;
					TCPC->CCA_CTRL.BITS.CC_DB_RD_DIS = 1;
				#endif
				#if(CONFIG_TYPECB_SUPPORT == 1)
					TCPC->CCB_CTRL.BITS.CC_BLOCK_DIS = 0;
					TCPC->CCB_CTRL.BITS.CC_DB_RD_DIS = 1;
				#endif
					delay_1us(1000);
					enum tc_cc_status cc1,cc2;
				#if(CONFIG_TYPECA_SUPPORT == 1)

					if(gd->bat_dead_flag_with_snk0)
					{
						pdlib_tcpc_get_cc(TYPEC_PORT_A,&cc1,&cc2);
						sleep_printk("\r\n 0cc:[%d %d 0x%x]\n",cc1,cc2,TCPC->CCA_STAT.WORD);
						extern bool tc_src_is_connected(enum tc_cc_status cc1,enum tc_cc_status cc2);
						if (!tc_src_is_connected(cc1,cc2))
						{
							gd->rd0_cnt++;
							sleep_printk("\r\n rd0_cnt = %d\n",gd->rd0_cnt);
							if(gd->rd0_cnt >= 10) {gd->bat_dead_flag_with_snk0 = 0;gd->bat_dead_flag_with_snk1 = 0;break;}
							else
							{
								TCPC->CCA_CTRL.BITS.CC_BLOCK_DIS = 1;
							}
						}
						else
						{
							gd->rd0_cnt = 0;
						}
					}
				#endif

				#if(CONFIG_TYPECB_SUPPORT == 1)
					if(gd->bat_dead_flag_with_snk1)
					{
						pdlib_tcpc_get_cc(TYPEC_PORT_B,&cc1,&cc2);
						sleep_printk("\r\n 0cc:[%d %d 0x%x]\n",cc1,cc2,TCPC->CCB_STAT.WORD);
						extern bool tc_src_is_connected(enum tc_cc_status cc1,enum tc_cc_status cc2);
						if (!tc_src_is_connected(cc1,cc2))
						{
							gd->rd1_cnt++;
							sleep_printk("\r\n rd1_cnt = %d\n",gd->rd1_cnt);
							if(gd->rd1_cnt >= 10) {gd->bat_dead_flag_with_snk0 = 0;gd->bat_dead_flag_with_snk1 = 0;;break;}
							else
							{
								TCPC->CCB_CTRL.BITS.CC_BLOCK_DIS = 1;
							}
						}
						else
						{
							gd->rd1_cnt = 0;
						}
					}
				#endif
				    SLP_vSleepToSleep();
				}
				else
				{
					hal_wdt_feed();
				#if(CONFIG_TYPECA_SUPPORT == 1)
					if(gd->tc0_lighting_mode)
					{
						TCPC->CCA_CTRL.BITS.CC_BLOCK_DIS = 0;
						TCPC->CCA_CTRL.BITS.CC_DB_RD_DIS = 1;
						enum tc_cc_status cc1,cc2;
						delay_1us(1000);
						pdlib_tcpc_get_cc(0, &cc1,&cc2);
						sleep_printk("\r\n 0cc:[%d %d]\n",cc1,cc2);
						if(cc1 != TYPEC_CC_RD && cc2 != TYPEC_CC_RD)
						{
							gd->light0_cnt++;
							if(gd->light0_cnt >= 10)
							{
								gd->tc0_lighting_mode = 0;
								sleep_printk("\r\n lighting_mode0 exit");
								break;
							}
						}
						else
							gd->light0_cnt = 0;
					#if(CONFIG_TYPECB_SUPPORT == 1)
						if(!gd->tc1_lighting_mode)
						{
							if(pdlib_get_drp_toggle_result(1) == TYPEC_DRP_SNK_CONNECTED)
							{
								break;
							}
							else if(pdlib_get_drp_toggle_result(1) == TYPEC_DRP_SRC_CONNECTED)
							{
								break;
							}
						}
					#endif
					}

				#endif

				#if(CONFIG_TYPECB_SUPPORT == 1)
					if(gd->tc1_lighting_mode)
					{
						TCPC->CCB_CTRL.BITS.CC_BLOCK_DIS = 0;
						TCPC->CCB_CTRL.BITS.CC_DB_RD_DIS = 1;
						enum tc_cc_status cc1,cc2;
						delay_1us(1000);
						pdlib_tcpc_get_cc(1, &cc1,&cc2);

						sleep_printk("\r\n [%d]cc:[%d %d]\n",1,cc1,cc2);
						if(cc1 != TYPEC_CC_RD && cc2 != TYPEC_CC_RD)
						{
							gd->light1_cnt++;
							if(gd->light1_cnt >= 10)
							{
								gd->tc1_lighting_mode = 0;
								sleep_printk("\r\n lighting_mode1 exit");
								break;
							}
						}
						else
							gd->light1_cnt = 0;

						if(!gd->tc0_lighting_mode)
						{
							if(pdlib_get_drp_toggle_result(0) == TYPEC_DRP_SNK_CONNECTED)
							{
								break;
							}
							else if(pdlib_get_drp_toggle_result(0) == TYPEC_DRP_SRC_CONNECTED)
							{
								break;
							}
						}
					}
				#endif

#if SLEEPQ_WAKEUP_ENABLE
					if(SLP_u8SleepModeQDetect())// need to normal
					{
/*						SYS->PWR_CTRL.WORD &= !SYS_PWR_CTRL_SLEEP_MODE_EN_Msk;
						hal_wdt_init_to_reset();// re_enable WD, hope to reset the MCU
						gd->reset_magicode = 55;
						TMR0->SPL_CTRL.WORD &= !TMR_SPL_CTRL_WKUP_EN_Msk;
						do
						{
							sleep_printk("\r\n wait to reset");
							reset_cnt++;
							delay_1ms(1000);
						}while (reset_cnt<4);*/
						sleep_printk("\r\n wake-up");
#if(CONFIG_SHIP_MODE_ENABLE_DEBUG ==1)
						if(++gd->ship_mode_cnt > SHIP_MODE_CNT)  gd->ship_mode_cnt= SHIP_MODE_CNT;
#endif
					}
					else
					{
					//	sleep_printk("\r\n sleep again");
						SLP_vSleepToSleep();
					//	SLP_vNormalToSleep();
					}
#else
					{
						uint8_t tc_ret = tc_check_wake();
						sleep_printk("\r\n tc wake[0x%x]", tc_ret);
						if (tc_ret) break;
					}
						SLP_vSleepToSleep();
#endif
				}
				break;
			case RST_SRC_PROTOCOL:
				gd ->ship_mode_cnt = 0;
				sleep_printk("\r\n sleep check- protocol");
				SYS->PWR_CTRL.WORD &= !SYS_PWR_CTRL_SLEEP_MODE_EN_Msk;
				gd->sigle_clicked = 0;
				break;
			case RST_SRC_GPIO:

				if(++gd->ship_mode_cnt > SHIP_MODE_CNT) gd->ship_mode_cnt=0;

				SYS->PWR_CTRL.WORD &= !SYS_PWR_CTRL_SLEEP_MODE_EN_Msk;

				// 锟斤拷锟斤拷欠锟斤拷锟斤拷锟叫э拷陌锟斤拷锟�锟斤拷锟斤拷锟斤拷锟斤拷锟铰硷拷
				if(PC6_KEY_PRESSED || PB4_TOUCH_PRESSED)
				{
				// 锟斤拷效锟侥伙拷锟斤拷锟铰硷拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
				 sleep_printk("\r\n GPIO wake-up: PC6=%d PB4=%d", PC6_KEY_PRESSED, PB4_TOUCH_PRESSED);
				 if(PB4_TOUCH_PRESSED&&!PC6_KEY_PRESSED){
					gd->touch_to_weakup = 1;
				 }else{
					gd->touch_to_weakup = 0;
				 }
				}
				else
				{
					sleep_printk("\r\n GPIO false wake-up, continue sleep");
					SLP_vSleepToSleep();
					break;
			    }

			#if(CONFIG_TYPECA_SUPPORT == 1)
				if(gd->tc0_lighting_mode) gd->tc0_lighting_mode = 0;
			#endif

			#if(CONFIG_TYPECB_SUPPORT == 1)
				if(gd->tc1_lighting_mode) gd->tc1_lighting_mode = 0;
			#endif

			#if(CONFIG_WPC_SUPPORT == 1)
				gd->wpc_disable = 0;
			#endif
				//key_ui_cnt = 20;
				key_ui_cnt = KEY_UI_DISPLAY_TICKS;
				sleep_printk("\r\n sleep check- GPIO");
				break;
			case RST_SRC_WARMUP_DONE:
			default:
				sleep_printk("\r\n wake_up");
				gd->power_on_magic = 0x00;
				sleep_printk("sleep power on\n");
				break;

		}
}

