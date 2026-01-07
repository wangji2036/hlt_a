/*
 * Code generation for system system '<S1>/SOC'
 *
 * Model                      : BMS_FixPoint
 * Model version              : 8.0
 * Simulink Coder version : 24.2 (R2024b) 21-Jun-2024
 *
 * Note that the functions contained in this file are part of a Simulink
 * model, and are not self-contained algorithms.
 */

#include "SOC.h"
#include "rtwtypes.h"
#include "BMS_FixPoint.h"
#include "BMS_FixPoint_private.h"
#include "BMS_data.h"
#include "zero_crossing_types.h"
#include "multiword_types.h"
#include "regdef.h"
#include "config.h"
#include "g_data.h"
/* Exported data definition */

/* Definition for custom storage class: Localizable */
static int32_T CellVoltsDelay;         /* '<S32>/CellVoltsDelay' */
static int16_T CellVolts_delay;        /* '<S18>/CellVolts_delay' */
static uint32_T CurrentRelaxTime;      /* '<S16>/CurrentRelaxTime' */
static boolean_T Init_OCVflag;         /* '<S7>/Init_OCVflag' */
static int32_T SOC_ModelCorrect_pct;   /* '<S21>/SOC_ModelCorrect_pct' */
static boolean_T VoltageTimeReset_flg; /* '<S13>/VoltageTimeReset_flg' */
static uint32_T Voltage_relax_time;    /* '<S13>/TimeSum' */

/* Definition for custom storage class: Reusable */
static int32_T VirtualOCV_Delay;       /* '<S20>/UdVirtualOcv' */

int32_T SigPr_PackCurr_mA;      /* '<Root>/Data Type Conversion1' */
uint16_T SigPr_CellVolts_mV;    /* '<Root>/Min' */
int16_T SigPr_CellTemps_C;      /* '<Root>/Min1' */
int32_T BMS_NvmSOC_mpct;        /* '<Root>/BMS_NvmSOC_mpct' */

 int16_T SigPr_CellTemps_C_s;    /* '<Root>/SigPr_CellTemps_C' */
 uint16_T SigPr_CellVolts_mV_s;  /* '<Root>/SigPr_CellVolts_mV' */
 int32_T SigPr_PackCurr_mA_s;    /* '<Root>/SigPr_PackCurr_mA' */

/* System initialize for atomic system: '<S3>/AhIntegralSOC' */
void AhIntegralSOC_Init(void)
{
  o_UnitDelay_DSTATE_s = true;
}

/* Output and update for atomic system: '<S3>/AhIntegralSOC' */
void AhIntegralSOC(void)
{
  /* local block i/o variables */
  int32_T SOC_AhIntegrator_mpct;
  int32_T SOC_udInitValue_mpct;
  int32_T SOC_InitValue_mpct;
  int32_T SOC_IntegralRealTime_mAs;
  int32_T tmp;
  boolean_T OR;
  SOC_IntegralRealTime_mAs = SigPr_PackCurr_mA * BMS_SampleTime_ms;
  if ((SOC_IntegralRealTime_mAs < 0) && (UnitDelay_DSTATE_s < MIN_int32_T
       - SOC_IntegralRealTime_mAs)) {
    UnitDelay_DSTATE_s = MIN_int32_T;
  } else if ((SOC_IntegralRealTime_mAs > 0) && (UnitDelay_DSTATE_s > MAX_int32_T
              - SOC_IntegralRealTime_mAs)) {
    UnitDelay_DSTATE_s = MAX_int32_T;
  } else {
    UnitDelay_DSTATE_s = SOC_IntegralRealTime_mAs + UnitDelay_DSTATE_s;
  }

  SOC_IntegralRealTime_mAs = div_nde_s32_floor(UnitDelay_DSTATE_s, 1000);
  if (SOC_IntegralRealTime_mAs < 0) {
    tmp = -SOC_IntegralRealTime_mAs;
  } else {
    tmp = SOC_IntegralRealTime_mAs;
  }

  OR = ((tmp >= 360) || SOC_OCVUpd_flg);
  if (SOC_OCVUpd_flg) {
    UnitDelay2_DSTATE_s = 0;
  } else if (OR) {
    UnitDelay2_DSTATE_s = UnitDelay1_DSTATE_s + SOC_IntegralRealTime_mAs;
  }

  SOC_AhIntegrator_mpct = div_nde_s32_floor(div_nde_s32_floor
    (UnitDelay2_DSTATE_s, 360) * 1000 * 10, SOH_capacity_mAh_s);
  if (o_UnitDelay_DSTATE_s) {
    SOC_udInitValue_mpct = BMS_NvmSOC_mpct;
  } else {
    SOC_udInitValue_mpct = SOC_udInitValue_mpct_DSTATE_s;
  }

  if (SOC_OCVUpd_flg) {
    SOC_InitValue_mpct = SOC_OCVSOC_mpct;
  } else {
    SOC_InitValue_mpct = SOC_udInitValue_mpct;
  }

  if ((SOC_InitValue_mpct < 0) && (SOC_AhIntegrator_mpct < MIN_int32_T
       - SOC_InitValue_mpct)) {
    SOC_AhIntegralSOC_mpct = MIN_int32_T;
  } else if ((SOC_InitValue_mpct > 0) && (SOC_AhIntegrator_mpct > MAX_int32_T
              - SOC_InitValue_mpct)) {
    SOC_AhIntegralSOC_mpct = MAX_int32_T;
  } else {
    SOC_AhIntegralSOC_mpct = SOC_InitValue_mpct + SOC_AhIntegrator_mpct;
  }

  UnitDelay1_DSTATE_s = UnitDelay2_DSTATE_s;
  if (OR) {
    UnitDelay_DSTATE_s = 0;
  }

  o_UnitDelay_DSTATE_s = false;
  SOC_udInitValue_mpct_DSTATE_s = SOC_InitValue_mpct;
}

/* Output and update for atomic system: '<S7>/Lookup_OCVSOC' */
void Lookup_OCVSOC(void)
{
  SOC_OCVSOC_mpct = look2_is16u16lu32n32ts_lElusMKZ(SigPr_CellTemps_C,
    SigPr_CellVolts_mV, P_TAxis_degC, P_OCVAxis_mV, P_OcvSOCDsg_mpct, (uint32_T *)
    &hjjparneo_m_bpIndex_s[0], ConstP_s.pooled10, 3U);
  if (SOC_OCVSOC_mpct > 100000) {
    SOC_OCVSOC_mpct = 100000;
  } else if (SOC_OCVSOC_mpct < 0) {
    SOC_OCVSOC_mpct = 0;
  }
}

/* System initialize for atomic system: '<S3>/OCVSOC' */
void OCVSOC_Init(void)
{
  Init_OCVflag = true;
  CellVolts_delay = 3000;
}

/* Output and update for atomic system: '<S3>/OCVSOC' */
void OCVSOC(void)
{
  /* local block i/o variables */
  uint32_T Debounce_Time_s;
  int32_T tmp;
  uint32_T SOC_DeltaTime_ms;
  int16_T u;
  if (SigPr_CellTemps_C > P_LowTemp_degC) {
    if (SigPr_CellTemps_C > P_NormalTemp_degC) {
      Debounce_Time_s = P_RelaxDurationNormalTemp_s;
    } else {
      Debounce_Time_s = P_RelaxDurationLowTemp_s;
    }
  } else {
    Debounce_Time_s = P_RelaxDurationExtremeLowTemp_s;
  }

  if (SigPr_PackCurr_mA < 0) {
    tmp = -SigPr_PackCurr_mA;
  } else {
    tmp = SigPr_PackCurr_mA;
  }

  if (tmp <= (int32_T)P_CurrentThresRelaxJudge_mA) {
    CurrentRelaxTime = CurrentRelaxTime + BMS_SampleTime_ms;
  } else {
    CurrentRelaxTime = 0U;
  }

  if (BMS_SampleTime_ms >= 60000) {
    SOC_DeltaTime_ms = (uint32_T)BMS_SampleTime_ms << 1;
  } else {
    SOC_DeltaTime_ms = 100000U;
  }

  if (VoltageTimeReset_flg && (voltage_time_judge_Trig_ZCE_s != POS_ZCSIG)) {
    u = (int16_T)((int16_T)SigPr_CellVolts_mV - CellVolts_delay);
    if (u < 0) {
      u = (int16_T)-u;
    }

    SOC_VoltGradientMatch_flg_s = (div_nde_s32_floor(1000 * u, (int32_T)
      (SOC_DeltaTime_ms / 1000U)) <= 1);
    CellVolts_delay = (int16_T)SigPr_CellVolts_mV;
  }

  voltage_time_judge_Trig_ZCE_s = (ZCSigState)VoltageTimeReset_flg;
  SOC_OCVUpd_flg = ((P_VoltMatchEnable_flg && (SOC_VoltGradientMatch_flg_s &&
    (tmp <= (int32_T)P_CurrentThresRelaxJudge_mA))) || (CurrentRelaxTime > 1000U
    * Debounce_Time_s) || (Init_OCVflag && (gd->SOC_SleepTime_s >= 1800U)));
  if (VoltageTimeReset_flg && (TimeSum_Reset_ZCE_s != POS_ZCSIG)) {
    Voltage_relax_time = 0U;
  }

  TimeSum_Reset_ZCE_s = (ZCSigState)VoltageTimeReset_flg;
  Lookup_OCVSOC();
  Init_OCVflag = false;
  VoltageTimeReset_flg = (SOC_DeltaTime_ms <= Voltage_relax_time);
  Voltage_relax_time = Voltage_relax_time + BMS_SampleTime_ms;
}

int32_T rt_sqrt_Us32_Ys32_Is64_f_s(int32_T u)
{
  int64m_T tmp;
  int64m_T tmp03_u;
  int32_T iBit;
  int32_T shiftMask;
  int32_T y;
  uint32_T tmp_0;

  /* Fixed-Point Sqrt Computation by the bisection method. */
  y = 0;
  shiftMask = 1073741824;
  sLong2MultiWord(u, &tmp03_u.chunks[0U], 2);
  for (iBit = 0; iBit < 31; iBit++) {
    int32_T tmp01_y;
    tmp01_y = y | shiftMask;
    tmp_0 = (uint32_T)tmp01_y;
    sMultiWordMul(&tmp_0, 1, &tmp_0, 1, &tmp.chunks[0U], 2);
    if (sMultiWordLe(&tmp.chunks[0U], &tmp03_u.chunks[0U], 2)) {
      y = tmp01_y;
    }

    shiftMask = (int32_T)((uint32_T)shiftMask >> 1U);
  }

  return y;
}

/* System initialize for atomic system: '<S3>/SOC_Correction' */
void SOC_Correction_Init(void)
{
  k_icLoad_s = true;
  k3_icLoad_s = true;
  k34_icLoad_s = true;
  k34f_icLoad_s = true;
  DiscreteTimeIntegrator_PrevRe_s = 2;
}

/* Enable for atomic system: '<S3>/SOC_Correction' */
void SOC_Correction_Enable(void)
{
  DiscreteTimeIntegrator_SYSTEM_s = 1U;
}

/* Output and update for atomic system: '<S3>/SOC_Correction' */
void SOC_Correction(void)
{
  /* local block i/o variables */
  int32_T SOH_VDCR_uV;
  int32_T SOC_CorrEachStep_mpct;
  int32_T SOC_CorrEachStep_upct;
  int32_T SOC_SOCSlope_mpctPermV;
  int32_T SOC_SocRangeCorrect_mpct;
  int32_T VSOCRawSOCmpct;
  if (P_ModelCorrEnable_flg) {
    SOC_SOCSlope_mpctPermV = SOC_ModelCorrect_pct;
  } else {
    SOC_SOCSlope_mpctPermV = 0;
  }

  gd->SOC_RawSOC_mpct = SOC_SOCSlope_mpctPermV + SOC_AhIntegralSOC_mpct;
  if (k3_icLoad_s) {
    Delay_DSTATE_s = SigPr_CellVolts_mV;
  }

  Delay_DSTATE_s = div_nde_s32_floor(ConstB_s.Add5_h * Delay_DSTATE_s +
    SigPr_CellVolts_mV, 10);
  if (k34_icLoad_s) {
    d_Delay_DSTATE_s = SigPr_PackCurr_mA;
  }

  d_Delay_DSTATE_s = div_nde_s32_floor(ConstB_s.Add5_ha * d_Delay_DSTATE_s +
    SigPr_PackCurr_mA, 10);
  if (k34f_icLoad_s) {
    VirtualOCV_Delay = Delay_DSTATE_s - div_nde_s32_floor(d_Delay_DSTATE_s *
      Cfg_DefultInitR_mOhm, 1000);
  }

  SOC_VirtualOCV_mV = VirtualOCV_Delay;
  SOC_ModelR0_mOhm = look2_is16s32lu32n32tu_bvHCJPGn(SigPr_CellTemps_C,
    gd->SOC_RawSOC_mpct, P_TAxis_degC, P_SOCAxis_mpct, P_R0Dsg_mOhm, (uint32_T *)
    &hjjpar_m_bpIndex_s[0], ConstP_s.pooled10, 3U);
  SOH_VDCR_uV = d_Delay_DSTATE_s * (int32_T)SOC_ModelR0_mOhm * Switch_s;
  if (DiscreteTimeIntegrator_SYSTEM_s == 0) {
    if ((SOC_OCVUpd_flg && (DiscreteTimeIntegrator_PrevRe_s <= 0)) ||
        ((!SOC_OCVUpd_flg) && (DiscreteTimeIntegrator_PrevRe_s == 1))) {
      DiscreteTimeIntegrator_DSTATE_s = 0;
    } else {
      DiscreteTimeIntegrator_DSTATE_s = (int32_T)SOC_ELAPS_T_s *
        DiscreteTimeIntegrator_PREV_U_s + DiscreteTimeIntegrator_DSTATE_s;
    }
  }

  SOC_ModelDCIR_mOhm = look2_is16s32lu32n32tu_bvHCJPGn(SigPr_CellTemps_C,
    gd->SOC_RawSOC_mpct, P_TAxis_degC, P_SOCAxis_mpct, P_DcirDsg_mOhm, (uint32_T *)
    &hjjparn_m_bpIndex_s[0], ConstP_s.pooled10, 3U);
  SOC_ModelOCV_mV = look2_is16s32lu16n16tu_EvbysC3W(SigPr_CellTemps_C,
    gd->SOC_RawSOC_mpct, P_TAxis_degC, P_SOCAxis_mpct, P_OCVDsg_mV, (uint32_T *)
    &hjjparne_m_bpIndex_s[0], ConstP_s.pooled10, 3U);
  k3_icLoad_s = false;
  k34_icLoad_s = false;
  k34f_icLoad_s = false;
  VirtualOCV_Delay = div_nde_s32_floor(((Delay_DSTATE_s - div_nde_s32_floor
    (SOH_VDCR_uV, 1000)) - mul_s32_hiSR(div_nde_s32_floor
    (DiscreteTimeIntegrator_DSTATE_s, 1000), 858993459, 1U)) * ConstB_s.Minus +
    9 * SOC_VirtualOCV_mV, 10);
  DiscreteTimeIntegrator_SYSTEM_s = 0U;
  DiscreteTimeIntegrator_PrevRe_s = (int8_T)SOC_OCVUpd_flg;
  DiscreteTimeIntegrator_PREV_U_s = div_nde_s32_floor((int32_T)
    (SOC_ModelDCIR_mOhm - SOC_ModelR0_mOhm) * Switch_s * d_Delay_DSTATE_s -
    mul_s32_hiSR(DiscreteTimeIntegrator_DSTATE_s, 858993459, 1U), 240);
  if (k_icLoad_s) {
    CellVoltsDelay = SigPr_CellVolts_mV;
  }

  CellVoltsDelay = div_nde_s32_floor(ConstB_s.Add5_b * CellVoltsDelay +
    SigPr_CellVolts_mV, 10);
  SOC_VirtOCVSOC_mpct = look2_is16s32lu32n32ts_WwgFj0xk(SigPr_CellTemps_C,
    SOC_VirtualOCV_mV, P_TAxis_degC, ConstP_s.pooled7, P_OcvSOCDsg_mpct,
    (uint32_T *)&hj_m_bpIndex_s[0], ConstP_s.pooled10, 3U);
  if (SOC_VirtOCVSOC_mpct > 100000) {
    SOC_VirtOCVSOC_mpct = 100000;
  } else if (SOC_VirtOCVSOC_mpct < 0) {
    SOC_VirtOCVSOC_mpct = 0;
  }

  if (SOC_OCVUpd_flg) {
    SOC_ModelCorrect_pct = 0;
  } else {
    SOC_SocRangeCorrect_mpct = look1_is32lu32n32Du32_pbinlcase
      (SOC_VirtOCVSOC_mpct, P_SocRangeAxis_mpct, P_SocRangeCorrect_mpct,
       (uint32_T *)&hjj_m_bpIndex_s, 4U);
    VSOCRawSOCmpct = SOC_VirtOCVSOC_mpct - gd->SOC_RawSOC_mpct;
    if(gd->charger_is_6801_flag)
    {
      SOC_SOCSlope_mpctPermV = look1_is32lu32n32Du32_pbinlcase(SOC_VirtualOCV_mV,
      ConstP_s.pooled7, P_SOCSlope_mpctPermV, (uint32_T *)&hjjp_m_bpIndex_s, 11U);
    }
	else
	{
      SOC_SOCSlope_mpctPermV = look1_is32lu32n32Du32_pbinlcase(SOC_VirtualOCV_mV,
      ConstP_s.pooled7, P_SOCSlope_mpctPermV, (uint32_T *)&hjjp_m_bpIndex_s, 31U);
     }
    SOC_CorrEachStep_upct = div_nde_s32_floor(SigPr_PackCurr_mA * (int32_T)
      SOC_ModelR0_mOhm, 1000);
    SOC_CorrEachStep_mpct = mul_s32_loSR(1288490189, (CellVoltsDelay -
      SOC_CorrEachStep_upct) - SOC_VirtualOCV_mV, 26U);
    SOC_CorrEachStep_upct = mul_s32_loSR(858993459, SOC_CorrEachStep_upct, 26U);
    SOC_CorrEachStep_mpct = rt_sqrt_Us32_Ys32_Is64_f_s(((mul_s32_loSR_sat
      (SOC_CorrEachStep_upct, SOC_CorrEachStep_upct, 5U) >> 5) +
      ConstB_s.stdvCellVADC2) + (mul_s32_loSR_sat(SOC_CorrEachStep_mpct,
      SOC_CorrEachStep_mpct, 5U) >> 5)) * SOC_SOCSlope_mpctPermV;
    if (VSOCRawSOCmpct < 0) {
      SOC_SOCSlope_mpctPermV = -VSOCRawSOCmpct;
    } else {
      SOC_SOCSlope_mpctPermV = VSOCRawSOCmpct;
    }

    SOC_SOCSlope_mpctPermV -= SOC_CorrEachStep_mpct;
    if (SOC_SOCSlope_mpctPermV > 100000) {
      SOC_SOCSlope_mpctPermV = 100000;
    } else if (SOC_SOCSlope_mpctPermV < 0) {
      SOC_SOCSlope_mpctPermV = 0;
    }

    SOC_SOCSlope_mpctPermV = look1_is32lu32n32Du32_pbinlcase
      (SOC_SOCSlope_mpctPermV, P_SocDeviationAxis_mpct,
       P_SocDeviationCorrect_upct, (uint32_T *)&hjjpa_m_bpIndex_s, 6U);
    if (SOC_CorrEachStep_mpct == 0) {
      SOC_CorrEachStep_mpct = 1;
    }

    SOC_ModelCorrect_pct = div_nde_s32_floor(div_nde_s32_floor(div_nde_s32_floor
      (SOC_SOCSlope_mpctPermV, SOC_CorrEachStep_mpct) * SOC_SocRangeCorrect_mpct,
      1000) * VSOCRawSOCmpct, 1000) + SOC_ModelCorrect_pct;
  }

  k_icLoad_s = false;
}

/* System initialize for function-call system: '<S1>/SOC' */
void SOC_Init(void)
{
  OCVSOC_Init();
  AhIntegralSOC_Init();
  SOC_Correction_Init();
  SOC_CHG_flg = ConstB_s.SOC_CHG_flg_c;
}

/* Enable for function-call system: '<S1>/SOC' */
void SOC_Enable(void)
{
  SOC_RESET_ELAPS_T_s = true;
  SOC_Correction_Enable();
}

/* Output and update for function-call system: '<S1>/SOC' */
void SOC(void)
{
  if (SOC_RESET_ELAPS_T_s) {
    SOC_ELAPS_T_s = 0U;
  } else {
    SOC_ELAPS_T_s = M_s->Timing.clockTick0 - SOC_PREV_T_s;
  }

  SOC_PREV_T_s = M_s->Timing.clockTick0;
  SOC_RESET_ELAPS_T_s = false;
  OCVSOC();
  AhIntegralSOC();
  SOC_Correction();
  SOC_CHG_flg = ConstB_s.SOC_CHG_flg_c;
}
