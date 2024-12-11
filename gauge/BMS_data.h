/*
 * BMS_data.h
 *
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * Code generation for model "BMS_FixPoint".
 *
 * Model version              : 7.88
 * Simulink Coder version : 24.2 (R2024b) 21-Jun-2024
 *
 */

#ifndef BMS_data_h_
#define BMS_data_h_
#include "rtwtypes.h"
#include "multiword_types.h"
#include "zero_crossing_types.h"
#include "BMS_FixPoint_types.h"

/* Exported data declaration */

/* Const memory section */
/* Declaration for custom storage class: Const */
extern const int32_T Cfg_DefultInitR_mOhm;/* Referenced by: '<S20>/Constant2' */

/* Volatile memory section */
/* Declaration for custom storage class: Volatile */
extern volatile int32_T Add2_DWORK1_s; /* '<S39>/Add2' */
extern volatile int32_T Delay_DSTATE_s;/* '<S26>/Delay' */
extern volatile int32_T DiscreteTimeIntegrator_DSTATE_s;/* '<S25>/Discrete-Time Integrator' */
extern volatile int32_T DiscreteTimeIntegrator_PREV_U_s;/* '<S25>/Discrete-Time Integrator' */
extern volatile int8_T DiscreteTimeIntegrator_PrevRe_s;/* '<S25>/Discrete-Time Integrator' */
extern volatile uint8_T DiscreteTimeIntegrator_SYSTEM_s;/* '<S25>/Discrete-Time Integrator' */
extern volatile uint32_T SOC_ELAPS_T_s;/* '<S1>/SOC' */
extern volatile uint32_T SOC_PREV_T_s; /* '<S1>/SOC' */
extern volatile boolean_T SOC_RESET_ELAPS_T_s;/* '<S1>/SOC' */
extern volatile boolean_T SOC_VoltGradientMatch_flg_s;/* '<S18>/Relational Operator1' */
extern volatile int32_T SOC_udInitValue_mpct_DSTATE_s;/* '<S6>/SOC_udInitValue_mpct' */
extern volatile uint8_T SOHC_Trig_ZCE_s;
extern volatile uint16_T SOH_capacity_mAh_s;/* '<S40>/Switch' */
extern volatile int32_T Switch_s;      /* '<S5>/Switch' */
extern volatile uint8_T TimeSum_Reset_ZCE_s;
extern volatile int32_T UnitDelay1_DSTATE_s;/* '<S9>/Unit Delay1' */
extern volatile int32_T UnitDelay2_DSTATE_s;/* '<S9>/Unit Delay2' */
extern volatile int32_T UnitDelay_DSTATE_s;/* '<S9>/Unit Delay' */
extern volatile int32_T d_Delay_DSTATE_s;/* '<S27>/Delay' */
extern volatile uint32_T h_m_bpIndex_s[2];/* '<S33>/OCV_DSG' */
extern volatile uint32_T hj_m_bpIndex_s[2];/* '<S19>/OCV_DSG' */
extern volatile uint32_T hjj_m_bpIndex_s;/* '<S19>/adaption in SOC range' */
extern volatile uint32_T hjjp_m_bpIndex_s;/* '<S22>/SOC_Slope' */
extern volatile uint32_T hjjpa_m_bpIndex_s;/* '<S19>/1-D Lookup Table1' */
extern volatile uint32_T hjjpar_m_bpIndex_s[2];/* '<S23>/R0_Discharge' */
extern volatile uint32_T hjjparn_m_bpIndex_s[2];/* '<S23>/DCIR_Discharge' */
extern volatile uint32_T hjjparne_m_bpIndex_s[2];/* '<S23>/OCV_Discharge' */
extern volatile uint32_T hjjparneo_m_bpIndex_s[2];/* '<S12>/OCV_DSG' */
extern volatile boolean_T icLoad_s;    /* '<S36>/SOCPack_UdDisplaySOC_pct' */
extern volatile boolean_T k34_icLoad_s;/* '<S27>/Delay' */
extern volatile boolean_T k34f_icLoad_s;/* '<S20>/UdVirtualOcv' */
extern volatile boolean_T k3_icLoad_s; /* '<S26>/Delay' */
extern volatile boolean_T k_icLoad_s;  /* '<S32>/CellVoltsDelay' */
extern volatile uint32_T m_bpIndex_s[2];/* '<S33>/DCIR_Discharge' */
extern volatile boolean_T o_UnitDelay_DSTATE_s;/* '<S6>/Unit Delay' */
extern volatile uint8_T voltage_time_judge_Trig_ZCE_s;

/* Data with Exported storage */
extern int32_T SOCPack_UsableSOC_pct_s;/* '<Root>/SOCPack_UsableSOC_pct' */

#endif                                 /* BMS_data_h_ */
