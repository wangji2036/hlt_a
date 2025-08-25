/*
 * BMS_FixPoint_data.c
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
#include "BMS_FixPoint.h"
#include "config.h"
/* Invariant block signals (default storage) */
const ConstB ConstB_s = {
  100000,                              /* '<S37>/Divide5' */
  55000,                               /* '<S41>/Add2' */
  -100,                                /* '<S38>/Divide1' */
  9,                                   /* '<S24>/Math Function1' */
  9,                                   /* '<S34>/Add5' */
  1,                                   /* '<S22>/Minus' */
  9,                                   /* '<S28>/Add5' */
  9                                    /* '<S29>/Add5' */
};

/* Constant parameters (default storage) */
const ConstP ConstP_s = {
  /* Pooled Parameter (Expression: P_OCVAxis_mV)
   * Referenced by:
   *   '<S33>/OCV_DSG'
   *   '<S19>/OCV_DSG'
   *   '<S22>/SOC_Slope'
   */
   #if(BUCKBOOST_USED_NU6801 == 1)
     { 2989, 3492, 3601, 3645, 3700, 3825, 3939, 4045, 4105, 4146, 4177, 4209 },

  /* Computed Parameter: DCIR_Discharge_tableData
   * Referenced by: '<S33>/DCIR_Discharge'
   */
  { 70, 70, 70, 84, 84, 84, 70, 70, 70, 77, 77, 77, 86, 86, 86, 75, 75, 75, 81,
    81, 81, 88, 88, 88, 100, 100, 100, 107, 107, 107, 112, 112, 112, 135, 135,
    135 },
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
  { 2U, 11U }
   #endif
   
#if(BUCKBOOST_USED_NU6805 == 1)
  { 6000, 6250, 6500, 6750, 7000, 7250, 7500, 7750, 8000, 8150, 8300, 8800 },

  /* Computed Parameter: DCIR_Discharge_tableData
   * Referenced by: '<S35>/DCIR_Discharge'
   */
  { 209, 209, 209, 209, 209, 209, 209, 209, 209, 148, 148, 148, 101, 101, 101,
    105, 105, 105, 106, 106, 106, 90, 90, 90, 111, 111, 111, 88, 88, 88, 84, 84,
    84, 92, 92, 92 },

  /* Pooled Parameter (Expression: )
   * Referenced by:
   *   '<S35>/DCIR_Discharge'
   *   '<S35>/OCV_DSG'
   *   '<S13>/OCV_CHG'
   *   '<S13>/OCV_DSG'
   *   '<S21>/OCV_CHG'
   *   '<S21>/OCV_DSG'
   *   '<S25>/DCIR_Charge'
   *   '<S25>/DCIR_Discharge'
   *   '<S25>/OCV_Charge'
   *   '<S25>/OCV_Discharge'
   *   '<S25>/R0_Charge'
   *   '<S25>/R0_Discharge'
   */
  { 2U, 11U }
  #endif
};
