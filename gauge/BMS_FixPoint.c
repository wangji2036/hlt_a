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
int32_T P_AtRateCurrent_mA = 5000;     /* Variable: P_AtRateCurrent_mA
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
int32_T P_AtRateCurrent_mA = 5000;     /* Variable: P_AtRateCurrent_mA
                                        * Referenced by: '<S4>/At_Rate_Current_mA'
                                        */
int32_T P_EmptyVoltage_mV = 6000;      /* Variable: P_EmptyVoltage_mV
                                        * Referenced by: '<S4>/Empty_Voltage_mV'
                                        */
const int32_T P_OcvSOCDsg_mpct[96] = { 0, 0, 0, 2385, 2385, 2385, 4771, 4771, 4771,
  7157, 7157, 7157, 9542, 9542, 9542, 11928, 11928, 11928, 14314, 14314, 14314,
  16700, 16700, 16700, 19085, 19085, 19085, 21470, 21470, 21470, 23856, 23856,
  23856, 28623, 28623, 28623, 33389, 33389, 33389, 38156, 38156, 38156, 42924,
  42924, 42924, 47691, 47691, 47691, 52456, 52456, 52456, 57222, 57222, 57222,
  61988, 61988, 61988, 66754, 66754, 66754, 71519, 71519, 71519, 76285, 76285,
  76285, 78671, 78671, 78671, 81057, 81057, 81057, 83442, 83442, 83442, 85828,
  85828, 85828, 88214, 88214, 88214, 90599, 90599, 90599, 92984, 92984, 92984,
  95370, 95370, 95370, 97756, 97756, 97756, 100000, 100000, 100000 } ;/* Variable: P_OcvSOCDsg_mpct
                                                                      * Referenced by:
                                                                      *   '<S33>/OCV_DSG'
                                                                      *   '<S12>/OCV_DSG'
                                                                      *   '<S19>/OCV_DSG'
                                                                      */

int32_T P_SOCAxis_mpct[32] = { 0, 2385, 4771, 7157, 9542, 11928, 14314, 16700,
  19085, 21470, 23856, 28623, 33389, 38156, 42924, 47691, 52456, 57222, 61988,
  66754, 71519, 76285, 78671, 81057, 83442, 85828, 88214, 90599, 92984, 95370,
  97756, 100000 } ;                    /* Variable: P_SOCAxis_mpct
                                        * Referenced by:
                                        *   '<S33>/DCIR_Discharge'
                                        *   '<S23>/DCIR_Discharge'
                                        *   '<S23>/OCV_Discharge'
                                        *   '<S23>/R0_Discharge'
                                        */

int32_T P_SOCSlope_mpctPermV[32] = { 5, 6, 10, 28, 55, 49, 41, 42, 47, 53, 57,
  85, 106, 101, 88, 70, 56, 46, 41, 60, 66, 55, 51, 50, 53, 66, 95, 140, 126, 82,
  48, 27 } ;                           /* Variable: P_SOCSlope_mpctPermV
                                        * Referenced by: '<S22>/SOC_Slope'
                                        */
#endif
int32_T P_SocDeviationAxis_mpct[7] = { 0, 1000, 2000, 8000, 10000, 20000, 100000
} ;                                    /* Variable: P_SocDeviationAxis_mpct
                                        * Referenced by: '<S19>/1-D Lookup Table1'
                                        */

int32_T P_SocDeviationCorrect_upct[7] = { 75, 7500, 20000, 50000, 100000, 800000,
  1000000 } ;                          /* Variable: P_SocDeviationCorrect_upct
                                        * Referenced by: '<S19>/1-D Lookup Table1'
                                        */

int32_T P_SocRangeAxis_mpct[5] = { 0, 10000, 20000, 25000, 100000 } ;/* Variable: P_SocRangeAxis_mpct
                                                                      * Referenced by: '<S19>/adaption in SOC range'
                                                                      */

int32_T P_SocRangeCorrect_mpct[5] = { 10, 10, 150, 800, 1000 } ;/* Variable: P_SocRangeCorrect_mpct
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
const uint32_T P_DcirDsg_mOhm[96] = { 138U, 138U, 138U, 108U, 108U, 108U, 95U, 95U,
  95U, 90U, 90U, 90U, 79U, 79U, 79U, 73U, 73U, 73U, 69U, 69U, 69U, 65U, 65U, 65U,
  64U, 64U, 64U, 64U, 64U, 64U, 60U, 60U, 60U, 58U, 58U, 58U, 60U, 60U, 60U, 60U,
  60U, 60U, 62U, 62U, 62U, 63U, 63U, 63U, 63U, 63U, 63U, 64U, 64U, 64U, 54U, 54U,
  54U, 53U, 53U, 53U, 55U, 55U, 55U, 57U, 57U, 57U, 61U, 61U, 61U, 61U, 61U, 61U,
  64U, 64U, 64U, 64U, 64U, 64U, 64U, 64U, 64U, 65U, 65U, 65U, 70U, 70U, 70U, 75U,
  75U, 75U, 84U, 84U, 84U, 132U, 132U, 132U } ;/* Variable: P_DcirDsg_mOhm
                                                * Referenced by: '<S23>/DCIR_Discharge'
                                                */

const uint32_T P_R0Dsg_mOhm[96] = { 46U, 46U, 46U, 46U, 46U, 46U, 47U, 47U, 47U, 45U,
  45U, 45U, 45U, 45U, 45U, 47U, 47U, 47U, 44U, 44U, 44U, 44U, 44U, 44U, 43U, 43U,
  43U, 43U, 43U, 43U, 43U, 43U, 43U, 41U, 41U, 41U, 40U, 40U, 40U, 40U, 40U, 40U,
  40U, 40U, 40U, 40U, 40U, 40U, 39U, 39U, 39U, 40U, 40U, 40U, 40U, 40U, 40U, 39U,
  39U, 39U, 39U, 39U, 39U, 39U, 39U, 39U, 39U, 39U, 39U, 39U, 39U, 39U, 40U, 40U,
  40U, 39U, 39U, 39U, 42U, 42U, 42U, 40U, 40U, 40U, 42U, 42U, 42U, 43U, 43U, 43U,
  43U, 43U, 43U, 42U, 42U, 42U } ;     /* Variable: P_R0Dsg_mOhm
                                        * Referenced by: '<S23>/R0_Discharge'
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
uint16_T P_Capacity_mAh = 10487U;      /* Variable: P_Capacity_mAh
                                        * Referenced by:
                                        *   '<S40>/SOH_capacity_mAh'
                                        *   '<S40>/Constant3'
                                        */
uint16_T P_OCVAxis_mV[32] = { 6083U, 6498U, 6726U, 6811U, 6854U, 6903U, 6961U,
  7018U, 7069U, 7114U, 7156U, 7212U, 7257U, 7304U, 7358U, 7426U, 7511U, 7615U,
  7730U, 7809U, 7881U, 7968U, 8015U, 8063U, 8108U, 8144U, 8169U, 8186U, 8205U,
  8234U, 8284U, 8366U } ;              /* Variable: P_OCVAxis_mV
                                        * Referenced by: '<S12>/OCV_DSG'
                                        */

const uint16_T P_OCVDsg_mV[96] = { 6083U, 6083U, 6083U, 6498U, 6498U, 6498U, 6726U,
  6726U, 6726U, 6811U, 6811U, 6811U, 6854U, 6854U, 6854U, 6903U, 6903U, 6903U,
  6961U, 6961U, 6961U, 7018U, 7018U, 7018U, 7069U, 7069U, 7069U, 7114U, 7114U,
  7114U, 7156U, 7156U, 7156U, 7212U, 7212U, 7212U, 7257U, 7257U, 7257U, 7304U,
  7304U, 7304U, 7358U, 7358U, 7358U, 7426U, 7426U, 7426U, 7511U, 7511U, 7511U,
  7615U, 7615U, 7615U, 7730U, 7730U, 7730U, 7809U, 7809U, 7809U, 7881U, 7881U,
  7881U, 7968U, 7968U, 7968U, 8015U, 8015U, 8015U, 8063U, 8063U, 8063U, 8108U,
  8108U, 8108U, 8144U, 8144U, 8144U, 8169U, 8169U, 8169U, 8186U, 8186U, 8186U,
  8205U, 8205U, 8205U, 8234U, 8234U, 8234U, 8284U, 8284U, 8284U, 8366U, 8366U,
  8366U } ;                            /* Variable: P_OCVDsg_mV
                                        * Referenced by: '<S23>/OCV_Discharge'
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

