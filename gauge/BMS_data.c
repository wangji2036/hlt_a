/*
 * BMS_data.c
 *
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * Code generation for model "BMS_FixPoint".
 *
 * Model version              : 8.0
 * Simulink Coder version : 24.2 (R2024b) 21-Jun-2024
 *
 */
#include "regdef.h"
#include "BMS_data.h"
#include "rtwtypes.h"
#include "multiword_types.h"
#include "zero_crossing_types.h"
#include "BMS_FixPoint_types.h"

/* Exported data definition */

/* Const memory section */
/* Definition for custom storage class: Const */
const int32_T Cfg_DefultInitR_mOhm = 240;/* Referenced by: '<S21>/Constant2' */

/* Volatile memory section */
/* Definition for custom storage class: Volatile */
volatile int32_T Add2_DWORK1_s;        /* '<S40>/Add2' */
volatile int32_T Delay_DSTATE_s;       /* '<S27>/Delay' */
volatile int32_T DiscreteTimeIntegrator_DSTATE_s;/* '<S26>/Discrete-Time Integrator' */
volatile int32_T DiscreteTimeIntegrator_PREV_U_s;/* '<S26>/Discrete-Time Integrator' */
volatile int8_T DiscreteTimeIntegrator_PrevRe_s;/* '<S26>/Discrete-Time Integrator' */
volatile uint8_T DiscreteTimeIntegrator_SYSTEM_s;/* '<S26>/Discrete-Time Integrator' */
volatile uint32_T SOC_ELAPS_T_s;       /* '<S1>/SOC' */
volatile uint32_T SOC_PREV_T_s;        /* '<S1>/SOC' */
volatile boolean_T SOC_RESET_ELAPS_T_s;/* '<S1>/SOC' */
volatile boolean_T SOC_VoltGradientMatch_flg_s;/* '<S19>/Relational Operator1' */
volatile int32_T SOC_udInitValue_mpct_DSTATE_s;/* '<S6>/SOC_udInitValue_mpct' */
volatile uint8_T SOHC_Trig_ZCE_s;
volatile uint16_T SOH_capacity_mAh_s;  /* '<S41>/Switch' */
volatile int32_T Switch_s;             /* '<S5>/Switch' */
volatile uint8_T TimeSum_Reset_ZCE_s;
volatile int32_T UnitDelay1_DSTATE_s;  /* '<S9>/Unit Delay1' */
volatile int32_T UnitDelay2_DSTATE_s;  /* '<S9>/Unit Delay2' */
volatile int32_T UnitDelay_DSTATE_s;   /* '<S9>/Unit Delay' */
volatile int32_T d_Delay_DSTATE_s;     /* '<S28>/Delay' */
volatile uint32_T h_m_bpIndex_s[2];    /* '<S34>/OCV_DSG' */
volatile uint32_T hj_m_bpIndex_s[2];   /* '<S20>/OCV_DSG' */
volatile uint32_T hjj_m_bpIndex_s;     /* '<S20>/adaption in SOC range' */
volatile uint32_T hjjp_m_bpIndex_s;    /* '<S23>/SOC_Slope' */
volatile uint32_T hjjpa_m_bpIndex_s;   /* '<S20>/1-D Lookup Table1' */
volatile uint32_T hjjpar_m_bpIndex_s[2];/* '<S24>/R0_Discharge' */
volatile uint32_T hjjparn_m_bpIndex_s[2];/* '<S24>/DCIR_Discharge' */
volatile uint32_T hjjparne_m_bpIndex_s[2];/* '<S24>/OCV_Discharge' */
volatile uint32_T hjjparneo_m_bpIndex_s[2];/* '<S13>/OCV_DSG' */
volatile boolean_T icLoad_s;           /* '<S37>/SOCPack_UdDisplaySOC_pct' */
volatile boolean_T k34_icLoad_s;       /* '<S28>/Delay' */
volatile boolean_T k34f_icLoad_s;      /* '<S21>/UdVirtualOcv' */
volatile boolean_T k3_icLoad_s;        /* '<S27>/Delay' */
volatile boolean_T k_icLoad_s;         /* '<S33>/CellVoltsDelay' */
volatile uint32_T m_bpIndex_s[2];      /* '<S34>/DCIR_Discharge' */
volatile boolean_T o_UnitDelay_DSTATE_s;/* '<S6>/Unit Delay' */
volatile uint8_T voltage_time_judge_Trig_ZCE_s;

/* Data with Exported storage */
int32_T SOCPack_UsableSOC_pct_s;       /* '<Root>/SOCPack_UsableSOC_pct' */
