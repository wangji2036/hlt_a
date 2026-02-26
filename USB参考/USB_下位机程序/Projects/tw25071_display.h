/**
 * @file    tw25071_display.h
 * @brief   TW-25071 数码管/背光板驱动头文件
 * @note    使用查理复用技术，7个引脚控制5组显示区域
 */

#ifndef __TW25071_DISPLAY_H
#define __TW25071_DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "wb7720.h"
#include "wb7720_gpio.h"

/* 显示状态定义 */
typedef enum {
    DISPLAY_OFF = 0,
    DISPLAY_ON  = 1
} DisplayState;

/* GPIO引脚定义 - 根据文档映射 */
/* PIN 1: PA4 */
#define TW_PIN1_PORT        GPIOA
#define TW_PIN1_PIN         GPIO_Pin_4

/* PIN 2: PA5 */
#define TW_PIN2_PORT        GPIOA
#define TW_PIN2_PIN         GPIO_Pin_5

/* PIN 3: PA6 */
#define TW_PIN3_PORT        GPIOA
#define TW_PIN3_PIN         GPIO_Pin_6

/* PIN 4: PA7 */
#define TW_PIN4_PORT        GPIOA
#define TW_PIN4_PIN         GPIO_Pin_7

/* PIN 5: PB0 */
#define TW_PIN5_PORT        GPIOB
#define TW_PIN5_PIN         GPIO_Pin_0

/* PIN 6: PB12 */
#define TW_PIN6_PORT        GPIOB
#define TW_PIN6_PIN         GPIO_Pin_12

/* PIN 7: PB14 */
#define TW_PIN7_PORT        GPIOB
#define TW_PIN7_PIN         GPIO_Pin_14

/* 七段数码管段码定义 (a-g) 用于数字显示 */
#define SEG_A   0x01
#define SEG_B   0x02
#define SEG_C   0x04
#define SEG_D   0x08
#define SEG_E   0x10
#define SEG_F   0x20
#define SEG_G   0x40

/* 函数声明 */

/**
 * @brief  初始化TW25071显示模块
 */
void TW25071_Init(void);

/**
 * @brief  刷新显示（在定时器中断中调用）
 */
void TW25071_Refresh(void);

/**
 * @brief  函数1: 控制警告图标
 * @param  state: DISPLAY_ON 亮, DISPLAY_OFF 灭
 */
void TW25071_SetWarningIcon(DisplayState state);

/**
 * @brief  函数2: 控制重置图标
 * @param  state: DISPLAY_ON 亮, DISPLAY_OFF 灭
 */
void TW25071_SetResetIcon(DisplayState state);

/**
 * @brief  函数3: 控制电池图标
 * @param  state: DISPLAY_ON 亮, DISPLAY_OFF 灭
 */
void TW25071_SetBatteryIcon(DisplayState state);

/**
 * @brief  函数4: 控制温度计图标
 * @param  state: DISPLAY_ON 亮, DISPLAY_OFF 灭
 */
void TW25071_SetThermometerIcon(DisplayState state);

/**
 * @brief  函数5: 控制中间三位数码管显示数字
 * @param  value: 0-999的数字 (超过999显示999)
 */
void TW25071_SetNumber(uint16_t value);

/**
 * @brief  函数6: 控制摄氏度符号 ℃
 * @param  state: DISPLAY_ON 亮, DISPLAY_OFF 灭
 */
void TW25071_SetCelsiusIcon(DisplayState state);

/**
 * @brief  函数7: 控制IN图标
 * @param  state: DISPLAY_ON 亮, DISPLAY_OFF 灭
 */
void TW25071_SetInIcon(DisplayState state);

/**
 * @brief  函数8: 控制圆形图标(绿色50)
 * @param  state: DISPLAY_ON 亮, DISPLAY_OFF 灭
 */
void TW25071_SetCircleIcon(DisplayState state);

/**
 * @brief  函数9: 控制百分号 %
 * @param  state: DISPLAY_ON 亮, DISPLAY_OFF 灭
 */
void TW25071_SetPercentIcon(DisplayState state);

/**
 * @brief  函数10: 控制OUT图标
 * @param  state: DISPLAY_ON 亮, DISPLAY_OFF 灭
 */
void TW25071_SetOutIcon(DisplayState state);

#ifdef __cplusplus
}
#endif

#endif /* __TW25071_DISPLAY_H */
