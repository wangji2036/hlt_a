#include "regdef.h"
#include "printk.h"
#include "g_data.h"
#include "nu103x.h"
#include "ddm.h"

void hal_ddm_dig_ping(void)
{
	DDM->GEN_CTRL.WORD = 0x03850055;
	DDM->MAG_CTRL.WORD = 0x00000304;
	DDM->PHA_CTRL.WORD = 0x00000304;
	DDM->I_Q_CTRL.WORD = 0x00000000;

//	DDM->GEN_CTRL.WORD = (ap->dig_ddm_ping_0003 << 24) | (ap->dig_ddm_ping_0002 << 16) | (ap->dig_ddm_ping_0001 << 8) | (ap->dig_ddm_ping_0000 << 0);
//	DDM->MAG_CTRL.WORD = (ap->dig_ddm_ping_0005 << 8) | (ap->dig_ddm_ping_0004 << 0);
//	DDM->PHA_CTRL.WORD = (ap->dig_ddm_ping_0009 << 8) | (ap->dig_ddm_ping_0008 << 0);
//
//	printk("\r\n -------------------------dig_ddm_ping: %X %X %X", DDM->GEN_CTRL.WORD, DDM->MAG_CTRL.WORD, DDM->PHA_CTRL.WORD);

//	if (ap->dig_ddm_ping_K == 0)
//	{
//		fml_nu103x_config(_1030_CFG_DMO2_VCAP_RATIO_K2);
//		printk(" K_Ratio-> K2");
//	}
//	else if (ap->dig_ddm_ping_K == 1)
//	{
//		fml_nu103x_config(_1030_CFG_DMO2_VCAP_RATIO_K3);
//		printk(" K_Ratio-> K3");
//	}
//	else if (ap->dig_ddm_ping_K == 2)
//	{
//		fml_nu103x_config(_1030_CFG_DMO2_VCAP_RATIO_K1);
//		printk(" K_Ratio-> K1");
//	}
}

void hal_ddm_dig_xfer(void)
{
//	DDM->GEN_CTRL.WORD = 0x03850099;
//	DDM->MAG_CTRL.WORD = 0x00000302;
//	DDM->PHA_CTRL.WORD = 0x00000202;
//	DDM->I_Q_CTRL.WORD = 0x00000000;

//	DDM->GEN_CTRL.WORD = (ap->dig_ddm_xfer_0003 << 24) | (ap->dig_ddm_xfer_0002 << 16) | (ap->dig_ddm_xfer_0001 << 8) | (ap->dig_ddm_xfer_0000 << 0);
//	DDM->MAG_CTRL.WORD = (ap->dig_ddm_xfer_0005 << 8) | (ap->dig_ddm_xfer_0004 << 0);
//	DDM->PHA_CTRL.WORD = (ap->dig_ddm_xfer_0009 << 8) | (ap->dig_ddm_xfer_0008 << 0);
//
//	printk("\r\n -------------------------dig_ddm_xfer: %X %X %X", DDM->GEN_CTRL.WORD, DDM->MAG_CTRL.WORD, DDM->PHA_CTRL.WORD);
//
//	if (ap->dig_ddm_xfer_K == 0)
//	{
//		fml_nu103x_config(_1030_CFG_DMO2_VCAP_RATIO_K2);
//		printk(" K_Ratio-> K2");
//	}
//	else if (ap->dig_ddm_xfer_K == 1)
//	{
//		fml_nu103x_config(_1030_CFG_DMO2_VCAP_RATIO_K3);
//		printk(" K_Ratio-> K3");
//	}
//	else if (ap->dig_ddm_xfer_K == 2)
//	{
//		fml_nu103x_config(_1030_CFG_DMO2_VCAP_RATIO_K1);
//		printk(" K_Ratio-> K1");
//	}
}
