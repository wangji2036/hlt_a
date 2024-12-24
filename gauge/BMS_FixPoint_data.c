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

/* Invariant block signals (default storage) */
const ConstB ConstB_s = {
  55000,                               /* '<S39>/Add2' */
  -10,                                 /* '<S36>/Divide1' */
  9,                                   /* '<S22>/Math Function1' */
  9,                                   /* '<S32>/Add5' */
  1,                                   /* '<S20>/Minus' */
  9,                                   /* '<S26>/Add5' */
  9,                                   /* '<S27>/Add5' */
  false                                /* '<S7>/Constant' */
};

/* Constant parameters (default storage) */
const ConstP ConstP_s = {
  /* Pooled Parameter (Expression: P_OCVAxis_mV)
   * Referenced by:
   *   '<S33>/OCV_DSG'
   *   '<S19>/OCV_DSG'
   *   '<S22>/SOC_Slope'
   */
  { 6083, 6498, 6726, 6811, 6854, 6903, 6961, 7018, 7069, 7114, 7156, 7212, 7257,
    7304, 7358, 7426, 7511, 7615, 7730, 7809, 7881, 7968, 8015, 8063, 8108, 8144,
    8169, 8186, 8205, 8234, 8284, 8366 },

  /* Computed Parameter: DCIR_Discharge_tableData
   * Referenced by: '<S33>/DCIR_Discharge'
   */
  { 138, 138, 138, 108, 108, 108, 95, 95, 95, 90, 90, 90, 79, 79, 79, 73, 73, 73,
    69, 69, 69, 65, 65, 65, 64, 64, 64, 64, 64, 64, 60, 60, 60, 58, 58, 58, 60,
    60, 60, 60, 60, 60, 62, 62, 62, 63, 63, 63, 63, 63, 63, 64, 64, 64, 54, 54,
    54, 53, 53, 53, 55, 55, 55, 57, 57, 57, 61, 61, 61, 61, 61, 61, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 65, 65, 65, 70, 70, 70, 75, 75, 75, 84, 84, 84, 132,
    132, 132 },

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
  { 2U, 31U }
};
