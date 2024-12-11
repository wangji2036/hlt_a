/*
 * Code generation for system system '<S1>/SOCPack'
 *
 * Model                      : BMS_FixPoint
 * Model version              : 7.88
 * Simulink Coder version : 24.2 (R2024b) 21-Jun-2024
 *
 * Note that the functions contained in this file are part of a Simulink
 * model, and are not self-contained algorithms.
 */

#include "SOCPack.h"
#include "BMS_FixPoint.h"
#include "rtwtypes.h"
#include "BMS_FixPoint_private.h"
#include "BMS_data.h"

/* Exported data definition */

/* Definition for custom storage class: Localizable */
static int32_T Empty_SOC_delay;        /* '<S33>/SOCPack_UdEmptySOC_mpct' */
static int32_T SOCPack_UdDisplaySOC_pct;/* '<S36>/SOCPack_UdDisplaySOC_pct' */
static int32_T SOCPack_UsableSOC_pct;  /* '<S35>/out1' */

/* Output and update for atomic system: '<S4>/Pack_Empty' */
void Pack_Empty(void)
{
  int32_T u0;
  SOCPack_UdEmptySOC_mpct = Empty_SOC_delay;
  if (SOCPack_UdEmptySOC_mpct > 20000) {
    SOCPack_SatuarationSoc_mpct = 20000;
  } else if (SOCPack_UdEmptySOC_mpct < 0) {
    SOCPack_SatuarationSoc_mpct = 0;
  } else {
    SOCPack_SatuarationSoc_mpct = SOCPack_UdEmptySOC_mpct;
  }

  SOCPack_EmptyDcr_mOhm = look2_is16s32lu32n32ts_7E7IgjCI(SigPr_CellTemps_C,
    SOCPack_SatuarationSoc_mpct, P_TAxis_degC, P_SOCAxis_mpct,
    ConstP_s.DCIR_Discharge_tableData, (uint32_T *)&m_bpIndex_s[0],
    ConstP_s.pooled9, 3U);
  SOCPack_EmptyU_mV = div_nde_s32_floor(P_AtRateCurrent_mA *
    SOCPack_EmptyDcr_mOhm, 1000) + P_EmptyVoltage_mV;
  SOCPack_PreEmptySOC_mpct = look2_is16s32lu32n32ts_WwgFj0xk(SigPr_CellTemps_C,
    SOCPack_EmptyU_mV, P_TAxis_degC, ConstP_s.pooled7, P_OcvSOCDsg_mpct,
    (uint32_T *)&h_m_bpIndex_s[0], ConstP_s.pooled9, 3U);
  if (SOCPack_PreEmptySOC_mpct > 30000) {
    SOCPack_PreEmptySOC_mpct = 30000;
  } else if (SOCPack_PreEmptySOC_mpct < 0) {
    SOCPack_PreEmptySOC_mpct = 0;
  }

  u0 = SOCPack_PreEmptySOC_mpct - SOCPack_UdEmptySOC_mpct;
  if (u0 > 10) {
    u0 = 10;
  } else if (u0 < -10) {
    u0 = -10;
  }

  SOCPack_EmptySOC_mpct = u0 + SOCPack_UdEmptySOC_mpct;
  Empty_SOC_delay = SOCPack_EmptySOC_mpct;
}

/* System initialize for atomic system: '<S4>/SOC_Filter' */
void SOC_Filter_Init(void)
{
  icLoad_s = true;
}

/* Output and update for atomic system: '<S4>/SOC_Filter' */
void SOC_Filter(void)
{
  /* local block i/o variables */
  int32_T SOCPack_DispUsableSocdeviation_pct;
  int32_T Lowerlimit;
  if (icLoad_s) {
    SOCPack_UdDisplaySOC_pct = SOCPack_UsableSOC_pct;
  }

  SOCPack_DispUsableSocdeviation_pct = SOCPack_UsableSOC_pct -
    SOCPack_UdDisplaySOC_pct;
  if (SigPr_PackCurr_mA > 0) {
    Lowerlimit = 100;
  } else {
    Lowerlimit = 0;
  }

  if (SOCPack_DispUsableSocdeviation_pct <= Lowerlimit) {
    if (SigPr_PackCurr_mA > 0) {
      Lowerlimit = 0;
    } else {
      Lowerlimit = ConstB_s.Divide1_n;
    }

    if (SOCPack_DispUsableSocdeviation_pct >= Lowerlimit) {
      Lowerlimit = SOCPack_DispUsableSocdeviation_pct;
    }
  }

  SOCPack_UdDisplaySOC_pct = SOCPack_UdDisplaySOC_pct + Lowerlimit;
  SOCPack_DisplaySOC_pct = div_nde_s32_floor(SOCPack_UdDisplaySOC_pct, 1000);
  icLoad_s = false;
}

/* System initialize for function-call system: '<S1>/SOCPack' */
void SOCPack_Init(void)
{
  SOC_Filter_Init();
}

/* Output and update for function-call system: '<S1>/SOCPack' */
void SOCPack(void)
{
  int32_T SOCPack_RealSOC_mpct;
  Pack_Empty();
  if (SOC_RawSOC_mpct <= 25000) {
    SOCPack_RealSOC_mpct = SOC_RawSOC_mpct;
  } else if (SOC_RawSOC_mpct >= 80000) {
    SOCPack_RealSOC_mpct = SOC_RawSOC_mpct;
  } else {
    if (ConstB_s.Add2_c <= 1) {
      SOCPack_RealSOC_mpct = 1;
    } else {
      SOCPack_RealSOC_mpct = ConstB_s.Add2_c;
    }

    SOCPack_RealSOC_mpct = (SOC_RawSOC_mpct - 25000) * div_nde_s32_floor
      (ConstB_s.Add2_c, SOCPack_RealSOC_mpct) + 25000;
    if (SOCPack_RealSOC_mpct > 100000) {
      SOCPack_RealSOC_mpct = 100000;
    } else if (SOCPack_RealSOC_mpct < 0) {
      SOCPack_RealSOC_mpct = 0;
    }
  }

  if (SOCPack_RealSOC_mpct <= SOCPack_EmptySOC_mpct + 2000) {
    SOCPack_UsableSOC_pct = 0;
  } else if (SOCPack_RealSOC_mpct >= 95000) {
    SOCPack_UsableSOC_pct = 100000;
  } else {
    SOCPack_UsableSOC_pct = ((SOCPack_RealSOC_mpct - SOCPack_EmptySOC_mpct) -
      2000) * div_nde_s32_floor(100000, 93000 - SOCPack_EmptySOC_mpct);
  }

  SOCPack_UsableSOC_pct_s = div_nde_s32_floor(SOCPack_UsableSOC_pct, 1000);
  SOC_Filter();
  SOCPack_RealSOC_pct = div_nde_s32_floor(SOCPack_RealSOC_mpct, 1000);
}
