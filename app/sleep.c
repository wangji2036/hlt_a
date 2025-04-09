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
#include "typec.h"
//uint8_t reset_magic_code;

#if ONLY7_5W_ENALBE
static uint16_t sleep_q_68nf_thd = 0;
static uint16_t sleep_f_68nf_thd = 0;
#else
#if CAPACITOR_300_NF
static uint16_t sleep_q_68nf_thd = 115;
static uint16_t sleep_f_68nf_thd = 1235;// sleep F -normal F,
#else
static uint16_t sleep_q_68nf_thd = 166;
static uint16_t sleep_f_68nf_thd = 1317;// sleep F -normal F,
#endif
#endif
void SLP_vNormalToSleep(void)
{
	sleep_printk("\r\n enter sleep");
	hal_wdt_feed();
	fm1210_sleep();
   SYS->PWR_CTRL.WORD = 0;
	gd->rd0_cnt = 0;
	gd->rd1_cnt = 0;
	gd->reset_magicode = 0;// magic code,important for sleep Q wake-up.
	gd->sleep_q_times = 0;
	if(!gd->tc0_lighting_mode) hal_tcpc_set_cc(TYPEC_PORT_A,TYPEC_CC_OPEN);
	else hal_tcpc_set_cc(TYPEC_PORT_A,TYPEC_CC_RP_DEF);
	if(!gd->tc1_lighting_mode) hal_tcpc_set_cc(TYPEC_PORT_B,TYPEC_CC_OPEN);
	else hal_tcpc_set_cc(TYPEC_PORT_B,TYPEC_CC_RP_DEF);
#if(BUCKBOOST_USED_NU6801 == 1)
    // enable all 6801 INT
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_INT_MASK,0x80);
	// 6801 sleep function and firmware work-round start
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,0x00);//09
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_BUBO_CTRL,0x09);//09
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VBUS_SET_H,0x1C);//0C
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VBUS_SET_L,0x50);//0D
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_BUBO_CTRL,0x0D);//09
	hal_wdt_feed();
	delay_1ms(500);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0x65);//
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0x37);//
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0x2D);//
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0xF9);//
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x51,0x29);//
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x51,0xCB);//
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x51,0xE2);//
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x51,0x6A);//

	hal_wdt_feed();
	delay_1ms(500);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x63,0x01);//
	hal_wdt_feed();
	delay_1ms(500);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_BUBO_CTRL,0x01);//09
	hal_wdt_feed();
	delay_1ms(500);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_MISC_CTRL,0x41);//10

	/* 6801 sleep function and firmware work-round end*/
/*#else if (BUCKBOOST_USED_NU6805 ==1)

	hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_Indt_Control,0x00);

    hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_Indt_Control,0x11);*/
#endif

#if(BUCKBOOST_USED_NU6805 == 1)
	hal_wdt_feed();

	uint8_t read;
	hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_discharge_Control,&read);
    hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_discharge_Control,read & (~0x0F));
//	uint8_t read;
	hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_Powerpath_Control,&read);
	hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_Powerpath_Control,read & (~0x07));
	hal_wdt_feed();

	hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_Mode_Control,&read);
    hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_Mode_Control,read & (~0x11));

	hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_IRQ_Event1,0xFF);
	hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_IRQ_Event2,0xFF);

/*    hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_Indt_Control,&read);
	hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_Indt_Control,read & (~0x07));*/
    hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_Indt_Control,0x03);
#endif
    TCPC->CCA_CTRL.WORD = 0;
    TCPC->CCB_CTRL.WORD = 0;
    TCPC->RXD_CTRL.WORD = 0;
    ECAP2->QDT_CTRL.WORD = 0;
	//tcpc wake up start.

	//SYS->PWR_CTRL.WORD &= !SYS_PWR_CTRL_TCPC_WKUP_DIS_Pos;
	//CCA
	  //(Enable CC, Disable RDB, Enter low power mode)


	if(!gd->bat_dead_flag)
	{
		if(!gd->tc0_lighting_mode)
		{
			TCPC->CCA_CTRL.BITS.CC_DB_RD_DIS = 1; // enable cc block
			TCPC->CCA_CTRL.BITS.CC_LPMODE_EN = 1; // enable cc block
			TCPC->CCA_CTRL.BITS.CC_BLOCK_DIS = 0; // enable cc block
			  //(Enable DRP)
			TCPC->CCA_ROLE.BITS.DRP_MODE = 1;
			TCPC->CCA_ROLE.BITS.CC1_ROLE = 1;
			TCPC->CCA_ROLE.BITS.CC2_ROLE = 1;
			TCPC->CCA_CMD_.BITS.CMD_TYPE = 0x99;//(Start DRP)
		}
		else
		{
			SYS->PWR_CTRL.BITS.TCPC_WKUP_DIS = 1;
			sleep_printk("\r\n lighting sleep");
		}
	}
	else
	{

	}
	//CCB
	  //(Enable CC, Disable RDB, Enter low power mode)

	if(!gd->bat_dead_flag)
	{
		if(!gd->tc1_lighting_mode)
		{
			TCPC->CCB_CTRL.BITS.CC_DB_RD_DIS = 1; // enable cc block
			TCPC->CCB_CTRL.BITS.CC_LPMODE_EN = 1; // enable cc block
			TCPC->CCB_CTRL.BITS.CC_BLOCK_DIS = 0; // enable cc block
			  //(Enable DRP)
			TCPC->CCB_ROLE.BITS.DRP_MODE = 1;
			TCPC->CCB_ROLE.BITS.CC1_ROLE = 1;
			TCPC->CCB_ROLE.BITS.CC2_ROLE = 1;
			TCPC->CCB_CMD_.BITS.CMD_TYPE = 0x99;//(Start DRP)
		}
		else
		{
			sleep_printk("\r\n lighting sleep");
			SYS->PWR_CTRL.BITS.TCPC_WKUP_DIS = 1;
		}
	}
	else
	{
		SYS->PWR_CTRL.BITS.GPIO_WKUP_DIS = 1;
		SYS->PWR_CTRL.BITS.TCPC_WKUP_DIS = 1;
		sleep_printk("\r\n sleep Rd");
	}
	_SET_ALL_PINS_IN_PUT();
	hal_wdt_feed();

	BADC->CTRL.WORD = 0;
	EADC->CTRL.WORD = 0;
	I2CS->CTRL.WORD = 0;
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
	I2CM->GEN_CTRL.WORD = 0;

	SYS->CLK_CTRL.WORD = 0;
	SYS->PRO_CTRL.WORD = 0;
	SYS->CLK_CTRL.BITS.XTAL_EN = 0;// disable XTAL
	SYS->PRO_CTRL.BITS.PVD_EN = 0;// disable PVD
 

	TMR1->GEN_CTRL.WORD = 0;
	TMR2->GEN_CTRL.WORD = 0;
	TMR3->GEN_CTRL.WORD = 0;
	TMR0->GEN_CTRL.WORD = 0;
	TMR0->LOAD_CNT.WORD = 16 * 100 * 1 - 1; //first Q,100ms start.
	TMR0->SPL_CTRL.WORD = (_TMR_CLK_SRC_LIRC << TMR_SPL_CTRL_CLK_SRC_Pos) | TMR_SPL_CTRL_WKUP_EN_Msk; //LIRC: 64K
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
	GPC->MODE.BITS.PIN6 = 0; //00:PB4 01:JTAG_DAT 10:BPWM8 11:RESERVED
	GPC->ITEN.BITS.PIN6 = 1;
    // charger irq wake up start
	GPD->I_EN.BITS.PIN1 = 1;
	GPD->MODE.BITS.PIN1 = 0; //00:PB4 01:JTAG_DAT 10:BPWM8 11:RESERVED
	GPD->ITEN.BITS.PIN1 = 1;
	GPD->ITTP.BITS.PIN1 = 0;
	hal_epwm_pwm_stop(EPWM1);
	fml_nu103x_config(_1030_CFG_ALL_RST);
	fml_nu103x_config(_1030_CFG_VDD_V5V_BUCK_DIS);
	fml_nu103x_config(_1030_CFG_LPM_EN_);

	WDT->CTRL.WORD = 0;
	SYS->PWR_CTRL.BITS.SLEEP_MODE_EN = 1;

}
void SLP_vSleepToSleep(void)
{

	gd->reset_magicode = 0;// magic code,important for sleep Q wake-up.
	if(!gd->bat_dead_flag)
	{
		if(gd->sleep_q_times <20)
		{
	        TMR0->LOAD_CNT.WORD = 16 * 50 * 1 - 1; // 50ms fast sleep Q to charge the DH2 CAP, work-round
	    }
	    else
	    {
		    TMR0->LOAD_CNT.WORD = 16 * 1000 * 1 - 1; //500ms
    	}

    }
	else
	{
		SYS->PWR_CTRL.BITS.GPIO_WKUP_DIS = 1;
		SYS->PWR_CTRL.BITS.TCPC_WKUP_DIS = 1;
		sleep_printk("\r\n batlow");
		TMR0->LOAD_CNT.WORD = 16 * 373 * 1 - 1; //500ms
	}
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

	sleep_printk("\r\n sleep again");

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
	hal_gpio_init_default();
    // key wake up start
	GPC->I_EN.BITS.PIN6 = 1;
	GPC->O_EN.BITS.PIN6 = 0;
	GPC->MODE.BITS.PIN6 = 0; //00:PB4 01:JTAG_DAT 10:BPWM8 11:RESERVED
	GPC->ITEN.BITS.PIN6 = 1;
    // charger irq wake up start
	GPD->I_EN.BITS.PIN1 = 1;
	GPD->MODE.BITS.PIN1 = 0; //00:PB4 01:JTAG_DAT 10:BPWM8 11:RESERVED
	GPD->ITEN.BITS.PIN1 = 1;
	GPD->ITTP.BITS.PIN1 = 0;
#if(BUCKBOOST_USED_NU6805 == 1)
	hal_wdt_feed();
	_SET_I2CM_SDA_OUTPUT();
	_SET_I2CM_SCL_OUTPUT();
	uint8_t read;
	hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_discharge_Control,&read);
    hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_discharge_Control,read & (~0x0F));
//	uint8_t read;
	hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_Powerpath_Control,&read);
	hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_Powerpath_Control,read & (~0x07));
	hal_wdt_feed();

	hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_Mode_Control,&read);
    hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_Mode_Control,read & (~0x11));

	hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_IRQ_Event1,0xFF);
	hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_IRQ_Event2,0xFF);

/*    hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_Indt_Control,&read);
	hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_Indt_Control,read & (~0x07));*/
    hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_Indt_Control,0x03);
#endif

	fml_nu103x_config(_1030_CFG_ALL_RST);
	fml_nu103x_config(_1030_CFG_VDD_V5V_BUCK_DIS);
	fml_nu103x_config(_1030_CFG_LPM_EN_);

	WDT->CTRL.WORD = 0;
	SYS->PWR_CTRL.BITS.SLEEP_MODE_EN = 1;

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

void RST_vCheck(void)
{
		sleep_printk("\r\n sleep check");
		gd->idle_to_sleep_cnt = 0;
		switch(SYS->OPR_STAT.BITS.RST_SRC)
		{
			case RST_SRC_1PTIMER:
				//sleep_printk("\r\n sleep check- timer[%d]",gd->reset_magicode);
				if(gd->reset_magicode == 55)
				{
					gd->reset_magicode = 0;
					sleep_printk("\r\n reset go on");
				}
				else if(gd->bat_dead_flag)
				{
					sleep_printk("\r\n set Rd");
					TCPC->CCA_CTRL.BITS.CC_BLOCK_DIS = 0;
					TCPC->CCA_CTRL.BITS.CC_DB_RD_DIS = 1;
					hal_tcpc_set_cc(TYPEC_PORT_A,TYPEC_CC_RD);
					TCPC->CCB_CTRL.BITS.CC_BLOCK_DIS = 0;
					TCPC->CCB_CTRL.BITS.CC_DB_RD_DIS = 1;
					hal_tcpc_set_cc(TYPEC_PORT_B,TYPEC_CC_RD);

					delay_1us(1000);
					enum tc_cc_status cc1,cc2;
					hal_tcpc_get_cc(TYPEC_PORT_A,&cc1,&cc2);
					sleep_printk("\r\n 0cc:[%d %d 0x%x]\n",cc1,cc2,TCPC->CCA_STAT.WORD);
					extern bool tc_snk_is_connected(enum tc_cc_status cc1,enum tc_cc_status cc2);
				    if (tc_snk_is_connected(cc1,cc2))
				    {
				    	gd->rd0_cnt++;
				    	sleep_printk("\r\n rd0_cnt = %d\n",gd->rd0_cnt);
				    	if(gd->rd0_cnt >= 10) break;
				    	else
						{
				    		TCPC->CCA_CTRL.BITS.CC_BLOCK_DIS = 1;
						}
				    }
				    else
				    {
				    	gd->rd0_cnt = 0;
				    	TCPC->CCA_CTRL.BITS.CC_BLOCK_DIS = 1;
				    }


					hal_tcpc_get_cc(TYPEC_PORT_B,&cc1,&cc2);
					//sleep_printk("\r\n 1cc:[%d %d]\n",cc1,cc2);
					sleep_printk("\r\n 1cc:[%d %d 0x%x]\n",cc1,cc2,TCPC->CCB_STAT.WORD);
				    if (tc_snk_is_connected(cc1,cc2))
				    {
				    	gd->rd1_cnt++;
				    	sleep_printk("\r\n rd1_cnt = %d\n",gd->rd1_cnt);
				    	if(gd->rd1_cnt >= 10) break;
				    	else
						{
				    		TCPC->CCB_CTRL.BITS.CC_BLOCK_DIS = 1;
						}
				    }
				    else
				    {
				    	gd->rd1_cnt = 0;
				    	TCPC->CCB_CTRL.BITS.CC_BLOCK_DIS = 1;
				    }
				    SLP_vSleepToSleep();
				}
				else
				{
					if(gd->tc0_lighting_mode)
					{
						extern bool tc_src_is_disconnected(struct tc_s * tc);
						enum tc_cc_status cc1,cc2;
						hal_tcpc_get_cc(0, &cc1,&cc2);

						sleep_printk("\r\n 0cc:[%d %d]\n",cc1,cc2);
						if(cc1 != TYPEC_CC_RD && cc2 != TYPEC_CC_RD)
						{
							gd->tc0_lighting_mode = 0;
							sleep_printk("\r\n lighting_mode exit");
							break;
						}

						if(!gd->tc1_lighting_mode)
						{
							if(hal_get_drp_toggle_result(1) == TYPEC_DRP_SNK_CONNECTED)
							{
								break;
							}
							else if(hal_get_drp_toggle_result(1) == TYPEC_DRP_SRC_CONNECTED)
							{
								break;
							}
						}
					}

					if(gd->tc1_lighting_mode)
					{
						extern bool tc_src_is_disconnected(struct tc_s * tc);
						enum tc_cc_status cc1,cc2;
						hal_tcpc_get_cc(1, &cc1,&cc2);

						sleep_printk("\r\n [%d]cc:[%d %d]\n",1,cc1,cc2);
						if(cc1 != TYPEC_CC_RD && cc2 != TYPEC_CC_RD)
						{
							gd->tc1_lighting_mode = 0;
							sleep_printk("\r\n lighting_mode exit");
							break;
						}

						if(!gd->tc0_lighting_mode)
						{
							if(hal_get_drp_toggle_result(0) == TYPEC_DRP_SNK_CONNECTED)
							{
								break;
							}
							else if(hal_get_drp_toggle_result(0) == TYPEC_DRP_SRC_CONNECTED)
							{
								break;
							}
						}
					}

					if(SLP_u8SleepModeQDetect())// need to normal
					{
						SYS->PWR_CTRL.WORD &= !SYS_PWR_CTRL_SLEEP_MODE_EN_Msk;
						hal_wdt_init_to_reset();// re_enable WD, hope to reset the MCU
						gd->reset_magicode = 55;
						TMR0->SPL_CTRL.WORD &= !TMR_SPL_CTRL_WKUP_EN_Msk;
						do
						{
							sleep_printk("\r\n wait to reset");
							reset_cnt++;
							delay_1ms(1000);
						}while (reset_cnt<4);
					}
					else
					{
					//	sleep_printk("\r\n sleep again");
						SLP_vSleepToSleep();
					//	SLP_vNormalToSleep();
					}
				}
				break;
			case RST_SRC_PROTOCOL:
				sleep_printk("\r\n sleep check- protocol");
				SYS->PWR_CTRL.WORD &= !SYS_PWR_CTRL_SLEEP_MODE_EN_Msk;
				if(gd->bat_dead_flag)
				{
					SLP_vSleepToSleep();
				}
				break;
			case RST_SRC_GPIO:
				sleep_printk("\r\n sleep check- GPIO");
				SYS->PWR_CTRL.WORD &= !SYS_PWR_CTRL_SLEEP_MODE_EN_Msk;
				if(gd->bat_dead_flag)
				{
					SLP_vSleepToSleep();
				}
				key_ui_cnt = 20;
				break;
			case RST_SRC_WARMUP_DONE:
			default:
				break;

		}
}

