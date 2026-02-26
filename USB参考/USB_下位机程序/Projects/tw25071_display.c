/**
 * @file    tw25071_display.c
 * @brief   TW-25071 数码管/背光板驱动实现
 * @note    使用查理复用技术，需要在定时器中断中调用TW25071_Refresh()进行扫描刷新
 */

#include "tw25071_display.h"

/* 显示缓冲区 - 存储当前显示状态 */
static struct {
    uint8_t warningIcon;      /* 警告图标 (4A) */
    uint8_t resetIcon;        /* 重置图标 (4E) */
    uint8_t batteryIcon;      /* 电池图标 (4F, 4G) */
    uint8_t thermometerIcon;  /* 温度计图标 (4B, 4C, 4D) */
    uint8_t digit1;           /* 数码管1段码 (DIG1: 1A-1G) */
    uint8_t digit2;           /* 数码管2段码 (DIG2: 2A-2G) */
    uint8_t digit3;           /* 数码管3段码 (DIG3: 3A-3G) */
    uint8_t celsiusIcon;      /* 摄氏度符号 (5A) */
    uint8_t inIcon;           /* IN图标 (5B) */
    uint8_t circleIcon;       /* 圆形图标 (5C) */
    uint8_t percentIcon;      /* 百分号 (5D) */
    uint8_t outIcon;          /* OUT图标 (5E, 5F) */
} displayBuffer;

/* 扫描索引 */
static uint8_t scanIndex = 0;

/* 数字0-9的七段码 (共阴极，对应a-g段) */
static const uint8_t digitSegments[10] = {
    0x3F,  /* 0: a,b,c,d,e,f */
    0x06,  /* 1: b,c */
    0x5B,  /* 2: a,b,d,e,g */
    0x4F,  /* 3: a,b,c,d,g */
    0x66,  /* 4: b,c,f,g */
    0x6D,  /* 5: a,c,d,f,g */
    0x7D,  /* 6: a,c,d,e,f,g */
    0x07,  /* 7: a,b,c */
    0x7F,  /* 8: a,b,c,d,e,f,g */
    0x6F   /* 9: a,b,c,d,f,g */
};

/* 段码定义结构 */
typedef struct {
    GPIO_TypeDef* anodePort;
    uint16_t anodePin;
    GPIO_TypeDef* cathodePort;
    uint16_t cathodePin;
} SegmentDef;

/* 所有段的定义 - 共35个段 */
/* DIG4 段定义 (7段) */
static const SegmentDef dig4Segments[7] = {
    {TW_PIN4_PORT, TW_PIN4_PIN, TW_PIN5_PORT, TW_PIN5_PIN},  /* 4A: 警告图标 */
    {TW_PIN4_PORT, TW_PIN4_PIN, TW_PIN6_PORT, TW_PIN6_PIN},  /* 4B: 温度计 */
    {TW_PIN4_PORT, TW_PIN4_PIN, TW_PIN7_PORT, TW_PIN7_PIN},  /* 4C: 温度计 */
    {TW_PIN5_PORT, TW_PIN5_PIN, TW_PIN1_PORT, TW_PIN1_PIN},  /* 4D: 温度计 */
    {TW_PIN5_PORT, TW_PIN5_PIN, TW_PIN2_PORT, TW_PIN2_PIN},  /* 4E: 重置图标 */
    {TW_PIN5_PORT, TW_PIN5_PIN, TW_PIN3_PORT, TW_PIN3_PIN},  /* 4F: 电池 */
    {TW_PIN5_PORT, TW_PIN5_PIN, TW_PIN4_PORT, TW_PIN4_PIN},  /* 4G: 电池 */
};

/* DIG1 段定义 (7段) */
static const SegmentDef dig1Segments[7] = {
    {TW_PIN1_PORT, TW_PIN1_PIN, TW_PIN2_PORT, TW_PIN2_PIN},  /* 1A */
    {TW_PIN1_PORT, TW_PIN1_PIN, TW_PIN3_PORT, TW_PIN3_PIN},  /* 1B */
    {TW_PIN1_PORT, TW_PIN1_PIN, TW_PIN4_PORT, TW_PIN4_PIN},  /* 1C */
    {TW_PIN1_PORT, TW_PIN1_PIN, TW_PIN5_PORT, TW_PIN5_PIN},  /* 1D */
    {TW_PIN1_PORT, TW_PIN1_PIN, TW_PIN6_PORT, TW_PIN6_PIN},  /* 1E */
    {TW_PIN1_PORT, TW_PIN1_PIN, TW_PIN7_PORT, TW_PIN7_PIN},  /* 1F */
    {TW_PIN2_PORT, TW_PIN2_PIN, TW_PIN1_PORT, TW_PIN1_PIN},  /* 1G */
};

/* DIG2 段定义 (7段) */
static const SegmentDef dig2Segments[7] = {
    {TW_PIN2_PORT, TW_PIN2_PIN, TW_PIN3_PORT, TW_PIN3_PIN},  /* 2A */
    {TW_PIN2_PORT, TW_PIN2_PIN, TW_PIN4_PORT, TW_PIN4_PIN},  /* 2B */
    {TW_PIN2_PORT, TW_PIN2_PIN, TW_PIN5_PORT, TW_PIN5_PIN},  /* 2C */
    {TW_PIN2_PORT, TW_PIN2_PIN, TW_PIN6_PORT, TW_PIN6_PIN},  /* 2D */
    {TW_PIN2_PORT, TW_PIN2_PIN, TW_PIN7_PORT, TW_PIN7_PIN},  /* 2E */
    {TW_PIN3_PORT, TW_PIN3_PIN, TW_PIN1_PORT, TW_PIN1_PIN},  /* 2F */
    {TW_PIN3_PORT, TW_PIN3_PIN, TW_PIN2_PORT, TW_PIN2_PIN},  /* 2G */
};

/* DIG3 段定义 (7段) */
static const SegmentDef dig3Segments[7] = {
    {TW_PIN3_PORT, TW_PIN3_PIN, TW_PIN4_PORT, TW_PIN4_PIN},  /* 3A */
    {TW_PIN3_PORT, TW_PIN3_PIN, TW_PIN5_PORT, TW_PIN5_PIN},  /* 3B */
    {TW_PIN3_PORT, TW_PIN3_PIN, TW_PIN6_PORT, TW_PIN6_PIN},  /* 3C */
    {TW_PIN3_PORT, TW_PIN3_PIN, TW_PIN7_PORT, TW_PIN7_PIN},  /* 3D */
    {TW_PIN4_PORT, TW_PIN4_PIN, TW_PIN1_PORT, TW_PIN1_PIN},  /* 3E */
    {TW_PIN4_PORT, TW_PIN4_PIN, TW_PIN2_PORT, TW_PIN2_PIN},  /* 3F */
    {TW_PIN4_PORT, TW_PIN4_PIN, TW_PIN3_PORT, TW_PIN3_PIN},  /* 3G */
};

/* DIG5 段定义 (6段) */
static const SegmentDef dig5Segments[6] = {
    {TW_PIN5_PORT, TW_PIN5_PIN, TW_PIN6_PORT, TW_PIN6_PIN},  /* 5A: ℃符号圆圈 */
    {TW_PIN5_PORT, TW_PIN5_PIN, TW_PIN7_PORT, TW_PIN7_PIN},  /* 5B: IN图标 */
    {TW_PIN6_PORT, TW_PIN6_PIN, TW_PIN1_PORT, TW_PIN1_PIN},  /* 5C: 圆形图标 */
    {TW_PIN6_PORT, TW_PIN6_PIN, TW_PIN2_PORT, TW_PIN2_PIN},  /* 5D: 百分号 */
    {TW_PIN6_PORT, TW_PIN6_PIN, TW_PIN3_PORT, TW_PIN3_PIN},  /* 5E: OUT图标 */
    {TW_PIN6_PORT, TW_PIN6_PIN, TW_PIN4_PORT, TW_PIN4_PIN},  /* 5F: OUT图标 */
};

/**
 * @brief  将所有引脚设置为高阻态（输入模式）
 */
static void TW25071_AllPinsHighZ(void)
{
    GPIO_Init(TW_PIN1_PORT, TW_PIN1_PIN, GPIO_MODE_IN | GPIO_PUPD_NOPULL);
    GPIO_Init(TW_PIN2_PORT, TW_PIN2_PIN, GPIO_MODE_IN | GPIO_PUPD_NOPULL);
    GPIO_Init(TW_PIN3_PORT, TW_PIN3_PIN, GPIO_MODE_IN | GPIO_PUPD_NOPULL);
    GPIO_Init(TW_PIN4_PORT, TW_PIN4_PIN, GPIO_MODE_IN | GPIO_PUPD_NOPULL);
    GPIO_Init(TW_PIN5_PORT, TW_PIN5_PIN, GPIO_MODE_IN | GPIO_PUPD_NOPULL);
    GPIO_Init(TW_PIN6_PORT, TW_PIN6_PIN, GPIO_MODE_IN | GPIO_PUPD_NOPULL);
    GPIO_Init(TW_PIN7_PORT, TW_PIN7_PIN, GPIO_MODE_IN | GPIO_PUPD_NOPULL);
}

/**
 * @brief  点亮单个段
 * @param  seg: 段定义指针
 */
static void TW25071_LightSegment(const SegmentDef* seg)
{
    /* 先将所有引脚设为高阻态 */
    TW25071_AllPinsHighZ();

    /* 设置阳极为输出高 */
    GPIO_Init(seg->anodePort, seg->anodePin, GPIO_MODE_OUT | GPIO_OTYPE_PP | GPIO_PUPD_NOPULL);
    GPIO_SetBits(seg->anodePort, seg->anodePin);

    /* 设置阴极为输出低 */
    GPIO_Init(seg->cathodePort, seg->cathodePin, GPIO_MODE_OUT | GPIO_OTYPE_PP | GPIO_PUPD_NOPULL);
    GPIO_ResetBits(seg->cathodePort, seg->cathodePin);
}

/**
 * @brief  初始化TW25071显示模块
 */
void TW25071_Init(void)
{
    /* 清空显示缓冲区 */
    displayBuffer.warningIcon = 0;
    displayBuffer.resetIcon = 0;
    displayBuffer.batteryIcon = 0;
    displayBuffer.thermometerIcon = 0;
    displayBuffer.digit1 = 0;
    displayBuffer.digit2 = 0;
    displayBuffer.digit3 = 0;
    displayBuffer.celsiusIcon = 0;
    displayBuffer.inIcon = 0;
    displayBuffer.circleIcon = 0;
    displayBuffer.percentIcon = 0;
    displayBuffer.outIcon = 0;

    /* 初始化扫描索引 */
    scanIndex = 0;

    /* 所有引脚设为高阻态 */
    TW25071_AllPinsHighZ();
}

/**
 * @brief  刷新显示（在定时器中断中调用，建议1-2ms间隔）
 * @note   采用动态扫描方式，每次调用点亮一个段
 */
void TW25071_Refresh(void)
{
    /* 总共需要扫描的段数: 7+7+7+7+6 = 34段 */
    #define TOTAL_SEGMENTS 34

    /* 先熄灭当前段 */
    TW25071_AllPinsHighZ();

    /* 根据扫描索引确定要点亮的段 */
    if (scanIndex < 7) {
        /* DIG4 区域 (0-6) */
        uint8_t segIdx = scanIndex;
        uint8_t shouldLight = 0;

        switch(segIdx) {
            case 0: shouldLight = displayBuffer.warningIcon; break;
            case 1: case 2: case 3: shouldLight = displayBuffer.thermometerIcon; break;
            case 4: shouldLight = displayBuffer.resetIcon; break;
            case 5: case 6: shouldLight = displayBuffer.batteryIcon; break;
        }

        if (shouldLight) {
            TW25071_LightSegment(&dig4Segments[segIdx]);
        }
    }
    else if (scanIndex < 14) {
        /* DIG1 区域 (7-13) */
        uint8_t segIdx = scanIndex - 7;
        if (displayBuffer.digit1 & (1 << segIdx)) {
            TW25071_LightSegment(&dig1Segments[segIdx]);
        }
    }
    else if (scanIndex < 21) {
        /* DIG2 区域 (14-20) */
        uint8_t segIdx = scanIndex - 14;
        if (displayBuffer.digit2 & (1 << segIdx)) {
            TW25071_LightSegment(&dig2Segments[segIdx]);
        }
    }
    else if (scanIndex < 28) {
        /* DIG3 区域 (21-27) */
        uint8_t segIdx = scanIndex - 21;
        if (displayBuffer.digit3 & (1 << segIdx)) {
            TW25071_LightSegment(&dig3Segments[segIdx]);
        }
    }
    else if (scanIndex < TOTAL_SEGMENTS) {
        /* DIG5 区域 (28-33) */
        uint8_t segIdx = scanIndex - 28;
        uint8_t shouldLight = 0;

        switch(segIdx) {
            case 0: shouldLight = displayBuffer.celsiusIcon; break;
            case 1: shouldLight = displayBuffer.inIcon; break;
            case 2: shouldLight = displayBuffer.circleIcon; break;
            case 3: shouldLight = displayBuffer.percentIcon; break;
            case 4: case 5: shouldLight = displayBuffer.outIcon; break;
        }

        if (shouldLight) {
            TW25071_LightSegment(&dig5Segments[segIdx]);
        }
    }

    /* 更新扫描索引 */
    scanIndex++;
    if (scanIndex >= TOTAL_SEGMENTS) {
        scanIndex = 0;
    }
}

/**
 * @brief  函数1: 控制警告图标 (段4A)
 * @param  state: DISPLAY_ON 亮, DISPLAY_OFF 灭
 */
void TW25071_SetWarningIcon(DisplayState state)
{
    displayBuffer.warningIcon = (state == DISPLAY_ON) ? 1 : 0;
}

/**
 * @brief  函数2: 控制重置图标 (段4E)
 * @param  state: DISPLAY_ON 亮, DISPLAY_OFF 灭
 */
void TW25071_SetResetIcon(DisplayState state)
{
    displayBuffer.resetIcon = (state == DISPLAY_ON) ? 1 : 0;
}

/**
 * @brief  函数3: 控制电池图标 (段4F, 4G)
 * @param  state: DISPLAY_ON 亮, DISPLAY_OFF 灭
 */
void TW25071_SetBatteryIcon(DisplayState state)
{
    displayBuffer.batteryIcon = (state == DISPLAY_ON) ? 1 : 0;
}

/**
 * @brief  函数4: 控制温度计图标 (段4B, 4C, 4D)
 * @param  state: DISPLAY_ON 亮, DISPLAY_OFF 灭
 */
void TW25071_SetThermometerIcon(DisplayState state)
{
    displayBuffer.thermometerIcon = (state == DISPLAY_ON) ? 1 : 0;
}

/**
 * @brief  函数5: 控制中间三位数码管显示数字
 * @param  value: 0-999的数字 (超过999显示999)
 * @note   物理位置从左到右: DIG1(百位) - DIG2(十位) - DIG3(个位)
 */
void TW25071_SetNumber(uint16_t value)
{
    /* 限制最大值 */
    if (value > 999) {
        value = 999;
    }

    /* 分解数字 */
    uint8_t hundreds = value / 100;
    uint8_t tens = (value / 10) % 10;
    uint8_t ones = value % 10;

    /* 设置段码 */
    /* DIG1显示百位，DIG2显示十位，DIG3显示个位 */
    if (value >= 100) {
        displayBuffer.digit1 = digitSegments[hundreds];
    } else {
        displayBuffer.digit1 = 0;  /* 百位为0时不显示 */
    }

    if (value >= 10) {
        displayBuffer.digit2 = digitSegments[tens];
    } else {
        displayBuffer.digit2 = 0;  /* 十位为0且百位为0时不显示 */
    }

    displayBuffer.digit3 = digitSegments[ones];  /* 个位始终显示 */
}

/**
 * @brief  函数6: 控制摄氏度符号 ℃ (段5A)
 * @param  state: DISPLAY_ON 亮, DISPLAY_OFF 灭
 */
void TW25071_SetCelsiusIcon(DisplayState state)
{
    displayBuffer.celsiusIcon = (state == DISPLAY_ON) ? 1 : 0;
}

/**
 * @brief  函数7: 控制IN图标 (段5B)
 * @param  state: DISPLAY_ON 亮, DISPLAY_OFF 灭
 */
void TW25071_SetInIcon(DisplayState state)
{
    displayBuffer.inIcon = (state == DISPLAY_ON) ? 1 : 0;
}

/**
 * @brief  函数8: 控制圆形图标/绿色50 (段5C)
 * @param  state: DISPLAY_ON 亮, DISPLAY_OFF 灭
 */
void TW25071_SetCircleIcon(DisplayState state)
{
    displayBuffer.circleIcon = (state == DISPLAY_ON) ? 1 : 0;
}

/**
 * @brief  函数9: 控制百分号 % (段5D)
 * @param  state: DISPLAY_ON 亮, DISPLAY_OFF 灭
 */
void TW25071_SetPercentIcon(DisplayState state)
{
    displayBuffer.percentIcon = (state == DISPLAY_ON) ? 1 : 0;
}

/**
 * @brief  函数10: 控制OUT图标 (段5E, 5F)
 * @param  state: DISPLAY_ON 亮, DISPLAY_OFF 灭
 */
void TW25071_SetOutIcon(DisplayState state)
{
    displayBuffer.outIcon = (state == DISPLAY_ON) ? 1 : 0;
}
