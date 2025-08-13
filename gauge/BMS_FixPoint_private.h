/*
 * BMS_FixPoint_private.h
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

#ifndef BMS_FixPoint_private_h_
#define BMS_FixPoint_private_h_
#include "rtwtypes.h"
#include "multiword_types.h"
#include "zero_crossing_types.h"
#include "BMS_FixPoint_types.h"
#include "BMS_FixPoint.h"
#ifndef UCHAR_MAX
#include <limits.h>
#endif

#if ( UCHAR_MAX != (0xFFU) ) || ( SCHAR_MAX != (0x7F) )
#error Code was generated for compiler with different sized uchar/char. \
Consider adjusting Test hardware word size settings on the \
Hardware Implementation pane to match your compiler word sizes as \
defined in limits.h of the compiler. Alternatively, you can \
select the Test hardware is the same as production hardware option and \
select the Enable portable word sizes option on the Code Generation > \
Verification pane for ERT based targets, which will disable the \
preprocessor word size checks.
#endif

#if ( USHRT_MAX != (0xFFFFU) ) || ( SHRT_MAX != (0x7FFF) )
#error Code was generated for compiler with different sized ushort/short. \
Consider adjusting Test hardware word size settings on the \
Hardware Implementation pane to match your compiler word sizes as \
defined in limits.h of the compiler. Alternatively, you can \
select the Test hardware is the same as production hardware option and \
select the Enable portable word sizes option on the Code Generation > \
Verification pane for ERT based targets, which will disable the \
preprocessor word size checks.
#endif

#if ( UINT_MAX != (0xFFFFFFFFU) ) || ( INT_MAX != (0x7FFFFFFF) )
#error Code was generated for compiler with different sized uint/int. \
Consider adjusting Test hardware word size settings on the \
Hardware Implementation pane to match your compiler word sizes as \
defined in limits.h of the compiler. Alternatively, you can \
select the Test hardware is the same as production hardware option and \
select the Enable portable word sizes option on the Code Generation > \
Verification pane for ERT based targets, which will disable the \
preprocessor word size checks.
#endif

#if ( ULONG_MAX != (0xFFFFFFFFU) ) || ( LONG_MAX != (0x7FFFFFFF) )
#error Code was generated for compiler with different sized ulong/long. \
Consider adjusting Test hardware word size settings on the \
Hardware Implementation pane to match your compiler word sizes as \
defined in limits.h of the compiler. Alternatively, you can \
select the Test hardware is the same as production hardware option and \
select the Enable portable word sizes option on the Code Generation > \
Verification pane for ERT based targets, which will disable the \
preprocessor word size checks.
#endif

/* Imported (extern) block signals */
extern int32_T SigPr_PackCurr_mA;      /* '<Root>/Data Type Conversion1' */
extern uint16_T SigPr_CellVolts_mV;    /* '<Root>/Min' */
extern int16_T SigPr_CellTemps_C;      /* '<Root>/Min1' */
extern int32_T BMS_NvmSOC_mpct;        /* '<Root>/BMS_NvmSOC_mpct' */
extern uint32_T SOC_SleepTime_s;       /* '<Root>/SOC_SleepTime_s' */
extern int32_T rt_sqrt_Us32_Ys32_Is64_f_s(int32_T u);
extern int32_T look2_is16u16lu32n32ts_lElusMKZ(int16_T u0, uint16_T u1, const
  int16_T bp0[], const uint16_T bp1[], const int32_T table[], uint32_T
  prevIndex[], const uint32_T maxIndex[], uint32_T stride);
extern int32_T look2_is16s32lu32n32ts_WwgFj0xk(int16_T u0, int32_T u1, const
  int16_T bp0[], const int32_T bp1[], const int32_T table[], uint32_T prevIndex[],
  const uint32_T maxIndex[], uint32_T stride);
extern uint32_T look2_is16s32lu32n32tu_bvHCJPGn(int16_T u0, int32_T u1, const
  int16_T bp0[], const int32_T bp1[], const uint32_T table[], uint32_T
  prevIndex[], const uint32_T maxIndex[], uint32_T stride);
extern uint16_T look2_is16s32lu16n16tu_EvbysC3W(int16_T u0, int32_T u1, const
  int16_T bp0[], const int32_T bp1[], const uint16_T table[], uint32_T
  prevIndex[], const uint32_T maxIndex[], uint32_T stride);
extern int32_T look1_is32lu32n32Du32_pbinlcase(int32_T u0, const int32_T bp0[],
  const int32_T table[], uint32_T prevIndex[], uint32_T maxIndex);
extern int32_T look2_is16s32lu32n32ts_7E7IgjCI(int16_T u0, int32_T u1, const
  int16_T bp0[], const int32_T bp1[], const int32_T table[], uint32_T prevIndex[],
  const uint32_T maxIndex[], uint32_T stride);
extern void sLong2MultiWord(int32_T u, uint32_T y[], int32_T n);
extern boolean_T sMultiWordLe(const uint32_T u1[], const uint32_T u2[], int32_T
  n);
extern int32_T sMultiWordCmp(const uint32_T u1[], const uint32_T u2[], int32_T n);
extern void sMultiWordMul(const uint32_T u1[], int32_T n1, const uint32_T u2[],
  int32_T n2, uint32_T y[], int32_T n);
extern int32_T div_nde_s32_floor(int32_T numerator, int32_T denominator);
extern void mul_wide_s32(int32_T in0, int32_T in1, uint32_T *ptrOutBitsHi,
  uint32_T *ptrOutBitsLo);
extern int32_T mul_s32_loSR_sat(int32_T a, int32_T b, uint32_T aShift);
extern int32_T mul_s32_loSR(int32_T a, int32_T b, uint32_T aShift);
extern int32_T mul_s32_hiSR(int32_T a, int32_T b, uint32_T aShift);
extern void mul_wide_u32(uint32_T in0, uint32_T in1, uint32_T *ptrOutBitsHi,
  uint32_T *ptrOutBitsLo);
extern uint32_T mul_u32_sr32(uint32_T a, uint32_T b);
extern uint32_T div_nzp_repeat_u32(uint32_T numerator, uint32_T denominator,
  uint32_T nRepeatSub);
extern void SOHC_Init(void);
extern void SOHC(void);
extern void SOH_Init(void);
extern void SOH(void);

/* Exported data declaration */

/* Data with Imported storage */
extern int16_T SigPr_CellTemps_C_s;    /* '<Root>/SigPr_CellTemps_C' */
extern uint16_T SigPr_CellVolts_mV_s;  /* '<Root>/SigPr_CellVolts_mV' */
extern int32_T SigPr_PackCurr_mA_s;    /* '<Root>/SigPr_PackCurr_mA' */

#endif                                 /* BMS_FixPoint_private_h_ */
