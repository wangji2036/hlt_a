/*
 * BMS_FixPoint.h
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
#include "config.h"
#ifndef BMS_FixPoint_h_
#define BMS_FixPoint_h_
#ifndef BMS_FixPoint_COMMON_INCLUDES_
#define BMS_FixPoint_COMMON_INCLUDES_
#include "rtwtypes.h"
#endif                                 /* BMS_FixPoint_COMMON_INCLUDES_ */

#include "BMS_FixPoint_types.h"
#include "typdef.h"
/* Includes for objects with custom storage classes */
#include "BMS_data.h"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

#define BMS_FixPoint_M                 (M_s)

/* Invariant block signals (default storage) */
typedef struct {
  const int32_T Divide5;               /* '<S37>/Divide5' */
  const int32_T Add2_c;                /* '<S41>/Add2' */
  const int32_T Divide1_n;             /* '<S38>/Divide1' */
  const int32_T stdvCellVADC2;         /* '<S24>/Math Function1' */
  const int32_T Add5_b;                /* '<S34>/Add5' */
  const int32_T Minus;                 /* '<S22>/Minus' */
  const int32_T Add5_h;                /* '<S28>/Add5' */
  const int32_T Add5_ha;               /* '<S29>/Add5' */
} ConstB;

/* Constant parameters (default storage) */
typedef struct {
  /* Pooled Parameter (Expression: P_OCVAxis_mV)
   * Referenced by:
   *   '<S33>/OCV_DSG'
   *   '<S19>/OCV_DSG'
   *   '<S22>/SOC_Slope'
   */
   
#if(BUCKBOOST_USED_NU6801 == 1)
     int32_T pooled7[12];

  /* Computed Parameter: DCIR_Discharge_tableData
   * Referenced by: '<S33>/DCIR_Discharge'
   */
     int32_T DCIR_Discharge_tableData[36];
#endif
#if(BUCKBOOST_USED_NU6805 == 1)
  int32_T pooled6[12];

  /* Computed Parameter: DCIR_Discharge_tableData
   * Referenced by: '<S33>/DCIR_Discharge'
   */
   int32_T DCIR_Discharge_tableData[36];
#endif
  /* Pooled Parameter (Expression: )
   * Referenced by:
   *   '<S33>/DCIR_Discharge'
   *   '<S33>/OCV_DSG'
   *   '<S12>/OCV_DSG'
   *   '<S19>/OCV_DSG'
   *   '<S23>/DCIR_Discharge'
   *   '<S23>/OCV_Discharge'
   *   '<S23>/R0_Discharge'
   */
  uint32_T pooled10[2];
} ConstP;

/* Real-time Model Data Structure */
struct tag_RTM {
  const char_T * volatile errorStatus;

  /*
   * Timing:
   * The following substructure contains information regarding
   * the timing information for the model.
   */
  struct {
    uint32_T clockTick0;
  } Timing;
};

extern const ConstB ConstB_s;          /* constant block i/o */

/* Constant parameters (default storage) */
extern const ConstP ConstP_s;

/*
 * Exported Global Signals
 *
 * Note: Exported global signals are block signals with an exported global
 * storage class designation.  Code generation will declare the memory for
 * these signals and export their symbols.
 *
 */
extern boolean_T SOC_OCVUpd_flg;       /* '<Root>/SOC_OCVUpd_flg' */
extern int32_T SOC_OCVSOC_mpct;        /* '<Root>/SOC_OCVSOC_mpct' */
extern int32_T SOC_AhIntegralSOC_mpct; /* '<Root>/SOC_AhIntegralSOC_mpct' */
extern int32_T SOC_RawSOC_mpct;        /* '<Root>/SOC_RawSOC_mpct' */
extern int32_T SOH_SOHR_pct;           /* '<Root>/SOH_SOHR_pct' */
extern int32_T SOH_Resistance_mOhm;    /* '<Root>/SOH_Resistance_mOhm' */
extern boolean_T SOC_CHG_flg;          /* '<Root>/SOC_CHG_flg' */
extern int32_T SOCPack_RealSOC_pct;    /* '<Root>/SOCPack_RealSOC_pct' */
extern int32_T SOCPack_EmptySOC_mpct;  /* '<Root>/SOCPack_EmptySOC_mpct' */
extern int32_T SOCPack_DisplaySOC_pct; /* '<Root>/SOCPack_DisplaySOC_pct' */
extern uint32_T Bal_BalTime_s;         /* '<Root>/Bal_BalTime_s' */
extern uint32_T SOC_ModelR0_mOhm;      /* '<S23>/Switch3' */
extern uint32_T SOC_ModelDCIR_mOhm;    /* '<S23>/Switch2' */
extern int32_T SOCPack_UdEmptySOC_mpct;/* '<S33>/SOCPack_UdEmptySOC_mpct' */
extern int32_T SOCPack_SatuarationSoc_mpct;/* '<S33>/Saturation' */
extern int32_T SOCPack_EmptyDcr_mOhm;  /* '<S33>/DCIR_Discharge' */
extern int32_T SOCPack_EmptyU_mV;      /* '<S33>/Add2' */
extern int32_T SOCPack_PreEmptySOC_mpct;/* '<S33>/Saturation4' */
extern int32_T SOC_VirtOCVSOC_mpct;    /* '<S19>/Saturation1' */
extern int32_T SOC_VirtualOCV_mV;      /* '<S20>/UdVirtualOcv' */
extern uint16_T BMS_SampleTime_ms;     /* '<S43>/Constant1' */
extern uint16_T SOC_ModelOCV_mV;       /* '<S23>/Switch1' */

/*
 * Exported Global Parameters
 *
 * Note: Exported global parameters are tunable parameters with an exported
 * global storage class designation.  Code generation will declare the memory for
 * these parameters and exports their symbols.
 *
 */
extern int32_T P_AtRateCurrent_mA;     /* Variable: P_AtRateCurrent_mA
                                        * Referenced by: '<S4>/At_Rate_Current_mA'
                                        */
extern int32_T P_EmptyVoltage_mV;      /* Variable: P_EmptyVoltage_mV
                                        * Referenced by: '<S4>/Empty_Voltage_mV'
                                        */

#if(BUCKBOOST_USED_NU6801 == 1)
extern const  int32_T P_OcvSOCDsg_mpct[36];   /* Variable: P_OcvSOCDsg_mpct
                                        * Referenced by:
                                        *   '<S33>/OCV_DSG'
                                        *   '<S12>/OCV_DSG'
                                        *   '<S19>/OCV_DSG'
                                        */
extern int32_T P_SOCAxis_mpct[12];     /* Variable: P_SOCAxis_mpct
                                        * Referenced by:
                                        *   '<S33>/DCIR_Discharge'
                                        *   '<S23>/DCIR_Discharge'
                                        *   '<S23>/OCV_Discharge'
                                        *   '<S23>/R0_Discharge'
                                        */
extern int32_T P_SOCSlope_mpctPermV[12];/* Variable: P_SOCSlope_mpctPermV
                                         * Referenced by: '<S22>/SOC_Slope'
                                         */
#endif

#if(BUCKBOOST_USED_NU6805 == 1)
extern const int32_T P_OcvSOCChg_mpct[36];   /* Variable: P_OcvSOCChg_mpct
                                        * Referenced by:
                                        *   '<S13>/OCV_CHG'
                                        *   '<S21>/OCV_CHG'
                                        */
extern const int32_T P_OcvSOCDsg_mpct[36];   /* Variable: P_OcvSOCDsg_mpct
                                        * Referenced by:
                                        *   '<S35>/OCV_DSG'
                                        *   '<S13>/OCV_DSG'
                                        *   '<S21>/OCV_DSG'
                                        */
extern const int32_T P_SOCAxis_mpct[12];     /* Variable: P_SOCAxis_mpct
                                        * Referenced by:
                                        *   '<S35>/DCIR_Discharge'
                                        *   '<S25>/DCIR_Charge'
                                        *   '<S25>/DCIR_Discharge'
                                        *   '<S25>/OCV_Charge'
                                        *   '<S25>/OCV_Discharge'
                                        *   '<S25>/R0_Charge'
                                        *   '<S25>/R0_Discharge'
                                        */
extern const int32_T P_SOCSlope_mpctPermV[12];/* Variable: P_SOCSlope_mpctPermV
                                         * Referenced by: '<S24>/SOC_Slope'

                                         */

#endif
extern const int32_T P_SocDeviationAxis_mpct[7];/* Variable: P_SocDeviationAxis_mpct
                                           * Referenced by: '<S19>/1-D Lookup Table1'
                                           */
extern const int32_T P_SocDeviationCorrect_upct[7];/* Variable: P_SocDeviationCorrect_upct
                                              * Referenced by: '<S19>/1-D Lookup Table1'
                                              */
extern const int32_T P_SocRangeAxis_mpct[5]; /* Variable: P_SocRangeAxis_mpct
                                        * Referenced by: '<S19>/adaption in SOC range'
                                        */
extern const int32_T P_SocRangeCorrect_mpct[5];/* Variable: P_SocRangeCorrect_mpct
                                          * Referenced by: '<S19>/adaption in SOC range'
                                          */
extern uint32_T P_CurrentThresRelaxJudge_mA;/* Variable: P_CurrentThresRelaxJudge_mA
                                             * Referenced by:
                                             *   '<S11>/Constant1'
                                             *   '<S13>/Constant1'
                                             */
#if(BUCKBOOST_USED_NU6801 == 1)
extern const  uint32_T P_DcirDsg_mOhm[36];    /* Variable: P_DcirDsg_mOhm
                                        * Referenced by: '<S23>/DCIR_Discharge'
                                        */
extern const  uint32_T P_R0Dsg_mOhm[36];      /* Variable: P_R0Dsg_mOhm
                                        * Referenced by: '<S23>/R0_Discharge'
                                        */
#endif

#if(BUCKBOOST_USED_NU6805 == 1)

extern const uint32_T P_DcirChg_mOhm[36];    /* Variable: P_DcirChg_mOhm
                                        * Referenced by: '<S25>/DCIR_Charge'
                                        */
extern const uint32_T P_DcirDsg_mOhm[36];    /* Variable: P_DcirDsg_mOhm
                                        * Referenced by: '<S25>/DCIR_Discharge'
                                        */
extern const uint32_T P_R0Chg_mOhm[36];      /* Variable: P_R0Chg_mOhm
                                        * Referenced by: '<S25>/R0_Charge'
                                        */
extern const uint32_T P_R0Dsg_mOhm[36];      /* Variable: P_R0Dsg_mOhm
                                        * Referenced by: '<S25>/R0_Discharge'
                                         */
#endif
extern uint32_T P_RelaxDurationExtremeLowTemp_s;
                                    /* Variable: P_RelaxDurationExtremeLowTemp_s
                                     * Referenced by: '<S11>/P_RelaxDurationExtremeLowTemp_s'
                                     */
extern uint32_T P_RelaxDurationLowTemp_s;/* Variable: P_RelaxDurationLowTemp_s
                                          * Referenced by: '<S11>/P_RelaxDurationLowTemp_s'
                                          */
extern uint32_T P_RelaxDurationNormalTemp_s;/* Variable: P_RelaxDurationNormalTemp_s
                                             * Referenced by: '<S11>/P_RelaxDurationNormalTemp_s'
                                             */
extern int16_T P_LowTemp_degC;         /* Variable: P_LowTemp_degC
                                        * Referenced by: '<S15>/Constant'
                                        */
extern int16_T P_NormalTemp_degC;      /* Variable: P_NormalTemp_degC
                                        * Referenced by: '<S14>/Constant'
                                        */
extern int16_T P_TAxis_degC[3];        /* Variable: P_TAxis_degC
                                        * Referenced by:
                                        *   '<S33>/DCIR_Discharge'
                                        *   '<S33>/OCV_DSG'
                                        *   '<S12>/OCV_DSG'
                                        *   '<S19>/OCV_DSG'
                                        *   '<S23>/DCIR_Discharge'
                                        *   '<S23>/OCV_Discharge'
                                        *   '<S23>/R0_Discharge'
                                        */
extern uint16_T P_Capacity_mAh;        /* Variable: P_Capacity_mAh
                                        * Referenced by:
                                        *   '<S40>/SOH_capacity_mAh'
                                        *   '<S40>/Constant3'
                                        */
#if(BUCKBOOST_USED_NU6801 == 1)
extern uint16_T P_OCVAxis_mV[12];      /* Variable: P_OCVAxis_mV
                                        * Referenced by: '<S12>/OCV_DSG'
                                        */
extern const  uint16_T P_OCVDsg_mV[36];       /* Variable: P_OCVDsg_mV
                                        * Referenced by: '<S23>/OCV_Discharge'
                                        */
#endif

#if(BUCKBOOST_USED_NU6805 == 1)
extern const uint16_T P_OCVAxis_mV[12];      /* Variable: P_OCVAxis_mV
                                        * Referenced by:
                                        *   '<S13>/OCV_CHG'
                                        *   '<S13>/OCV_DSG'
                                        */
extern const uint16_T P_OCVChg_mV[36];       /* Variable: P_OCVChg_mV
                                        * Referenced by: '<S25>/OCV_Charge'
                                        */
extern const uint16_T P_OCVDsg_mV[36];       /* Variable: P_OCVDsg_mV
                                        * Referenced by: '<S25>/OCV_Discharge'
                                        */

#endif
extern uint16_T P_SampleTime_ms;       /* Variable: P_SampleTime_ms
                                        * Referenced by: '<S43>/Constant1'
                                        */
extern boolean_T P_ModelCorrEnable_flg;/* Variable: P_ModelCorrEnable_flg
                                        * Referenced by: '<S21>/Constant3'
                                        */
extern boolean_T P_VoltMatchEnable_flg;/* Variable: P_VoltMatchEnable_flg
                                        * Referenced by: '<S13>/Constant2'
                                        */

/* Model entry point functions */
extern void Init(void);
extern void Cyclic(void);

/* Real-time Model object */
extern RT_MODEL *const M_s;

/*-
 * The generated code includes comments that allow you to trace directly
 * back to the appropriate location in the model.  The basic format
 * is <system>/block_name, where system is the system number (uniquely
 * assigned by Simulink) and block_name is the name of the block.
 *
 * Use the MATLAB hilite_system command to trace the generated code back
 * to the model.  For example,
 *
 * hilite_system('<S3>')    - opens system 3
 * hilite_system('<S3>/Kp') - opens and selects block Kp which resides in S3
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'BMS_FixPoint'
 * '<S1>'   : 'BMS_FixPoint/BMS'
 * '<S2>'   : 'BMS_FixPoint/Mode_Management'
 * '<S3>'   : 'BMS_FixPoint/BMS/SOC'
 * '<S4>'   : 'BMS_FixPoint/BMS/SOCPack'
 * '<S5>'   : 'BMS_FixPoint/BMS/SOH'
 * '<S6>'   : 'BMS_FixPoint/BMS/SOC/AhIntegralSOC'
 * '<S7>'   : 'BMS_FixPoint/BMS/SOC/OCVSOC'
 * '<S8>'   : 'BMS_FixPoint/BMS/SOC/SOC_Correction'
 * '<S9>'   : 'BMS_FixPoint/BMS/SOC/AhIntegralSOC/Integraiton'
 * '<S10>'  : 'BMS_FixPoint/BMS/SOC/AhIntegralSOC/Integraiton/Over0.1mAh'
 * '<S11>'  : 'BMS_FixPoint/BMS/SOC/OCVSOC/CurrentRelax_Check'
 * '<S12>'  : 'BMS_FixPoint/BMS/SOC/OCVSOC/Lookup_OCVSOC'
 * '<S13>'  : 'BMS_FixPoint/BMS/SOC/OCVSOC/VoltageRelax_Check'
 * '<S14>'  : 'BMS_FixPoint/BMS/SOC/OCVSOC/CurrentRelax_Check/Compare To Constant'
 * '<S15>'  : 'BMS_FixPoint/BMS/SOC/OCVSOC/CurrentRelax_Check/Compare To Constant1'
 * '<S16>'  : 'BMS_FixPoint/BMS/SOC/OCVSOC/CurrentRelax_Check/debounce'
 * '<S17>'  : 'BMS_FixPoint/BMS/SOC/OCVSOC/VoltageRelax_Check/Over60s'
 * '<S18>'  : 'BMS_FixPoint/BMS/SOC/OCVSOC/VoltageRelax_Check/voltage_time_judge'
 * '<S19>'  : 'BMS_FixPoint/BMS/SOC/SOC_Correction/Correction_EachStep'
 * '<S20>'  : 'BMS_FixPoint/BMS/SOC/SOC_Correction/Model'
 * '<S21>'  : 'BMS_FixPoint/BMS/SOC/SOC_Correction/Model_Ah_Merge'
 * '<S22>'  : 'BMS_FixPoint/BMS/SOC/SOC_Correction/StandardError'
 * '<S23>'  : 'BMS_FixPoint/BMS/SOC/SOC_Correction/Model/Impedance for Raw SOC'
 * '<S24>'  : 'BMS_FixPoint/BMS/SOC/SOC_Correction/Model/OCV adaption on current'
 * '<S25>'  : 'BMS_FixPoint/BMS/SOC/SOC_Correction/Model/Subsystem1'
 * '<S26>'  : 'BMS_FixPoint/BMS/SOC/SOC_Correction/Model/rolling average'
 * '<S27>'  : 'BMS_FixPoint/BMS/SOC/SOC_Correction/Model/rolling average2'
 * '<S28>'  : 'BMS_FixPoint/BMS/SOC/SOC_Correction/Model/OCV adaption on current/Compare To Constant'
 * '<S29>'  : 'BMS_FixPoint/BMS/SOC/SOC_Correction/Model/OCV adaption on current/Compare To Constant1'
 * '<S30>'  : 'BMS_FixPoint/BMS/SOC/SOC_Correction/Model/OCV adaption on current/Compare To Constant2'
 * '<S31>'  : 'BMS_FixPoint/BMS/SOC/SOC_Correction/Model/OCV adaption on current/Compare To Constant3'
 * '<S32>'  : 'BMS_FixPoint/BMS/SOC/SOC_Correction/StandardError/rolling average'
 * '<S33>'  : 'BMS_FixPoint/BMS/SOCPack/Pack_Empty'
 * '<S34>'  : 'BMS_FixPoint/BMS/SOCPack/Pack_Full'
 * '<S35>'  : 'BMS_FixPoint/BMS/SOCPack/SOC Mapping'
 * '<S36>'  : 'BMS_FixPoint/BMS/SOCPack/SOC_Filter'
 * '<S37>'  : 'BMS_FixPoint/BMS/SOCPack/Unfiltered_SOCPack_RealSOC_pct'
 * '<S38>'  : 'BMS_FixPoint/BMS/SOCPack/SOC_Filter/Saturation Dynamic'
 * '<S39>'  : 'BMS_FixPoint/BMS/SOCPack/Unfiltered_SOCPack_RealSOC_pct/Pack_Weighting'
 * '<S40>'  : 'BMS_FixPoint/BMS/SOH/SOHC'
 * '<S41>'  : 'BMS_FixPoint/Mode_Management/Variant Subsystem'
 * '<S42>'  : 'BMS_FixPoint/Mode_Management/enable'
 * '<S43>'  : 'BMS_FixPoint/Mode_Management/Variant Subsystem/SimpleModeMangement'
 * '<S44>'  : 'BMS_FixPoint/Mode_Management/Variant Subsystem/SimpleModeMangement/Chart'
 */
#endif                                 /* BMS_FixPoint_h_ */
