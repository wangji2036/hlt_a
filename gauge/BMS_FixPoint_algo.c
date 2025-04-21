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


