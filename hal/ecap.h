#ifndef ECAP_H_
#define ECAP_H_

/**
  * @brief  Initializes the ECAP peripheral.
  * @param  ecap: Pointer to the ECAP instance.
  * @param  mode: Operating mode of the ECAP.
  * @retval void
  */
void hal_ecap_init(TS_ECAP *ecap, enum ECAP_FUNC_MODE mode);

/**
  * @brief  Opens the ECAP capture functionality.
  * @param  ecap: Pointer to the ECAP instance.
  * @retval void
  */
void hal_ecap_open(TS_ECAP *ecap);

/**
  * @brief  Closes the ECAP capture functionality.
  * @param  ecap: Pointer to the ECAP instance.
  * @retval void
  */
void hal_ecap_close(TS_ECAP *ecap);

/**
  * @brief  Registers a callback function for ECAP interrupts.
  * @param  func: Pointer to the callback function.
  * @retval void
  */
void hal_ecap_reg_int_cb(void (*func)(uint8_t, uint16_t));

//	ECAP2->OVER_CNT.WORD = (1125 * 2 << ECAP_OVER_CNT_OVERFLOW_CNT_Pos);
//	ECAP2->DDM_CTRL.WORD = (_ECAP_DDMCAP_TIMER_CLKDIV_32 << ECAP_DDM_CTRL_CLOCK_DIV_SEL_Pos) | (_ECAP_EDGE_TYPE_RISEING_FALLING << ECAP_DDM_CTRL_EDGE_TYPE_SEL_Pos);
//	ECAP2->GEN_CTRL.WORD = (_ECAP_FUNC_MODE_DDM << ECAP_GEN_CTRL_FUNC_WORKING_MODE_Pos) | (_ECAP_INPUT_CHAN_INR_DDM_OUT << ECAP_GEN_CTRL_INPUT_CHANNEL_SEL_Pos) |
//			          (_ECAP_DEGILTC_TIME_80us << ECAP_GEN_CTRL_DEGLITEC_TIME_SEL_Pos) | ECAP_GEN_CTRL_OVERFLOW_INT_EN_Msk | ECAP_GEN_CTRL_EDGE_DET_INT_EN_Msk;
//	ECAP2->STS_FLAG.WORD = ECAP_STS_FLAG_EDGE_DET_FLAG_Msk | ECAP_STS_FLAG_OVERFLOW_FLAG_Msk;
//
//	ECAP4->OVER_CNT.WORD = (1125 * 2 << ECAP_OVER_CNT_OVERFLOW_CNT_Pos);
//	ECAP4->DDM_CTRL.WORD = (_ECAP_DDMCAP_TIMER_CLKDIV_32 << ECAP_DDM_CTRL_CLOCK_DIV_SEL_Pos) | (_ECAP_EDGE_TYPE_RISEING_FALLING << ECAP_DDM_CTRL_EDGE_TYPE_SEL_Pos);
//	ECAP4->GEN_CTRL.WORD = (_ECAP_FUNC_MODE_DDM << ECAP_GEN_CTRL_FUNC_WORKING_MODE_Pos) | (_ECAP_INPUT_CHAN_INR_DDM_OUT << ECAP_GEN_CTRL_INPUT_CHANNEL_SEL_Pos) |
//			          (_ECAP_DEGILTC_TIME_80us << ECAP_GEN_CTRL_DEGLITEC_TIME_SEL_Pos) | ECAP_GEN_CTRL_OVERFLOW_INT_EN_Msk | ECAP_GEN_CTRL_EDGE_DET_INT_EN_Msk;
//	ECAP4->STS_FLAG.WORD = ECAP_STS_FLAG_EDGE_DET_FLAG_Msk | ECAP_STS_FLAG_OVERFLOW_FLAG_Msk;

//	hal_ecap_open(ECAP2);
//	hal_ecap_open(ECAP4);

void hal_ecap_dig_ddm_init(void);

#endif /* ECAP_H_ */
