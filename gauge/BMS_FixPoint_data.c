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
  55000,                               /* '<S40>/Add2' */
  -100,                                /* '<S37>/Divide1' */
  9,                                   /* '<S23>/Math Function1' */
  9,                                   /* '<S33>/Add5' */
  1,                                   /* '<S21>/Minus' */
  9,                                   /* '<S27>/Add5' */
  9,                                   /* '<S28>/Add5' */
  false                                /* '<S7>/Constant' */
};

/* Constant parameters (default storage) */
const ConstP ConstP_s = {
  /* Pooled Parameter (Expression: P_OCVAxis_mV)
   * Referenced by:
   *   '<S34>/OCV_DSG'
   *   '<S20>/OCV_DSG'
   *   '<S23>/SOC_Slope'
   */
  { 6000, 6250, 6500, 6750, 7000, 7250, 7500, 7750, 8000, 8150, 8300, 8800 },

  /* Computed Parameter: DCIR_Discharge_tableData
   * Referenced by: '<S34>/DCIR_Discharge'
   */
  { 209, 209, 209, 209, 209, 209, 209, 209, 209, 148, 148, 148, 101, 101, 101,
    105, 105, 105, 106, 106, 106, 90, 90, 90, 111, 111, 111, 88, 88, 88, 84, 84,
    84, 92, 92, 92 },

  /* Pooled Parameter (Expression: )
   * Referenced by:
   *   '<S34>/DCIR_Discharge'
   *   '<S34>/OCV_DSG'
   *   '<S13>/OCV_DSG'
   *   '<S20>/OCV_DSG'
   *   '<S24>/DCIR_Discharge'
   *   '<S24>/OCV_Discharge'
   *   '<S24>/R0_Discharge'
   */
  { 2U, 11U }
};
