/*
 * Code generation for system system '<S1>/SOC'
 * For more details, see corresponding source file SOC.c
 *
 */

#ifndef SOC_h_
#define SOC_h_
#ifndef BMS_FixPoint_COMMON_INCLUDES_
#define BMS_FixPoint_COMMON_INCLUDES_
#include "rtwtypes.h"
#endif                                 /* BMS_FixPoint_COMMON_INCLUDES_ */

extern void AhIntegralSOC_Init(void);
extern void AhIntegralSOC(void);
extern void Lookup_OCVSOC(void);
extern void OCVSOC_Init(void);
extern void OCVSOC(void);
extern void SOC_Correction_Init(void);
extern void SOC_Correction_Enable(void);
extern void SOC_Correction(void);
extern void SOC_Init(void);
extern void SOC_Enable(void);
extern void SOC(void);

extern int16_T SigPr_CellTemps_C_s;    /* '<Root>/SigPr_CellTemps_C' */
extern uint16_T SigPr_CellVolts_mV_s;  /* '<Root>/SigPr_CellVolts_mV' */
extern int32_T SigPr_PackCurr_mA_s;    /* '<Root>/SigPr_PackCurr_mA' */

#endif                                 /* SOC_h_ */
