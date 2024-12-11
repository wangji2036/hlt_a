#include "regdef.h"
#include "g_data.h"

#define AP_CFG_ROM_ADDR_BASE    (0x00001600)
#define AP_CFG_RAM_ADDR_BASE    (0x20000000)
#define G_DATA_RAM_ADDR_BASE    (0x20000200)

volatile struct ap_t *ap = (struct ap_t *)(AP_CFG_RAM_ADDR_BASE);
volatile struct gd_t *gd = (struct gd_t *)(G_DATA_RAM_ADDR_BASE);

void ap_data_init(void)
{
	uint32_t i;

	for (i=0; i<256; i++)
	{
		__write_08bits(AP_CFG_RAM_ADDR_BASE + i, __read_08bits(AP_CFG_ROM_ADDR_BASE + i));
	}

	ap->tntc_otp_dis = 1;//0
	ap->tntc_otp_thd = 85;//80
	ap->tntc_otp_hys = 30;

	ap->tntc_utp_dis = 0;
	ap->tntc_utp_thd = -5;
	ap->tntc_utp_hys = 5;

	ap->tdie_otp_dis = 0;
	ap->tdie_otp_thd = 100;
	ap->tdie_otp_hys = 30;

	ap->tdie_utp_dis = 0;
	ap->tdie_utp_thd = -30;
	ap->tdie_utp_hys = 30;

	ap->isns_ocp_dis = 0;
	ap->isns_ocp_thd = 2500;
	ap->isns_ocp_hys = 1000;

	ap->vbus_ovp_dis = 0;
	ap->vbus_ovp_thd = 16000;//10500
	ap->vbus_ovp_hys = 1000;

	ap->vbus_uvp_dis = 0;
	ap->vbus_uvp_thd = 8000;
	ap->vbus_uvp_hys = 500;

	ap->vbus_dpl_dis = 0;
	ap->vbus_dpl_thd = 4500;
	ap->vbus_dpl_hys = 200;

	ap->vpwr_ovp_dis = 0;
	ap->vpwr_ovp_thd = 20500;
	ap->vpwr_ovp_hys = 1000;

	ap->pout_opp_dis = 0;
	ap->pout_opp_thd = 35000;//25000
	ap->pout_opp_hys = 5000;


	ap->ptmc = 0x005c;
	ap->mpp_dither_en = 0;

	ap->pin_max_cnt = 10;
	ap->pin_fod_cnt = 0xff;
	ap->pin_fod_dis = 0;

	ap->rpp_fod_cnt = 5;
	ap->rpp_fod_dis = 1;

	ap->dig_ping_volt_5v = 5000;
	ap->dig_ping_perd_5v = 144000000 / 127772;
	ap->dig_ping_duty_5v = 500;
	ap->dig_ping_phas_5v = 0;

	ap->dig_ping_volt_6v = 6000;
	ap->dig_ping_perd_6v = 144000000 / 127772;
	ap->dig_ping_duty_6v = 400;
	ap->dig_ping_phas_6v = 0;

	ap->dig_ping_volt_9v = 11000;
	ap->dig_ping_perd_9v = 1127;
	ap->dig_ping_duty_9v = 250;
	ap->dig_ping_phas_9v = 0;

	ap->dig_ping_volt_12v = 12000;
	ap->dig_ping_perd_12v = 144000000 / 127772;
	ap->dig_ping_duty_12v = 200;
	ap->dig_ping_phas_12v = 0;

	ap->q_factor_base_value = 240; // 390;
	ap->q_factor_reco_value =  20;
	ap->q_factor_limH_value = 500;
	ap->q_factor_limL_value =  	0;
	ap->fs_base_value = 800; // 2190;
	ap->fs_reco_value = 20;
	ap->fs_limH_value = 3000;
	ap->fs_limL_value = 10;

	ap->q_factor_obj_value = 30;//50
	ap->q_factor_stable_value = 30;
	ap->fs_obj_value = 30;//50
	ap->fs_stable_value = 30;

	ap->t_next_ping = 200;
}

void gd_data_init(void)
{
	uint32_t addr;

	for (addr=G_DATA_RAM_ADDR_BASE; addr<G_DATA_RAM_ADDR_BASE + sizeof(struct gd_t); addr++)
	{
		__write_08bits(addr, 0);
	}

	gd->tx_infos.t_next_ping = ap->t_next_ping;
}
