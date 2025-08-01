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
uint32_T SOC_ModelR0_mOhm;             /* '<S24>/Switch3' */
uint32_T SOC_ModelDCIR_mOhm;           /* '<S24>/Switch2' */
int32_T SOCPack_UdEmptySOC_mpct;       /* '<S34>/SOCPack_UdEmptySOC_mpct' */
int32_T SOCPack_SatuarationSoc_mpct;   /* '<S34>/Saturation' */
int32_T SOCPack_EmptyDcr_mOhm;         /* '<S34>/DCIR_Discharge' */
int32_T SOCPack_EmptyU_mV;             /* '<S34>/Add2' */
int32_T SOCPack_PreEmptySOC_mpct;      /* '<S34>/Saturation4' */
int32_T SOC_VirtOCVSOC_mpct;           /* '<S20>/Saturation1' */
int32_T SOC_VirtualOCV_mV;             /* '<S21>/UdVirtualOcv' */
uint16_T BMS_SampleTime_ms;            /* '<S44>/Constant1' */
uint16_T SOC_ModelOCV_mV;              /* '<S24>/Switch1' */

/* Exported block parameters */
int32_T P_AtRateCurrent_mA = 1000;     /* Variable: P_AtRateCurrent_mA
                                        * Referenced by: '<S4>/At_Rate_Current_mA'
                                        */
int32_T P_EmptyVoltage_mV = 6000;      /* Variable: P_EmptyVoltage_mV
                                        * Referenced by: '<S4>/Empty_Voltage_mV'
                                        */
const int32_T P_OcvSOCDsg_mpct[36] = { 0, 0, 0, 4974, 4974, 4974, 11279, 11279, 11279,
  18132, 18132, 18132, 26450, 26450, 26450, 35881, 35881, 35881, 45722, 45722,
  45722, 59977, 59977, 59977, 69735, 69735, 69735, 79337, 79337, 79337, 93377,
  93377, 93377, 100000, 100000, 100000 } ;/* Variable: P_OcvSOCDsg_mpct
                                           * Referenced by:
                                           *   '<S34>/OCV_DSG'
                                           *   '<S13>/OCV_DSG'
                                           *   '<S20>/OCV_DSG'
                                           */

int32_T P_SOCAxis_mpct[12] = { 0, 5000, 10000, 20000, 30000, 40000, 50000, 60000,
  70000, 80000, 90000, 100000 } ;      /* Variable: P_SOCAxis_mpct
                                        * Referenced by:
                                        *   '<S34>/DCIR_Discharge'
                                        *   '<S24>/DCIR_Discharge'
                                        *   '<S24>/OCV_Discharge'
                                        *   '<S24>/R0_Discharge'
                                        */

int32_T P_SOCSlope_mpctPermV[12] = { 20, 20, 24, 27, 36, 38, 40, 69, 38, 65, 178,
  17 } ;                               /* Variable: P_SOCSlope_mpctPermV
                                        * Referenced by: '<S23>/SOC_Slope'
                                        */

int32_T P_SocDeviationAxis_mpct[7] = { 0, 1000, 2000, 8000, 10000, 20000, 100000
} ;                                    /* Variable: P_SocDeviationAxis_mpct
                                        * Referenced by: '<S20>/1-D Lookup Table1'
                                        */

int32_T P_SocDeviationCorrect_upct[7] = { 75, 7500, 20000, 50000, 100000, 800000,
  1000000 } ;                          /* Variable: P_SocDeviationCorrect_upct
                                        * Referenced by: '<S20>/1-D Lookup Table1'
                                        */

int32_T P_SocRangeAxis_mpct[5] = { 0, 10000, 20000, 25000, 100000 } ;/* Variable: P_SocRangeAxis_mpct
                                                                      * Referenced by: '<S20>/adaption in SOC range'
                                                                      */

int32_T P_SocRangeCorrect_mpct[5] = { 10, 10, 150, 800, 1000 } ;/* Variable: P_SocRangeCorrect_mpct
                                                                 * Referenced by: '<S20>/adaption in SOC range'
                                                                 */

uint32_T P_CurrentThresRelaxJudge_mA = 40U;/* Variable: P_CurrentThresRelaxJudge_mA
                                            * Referenced by:
                                            *   '<S12>/Constant1'
                                            *   '<S14>/Constant1'
                                            */
const uint32_T P_DcirDsg_mOhm[36] = { 209U, 209U, 209U, 209U, 209U, 209U, 209U, 209U,
  209U, 148U, 148U, 148U, 101U, 101U, 101U, 105U, 105U, 105U, 106U, 106U, 106U,
  90U, 90U, 90U, 111U, 111U, 111U, 88U, 88U, 88U, 84U, 84U, 84U, 92U, 92U, 92U }
;                                      /* Variable: P_DcirDsg_mOhm
                                        * Referenced by: '<S24>/DCIR_Discharge'
                                        */

const uint32_T P_R0Dsg_mOhm[36] = { 64U, 64U, 64U, 62U, 62U, 62U, 61U, 61U, 61U, 56U,
  56U, 56U, 54U, 54U, 54U, 54U, 54U, 54U, 54U, 54U, 54U, 54U, 54U, 54U, 54U, 54U,
  54U, 55U, 55U, 55U, 57U, 57U, 57U, 60U, 60U, 60U } ;/* Variable: P_R0Dsg_mOhm
                                                       * Referenced by: '<S24>/R0_Discharge'
                                                       */

uint32_T P_RelaxDurationExtremeLowTemp_s = 7200U;
                                    /* Variable: P_RelaxDurationExtremeLowTemp_s
                                     * Referenced by: '<S12>/P_RelaxDurationExtremeLowTemp_s'
                                     */
uint32_T P_RelaxDurationLowTemp_s = 3600U;/* Variable: P_RelaxDurationLowTemp_s
                                           * Referenced by: '<S12>/P_RelaxDurationLowTemp_s'
                                           */
uint32_T P_RelaxDurationNormalTemp_s = 1500U;/* Variable: P_RelaxDurationNormalTemp_s
                                              * Referenced by: '<S12>/P_RelaxDurationNormalTemp_s'
                                              */
int16_T P_LowTemp_degC = 10;           /* Variable: P_LowTemp_degC
                                        * Referenced by: '<S16>/Constant'
                                        */
int16_T P_NormalTemp_degC = 20;        /* Variable: P_NormalTemp_degC
                                        * Referenced by: '<S15>/Constant'
                                        */
int16_T P_TAxis_degC[3] = { 0, 25, 45 } ;/* Variable: P_TAxis_degC
                                          * Referenced by:
                                          *   '<S34>/DCIR_Discharge'
                                          *   '<S34>/OCV_DSG'
                                          *   '<S13>/OCV_DSG'
                                          *   '<S20>/OCV_DSG'
                                          *   '<S24>/DCIR_Discharge'
                                          *   '<S24>/OCV_Discharge'
                                          *   '<S24>/R0_Discharge'
                                          */

uint16_T P_Capacity_mAh = 5000U;       /* Variable: P_Capacity_mAh
                                        * Referenced by:
                                        *   '<S41>/SOH_capacity_mAh'
                                        *   '<S41>/Constant3'
                                        */
uint16_T P_OCVAxis_mV[12] = { 6000U, 6250U, 6500U, 6750U, 7000U, 7250U, 7500U,
  7750U, 8000U, 8150U, 8300U, 8800U } ;/* Variable: P_OCVAxis_mV
                                        * Referenced by: '<S13>/OCV_DSG'
                                        */

const uint16_T P_OCVDsg_mV[36] = { 6002U, 6002U, 6002U, 6251U, 6251U, 6251U, 6455U,
  6455U, 6455U, 6820U, 6820U, 6820U, 7096U, 7096U, 7096U, 7358U, 7358U, 7358U,
  7606U, 7606U, 7606U, 7750U, 7750U, 7750U, 8007U, 8007U, 8007U, 8160U, 8160U,
  8160U, 8216U, 8216U, 8216U, 8778U, 8778U, 8778U } ;/* Variable: P_OCVDsg_mV
                                                      * Referenced by: '<S24>/OCV_Discharge'
                                                      */

uint16_T P_SampleTime_ms = 100U;       /* Variable: P_SampleTime_ms
                                        * Referenced by: '<S44>/Constant1'
                                        */
boolean_T P_ModelCorrEnable_flg = true;/* Variable: P_ModelCorrEnable_flg
                                        * Referenced by: '<S22>/Constant3'
                                        */
boolean_T P_VoltMatchEnable_flg = false;/* Variable: P_VoltMatchEnable_flg
                                         * Referenced by: '<S14>/Constant2'
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

