#ifndef PFOD_H_
#define PFOD_H_
#define pcoil_factor		160//156->NU226  //360K-156, 128K-63, ACR
#define pcircuit_factor		 40//Rcircuit    //

#define PFO_10W_THD 		265
#define PFO_10W_RECO 		251//THD*0.95
#define PFO_15W_THD 		385
#define PFO_15W_RECO 		365//THD*0.95

#define PFO_THD_APL_MPP		550//750
#define PFO_RECO_APL_MPP 	365//500

#define FOD_MAX_CNT			25
enum prx_type_t {
	EPRX_TYPE_UNKNOWN    = 0x00,
	EPRX_TYPE_SAMSUNG    = 0x01,
	EPRX_TYPE_APPLE_STD  = 0x02,
	EPRX_TYPE_APPLE_MAG  = 0x03,
	EPRX_TYPE_XIAOMI_BPP = 0x04,
	EPRX_TYPE_XIAOMI_EPP = 0x05,
	EPRX_TYPE_NUVOLTA    = 0x06,
	EPRX_TYPE_HUAWEI     = 0x07,
	EPRX_TYPE_MEIZU      = 0x08,
	EPRX_TYPE_GOOGLE     = 0x09,
	ERX_TYPE_APPLE_MPP,
	ERX_TYPE_NVT_MPP,
	ERX_TYPE_NOK9_MPP_SGS,
	ERX_TYPE_NOK9_MPP_HONK,
	ERX_TYPE_NOK9_MPP_MICRO,
	ERX_TYPE_GRL_MPP,
	ERX_TYPE_YBZ_BPP_FIXTURE,
	ERX_TYPE_YBZ_EPP_FIXTURE,
	ERX_TYPE_YBZ_MPP_FIXTURE,
	ERX_TYPE_YBZ_PPDE_FIXTURE,

	EPRX_TYPE_NOK9_BPP_FOD_TPR_5,
	EPRX_TYPE_NOK9_EPP_FOD_TPR_7,
	EPRX_TYPE_NOK9_EPP_FOD_TPR_1F,
	EPRX_TYPE_NOK9_EPP_FOD_TPR_MP3,
	EPRX_TYPE_NOK9_EPP_FOD_TPR_MP4,
	EPRX_TYPE_NOK9_EPP_FOD_TPR_MP1B,

	EPRX_TYPE_NOKIA_MPP_21201,
	EPRX_TYPE_BPP_12702,
	EPRX_TYPE_BPP_TWS_12168,
	EPRX_TYPE_BPP_TWS_12175,
};

enum pfod_evnt_t {
	EXFER_FOD_EVENT_NONE                 = 0x00,
	EXFER_FOD_EVENT_NOK9_BPP_FOD_DISABLE = 0x01,
	EXFER_FOD_EVENT_NOK9_EPP_FOD_DISABLE = 0x02,
	EXFER_FOD_EVENT_NOK9_BPP_FOD_TPR_XXX = 0x03,
	EXFER_FOD_EVENT_NOK9_EPP_FOD_TPR_XX7 = 0x04,
	EXFER_FOD_EVENT_NOK9_EPP_FOD_TPR_MP3 = 0x05,
};

extern uint8_t fod_count_filter;

extern uint8_t fod_enable;
extern uint8_t pfo_en_reco;//fixture and bpp set 0

extern uint8_t fod_count_filter;
extern uint8_t fod_count;
extern uint8_t mpla_count;

extern uint16_t pfo_thd;
extern uint16_t pfo_thd_reco;

extern uint16_t p_rect_max;

extern uint8_t pfo_values_index;
extern int32_t pfo_values[5];
extern int32_t pfo;
extern int32_t pfo_avg;

void pfod_init(void);
uint8_t pfod_common(void);

void pfod_mpla_init(void);
uint8_t pfod_mpla(void);

void pfod_dploss_init(void);
void pfod_dploss_cal(void);
uint8_t pfod_dploss_cal_cmt(uint16_t *alpha, uint16_t *beta);
uint8_t pfod_dploss(void);
void pfod_log_print(void);
uint8_t pfod_action(void);
uint32_t ploss_calc(uint8_t mode);

#endif /* PFOD_H_ */
