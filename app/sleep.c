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
#include"sw7201.h"
#include "fmc.h"
#include"qdt.h"
#include"led.h"
#include"nu103x.h"
#include"eadc.h"
#include"fm1210.h"
#include"ask.h"
#include"bpwm.h"
//uint8_t reset_magic_code;

void SLP_vNormalToSleep(void)
{
	printk("\r\n enter sleep");
	//hal_wdt_feed();
	//WDT->CTRL.WORD &= !WDT_CTRL_MODU_EN_Msk;
	//hal_wdt_init();
	WDT->CTRL.BITS.MODU_EN =0;
	fm1210_sleep();

	gd->reset_magicode = 0;// magic code,important for sleep Q wake-up.
	/* shut down module to lower power--- start*/
	//hal_wdt_stop();
	SYS->CLK_CTRL.BITS.XTAL_EN = 0;// disable XTAL
	SYS->PRO_CTRL.BITS.PVD_EN = 0;// disable PVD
	// disable PLL
	//how to shut HCLK?
	//
	FMC->FMC_CMD_CTRL.WORD = _FMC_CMD_ALL_CTRL_DISABLE;// off flash
	//How to shut sram?
	ECAP1->GEN_CTRL.WORD &= ~ECAP_GEN_CTRL_CAP_EN_Msk;//ecap
	ECAP2->GEN_CTRL.WORD &= ~ECAP_GEN_CTRL_CAP_EN_Msk;//ecap
	ECAP4->GEN_CTRL.WORD &= ~ECAP_GEN_CTRL_CAP_EN_Msk;//ecap
	DPDM_QC_SINK->BC1P2_INTMSK_CTRL.BITS.DPDM_EN = 0;
	DPDM_QC_SINK->BC1P2_INTMSK_CTRL.BITS.BC1P2_EN = 0;
    // enable all 6801 INT
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_INT_MASK,0x00);
	// 6801 sleep function and firmware work-round start
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_BUBO_CTRL,0x09);//09
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VBUS_SET_H,0x1C);//0C
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VBUS_SET_L,0x50);//0D
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_BUBO_CTRL,0x0D);//09
	delay_1ms(500);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0x65);//
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0x37);//
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0x2D);//
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0xF9);//
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x51,0x29);//
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x51,0xCB);//
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x51,0xE2);//
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x51,0x6A);//
	delay_1ms(500);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x63,0x01);//
	delay_1ms(500);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_BUBO_CTRL,0x01);//09
	delay_1ms(500);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_MISC_CTRL,0x41);//10
	/* 6801 sleep function and firmware work-round end*/
	_SET_ALL_PINS_IN_PUT();
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
	fml_nu103x_config(_1030_CFG_ALL_RST);
	fml_nu103x_config(_1030_CFG_LPM_EN_);
	//tcpc wake up start.
	SYS->PWR_CTRL.WORD &= !SYS_PWR_CTRL_TCPC_WKUP_DIS_Pos;
	//CCA
	  //(Enable CC, Disable RDB, Enter low power mode)
	TCPC->CCA_CTRL.BITS.CC_DB_RD_DIS = 1; // enable cc block
	TCPC->CCA_CTRL.BITS.CC_LPMODE_EN = 1; // enable cc block
	TCPC->CCA_CTRL.BITS.CC_BLOCK_DIS = 0; // enable cc block
	  //(Enable DRP)
	TCPC->CCA_ROLE.BITS.DRP_MODE = 1;
	TCPC->CCA_ROLE.BITS.CC1_ROLE = 1;
	TCPC->CCA_ROLE.BITS.CC2_ROLE = 1;
	TCPC->CCA_CMD_.BITS.CMD_TYPE = 0x99;//(Start DRP)
	//CCB
	  //(Enable CC, Disable RDB, Enter low power mode)
	TCPC->CCB_CTRL.BITS.CC_DB_RD_DIS = 1; // enable cc block
	TCPC->CCB_CTRL.BITS.CC_LPMODE_EN = 1; // enable cc block
	TCPC->CCB_CTRL.BITS.CC_BLOCK_DIS = 0; // enable cc block
	  //(Enable DRP)
	TCPC->CCB_ROLE.BITS.DRP_MODE = 1;
	TCPC->CCB_ROLE.BITS.CC1_ROLE = 1;
	TCPC->CCB_ROLE.BITS.CC2_ROLE = 1;
	TCPC->CCB_CMD_.BITS.CMD_TYPE = 0x99;//(Start DRP)

/*
	GPB->I_EN.BITS.PIN7 = 1;
	GPB->O_EN.BITS.PIN7 = 0;
	GPB->DOUT.BITS.PIN7 = 0;
	GPB->ODEN.BITS.PIN7 = 0;
	GPB->PUEN.BITS.PIN7 = 0;
	GPB->PDEN.BITS.PIN7 = 0;
	GPB->MODE.BITS.PIN7 = 0; //00:PB7 01:UART1_TXD 10:RESERVED 11:RESERVED
*/
//	GPB->MODE.BITS.PIN7 = 0; //00:PB7 01:UART1_TXD 10:RESERVED 11:RESERVED
	// timer wake up start.
	TMR1->GEN_CTRL.WORD &= !TMR_GEN_CTRL_CNT_EN_Msk;
	TMR2->GEN_CTRL.WORD &= !TMR_GEN_CTRL_CNT_EN_Msk;
	TMR3->GEN_CTRL.WORD &= !TMR_GEN_CTRL_CNT_EN_Msk;
	TMR0->LOAD_CNT.WORD = 16000 - 1; //0.0625 ms per cnt
	TMR0->SPL_CTRL.WORD = (_TMR_CLK_SRC_LIRC << TMR_SPL_CTRL_CLK_SRC_Pos) | TMR_SPL_CTRL_WKUP_EN_Msk; //LIRC: 64K
	TMR0->GEN_CTRL.WORD = (2 << TMR_GEN_CTRL_CLK_PSC_Pos) | (_TMR_OP_MODE_PERIODIC << TMR_GEN_CTRL_OP_MODE_Pos) | TMR_GEN_CTRL_INT_EN_Msk | TMR_GEN_CTRL_CNT_EN_Msk; //16K
	printk("\r\n enter fully sleep");
	//WDT->CTRL.WORD = (WDT_TIMEOUT << WDT_CTRL_WDT_CNT_Pos) | WDT_CTRL_RST_EN_Msk | WDT_CTRL_LOAD_EN_Msk | WDT_CTRL_MODU_EN_Msk;
	//WDT->CTRL.WORD = (WDT_TIMEOUT << WDT_CTRL_WDT_CNT_Pos) | WDT_CTRL_RST_EN_Msk | WDT_CTRL_LOAD_EN_Msk | WDT_CTRL_MODU_EN_Msk;
//	WDT->CTRL.BITS.RST_EN=0;
//	WDT->CTRL.BITS.INT_EN=0;
//	WDT->CTRL.BITS.MODU_EN =0;
	SYS->PWR_CTRL.WORD |= SYS_PWR_CTRL_SLEEP_MODE_EN_Msk;// enable sleep

}
void SLP_vSleepToSleep(void)
{
	printk("\r\n sleep to sleep");
	fml_nu103x_config(_1030_CFG_ALL_RST);
	fml_nu103x_config(_1030_CFG_LPM_EN_);
	gd->reset_magicode = 0;// magic code,important for sleep Q wake-up.
	SYS->PWR_CTRL.WORD |= SYS_PWR_CTRL_SLEEP_MODE_EN_Msk;// enable sleep

}
void SLP_vSleepQToSleep(void)
{

}

void SLP_vSleepQToNormal(void)
{

}
uint8_t sleep_idle_qdt_back_to_normal(void)
{
	return (gd->tx_infos.q_fact + ap->q_factor_reco_value > ap->q_factor_base_value -15 && gd->tx_infos.q_fact < ap->q_factor_limH_value &&
			gd->tx_infos.f_self + ap->fs_reco_value > ap->fs_base_value && gd->tx_infos.f_self < ap->fs_limH_value);
}
uint8_t SLP_u8SleepModeQDetect(void)
{
	fml_nu103x_config(_1030_CFG_LPM_DIS);
	fml_nu103x_por_rst();
	ctx_switch(4);
	//delay_1ms(5);
	fml_qdt_detect((uint32_t *)&gd->tx_infos.q_fact, (uint32_t *)&gd->tx_infos.f_self);
	printk("\r\n sleep: [%d] [q:%d,%d,%d] [f:%d,%d,%d] ",gd->ptx_idle_phase_status,
			gd->tx_infos.q_fact, ap->q_factor_base_value, gd->tx_infos.q_fact - ap->q_factor_base_value,
			gd->tx_infos.f_self, ap->fs_base_value, gd->tx_infos.f_self - ap->fs_base_value);
	uint8_t u8NeedToNormal = 0;

	switch (gd->ptx_idle_phase_status)
	{
		case WPC_IDLE_STAT_STANDBY:
			if ((gd->tx_infos.q_fact  + ap->q_factor_obj_value  < ap->q_factor_base_value -15) ||
				(gd->tx_infos.f_self > ap->fs_limL_value && gd->tx_infos.f_self + ap->fs_obj_value < ap->fs_base_value))
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
	printk("\r\n Q wake-up? [%d]",u8NeedToNormal);
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

void RST_vCheck(void)
{
		printk("\r\n sleep check");
		gd->idle_to_sleep_cnt = 0;
		switch(SYS->OPR_STAT.BITS.RST_SRC)
		{
			case RST_SRC_1PTIMER:
				printk("\r\n sleep check- timer[%d]",gd->reset_magicode);
				if(gd->reset_magicode == 55)
				{
					gd->reset_magicode = 0;
					printk("\r\n reset go on");
				}
				else
				{
					if(SLP_u8SleepModeQDetect())// need to normal
					{
						SYS->PWR_CTRL.WORD &= !SYS_PWR_CTRL_SLEEP_MODE_EN_Msk;
						hal_wdt_init_to_reset();// re_enable WD, hope to reset the MCU
						gd->reset_magicode = 55;
						TMR0->SPL_CTRL.WORD &= !TMR_SPL_CTRL_WKUP_EN_Msk;
						do
						{
							printk("\r\n wait to reset");
							reset_cnt++;
							delay_1ms(1000);
						}while (reset_cnt<4);
					}
					else
					{
						printk("\r\n sleep again");
						SLP_vSleepToSleep();
					//	SLP_vNormalToSleep();
					}
				}
				break;
			case RST_SRC_PROTOCOL:
				printk("\r\n sleep check- protocol");
				SYS->PWR_CTRL.WORD &= !SYS_PWR_CTRL_SLEEP_MODE_EN_Msk;
				break;
			case RST_SRC_GPIO:
				printk("\r\n sleep check- GPIO");
				SYS->PWR_CTRL.WORD &= !SYS_PWR_CTRL_SLEEP_MODE_EN_Msk;
				//SLP_vSleepToSleep();
				break;
			case RST_SRC_WARMUP_DONE:
			default:
				break;

		}
}

