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
   *   '<S35>/OCV_DSG'
   *   '<S21>/OCV_CHG'
   *   '<S21>/OCV_DSG'
   *   '<S24>/SOC_Slope'
   */
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
};
