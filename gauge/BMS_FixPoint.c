/*
 * BMS_FixPoint.c
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

#include "BMS_FixPoint.h"
#include "BMS_FixPoint_private.h"
#include "rtwtypes.h"
#include "SOC.h"
#include "SOCPack.h"
#include "BMS_data.h"
#include "zero_crossing_types.h"
#include "regdef.h"
#include "printk.h"
#include "config.h"
#include "g_data.h"
/* Exported block signals */
boolean_T SOC_OCVUpd_flg;              /* '<Root>/SOC_OCVUpd_flg' */
int32_T SOC_OCVSOC_mpct;               /* '<Root>/SOC_OCVSOC_mpct' */
int32_T SOC_AhIntegralSOC_mpct;        /* '<Root>/SOC_AhIntegralSOC_mpct' */
//int32_T SOC_RawSOC_mpct;               /* '<Root>/SOC_RawSOC_mpct' */
int32_T SOH_SOHR_pct;                  /* '<Root>/SOH_SOHR_pct' */
int32_T SOH_Resistance_mOhm;           /* '<Root>/SOH_Resistance_mOhm' */
boolean_T SOC_CHG_flg;                 /* '<Root>/SOC_CHG_flg' */
int32_T SOCPack_RealSOC_pct;           /* '<Root>/SOCPack_RealSOC_pct' */
int32_T SOCPack_EmptySOC_mpct;         /* '<Root>/SOCPack_EmptySOC_mpct' */
int32_T SOCPack_DisplaySOC_pct;        /* '<Root>/SOCPack_DisplaySOC_pct' */
uint32_T Bal_BalTime_s;                /* '<Root>/Bal_BalTime_s' */
uint32_T SOC_ModelR0_mOhm;             /* '<S23>/Switch3' */
uint32_T SOC_ModelDCIR_mOhm;           /* '<S23>/Switch2' */
int32_T SOCPack_UdEmptySOC_mpct;       /* '<S33>/SOCPack_UdEmptySOC_mpct' */
int32_T SOCPack_SatuarationSoc_mpct;   /* '<S33>/Saturation' */
int32_T SOCPack_EmptyDcr_mOhm;         /* '<S33>/DCIR_Discharge' */
int32_T SOCPack_EmptyU_mV;             /* '<S33>/Add2' */
int32_T SOCPack_PreEmptySOC_mpct;      /* '<S33>/Saturation4' */
int32_T SOC_VirtOCVSOC_mpct;           /* '<S19>/Saturation1' */
int32_T SOC_VirtualOCV_mV;             /* '<S20>/UdVirtualOcv' */
uint16_T BMS_SampleTime_ms;            /* '<S43>/Constant1' */
uint16_T SOC_ModelOCV_mV;              /* '<S23>/Switch1' */

#if(BUCKBOOST_USED_NU6801 == 1)
/* Exported block parameters */
int32_T P_AtRateCurrent_mA = 3000;     /* Variable: P_AtRateCurrent_mA
                                        * Referenced by: '<S4>/At_Rate_Current_mA'
                                        */
int32_T P_EmptyVoltage_mV = 3000;      /* Variable: P_EmptyVoltage_mV
                                        * Referenced by: '<S4>/Empty_Voltage_mV'
                                        */
const int32_T P_OcvSOCDsg_mpct[36] = { 0, 0, 0, 12424, 12424, 12424, 24849, 24849,
  24849, 37273, 37273, 37273, 49698, 49698, 49698, 62122, 62122, 62122, 74547,
  74547, 74547, 85077, 85077, 85077, 91437, 91437, 91437, 95451, 95451, 95451,
  97891, 97891, 97891, 100000, 100000, 100000 } ;/* Variable: P_OcvSOCDsg_mpct
                                                  * Referenced by:
                                                  *   '<S33>/OCV_DSG'
                                                  *   '<S12>/OCV_DSG'
                                                  *   '<S19>/OCV_DSG'
                                                  */

int32_T P_SOCAxis_mpct[12] = { 0, 12424, 24849, 37273, 49698, 62122, 74547,
  85077, 91437, 95451, 97891, 100000 } ;/* Variable: P_SOCAxis_mpct
                                         * Referenced by:
                                         *   '<S33>/DCIR_Discharge'
                                         *   '<S23>/DCIR_Discharge'
                                         *   '<S23>/OCV_Discharge'
                                         *   '<S23>/R0_Discharge'
                                         */

int32_T P_SOCSlope_mpctPermV[12] = { 25, 25, 114, 282, 226, 99, 109, 99, 106, 98,
  79, 66 } ;                           /* Variable: P_SOCSlope_mpctPermV
                                        * Referenced by: '<S22>/SOC_Slope'
                                        */
#endif


#if(BUCKBOOST_USED_NU6805 == 1)
/* Exported block parameters */
/* Exported block parameters */
int32_T P_AtRateCurrent_mA = 1000;     /* Variable: P_AtRateCurrent_mA
                                        * Referenced by: '<S4>/At_Rate_Current_mA'
                                        */
int32_T P_EmptyVoltage_mV = 6000;      /* Variable: P_EmptyVoltage_mV
                                        * Referenced by: '<S4>/Empty_Voltage_mV'
                                        */
const int32_T P_OcvSOCChg_mpct[60] = { 0, 0, 0, 0, 0, 0, 1088, 1088, 1088, 1794, 1794,
  1794, 2500, 2500, 2500, 3913, 3913, 3913, 4619, 4619, 4619, 8252, 8252, 8252,
  16137, 16137, 16137, 28183, 28183, 28183, 42725, 42725, 42725, 51350, 51350,
  51350, 56572, 56572, 56572, 61006, 61006, 61006, 68572, 68572, 68572, 75664,
  75664, 75664, 82318, 82318, 82318, 89089, 89089, 89089, 94016, 94016, 94016,
  100000, 100000, 100000 } ;           /* Variable: P_OcvSOCChg_mpct
                                        * Referenced by:
                                        *   '<S13>/OCV_CHG'
                                        *   '<S21>/OCV_CHG'
                                        */

const int32_T P_OcvSOCDsg_mpct[60] = { 0, 0, 0, 0, 0, 0, 722, 722, 722, 1410, 1410,
  1410, 2229, 2229, 2229, 3867, 3867, 3867, 5228, 5228, 5228, 12204, 12204,
  12204, 20984, 20984, 20984, 35236, 35236, 35236, 46473, 46473, 46473, 53786,
  53786, 53786, 58682, 58682, 58682, 63807, 63807, 63807, 71639, 71639, 71639,
  78879, 78879, 78879, 85368, 85368, 85368, 91589, 91589, 91589, 95712, 95712,
  95712, 100000, 100000, 100000 } ;    /* Variable: P_OcvSOCDsg_mpct
                                        * Referenced by:
                                        *   '<S35>/OCV_DSG'
                                        *   '<S13>/OCV_DSG'
                                        *   '<S21>/OCV_DSG'
                                        */

const int32_T P_SOCAxis_mpct[20] = { 0, 5000, 10000, 15000, 20000, 25000, 30000, 35000,
  40000, 45000, 50000, 60000, 65000, 70000, 75000, 80000, 85000, 90000, 95000,
  100000 } ;                           /* Variable: P_SOCAxis_mpct
                                        * Referenced by:
                                        *   '<S35>/DCIR_Discharge'
                                        *   '<S25>/DCIR_Charge'
                                        *   '<S25>/DCIR_Discharge'
                                        *   '<S25>/OCV_Charge'
                                        *   '<S25>/OCV_Discharge'
                                        *   '<S25>/R0_Charge'
                                        *   '<S25>/R0_Discharge'
                                        */

const int32_T P_SOCSlope_mpctPermV[20] = { 7, 7, 74, 71, 90, 122, 171, 169, 129, 105,
  84, 121, 46, 50, 51, 48, 45, 43, 43, 23 } ;/* Variable: P_SOCSlope_mpctPermV
                                              * Referenced by: '<S24>/SOC_Slope'
                                              */

#endif
const int32_T P_SocDeviationAxis_mpct[7] = { 0, 1000, 2000, 8000, 10000, 20000, 100000
} ;                                    /* Variable: P_SocDeviationAxis_mpct
                                        * Referenced by: '<S19>/1-D Lookup Table1'
                                        */

const int32_T P_SocDeviationCorrect_upct[7] = { 75, 7500, 20000, 50000, 100000, 800000,
  1000000 } ;                          /* Variable: P_SocDeviationCorrect_upct
                                        * Referenced by: '<S19>/1-D Lookup Table1'
                                        */

const int32_T P_SocRangeAxis_mpct[5] = { 0, 10000, 20000, 25000, 100000 } ;/* Variable: P_SocRangeAxis_mpct
                                                                      * Referenced by: '<S19>/adaption in SOC range'
                                                                      */

const int32_T P_SocRangeCorrect_mpct[5] = { 10, 10, 150, 800, 1000 } ;/* Variable: P_SocRangeCorrect_mpct
                                                                 * Referenced by: '<S19>/adaption in SOC range'
                                                                 */

uint32_T P_CurrentThresRelaxJudge_mA = 40U;/* Variable: P_CurrentThresRelaxJudge_mA
                                            * Referenced by:
                                            *   '<S11>/Constant1'
                                            *   '<S13>/Constant1'
                                            */
#if(BUCKBOOST_USED_NU6801 == 1)
const uint32_T P_DcirDsg_mOhm[36] = { 70U, 70U, 70U, 84U, 84U, 84U, 70U, 70U, 70U, 77U,
  77U, 77U, 86U, 86U, 86U, 75U, 75U, 75U, 81U, 81U, 81U, 88U, 88U, 88U, 100U,
  100U, 100U, 107U, 107U, 107U, 112U, 112U, 112U, 135U, 135U, 135U } ;/* Variable: P_DcirDsg_mOhm
                                                                      * Referenced by: '<S23>/DCIR_Discharge'
                                                                      */

const uint32_T P_R0Dsg_mOhm[36] = { 52U, 52U, 52U, 52U, 52U, 52U, 45U, 45U, 45U, 42U,
  42U, 42U, 41U, 41U, 41U, 41U, 41U, 41U, 41U, 41U, 41U, 41U, 41U, 41U, 41U, 41U,
  41U, 44U, 44U, 44U, 45U, 45U, 45U, 43U, 43U, 43U } ;/* Variable: P_R0Dsg_mOhm
                                                       * Referenced by: '<S23>/R0_Discharge'
                                                       */
#endif
											
											
#if(BUCKBOOST_USED_NU6805 == 1)
const uint32_T P_DcirChg_mOhm[60] = { 157U, 157U, 157U, 157U, 157U, 157U, 169U, 169U,
  169U, 167U, 167U, 167U, 165U, 165U, 165U, 162U, 162U, 162U, 162U, 162U, 162U,
  163U, 163U, 163U, 164U, 164U, 164U, 166U, 166U, 166U, 167U, 167U, 167U, 163U,
  163U, 163U, 157U, 157U, 157U, 156U, 156U, 156U, 161U, 161U, 161U, 167U, 167U,
  167U, 183U, 183U, 183U, 199U, 199U, 199U, 211U, 211U, 211U, 297U, 297U, 297U }
;                                      /* Variable: P_DcirChg_mOhm
                                        * Referenced by: '<S25>/DCIR_Charge'
                                        */

const uint32_T P_DcirDsg_mOhm[60] = { 780U, 780U, 780U, 744U, 744U, 744U, 280U, 280U,
  280U, 219U, 219U, 219U, 194U, 194U, 194U, 177U, 177U, 177U, 174U, 174U, 174U,
  169U, 169U, 169U, 166U, 166U, 166U, 162U, 162U, 162U, 161U, 161U, 161U, 163U,
  163U, 163U, 187U, 187U, 187U, 192U, 192U, 192U, 193U, 193U, 193U, 193U, 193U,
  193U, 193U, 193U, 193U, 196U, 196U, 196U, 200U, 200U, 200U, 204U, 204U, 204U }
;                                      /* Variable: P_DcirDsg_mOhm
                                        * Referenced by: '<S25>/DCIR_Discharge'
                                        */

const uint32_T P_R0Chg_mOhm[60] = { 119U, 119U, 119U, 119U, 119U, 119U, 112U, 112U,
  112U, 110U, 110U, 110U, 109U, 109U, 109U, 109U, 109U, 109U, 109U, 109U, 109U,
  109U, 109U, 109U, 109U, 109U, 109U, 109U, 109U, 109U, 107U, 107U, 107U, 107U,
  107U, 107U, 106U, 106U, 106U, 105U, 105U, 105U, 105U, 105U, 105U, 105U, 105U,
  105U, 104U, 104U, 104U, 106U, 106U, 106U, 108U, 108U, 108U, 107U, 107U, 107U }
;                                      /* Variable: P_R0Chg_mOhm
                                        * Referenced by: '<S25>/R0_Charge'
                                        */

const uint32_T P_R0Dsg_mOhm[60] = { 119U, 119U, 119U, 113U, 113U, 113U, 113U, 113U,
  113U, 113U, 113U, 113U, 113U, 113U, 113U, 111U, 111U, 111U, 110U, 110U, 110U,
  111U, 111U, 111U, 110U, 110U, 110U, 110U, 110U, 110U, 109U, 109U, 109U, 109U,
  109U, 109U, 110U, 110U, 110U, 110U, 110U, 110U, 111U, 111U, 111U, 111U, 111U,
  111U, 114U, 114U, 114U, 117U, 117U, 117U, 116U, 116U, 116U, 115U, 115U, 115U }
;                                      /* Variable: P_R0Dsg_mOhm
                                        * Referenced by: '<S25>/R0_Discharge'
                                        */
#endif
uint32_T P_RelaxDurationExtremeLowTemp_s = 7200U;
                                    /* Variable: P_RelaxDurationExtremeLowTemp_s
                                     * Referenced by: '<S11>/P_RelaxDurationExtremeLowTemp_s'
                                     */
uint32_T P_RelaxDurationLowTemp_s = 3600U;/* Variable: P_RelaxDurationLowTemp_s
                                           * Referenced by: '<S11>/P_RelaxDurationLowTemp_s'
                                           */
uint32_T P_RelaxDurationNormalTemp_s = 1500U;/* Variable: P_RelaxDurationNormalTemp_s
                                              * Referenced by: '<S11>/P_RelaxDurationNormalTemp_s'
                                              */
int16_T P_LowTemp_degC = 10;           /* Variable: P_LowTemp_degC
                                        * Referenced by: '<S15>/Constant'
                                        */
int16_T P_NormalTemp_degC = 20;        /* Variable: P_NormalTemp_degC
                                        * Referenced by: '<S14>/Constant'
                                        */
int16_T P_TAxis_degC[3] = { 0, 25, 45 } ;/* Variable: P_TAxis_degC
                                          * Referenced by:
                                          *   '<S33>/DCIR_Discharge'
                                          *   '<S33>/OCV_DSG'
                                          *   '<S12>/OCV_DSG'
                                          *   '<S19>/OCV_DSG'
                                          *   '<S23>/DCIR_Discharge'
                                          *   '<S23>/OCV_Discharge'
                                          *   '<S23>/R0_Discharge'
                                          */
#if(BUCKBOOST_USED_NU6801 == 1)
uint16_T P_Capacity_mAh = 5374U;       /* Variable: P_Capacity_mAh
                                        * Referenced by:
                                        *   '<S40>/SOH_capacity_mAh'
                                        *   '<S40>/Constant3'
                                        */
uint16_T P_OCVAxis_mV[12] = { 2989U, 3492U, 3601U, 3645U, 3700U, 3825U, 3939U,
  4045U, 4105U, 4146U, 4177U, 4209U } ;/* Variable: P_OCVAxis_mV
                                        * Referenced by: '<S12>/OCV_DSG'
                                        */

const uint16_T P_OCVDsg_mV[36] = { 2989U, 2989U, 2989U, 3492U, 3492U, 3492U, 3601U,
  3601U, 3601U, 3645U, 3645U, 3645U, 3700U, 3700U, 3700U, 3825U, 3825U, 3825U,
  3939U, 3939U, 3939U, 4045U, 4045U, 4045U, 4105U, 4105U, 4105U, 4146U, 4146U,
  4146U, 4177U, 4177U, 4177U, 4209U, 4209U, 4209U } ;/* Variable: P_OCVDsg_mV
                                                      * Referenced by: '<S23>/OCV_Discharge'
                                                      */
#endif

#if(BUCKBOOST_USED_NU6805 == 1)
uint16_T P_Capacity_mAh = 5000U;      /* Variable: P_Capacity_mAh
                                        * Referenced by:
                                        *   '<S40>/SOH_capacity_mAh'
                                        *   '<S40>/Constant3'
                                        */
const uint16_T P_OCVAxis_mV[20] = { 6000U, 6646U, 6800U, 6900U, 7000U, 7200U, 7300U,
  7400U, 7500U, 7600U, 7700U, 7800U, 7900U, 8000U, 8150U, 8300U, 8450U, 8600U,
  8700U, 8800U } ;                     /* Variable: P_OCVAxis_mV
                                        * Referenced by:
                                        *   '<S13>/OCV_CHG'
                                        *   '<S13>/OCV_DSG'
                                        */

const uint16_T P_OCVChg_mV[60] = { 6646U, 6646U, 6646U, 7354U, 7354U, 7354U, 7415U,
  7415U, 7415U, 7486U, 7486U, 7486U, 7544U, 7544U, 7544U, 7583U, 7583U, 7583U,
  7609U, 7609U, 7609U, 7637U, 7637U, 7637U, 7675U, 7675U, 7675U, 7723U, 7723U,
  7723U, 7783U, 7783U, 7783U, 7870U, 7870U, 7870U, 7977U, 7977U, 7977U, 8079U,
  8079U, 8079U, 8179U, 8179U, 8179U, 8285U, 8285U, 8285U, 8397U, 8397U, 8397U,
  8510U, 8510U, 8510U, 8619U, 8619U, 8619U, 8800U, 8800U, 8800U } ;/* Variable: P_OCVChg_mV
                                                                    * Referenced by: '<S25>/OCV_Charge'
                                                                    */

const uint16_T P_OCVDsg_mV[60] = { 6646U, 6646U, 6646U, 7297U, 7297U, 7297U, 7370U,
  7370U, 7370U, 7438U, 7438U, 7438U, 7491U, 7491U, 7491U, 7534U, 7534U, 7534U,
  7567U, 7567U, 7567U, 7598U, 7598U, 7598U, 7637U, 7637U, 7637U, 7684U, 7684U,
  7684U, 7742U, 7742U, 7742U, 7820U, 7820U, 7820U, 7927U, 7927U, 7927U, 8023U,
  8023U, 8023U, 8119U, 8119U, 8119U, 8218U, 8218U, 8218U, 8325U, 8325U, 8325U,
  8441U, 8441U, 8441U, 8561U, 8561U, 8561U, 8800U, 8800U, 8800U } ;/* Variable: P_OCVDsg_mV
                                                                    * Referenced by: '<S25>/OCV_Discharge'
                                                                    */

#endif
uint16_T P_SampleTime_ms = 100U;       /* Variable: P_SampleTime_ms
                                        * Referenced by: '<S43>/Constant1'
                                        */
boolean_T P_ModelCorrEnable_flg = true;/* Variable: P_ModelCorrEnable_flg
                                        * Referenced by: '<S21>/Constant3'
                                        */
boolean_T P_VoltMatchEnable_flg = false;/* Variable: P_VoltMatchEnable_flg
                                         * Referenced by: '<S13>/Constant2'
                                         */
static boolean_T Soc_Initialed = false;
/* Real-time model */
static RT_MODEL M_s_;
RT_MODEL *const M_s = &M_s_;
int32_T look2_is16u16lu32n32ts_lElusMKZ(int16_T u0, uint16_T u1, const int16_T
  bp0[], const uint16_T bp1[], const int32_T table[], uint32_T prevIndex[],
  const uint32_T maxIndex[], uint32_T stride)
{
  int32_T y;
  int32_T yL_0d0;
  int32_T yR_0d0;
  uint32_T bpIndices[2];
  uint32_T fractions[2];
  uint32_T bpIdx;
  uint32_T found;
  uint32_T frac;
  uint32_T iLeft;

  /* Column-major Lookup 2-D
     Canonical function name: look2_is16u16lu32n32ts32Du32du32_pbinlcase
     Search method: 'binary'
     Use previous index: 'on'
     Interpolation method: 'Linear point-slope'
     Extrapolation method: 'Clip'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'off'
     Rounding mode: 'simplest'
   */
  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Clip'
     Use previous index: 'on'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'off'
     Rounding mode: 'simplest'
   */
  if (u0 <= bp0[0U]) {
    bpIdx = 0U;
    frac = 0U;
  } else if (u0 < bp0[maxIndex[0U]]) {
    /* Binary Search using Previous Index */
    bpIdx = prevIndex[0U];
    iLeft = 0U;
    frac = maxIndex[0U];
    found = 0U;
    while (found == 0U) {
      if (u0 < bp0[bpIdx]) {
        frac = bpIdx - 1U;
        bpIdx = ((bpIdx + iLeft) - 1U) >> 1U;
      } else if (u0 < bp0[bpIdx + 1U]) {
        found = 1U;
      } else {
        iLeft = bpIdx + 1U;
        bpIdx = ((bpIdx + frac) + 1U) >> 1U;
      }
    }

    int16_T bpLeftVar;
    bpLeftVar = bp0[bpIdx];
    frac = div_nzp_repeat_u32((uint32_T)(u0 - bpLeftVar) << 16, (uint32_T)
      (bp0[bpIdx + 1U] - bpLeftVar), 16U);
  } else {
    bpIdx = maxIndex[0U];
    frac = 0U;
  }

  prevIndex[0U] = bpIdx;
  fractions[0U] = frac;
  bpIndices[0U] = bpIdx;

  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Clip'
     Use previous index: 'on'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'off'
     Rounding mode: 'simplest'
   */
  if (u1 <= bp1[0U]) {
    bpIdx = 0U;
    frac = 0U;
  } else if (u1 < bp1[maxIndex[1U]]) {
    /* Binary Search using Previous Index */
    bpIdx = prevIndex[1U];
    iLeft = 0U;
    frac = maxIndex[1U];
    found = 0U;
    while (found == 0U) {
      if (u1 < bp1[bpIdx]) {
        frac = bpIdx - 1U;
        bpIdx = ((bpIdx + iLeft) - 1U) >> 1U;
      } else if (u1 < bp1[bpIdx + 1U]) {
        found = 1U;
      } else {
        iLeft = bpIdx + 1U;
        bpIdx = ((bpIdx + frac) + 1U) >> 1U;
      }
    }

    uint16_T bpLeftVar_0;
    bpLeftVar_0 = bp1[bpIdx];
    frac = div_nzp_repeat_u32(((uint32_T)u1 - bpLeftVar_0) << 16, (uint32_T)
      bp1[bpIdx + 1U] - bpLeftVar_0, 16U);
  } else {
    bpIdx = maxIndex[1U];
    frac = 0U;
  }

  prevIndex[1U] = bpIdx;

  /* Column-major Interpolation 2-D
     Interpolation method: 'Linear point-slope'
     Use last breakpoint for index at or above upper limit: 'on'
     Rounding mode: 'simplest'
     Overflow mode: 'wrapping'
   */
  iLeft = bpIdx * stride + bpIndices[0U];
  if (bpIndices[0U] == maxIndex[0U]) {
    y = table[iLeft];
  } else {
    yR_0d0 = table[iLeft + 1U];
    yL_0d0 = table[iLeft];
    if (yR_0d0 >= yL_0d0) {
      y = (int32_T)mul_u32_sr32(fractions[0U], (uint32_T)yR_0d0 - (uint32_T)
        yL_0d0) + yL_0d0;
    } else {
      y = yL_0d0 - (int32_T)mul_u32_sr32(fractions[0U], (uint32_T)yL_0d0 -
        (uint32_T)yR_0d0);
    }
  }

  if (bpIdx == maxIndex[1U]) {
  } else {
    iLeft += stride;
    if (bpIndices[0U] == maxIndex[0U]) {
      yR_0d0 = table[iLeft];
    } else {
      yR_0d0 = table[iLeft + 1U];
      yL_0d0 = table[iLeft];
      if (yR_0d0 >= yL_0d0) {
        yR_0d0 = (int32_T)mul_u32_sr32(fractions[0U], (uint32_T)yR_0d0 -
          (uint32_T)yL_0d0) + yL_0d0;
      } else {
        yR_0d0 = yL_0d0 - (int32_T)mul_u32_sr32(fractions[0U], (uint32_T)yL_0d0
          - (uint32_T)yR_0d0);
      }
    }

    if (yR_0d0 >= y) {
      y += (int32_T)mul_u32_sr32(frac, (uint32_T)yR_0d0 - (uint32_T)y);
    } else {
      y -= (int32_T)mul_u32_sr32(frac, (uint32_T)y - (uint32_T)yR_0d0);
    }
  }

  return y;
}

int32_T look2_is16s32lu32n32ts_WwgFj0xk(int16_T u0, int32_T u1, const int16_T
  bp0[], const int32_T bp1[], const int32_T table[], uint32_T prevIndex[], const
  uint32_T maxIndex[], uint32_T stride)
{
  int32_T bpLeftVar_0;
  int32_T y;
  int32_T yL_0d0;
  uint32_T bpIndices[2];
  uint32_T fractions[2];
  uint32_T bpIdx;
  uint32_T found;
  uint32_T frac;
  uint32_T iLeft;

  /* Column-major Lookup 2-D
     Canonical function name: look2_is16s32lu32n32ts32Du32du32_pbinlcase
     Search method: 'binary'
     Use previous index: 'on'
     Interpolation method: 'Linear point-slope'
     Extrapolation method: 'Clip'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'off'
     Rounding mode: 'simplest'
   */
  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Clip'
     Use previous index: 'on'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'off'
     Rounding mode: 'simplest'
   */
  if (u0 <= bp0[0U]) {
    bpIdx = 0U;
    frac = 0U;
  } else if (u0 < bp0[maxIndex[0U]]) {
    /* Binary Search using Previous Index */
    bpIdx = prevIndex[0U];
    iLeft = 0U;
    frac = maxIndex[0U];
    found = 0U;
    while (found == 0U) {
      if (u0 < bp0[bpIdx]) {
        frac = bpIdx - 1U;
        bpIdx = ((bpIdx + iLeft) - 1U) >> 1U;
      } else if (u0 < bp0[bpIdx + 1U]) {
        found = 1U;
      } else {
        iLeft = bpIdx + 1U;
        bpIdx = ((bpIdx + frac) + 1U) >> 1U;
      }
    }

    int16_T bpLeftVar;
    bpLeftVar = bp0[bpIdx];
    frac = div_nzp_repeat_u32((uint32_T)(u0 - bpLeftVar) << 16, (uint32_T)
      (bp0[bpIdx + 1U] - bpLeftVar), 16U);
  } else {
    bpIdx = maxIndex[0U];
    frac = 0U;
  }

  prevIndex[0U] = bpIdx;
  fractions[0U] = frac;
  bpIndices[0U] = bpIdx;

  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Clip'
     Use previous index: 'on'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'off'
     Rounding mode: 'simplest'
   */
  if (u1 <= bp1[0U]) {
    bpIdx = 0U;
    frac = 0U;
  } else if (u1 < bp1[maxIndex[1U]]) {
    /* Binary Search using Previous Index */
    bpIdx = prevIndex[1U];
    iLeft = 0U;
    frac = maxIndex[1U];
    found = 0U;
    while (found == 0U) {
      if (u1 < bp1[bpIdx]) {
        frac = bpIdx - 1U;
        bpIdx = ((bpIdx + iLeft) - 1U) >> 1U;
      } else if (u1 < bp1[bpIdx + 1U]) {
        found = 1U;
      } else {
        iLeft = bpIdx + 1U;
        bpIdx = ((bpIdx + frac) + 1U) >> 1U;
      }
    }

    bpLeftVar_0 = bp1[bpIdx];
    frac = div_nzp_repeat_u32((uint32_T)u1 - (uint32_T)bpLeftVar_0, (uint32_T)
      bp1[bpIdx + 1U] - (uint32_T)bpLeftVar_0, 32U);
  } else {
    bpIdx = maxIndex[1U];
    frac = 0U;
  }

  prevIndex[1U] = bpIdx;

  /* Column-major Interpolation 2-D
     Interpolation method: 'Linear point-slope'
     Use last breakpoint for index at or above upper limit: 'on'
     Rounding mode: 'simplest'
     Overflow mode: 'wrapping'
   */
  iLeft = bpIdx * stride + bpIndices[0U];
  if (bpIndices[0U] == maxIndex[0U]) {
    y = table[iLeft];
  } else {
    bpLeftVar_0 = table[iLeft + 1U];
    yL_0d0 = table[iLeft];
    if (bpLeftVar_0 >= yL_0d0) {
      y = (int32_T)mul_u32_sr32(fractions[0U], (uint32_T)bpLeftVar_0 - (uint32_T)
        yL_0d0) + yL_0d0;
    } else {
      y = yL_0d0 - (int32_T)mul_u32_sr32(fractions[0U], (uint32_T)yL_0d0 -
        (uint32_T)bpLeftVar_0);
    }
  }

  if (bpIdx == maxIndex[1U]) {
  } else {
    iLeft += stride;
    if (bpIndices[0U] == maxIndex[0U]) {
      bpLeftVar_0 = table[iLeft];
    } else {
      bpLeftVar_0 = table[iLeft + 1U];
      yL_0d0 = table[iLeft];
      if (bpLeftVar_0 >= yL_0d0) {
        bpLeftVar_0 = (int32_T)mul_u32_sr32(fractions[0U], (uint32_T)bpLeftVar_0
          - (uint32_T)yL_0d0) + yL_0d0;
      } else {
        bpLeftVar_0 = yL_0d0 - (int32_T)mul_u32_sr32(fractions[0U], (uint32_T)
          yL_0d0 - (uint32_T)bpLeftVar_0);
      }
    }

    if (bpLeftVar_0 >= y) {
      y += (int32_T)mul_u32_sr32(frac, (uint32_T)bpLeftVar_0 - (uint32_T)y);
    } else {
      y -= (int32_T)mul_u32_sr32(frac, (uint32_T)y - (uint32_T)bpLeftVar_0);
    }
  }

  return y;
}

int32_T look1_is32lu32n32Du32_pbinlcase(int32_T u0, const int32_T bp0[], const
  int32_T table[], uint32_T prevIndex[], uint32_T maxIndex)
{
  int32_T bpLeftVar;
  int32_T y;
  uint32_T bpIdx;
  uint32_T frac;

  /* Column-major Lookup 1-D
     Search method: 'binary'
     Use previous index: 'on'
     Interpolation method: 'Linear point-slope'
     Extrapolation method: 'Clip'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'off'
     Rounding mode: 'simplest'
   */
  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Clip'
     Use previous index: 'on'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'off'
     Rounding mode: 'simplest'
   */
  if (u0 <= bp0[0U]) {
    bpIdx = 0U;
    frac = 0U;
  } else if (u0 < bp0[maxIndex]) {
    uint32_T found;
    uint32_T iRght;

    /* Binary Search using Previous Index */
    bpIdx = prevIndex[0U];
    frac = 0U;
    iRght = maxIndex;
    found = 0U;
    while (found == 0U) {
      if (u0 < bp0[bpIdx]) {
        iRght = bpIdx - 1U;
        bpIdx = ((bpIdx + frac) - 1U) >> 1U;
      } else if (u0 < bp0[bpIdx + 1U]) {
        found = 1U;
      } else {
        frac = bpIdx + 1U;
        bpIdx = ((bpIdx + iRght) + 1U) >> 1U;
      }
    }

    bpLeftVar = bp0[bpIdx];
    frac = div_nzp_repeat_u32((uint32_T)u0 - (uint32_T)bpLeftVar, (uint32_T)
      bp0[bpIdx + 1U] - (uint32_T)bpLeftVar, 32U);
  } else {
    bpIdx = maxIndex;
    frac = 0U;
  }

  prevIndex[0U] = bpIdx;

  /* Column-major Interpolation 1-D
     Interpolation method: 'Linear point-slope'
     Use last breakpoint for index at or above upper limit: 'on'
     Rounding mode: 'simplest'
     Overflow mode: 'wrapping'
   */
  if (bpIdx == maxIndex) {
    y = table[bpIdx];
  } else {
    int32_T yL_0d0;
    bpLeftVar = table[bpIdx + 1U];
    yL_0d0 = table[bpIdx];
    if (bpLeftVar >= yL_0d0) {
      y = (int32_T)mul_u32_sr32(frac, (uint32_T)bpLeftVar - (uint32_T)yL_0d0) +
        yL_0d0;
    } else {
      y = yL_0d0 - (int32_T)mul_u32_sr32(frac, (uint32_T)yL_0d0 - (uint32_T)
        bpLeftVar);
    }
  }

  return y;
}
uint32_T look2_is16s32lu32n32tu_bvHCJPGn(int16_T u0, int32_T u1, const int16_T
  bp0[], const int32_T bp1[], const uint32_T table[], uint32_T prevIndex[],
  const uint32_T maxIndex[], uint32_T stride)
{
  uint32_T bpIndices[2];
  uint32_T fractions[2];
  uint32_T bpIdx;
  uint32_T found;
  uint32_T frac;
  uint32_T iLeft;
  uint32_T iRght;
  uint32_T y;

  /* Column-major Lookup 2-D
     Canonical function name: look2_is16s32lu32n32tu32_pbinlcase
     Search method: 'binary'
     Use previous index: 'on'
     Interpolation method: 'Linear point-slope'
     Extrapolation method: 'Clip'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'off'
     Rounding mode: 'simplest'
   */
  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Clip'
     Use previous index: 'on'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'off'
     Rounding mode: 'simplest'
   */
  if (u0 <= bp0[0U]) {
    bpIdx = 0U;
    frac = 0U;
  } else if (u0 < bp0[maxIndex[0U]]) {
    /* Binary Search using Previous Index */
    bpIdx = prevIndex[0U];
    iLeft = 0U;
    iRght = maxIndex[0U];
    found = 0U;
    while (found == 0U) {
      if (u0 < bp0[bpIdx]) {
        iRght = bpIdx - 1U;
        bpIdx = ((bpIdx + iLeft) - 1U) >> 1U;
      } else if (u0 < bp0[bpIdx + 1U]) {
        found = 1U;
      } else {
        iLeft = bpIdx + 1U;
        bpIdx = ((bpIdx + iRght) + 1U) >> 1U;
      }
    }

    int16_T bpLeftVar;
    bpLeftVar = bp0[bpIdx];
    frac = div_nzp_repeat_u32((uint32_T)(u0 - bpLeftVar) << 16, (uint32_T)
      (bp0[bpIdx + 1U] - bpLeftVar), 16U);
  } else {
    bpIdx = maxIndex[0U];
    frac = 0U;
  }

  prevIndex[0U] = bpIdx;
  fractions[0U] = frac;
  bpIndices[0U] = bpIdx;

  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Clip'
     Use previous index: 'on'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'off'
     Rounding mode: 'simplest'
   */
  if (u1 <= bp1[0U]) {
    bpIdx = 0U;
    frac = 0U;
  } else if (u1 < bp1[maxIndex[1U]]) {
    /* Binary Search using Previous Index */
    bpIdx = prevIndex[1U];
    iLeft = 0U;
    iRght = maxIndex[1U];
    found = 0U;
    while (found == 0U) {
      if (u1 < bp1[bpIdx]) {
        iRght = bpIdx - 1U;
        bpIdx = ((bpIdx + iLeft) - 1U) >> 1U;
      } else if (u1 < bp1[bpIdx + 1U]) {
        found = 1U;
      } else {
        iLeft = bpIdx + 1U;
        bpIdx = ((bpIdx + iRght) + 1U) >> 1U;
      }
    }

    int32_T bpLeftVar_0;
    bpLeftVar_0 = bp1[bpIdx];
    frac = div_nzp_repeat_u32((uint32_T)u1 - (uint32_T)bpLeftVar_0, (uint32_T)
      bp1[bpIdx + 1U] - (uint32_T)bpLeftVar_0, 32U);
  } else {
    bpIdx = maxIndex[1U];
    frac = 0U;
  }

  prevIndex[1U] = bpIdx;

  /* Column-major Interpolation 2-D
     Interpolation method: 'Linear point-slope'
     Use last breakpoint for index at or above upper limit: 'on'
     Rounding mode: 'simplest'
     Overflow mode: 'wrapping'
   */
  found = bpIdx * stride + bpIndices[0U];
  if (bpIndices[0U] == maxIndex[0U]) {
    y = table[found];
  } else {
    iLeft = table[found + 1U];
    iRght = table[found];
    if (iLeft >= iRght) {
      y = mul_u32_sr32(fractions[0U], iLeft - iRght) + iRght;
    } else {
      y = iRght - mul_u32_sr32(fractions[0U], iRght - iLeft);
    }
  }

  if (bpIdx == maxIndex[1U]) {
  } else {
    iLeft = found + stride;
    if (bpIndices[0U] == maxIndex[0U]) {
      iLeft = table[iLeft];
    } else {
      iRght = table[iLeft + 1U];
      iLeft = table[iLeft];
      if (iRght >= iLeft) {
        iLeft += mul_u32_sr32(fractions[0U], iRght - iLeft);
      } else {
        iLeft -= mul_u32_sr32(fractions[0U], iLeft - iRght);
      }
    }

    if (iLeft >= y) {
      y += mul_u32_sr32(frac, iLeft - y);
    } else {
      y -= mul_u32_sr32(frac, y - iLeft);
    }
  }

  return y;
}

uint16_T look2_is16s32lu16n16tu_EvbysC3W(int16_T u0, int32_T u1, const int16_T
  bp0[], const int32_T bp1[], const uint16_T table[], uint32_T prevIndex[],
  const uint32_T maxIndex[], uint32_T stride)
{
  uint32_T bpIndices[2];
  uint32_T bpIdx;
  uint32_T found;
  uint32_T iLeft;
  uint32_T iRght;
  uint16_T fractions[2];
  uint16_T frac;
  uint16_T y;
  uint16_T yL_0d0;
  uint16_T yR_0d0;

  /* Column-major Lookup 2-D
     Canonical function name: look2_is16s32lu16n16tu16_pbinlcase
     Search method: 'binary'
     Use previous index: 'on'
     Interpolation method: 'Linear point-slope'
     Extrapolation method: 'Clip'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'off'
     Rounding mode: 'simplest'
   */
  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Clip'
     Use previous index: 'on'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'off'
     Rounding mode: 'simplest'
   */
  if (u0 <= bp0[0U]) {
    bpIdx = 0U;
    frac = 0U;
  } else if (u0 < bp0[maxIndex[0U]]) {
    /* Binary Search using Previous Index */
    bpIdx = prevIndex[0U];
    iLeft = 0U;
    iRght = maxIndex[0U];
    found = 0U;
    while (found == 0U) {
      if (u0 < bp0[bpIdx]) {
        iRght = bpIdx - 1U;
        bpIdx = ((bpIdx + iLeft) - 1U) >> 1U;
      } else if (u0 < bp0[bpIdx + 1U]) {
        found = 1U;
      } else {
        iLeft = bpIdx + 1U;
        bpIdx = ((bpIdx + iRght) + 1U) >> 1U;
      }
    }

    int16_T bpLeftVar;
    bpLeftVar = bp0[bpIdx];
    frac = (uint16_T)(((uint32_T)(u0 - bpLeftVar) << 16) / (uint32_T)(bp0[bpIdx
      + 1U] - bpLeftVar));
  } else {
    bpIdx = maxIndex[0U];
    frac = 0U;
  }

  prevIndex[0U] = bpIdx;
  fractions[0U] = frac;
  bpIndices[0U] = bpIdx;

  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Clip'
     Use previous index: 'on'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'off'
     Rounding mode: 'simplest'
   */
  if (u1 <= bp1[0U]) {
    bpIdx = 0U;
    frac = 0U;
  } else if (u1 < bp1[maxIndex[1U]]) {
    /* Binary Search using Previous Index */
    bpIdx = prevIndex[1U];
    iLeft = 0U;
    iRght = maxIndex[1U];
    found = 0U;
    while (found == 0U) {
      if (u1 < bp1[bpIdx]) {
        iRght = bpIdx - 1U;
        bpIdx = ((bpIdx + iLeft) - 1U) >> 1U;
      } else if (u1 < bp1[bpIdx + 1U]) {
        found = 1U;
      } else {
        iLeft = bpIdx + 1U;
        bpIdx = ((bpIdx + iRght) + 1U) >> 1U;
      }
    }

    int32_T bpLeftVar_0;
    bpLeftVar_0 = bp1[bpIdx];
    frac = (uint16_T)div_nzp_repeat_u32((uint32_T)u1 - (uint32_T)bpLeftVar_0,
      (uint32_T)bp1[bpIdx + 1U] - (uint32_T)bpLeftVar_0, 16U);
  } else {
    bpIdx = maxIndex[1U];
    frac = 0U;
  }

  prevIndex[1U] = bpIdx;

  /* Column-major Interpolation 2-D
     Interpolation method: 'Linear point-slope'
     Use last breakpoint for index at or above upper limit: 'on'
     Rounding mode: 'simplest'
     Overflow mode: 'wrapping'
   */
  iLeft = bpIdx * stride + bpIndices[0U];
  if (bpIndices[0U] == maxIndex[0U]) {
    y = table[iLeft];
  } else {
    yR_0d0 = table[iLeft + 1U];
    yL_0d0 = table[iLeft];
    if (yR_0d0 >= yL_0d0) {
      y = (uint16_T)((uint32_T)(uint16_T)(((uint32_T)(uint16_T)((uint32_T)yR_0d0
        - yL_0d0) * fractions[0U]) >> 16) + yL_0d0);
    } else {
      y = (uint16_T)((uint32_T)yL_0d0 - (uint16_T)(((uint32_T)(uint16_T)
        ((uint32_T)yL_0d0 - yR_0d0) * fractions[0U]) >> 16));
    }
  }

  if (bpIdx == maxIndex[1U]) {
  } else {
    iLeft += stride;
    if (bpIndices[0U] == maxIndex[0U]) {
      yR_0d0 = table[iLeft];
    } else {
      yR_0d0 = table[iLeft + 1U];
      yL_0d0 = table[iLeft];
      if (yR_0d0 >= yL_0d0) {
        yR_0d0 = (uint16_T)((uint32_T)(uint16_T)(((uint32_T)(uint16_T)((uint32_T)
          yR_0d0 - yL_0d0) * fractions[0U]) >> 16) + yL_0d0);
      } else {
        yR_0d0 = (uint16_T)((uint32_T)yL_0d0 - (uint16_T)(((uint32_T)(uint16_T)
          ((uint32_T)yL_0d0 - yR_0d0) * fractions[0U]) >> 16));
      }
    }

    if (yR_0d0 >= y) {
      y = (uint16_T)((uint32_T)(uint16_T)(((uint32_T)(uint16_T)((uint32_T)yR_0d0
        - y) * frac) >> 16) + y);
    } else {
      y = (uint16_T)((uint32_T)y - (uint16_T)(((uint32_T)(uint16_T)((uint32_T)y
        - yR_0d0) * frac) >> 16));
    }
  }

  return y;
}



int32_T look2_is16s32lu32n32ts_7E7IgjCI(int16_T u0, int32_T u1, const int16_T
  bp0[], const int32_T bp1[], const int32_T table[], uint32_T prevIndex[], const
  uint32_T maxIndex[], uint32_T stride)
{
  int32_T bpLeftVar_0;
  int32_T y;
  int32_T yL_0d0;
  uint32_T bpIndices[2];
  uint32_T fractions[2];
  uint32_T bpIdx;
  uint32_T found;
  uint32_T frac;
  uint32_T iLeft;

  /* Column-major Lookup 2-D
     Canonical function name: look2_is16s32lu32n32ts32Du32du32_pbinlcafe
     Search method: 'binary'
     Use previous index: 'on'
     Interpolation method: 'Linear point-slope'
     Extrapolation method: 'Clip'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'off'
     Rounding mode: 'floor'
   */
  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Clip'
     Use previous index: 'on'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'off'
     Rounding mode: 'floor'
   */
  if (u0 <= bp0[0U]) {
    bpIdx = 0U;
    frac = 0U;
  } else if (u0 < bp0[maxIndex[0U]]) {
    /* Binary Search using Previous Index */
    bpIdx = prevIndex[0U];
    iLeft = 0U;
    frac = maxIndex[0U];
    found = 0U;
    while (found == 0U) {
      if (u0 < bp0[bpIdx]) {
        frac = bpIdx - 1U;
        bpIdx = ((bpIdx + iLeft) - 1U) >> 1U;
      } else if (u0 < bp0[bpIdx + 1U]) {
        found = 1U;
      } else {
        iLeft = bpIdx + 1U;
        bpIdx = ((bpIdx + frac) + 1U) >> 1U;
      }
    }

    int16_T bpLeftVar;
    bpLeftVar = bp0[bpIdx];
    frac = div_nzp_repeat_u32((uint32_T)(u0 - bpLeftVar) << 16, (uint32_T)
      (bp0[bpIdx + 1U] - bpLeftVar), 16U);
  } else {
    bpIdx = maxIndex[0U];
    frac = 0U;
  }

  prevIndex[0U] = bpIdx;
  fractions[0U] = frac;
  bpIndices[0U] = bpIdx;

  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Clip'
     Use previous index: 'on'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'off'
     Rounding mode: 'floor'
   */
  if (u1 <= bp1[0U]) {
    bpIdx = 0U;
    frac = 0U;
  } else if (u1 < bp1[maxIndex[1U]]) {
    /* Binary Search using Previous Index */
    bpIdx = prevIndex[1U];
    iLeft = 0U;
    frac = maxIndex[1U];
    found = 0U;
    while (found == 0U) {
      if (u1 < bp1[bpIdx]) {
        frac = bpIdx - 1U;
        bpIdx = ((bpIdx + iLeft) - 1U) >> 1U;
      } else if (u1 < bp1[bpIdx + 1U]) {
        found = 1U;
      } else {
        iLeft = bpIdx + 1U;
        bpIdx = ((bpIdx + frac) + 1U) >> 1U;
      }
    }

    bpLeftVar_0 = bp1[bpIdx];
    frac = div_nzp_repeat_u32((uint32_T)u1 - (uint32_T)bpLeftVar_0, (uint32_T)
      bp1[bpIdx + 1U] - (uint32_T)bpLeftVar_0, 32U);
  } else {
    bpIdx = maxIndex[1U];
    frac = 0U;
  }

  prevIndex[1U] = bpIdx;

  /* Column-major Interpolation 2-D
     Interpolation method: 'Linear point-slope'
     Use last breakpoint for index at or above upper limit: 'on'
     Rounding mode: 'floor'
     Overflow mode: 'wrapping'
   */
  iLeft = bpIdx * stride + bpIndices[0U];
  if (bpIndices[0U] == maxIndex[0U]) {
    y = table[iLeft];
  } else {
    bpLeftVar_0 = table[iLeft + 1U];
    yL_0d0 = table[iLeft];
    if (bpLeftVar_0 >= yL_0d0) {
      y = (int32_T)mul_u32_sr32(fractions[0U], (uint32_T)bpLeftVar_0 - (uint32_T)
        yL_0d0) + yL_0d0;
    } else {
      y = yL_0d0 - (int32_T)mul_u32_sr32(fractions[0U], (uint32_T)yL_0d0 -
        (uint32_T)bpLeftVar_0);
    }
  }

  if (bpIdx == maxIndex[1U]) {
  } else {
    iLeft += stride;
    if (bpIndices[0U] == maxIndex[0U]) {
      bpLeftVar_0 = table[iLeft];
    } else {
      bpLeftVar_0 = table[iLeft + 1U];
      yL_0d0 = table[iLeft];
      if (bpLeftVar_0 >= yL_0d0) {
        bpLeftVar_0 = (int32_T)mul_u32_sr32(fractions[0U], (uint32_T)bpLeftVar_0
          - (uint32_T)yL_0d0) + yL_0d0;
      } else {
        bpLeftVar_0 = yL_0d0 - (int32_T)mul_u32_sr32(fractions[0U], (uint32_T)
          yL_0d0 - (uint32_T)bpLeftVar_0);
      }
    }

    if (bpLeftVar_0 >= y) {
      y += (int32_T)mul_u32_sr32(frac, (uint32_T)bpLeftVar_0 - (uint32_T)y);
    } else {
      y -= (int32_T)mul_u32_sr32(frac, (uint32_T)y - (uint32_T)bpLeftVar_0);
    }
  }

  return y;
}

void sLong2MultiWord(int32_T u, uint32_T y[], int32_T n)
{
  int32_T i;
  uint32_T yi;
  y[0] = (uint32_T)u;
  yi = u < 0 ? MAX_uint32_T : 0U;
  for (i = 1; i < n; i++) {
    y[i] = yi;
  }
}

boolean_T sMultiWordLe(const uint32_T u1[], const uint32_T u2[], int32_T n)
{
  return sMultiWordCmp(u1, u2, n) <= 0;
}

int32_T sMultiWordCmp(const uint32_T u1[], const uint32_T u2[], int32_T n)
{
  int32_T y;
  uint32_T su1;
  su1 = u1[n - 1] & 2147483648U;
  if ((u2[n - 1] & 2147483648U) != su1) {
    y = su1 != 0U ? -1 : 1;
  } else {
    int32_T i;
    y = 0;
    i = n;
    while ((y == 0) && (i > 0)) {
      uint32_T u2i;
      i--;
      su1 = u1[i];
      u2i = u2[i];
      if (su1 != u2i) {
        y = su1 > u2i ? 1 : -1;
      }
    }
  }

  return y;
}

void sMultiWordMul(const uint32_T u1[], int32_T n1, const uint32_T u2[], int32_T
                   n2, uint32_T y[], int32_T n)
{
  int32_T i;
  int32_T j;
  int32_T k;
  uint32_T cb;
  uint32_T cb1;
  uint32_T yk;
  boolean_T isNegative1;
  boolean_T isNegative2;
  isNegative1 = ((u1[n1 - 1] & 2147483648U) != 0U);
  isNegative2 = ((u2[n2 - 1] & 2147483648U) != 0U);
  cb1 = 1U;

  /* Initialize output to zero */
  for (k = 0; k < n; k++) {
    y[k] = 0U;
  }

  for (i = 0; i < n1; i++) {
    int32_T ni;
    uint32_T a0;
    uint32_T a1;
    uint32_T cb2;
    uint32_T u1i;
    cb = 0U;
    u1i = u1[i];
    if (isNegative1) {
      u1i = ~u1i + cb1;
      cb1 = (uint32_T)(u1i < cb1);
    }

    a1 = u1i >> 16U;
    a0 = u1i & 65535U;
    cb2 = 1U;
    ni = n - i;
    ni = n2 <= ni ? n2 : ni;
    k = i;
    for (j = 0; j < ni; j++) {
      uint32_T b1;
      uint32_T w01;
      uint32_T w10;
      u1i = u2[j];
      if (isNegative2) {
        u1i = ~u1i + cb2;
        cb2 = (uint32_T)(u1i < cb2);
      }

      b1 = u1i >> 16U;
      u1i &= 65535U;
      w10 = a1 * u1i;
      w01 = a0 * b1;
      yk = y[k] + cb;
      cb = (uint32_T)(yk < cb);
      u1i *= a0;
      yk += u1i;
      cb += (uint32_T)(yk < u1i);
      u1i = w10 << 16U;
      yk += u1i;
      cb += (uint32_T)(yk < u1i);
      u1i = w01 << 16U;
      yk += u1i;
      cb += (uint32_T)(yk < u1i);
      y[k] = yk;
      cb += w10 >> 16U;
      cb += w01 >> 16U;
      cb += a1 * b1;
      k++;
    }

    if (k < n) {
      y[k] = cb;
    }
  }

  /* Apply sign */
  if (isNegative1 != isNegative2) {
    cb = 1U;
    for (k = 0; k < n; k++) {
      yk = ~y[k] + cb;
      y[k] = yk;
      cb = (uint32_T)(yk < cb);
    }
  }
}

int32_T div_nde_s32_floor(int32_T numerator, int32_T denominator)
{
  return (((numerator < 0) != (denominator < 0)) && (numerator % denominator !=
           0) ? -1 : 0) + numerator / denominator;
}

void mul_wide_s32(int32_T in0, int32_T in1, uint32_T *ptrOutBitsHi, uint32_T
                  *ptrOutBitsLo)
{
  uint32_T absIn0;
  uint32_T absIn1;
  uint32_T in0Hi;
  uint32_T in0Lo;
  uint32_T in1Hi;
  uint32_T productHiLo;
  uint32_T productLoHi;
  absIn0 = in0 < 0 ? ~(uint32_T)in0 + 1U : (uint32_T)in0;
  absIn1 = in1 < 0 ? ~(uint32_T)in1 + 1U : (uint32_T)in1;
  in0Hi = absIn0 >> 16U;
  in0Lo = absIn0 & 65535U;
  in1Hi = absIn1 >> 16U;
  absIn0 = absIn1 & 65535U;
  productHiLo = in0Hi * absIn0;
  productLoHi = in0Lo * in1Hi;
  absIn0 *= in0Lo;
  absIn1 = 0U;
  in0Lo = (productLoHi << 16U) + absIn0;
  if (in0Lo < absIn0) {
    absIn1 = 1U;
  }

  absIn0 = in0Lo;
  in0Lo += productHiLo << 16U;
  if (in0Lo < absIn0) {
    absIn1++;
  }

  absIn0 = (((productLoHi >> 16U) + (productHiLo >> 16U)) + in0Hi * in1Hi) +
    absIn1;
  if ((in0 != 0) && ((in1 != 0) && ((in0 > 0) != (in1 > 0)))) {
    absIn0 = ~absIn0;
    in0Lo = ~in0Lo;
    in0Lo++;
    if (in0Lo == 0U) {
      absIn0++;
    }
  }

  *ptrOutBitsHi = absIn0;
  *ptrOutBitsLo = in0Lo;
}

int32_T mul_s32_loSR_sat(int32_T a, int32_T b, uint32_T aShift)
{
  int32_T result;
  uint32_T u32_chi;
  uint32_T u32_clo;
  mul_wide_s32(a, b, &u32_chi, &u32_clo);
  u32_clo = u32_chi << (32U - aShift) | u32_clo >> aShift;
  u32_chi = (uint32_T)((int32_T)u32_chi >> aShift);
  if (((int32_T)u32_chi > 0) || ((u32_chi == 0U) && (u32_clo >= 2147483648U))) {
    result = MAX_int32_T;
  } else if (((int32_T)u32_chi < -1) || (((int32_T)u32_chi == -1) && (u32_clo <
               2147483648U))) {
    result = MIN_int32_T;
  } else {
    result = (int32_T)u32_clo;
  }

  return result;
}

int32_T mul_s32_loSR(int32_T a, int32_T b, uint32_T aShift)
{
  uint32_T u32_chi;
  uint32_T u32_clo;
  mul_wide_s32(a, b, &u32_chi, &u32_clo);
  u32_clo = u32_chi << (32U - aShift) | u32_clo >> aShift;
  return (int32_T)u32_clo;
}

int32_T mul_s32_hiSR(int32_T a, int32_T b, uint32_T aShift)
{
  uint32_T u32_chi;
  uint32_T u32_clo;
  mul_wide_s32(a, b, &u32_chi, &u32_clo);
  return (int32_T)u32_chi >> aShift;
}

void mul_wide_u32(uint32_T in0, uint32_T in1, uint32_T *ptrOutBitsHi, uint32_T
                  *ptrOutBitsLo)
{
  uint32_T in0Hi;
  uint32_T in0Lo;
  uint32_T in1Hi;
  uint32_T in1Lo;
  uint32_T outBitsLo;
  uint32_T productHiLo;
  uint32_T productLoHi;
  in0Hi = in0 >> 16U;
  in0Lo = in0 & 65535U;
  in1Hi = in1 >> 16U;
  in1Lo = in1 & 65535U;
  productHiLo = in0Hi * in1Lo;
  productLoHi = in0Lo * in1Hi;
  in0Lo *= in1Lo;
  in1Lo = 0U;
  outBitsLo = (productLoHi << 16U) + in0Lo;
  if (outBitsLo < in0Lo) {
    in1Lo = 1U;
  }

  in0Lo = outBitsLo;
  outBitsLo += productHiLo << 16U;
  if (outBitsLo < in0Lo) {
    in1Lo++;
  }

  *ptrOutBitsHi = (((productLoHi >> 16U) + (productHiLo >> 16U)) + in0Hi * in1Hi)
    + in1Lo;
  *ptrOutBitsLo = outBitsLo;
}

uint32_T mul_u32_sr32(uint32_T a, uint32_T b)
{
  uint32_T result;
  uint32_T u32_clo;
  mul_wide_u32(a, b, &result, &u32_clo);
  return result;
}

uint32_T div_nzp_repeat_u32(uint32_T numerator, uint32_T denominator, uint32_T
  nRepeatSub)
{
  uint32_T iRepeatSub;
  uint32_T localNumerator;
  uint32_T quotient;
  quotient = numerator / denominator;
  localNumerator = numerator % denominator;
  for (iRepeatSub = 0U; iRepeatSub < nRepeatSub; iRepeatSub++) {
    boolean_T numeratorExtraBit;
    numeratorExtraBit = (localNumerator >= 2147483648U);
    localNumerator <<= 1U;
    quotient <<= 1U;
    if (numeratorExtraBit || (localNumerator >= denominator)) {
      quotient++;
      localNumerator -= denominator;
    }
  }

  return quotient;
}

/* System initialize for trigger system: '<S5>/SOHC' */
void SOHC_Init(void)
{
  SOH_capacity_mAh_s = P_Capacity_mAh;
}

/* Output and update for trigger system: '<S5>/SOHC' */
void SOHC(void)
{
  if (SOC_OCVUpd_flg && (SOHC_Trig_ZCE_s != POS_ZCSIG)) {
    SOH_capacity_mAh_s = P_Capacity_mAh;
  }

  SOHC_Trig_ZCE_s = (ZCSigState)SOC_OCVUpd_flg;
}

/* System initialize for function-call system: '<S1>/SOH' */
void SOH_Init(void)
{
  SOHC_Init();
}

/* Output and update for function-call system: '<S1>/SOH' */
void SOH(void)
{
  SOH_Resistance_mOhm = 0;
  SOHC();
  Switch_s = 1;
}
/* Model step function */
/* Model initialize function */
void Init(void)
{
  SOHC_Trig_ZCE_s = POS_ZCSIG;
  voltage_time_judge_Trig_ZCE_s = POS_ZCSIG;
  TimeSum_Reset_ZCE_s = POS_ZCSIG;
  SOH_Init();
  SOC_Init();
  SOCPack_Init();
  SOC_Enable();
}
void Cyclic(void)
{
	if(Soc_Initialed)
	{
	  SigPr_CellVolts_mV = SigPr_CellVolts_mV_s;
	  SigPr_CellTemps_C = SigPr_CellTemps_C_s;
	  SigPr_PackCurr_mA = SigPr_PackCurr_mA_s;
	  BMS_SampleTime_ms = P_SampleTime_ms;
	  SOH();
	  SOC();
	  SOCPack();
	  SOH_SOHR_pct = Switch_s * 100;

	  /* Update absolute time for base rate */
	  /* The "clockTick0" counts the number of times the code of this task has
	   * been executed. The resolution of this integer timer is 1.0, which is the step size
	   * of the task. Size of "clockTick0" ensures timer will not overflow during the
	   * application lifespan selected.
	   */
	  M_s->Timing.clockTick0++;
	}
	else
	{
		Soc_Initialed = true;
		Init();
		BMS_NvmSOC_mpct = gd->SOC_RawSOC_mpct;
		printk("\r\n gauge initialed");
	}
}

