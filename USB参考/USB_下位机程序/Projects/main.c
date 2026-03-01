/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>

#include "wb7720.h"
#include "wb7720_tim.h"
#include "usbd_core.h"
#include "tw25071_display.h"
#include "bsp_uart0.h"
#include "config.h"

void mcu_stop_mode(void);

#ifndef BMS_REGISTERS_H
#define BMS_REGISTERS_H

#define REG_SOC_PCT                  0x00    // ???????(%)
#define REG_CHARGE_STATE             0x17    // ????
#define REG_DISCHARGE_STATE          0x18    // ????
#define REG_LED_BRIGHTNESS           0x25    // ?????
#define REG_LED_STATE                0x26    // ?????
#define REG_FAN_SPEED                0x27    // ??????
#define REG_PROTECTION_FLAG          0x28    // ????
#define REG_FW_VERSION_MAJOR         0x29    // ?????
#define REG_FW_VERSION_MINOR         0x2A    // ?????
#define REG_HW_VERSION               0x2B    // ????
#define REG_CELL_COUNT               0x2C    // ????
/* 0x37~0x4D: 异常日志滚动写入区 (REG_EXC_*, 见 config.h) */
/* 0x4E~0x4F: REG_SLEEP_CMD / REG_WAKE_CMD (见 config.h) */

#define REG_CAPACITY_MAH             0x01    // ???(mAh)
#define REG_VBAT_MV                  0x05    // ????(mV)
#define REG_IBAT_MA                  0x07    // ????(mA)
#define REG_TEMP_DC                  0x09    // ????(0.1?)
#define REG_CYCLE_COUNT              0x0B    // ????
#define REG_R_INTERNAL_MOHM          0x0D    // ??(mO)
#define REG_SOH_PCT_X100             0x0F    // ???(0.01%)
#define REG_ERR_OVERTEMP_CNT         0x11    // ??????
#define REG_ERR_OVERVOLT_CNT         0x13    // ??????
#define REG_ERR_OVERCURR_CNT         0x15    // ??????
#define REG_INPUT_VOLTAGE_MV         0x19    // ????(mV)
#define REG_INPUT_CURRENT_MA         0x1B    // ????(mA)
#define REG_OUTPUT_VOLTAGE_MV        0x1D    // ????(mV)
#define REG_OUTPUT_CURRENT_MA        0x1F    // ????(mA)
#define REG_REMAINING_TIME_CHARGE    0x21    // ??????(??)
#define REG_REMAINING_TIME_DISCHARGE 0x23    // ??????(??)
#define REG_CELL1_VOLTAGE_MV         0x2D    // ??1??(mV)
#define REG_CELL2_VOLTAGE_MV         0x2F    // ??2??(mV)
#define REG_CELL3_VOLTAGE_MV         0x31    // ??3??(mV)
#define REG_CELL4_VOLTAGE_MV         0x33    // ??4??(mV)
#define REG_PCB_TEMP_DC              0x35    // PCB??(0.1?)
/* 0x38~0x4D 已分配给异常日志区 (见 config.h REG_EXC_*), 旧定义已移除:
 * REG_DISCHARGE_CUTOFF_VOLTAGE(0x38), REG_CHARGE_CUTOFF_VOLTAGE(0x3A),
 * REG_TOTAL_CHARGE_CAPACITY(0x3C), REG_TOTAL_DISCHARGE_CAPACITY(0x40) */

/* REG_SLEEP_CMD(0x4E) / REG_WAKE_CMD(0x4F) 已迁移至 config.h (原 0x44/0x45 落在异常日志区内) */

#define REG_RESERVED_START           0x46    // ????????
#define REG_RESERVED_END             0xFF    // ????????

// 工程模式寄存器组
#define REG_WORK_MODE               0x50    // 工作模式: 0x00=用户, 0xA5=工程
#define ENGINEERING_MODE_KEY        0xA5    // 进入工程模式的密钥

#define REG_ENG_CURRENT_DATE        0x60    // 工程当前日期 Year(u16)+Month(u8)+Day(u8)
#define REG_ENG_PRODUCTION_DATE     0x70    // 工程生产日期 Year(u16)+Month(u8)+Day(u8)
#define REG_ENG_CYCLE_CHG_COUNT     0x80    // 循环充次数 u16 little-endian (2字节)

#endif /* BMS_REGISTERS_H */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint8_t i2c_buff[256] = {0};
uint32_t i2c_cnt     = 0;

#define Brand_STRING		"Nu17113"	

volatile uint32_t ep_out_evt = 0;
volatile uint32_t ep_in_busy = 0;

static __ALIGNED(4) uint8_t Vendor_Request[64];
static __ALIGNED(4) uint8_t Vendor_Response[64];

/* 数码管显示相关变量 */
static volatile uint32_t display_tick_count = 0;  /* 1ms计数器 */
static uint8_t display_number = 0;                 /* 当前显示数字 0-100 */

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


void usb_disable(void)
{
    USBD_DeInit();
    USBD_Disconnect();
}

void usb_enable(void)
{
    USBD_Init();
    USBD_Connect();
}

void ubs_chip_sleep(void)
{

}

void ubs_chip_wake(void)
{

}


uint16_t crc16(const uint8_t* p, uint16_t n){
  uint16_t c=0xFFFF;
  for(uint16_t i=0;i<n;i++){
    c^=((uint16_t)p[i]<<8);
    for(int b=0;b<8;b++){
      if(c&0x8000) c=(c<<1)^0x1021;
      else c=(c<<1);
    }
  }
  return c;
}

static uint8_t report_buffer[64] ;
static uint8_t index = 0;

/* -----------------------------------------------------------------------
 * 异常日志本地缓存系统 (v1.3)
 * WB7720 在每次组装 Type 0x01 时顺带检查 i2c_buff 异常日志区,
 * 按 record_id 去重, 最多缓存 EXC_CACHE_MAX(5) 条 20B 记录.
 * ----------------------------------------------------------------------- */
static uint8_t exc_cache[EXC_CACHE_MAX][EXC_RECORD_SIZE];  /* 5x20B 本地缓存 */
static uint8_t exc_cache_count = 0;   /* 当前缓存条数 (0~5) */
static uint8_t exc_page_idx = 0;      /* Type 0x02 输出页码 (0~2, 循环) */

/* 从 20B 记录偏移 +16 提取 u32 LE record_id */
static uint32_t get_record_id(const uint8_t *rec)
{
    return (uint32_t)rec[16]
         | ((uint32_t)rec[17] << 8)
         | ((uint32_t)rec[18] << 16)
         | ((uint32_t)rec[19] << 24);
}

/* 检查 i2c_buff[REG_EXC_RECORD] 中的记录, 若 record_id 非 0 且不在缓存中则追加 */
static void exc_cache_update(void)
{
    const uint8_t *src = &i2c_buff[REG_EXC_RECORD];
    uint32_t rid = get_record_id(src);

    /* record_id == 0 表示无有效记录, 跳过 */
    if (rid == 0) return;

    /* 缓存已满, 不再追加 */
    if (exc_cache_count >= EXC_CACHE_MAX) return;

    /* 按 record_id 去重: 检查是否已在缓存中 */
    for (uint8_t i = 0; i < exc_cache_count; i++) {
        if (get_record_id(exc_cache[i]) == rid) return;
    }

    /* 追加到缓存 */
    memcpy(exc_cache[exc_cache_count], src, EXC_RECORD_SIZE);
    exc_cache_count++;
}

void update_report_buffer_0(void)
{
	memset(report_buffer, 0, sizeof(report_buffer));
	
	uint32_t bat_mAh =  (uint32_t )(i2c_buff[REG_CAPACITY_MAH] | i2c_buff[REG_CAPACITY_MAH + 1] << 8 | i2c_buff[REG_CAPACITY_MAH + 2] << 16 | i2c_buff[REG_CAPACITY_MAH + 3] << 24) ;
	uint16_t adc_vbat = (uint16_t )(i2c_buff[REG_VBAT_MV] | i2c_buff[REG_VBAT_MV + 1] << 8);
	int16_t adc_ibat = (int16_t )(i2c_buff[REG_IBAT_MA] | i2c_buff[REG_IBAT_MA + 1] << 8);
	uint16_t cycle_cnt = (uint16_t )(i2c_buff[REG_CYCLE_COUNT] | i2c_buff[REG_CYCLE_COUNT + 1] << 8);
	uint16_t Bat_Rdc = (uint16_t )(i2c_buff[REG_R_INTERNAL_MOHM] | i2c_buff[REG_R_INTERNAL_MOHM + 1] << 8);
	uint16_t Bat_SoH = (uint16_t )(i2c_buff[REG_SOH_PCT_X100] | i2c_buff[REG_SOH_PCT_X100 + 1] << 8);
	int16_t battery_temp = (int16_t )(i2c_buff[REG_TEMP_DC] | i2c_buff[REG_TEMP_DC + 1] << 8);
	int16_t baord_temp = (int16_t )(i2c_buff[REG_PCB_TEMP_DC] | i2c_buff[REG_PCB_TEMP_DC + 1] << 8);
	
	uint16_t Bat_Cell1_Voltage = (uint16_t )(i2c_buff[REG_CELL1_VOLTAGE_MV] | i2c_buff[REG_CELL1_VOLTAGE_MV + 1] << 8);
  uint16_t Bat_Cell2_Voltage = (uint16_t )(i2c_buff[REG_CELL2_VOLTAGE_MV] | i2c_buff[REG_CELL2_VOLTAGE_MV + 1] << 8);
	
	uint8_t len =  35 + sizeof(Brand_STRING) - 1;  // 35 + 8 - 1 = 42

	
	uint16_t crc = 0;
	report_buffer[0] = 0x05;											//- SOF?????0xA5 0x5A
	report_buffer[1] = 0xA5;											//- SOF?????0xA5 0x5A
	report_buffer[2] = 0x5A;

	report_buffer[3] = 0x02;											//| Ver | u8 | ???????????? 0x02 |
	report_buffer[4] = 0x01;											//| Type | u8 | ????????x10=GET_TELEMETRY??x01=TELEMETRY |
	report_buffer[5] = index++;											//| Seq | u16 | ???????????????16 ?????????????????|
	report_buffer[6] = 0;
	report_buffer[7] = len;												//| Len | u16 | ???????????ELEMETRY ?????35?????34 + BrandLen(1)???????????|
	report_buffer[8] = 0;

	report_buffer[9] =   i2c_buff[REG_SOC_PCT];  //1						//| 1 | SOC_pct | u8 | % | 0..100 |
	report_buffer[10] =  (uint8_t)(bat_mAh >>0);//2						//| 2 | Capacity_mAh | u32 | mAh | ??????????????|
	report_buffer[11] =  (uint8_t)(bat_mAh >>8);//3
	report_buffer[12] =  (uint8_t)(bat_mAh >>16);//4
	report_buffer[13] =  (uint8_t)(bat_mAh >>24);//5
	report_buffer[14] =  (uint8_t)((adc_vbat /10) >>0);//6		//| 3 | TotalVoltage_V_x100 | u16 | V?100 | ???????????????=V?100??|
	report_buffer[15] =  (uint8_t)((adc_vbat /10) >>8);//7
	report_buffer[16] =  (uint8_t)((adc_ibat /10) >>0);//8		//| 4 | TotalCurrent_A_x100 | s16 | A?100 | ???????????????=A?100???????c?????????|
	report_buffer[17] =  (uint8_t)((adc_ibat /10) >>8);//9
	report_buffer[18] =  (uint8_t)((adc_vbat * adc_ibat /100000) >> 0);//10						//| 5 | Power_W_x10 | s16 | W?10 | ?????????????????W?10???????????|
	report_buffer[19] =  (uint8_t)((adc_vbat * adc_ibat /100000) >> 8);//11

	report_buffer[20] = i2c_buff[REG_CHARGE_STATE];//12								//| 6 | ChargeState | u8 | enum | 0=?????=?e??????=?e???? |
	report_buffer[21] = cycle_cnt >> 0;//13							//| 7 | CycleCount | u16 | ??| ???????????? |//| 7 | CycleCount | u16 | ??| ???????????? |
	report_buffer[22] = cycle_cnt >> 8;//14
	report_buffer[23] = (uint8_t)battery_temp;//15								//| 8 | BatteryTemp_C | s16 | ?C | ???????????? |
	report_buffer[24] = (uint8_t)(battery_temp >> 8);//16
	report_buffer[25] = (uint8_t)(baord_temp >>0);//17					//| 9 | BoardTemp_C | s16 | ?C | ???????????? |
	report_buffer[26] = (uint8_t)(baord_temp >>8);;//18
	report_buffer[27] = 2;//19											//| 10 | CellCount | u8 | ??| ???????????? |
	report_buffer[28] = (uint8_t)((Bat_Cell1_Voltage /10) >>0);;//20		//| 11 | Cell1Voltage_V_x100 | u16 | V?100 | ???1??????????????V?100??|
	report_buffer[29] = (uint8_t)((Bat_Cell1_Voltage /10) >>8);;//21
	report_buffer[30] = (uint8_t)((Bat_Cell2_Voltage /10) >>0);;//22											//| 12 | Cell2Voltage_V_x100 | u16 | V?100 | ???2???????????~ 0xFFFF |
	report_buffer[31] = (uint8_t)((Bat_Cell2_Voltage /10) >>8);;//23
	report_buffer[32] = Bat_Rdc;//24											//| 13 | R_internal_mOhm | u16 | m? | ???????????0xFFFF??|
	report_buffer[33] = 0;//25
	report_buffer[34] = (uint8_t)(Bat_SoH *100 >>0);;//26					//| 14 | SOH_pct_x100 | u16 | 0.01% | ???????????%?100????????? |
	report_buffer[35] = (uint8_t)(Bat_SoH *100 >>8);//27
	report_buffer[36] = i2c_buff[REG_ERR_OVERTEMP_CNT];//28             //| 15 | ErrOverTempCnt | u16 | ??| 过温异常次数 |
	report_buffer[37] = i2c_buff[REG_ERR_OVERTEMP_CNT + 1];//29
	report_buffer[38] = i2c_buff[REG_ERR_OVERVOLT_CNT];//30             //| 16 | ErrOverVoltCnt | u16 | ??| 过压异常次数 |
	report_buffer[39] = i2c_buff[REG_ERR_OVERVOLT_CNT + 1];//31
	report_buffer[40] = i2c_buff[REG_ERR_OVERCURR_CNT];//32             //| 17 | ErrOverCurrCnt | u16 | ??| 过流异常次数 |
	report_buffer[41] = i2c_buff[REG_ERR_OVERCURR_CNT + 1];//33
	report_buffer[42] = exc_cache_count;//34                              //| 18 | ExceptionLogCount | u8 | 条 | 来自 exc_cache 实际缓存条数 |




	report_buffer[43] = sizeof(Brand_STRING) - 1;//35											//| 19+ExceptionLogCount | BrandLen | u8 | ??? | ????????????0 ??????????????|

	char* str = Brand_STRING;
	for(uint8_t i = 0; i < sizeof(Brand_STRING) - 1; i++)
	{
		report_buffer[44 + i] = str[i];
	}//50
	

	crc = crc16(&report_buffer[3],41 + sizeof(Brand_STRING) - 1);//48
	report_buffer[44 + sizeof(Brand_STRING) -1] = crc >>0;//35
	report_buffer[45 + sizeof(Brand_STRING) -1] = crc >>8;//36

	/* 每次组装 Type 0x01 遥测报告后, 顺带检查异常日志区并缓存新记录 */
	exc_cache_update();
}


/* Type 0x02 异常日志, 从 exc_cache 分页输出 (v1.3)
 *
 * 分页策略 (每页最多 2 条, 共 3 页):
 *   page 0: exc_cache[0] + exc_cache[1]  (OffsetPage=0)
 *   page 1: exc_cache[2] + exc_cache[3]  (OffsetPage=1)
 *   page 2: exc_cache[4]                 (OffsetPage=2)
 * 页码由 exc_page_idx 自动推进: 0→1→2→0→...
 * 无记录时发 ReturnCount=0 的空包 (Len=2).
 *
 * Payload (从偏移9):
 *   +0: OffsetPage (u8)
 *   +1: ReturnCount (u8, 0~2)
 *   +2...: ExceptionLogs (20B × ReturnCount)
 *
 * 最大包: 9 + 2 + 40 + 2 = 53B (在 64B 限制内)
 */
void update_report_buffer_1(void)
{
    memset(report_buffer, 0, sizeof(report_buffer));

    /* 计算当前页的起始索引和返回条数 */
    uint8_t page_start = exc_page_idx * 2;  /* 0, 2, 4 */
    uint8_t return_count = 0;
    if (exc_cache_count > page_start) {
        return_count = exc_cache_count - page_start;
        if (return_count > 2) return_count = 2;
    }

    uint16_t len = (uint16_t)return_count * EXC_RECORD_SIZE + 2;

    /* --- 协议头 (9B) --- */
    report_buffer[0] = 0x05;  /* SOF[0] */
    report_buffer[1] = 0xA5;  /* SOF[1] */
    report_buffer[2] = 0x5A;  /* SOF[2] */

    report_buffer[3] = 0x02;                          /* Ver */
    report_buffer[4] = REPORT_TYPE_EXCEPTION_LOG;     /* Type = 0x02 */
    report_buffer[5] = index++;                       /* Seq Lo */
    report_buffer[6] = 0;                             /* Seq Hi */
    report_buffer[7] = (uint8_t)(len >> 0);           /* Len Lo */
    report_buffer[8] = (uint8_t)(len >> 8);           /* Len Hi */

    /* --- Payload --- */
    report_buffer[9]  = exc_page_idx;     /* OffsetPage */
    report_buffer[10] = return_count;     /* ReturnCount */

    /* 从 exc_cache 中 memcpy 记录到 report_buffer (20B 对齐) */
    for (uint8_t i = 0; i < return_count; i++) {
        memcpy(&report_buffer[11 + i * EXC_RECORD_SIZE],
               exc_cache[page_start + i],
               EXC_RECORD_SIZE);
    }

    /* --- CRC16 --- */
    uint16_t crc = crc16(&report_buffer[3], (uint16_t)(6 + len));
    uint16_t crc_pos = (uint16_t)(9 + len);
    report_buffer[crc_pos]     = (uint8_t)(crc >> 0);
    report_buffer[crc_pos + 1] = (uint8_t)(crc >> 8);

    /* 页码自动推进: 0→1→2→0→... */
    exc_page_idx++;
    if (exc_page_idx > 2) exc_page_idx = 0;
}


/* Type 0x03 设备信息, SubIdx 单内容 (v1.2)
 * CMD 0x02: Vendor_Request[1] = sub_idx
 * Payload (从偏移9), Len = 41:
 *   +0:     SubIdx (u8)
 *   +1~+20: Field1 (char[20])
 *   +21~+40: Field2 (char[20])
 * CRC = crc16(&report_buffer[3], 6 + 41) = crc16(&report_buffer[3], 47)
 *
 * SubIdx 对应内容 (从 i2c_buff 读取):
 *   0x00: manufacturer_name (0x92~0xA5, 20B) + model_name (0xA6~0xB9, 20B)
 *   0x01: battery_mfr (0xBA~0xCD, 20B) + battery_model (0xCE~0xE1, 20B)
 *   0x02: battery_prod_date (0xE2~0xF5, 20B) + safety_years (1B = SAFETY_SERVICE_YEARS) + reserved (19B, 0x00)
 */
void update_report_buffer_device_info(uint8_t sub_idx)
{
    memset(report_buffer, 0, sizeof(report_buffer));

    const uint8_t PAYLOAD_LEN = 41;  /* SubIdx(1) + Field1(20) + Field2(20) */

    report_buffer[0] = 0x05;  /* SOF[0] */
    report_buffer[1] = 0xA5;  /* SOF[1] */
    report_buffer[2] = 0x5A;  /* SOF[2] */

    report_buffer[3] = 0x02;                        /* Ver */
    report_buffer[4] = REPORT_TYPE_DEVICE_INFO;     /* Type = 0x03 */
    report_buffer[5] = index++;                     /* Seq Lo */
    report_buffer[6] = 0;                           /* Seq Hi */
    report_buffer[7] = PAYLOAD_LEN;                 /* Len Lo */
    report_buffer[8] = 0;                           /* Len Hi */

    report_buffer[9] = sub_idx;                     /* SubIdx */

    switch (sub_idx) {
        case 0x00:
            /* Field1: manufacturer_name (20B from i2c_buff[0x92..0xA5]) */
            memcpy(&report_buffer[10], &i2c_buff[REG_PROD_MANUFACTURER], 20);
            /* Field2: model_name (20B from i2c_buff[0xA6..0xB9]) */
            memcpy(&report_buffer[30], &i2c_buff[REG_PROD_MODEL], 20);
            break;
        case 0x01:
            /* Field1: battery_mfr (20B from i2c_buff[0xBA..0xCD]) */
            memcpy(&report_buffer[10], &i2c_buff[REG_PROD_BATTERY_MFR], 20);
            /* Field2: battery_model (20B from i2c_buff[0xCE..0xE1]) */
            memcpy(&report_buffer[30], &i2c_buff[REG_PROD_BATTERY_MODEL], 20);
            break;
        case 0x02:
            /* Field1: battery_prod_date (20B from i2c_buff[0xE2..0xF5]) */
            memcpy(&report_buffer[10], &i2c_buff[REG_PROD_PROD_DATE], 20);
            /* Field2: safety_years (1B hardcoded) + reserved (19B, 0x00) */
            report_buffer[30] = SAFETY_SERVICE_YEARS;
            /* report_buffer[31..49] already 0 from memset */
            break;
        default:
            /* 未知 SubIdx: Field1/Field2 全郥0 */
            break;
    }

    /* CRC = crc16(&report_buffer[3], 6 + Len) = crc16(&report_buffer[3], 47) */
    uint16_t crc = crc16(&report_buffer[3], (uint16_t)(6 + PAYLOAD_LEN));
    report_buffer[9 + PAYLOAD_LEN]     = (uint8_t)(crc >> 0);  /* report_buffer[50] */
    report_buffer[9 + PAYLOAD_LEN + 1] = (uint8_t)(crc >> 8);  /* report_buffer[51] */
}


void user_init(void) {

    SystemCoreClockUpdate();
		//usb_disable();
    USBD_Disconnect();
    SysTick_DelayMs(1500);

    USBD_Init();
    USBD_Connect();

    RCC_APBPeriphClockCmd(RCC_APBPeriph_GPIO | RCC_APBENR_I2CEN, ENABLE);
    I2C_DeInit();

    /*
      PC0  (I2C_SCL)
      PC1  (I2C_SDA)
    */
    GPIO_Init(GPIOC, GPIO_Pin_0 | GPIO_Pin_1, GPIO_MODE_AF | GPIO_OTYPE_OD | GPIO_PUPD_NOPULL | GPIO_SPEED_HIGH | GPIO_AF2);

    I2C_OwnAddressConfig(0x42);
    I2C_Config(I2C_CON_AA);
    I2C_Cmd(ENABLE);

    NVIC_SetPriority(I2C_IRQn, 0);
    NVIC_EnableIRQ(I2C_IRQn);

    /* 初始化数码管显示 */
    TW25071_Init();
    TW25071_SetPercentIcon(DISPLAY_ON);  /* 显示百分号 */
    TW25071_SetNumber(0);                 /* 初始显示0 */

    /* 初始化UART0 debug串口 (PA2 = UART0_TX, 115200bps) */
    uart0_init(SystemCoreClock, 115200);
    printf("UART0 debug init OK\r\n");

    /* 测试：配置PA4为推挽输出 */
    GPIO_Init(GPIOA, GPIO_Pin_4, GPIO_MODE_OUT | GPIO_OTYPE_PP | GPIO_PUPD_NOPULL);

    //usb_disable();

		//usb_enable();
}

/* Infinite loop */
void user_loop(void) {
    /* 测试：PB0和PA4高低翻转 */
    //static uint8_t gpio_state = 0;
    //if (gpio_state) {
     //   GPIO_SetBits(GPIOB, GPIO_Pin_0);
     //   GPIO_SetBits(GPIOA, GPIO_Pin_4);
     //   gpio_state = 0;
    //} else {
    //    GPIO_ResetBits(GPIOB, GPIO_Pin_0);
    //    GPIO_ResetBits(GPIOA, GPIO_Pin_4);
    //    gpio_state = 1;
    //}
    //SysTick_DelayNticks((SystemCoreClock / 1000) * 500);  /* 延时500ms */
    //return;  /* 暂时跳过后面的代码 */

    /* 刷新数码管显示 */
    //TW25071_Refresh();

    /* 数码管0-100循环显示，1秒间隔 */
    
		static uint32_t loop_count = 0;
    loop_count++;
    if (loop_count >= 10000) {  /* 约1秒更新一次数字 */
        loop_count = 0;
    //    TW25071_SetNumber(display_number);
    //    display_number++;
    //    if (display_number > 100) {
    //        display_number = 0;
    //    }
			printf("MODE=0x%02X DATE=%04d-%02d-%02d PROD=%04d-%02d-%02d CYCLE=%u\r\n",
				i2c_buff[REG_WORK_MODE],
				(uint16_t)(i2c_buff[REG_ENG_CURRENT_DATE] | (i2c_buff[REG_ENG_CURRENT_DATE+1]<<8)),
				i2c_buff[REG_ENG_CURRENT_DATE+2], i2c_buff[REG_ENG_CURRENT_DATE+3],
				(uint16_t)(i2c_buff[REG_ENG_PRODUCTION_DATE] | (i2c_buff[REG_ENG_PRODUCTION_DATE+1]<<8)),
				i2c_buff[REG_ENG_PRODUCTION_DATE+2], i2c_buff[REG_ENG_PRODUCTION_DATE+3],
				(uint16_t)(i2c_buff[REG_ENG_CYCLE_CHG_COUNT] | (i2c_buff[REG_ENG_CYCLE_CHG_COUNT+1]<<8)));
    }
		

		if(i2c_buff[REG_SLEEP_CMD])
		{
			i2c_buff[REG_SLEEP_CMD] = 0;
			SysTick_DelayNticks((SystemCoreClock / 1000) * 100); // delay 100ms
			mcu_stop_mode();
		}
		
		if(i2c_buff[REG_WAKE_CMD])
		{
			i2c_buff[REG_WAKE_CMD] = 0;
		usb_enable();
		}
		

    if (ep_out_evt) {
        int flag;

        flag = 0;
        __disable_irq();
        if (ep_out_evt) {
            uint32_t len = USBD_HW_GetRxDataCount(0x01);
            memset(Vendor_Request, 0, sizeof(Vendor_Request));
            USBD_HW_ReadEP(0x01, Vendor_Request, len);
            flag = 1;
        }
        __enable_irq();

        if (flag) {
            memset(Vendor_Response, 0, sizeof(Vendor_Response));
            static uint8_t rr_index = 0;  /* round-robin计数器, 与 Seq 的 index 分离 */
            if (Vendor_Request[0] == CMD_READ_DEVICE_INFO) {
                /* CMD 0x02: 读设备信息, Vendor_Request[1] = sub_idx */
                update_report_buffer_device_info(Vendor_Request[1]);
            } else if (Vendor_Request[0] == CMD_WRITE_REGISTER) {
                /* CMD 0x0C: 写寄存器后返回 Type 0x01 (不走 round-robin) */
                update_report_buffer_0();
            } else {
                /* CMD 0x01 (CMD_READ_STATUS): round-robin
                 * rr_index 0~3: 返回 Type 0x01 遥测
                 * rr_index 4  : 返回 Type 0x02 异常日志 */
                if (rr_index >= 4) {
                    update_report_buffer_1();
                } else {
                    update_report_buffer_0();
                }
                rr_index++;
                if (rr_index > 4) rr_index = 0;
            }
            memcpy(Vendor_Response, report_buffer, sizeof(Vendor_Response));

            if (Vendor_Request[0] == 0x0C) {
                /* 写寄存器命令: [0]=0x0C, [1]=寄存器地址, [2]=数据长度, [3..]=数据 */
                uint8_t reg_addr = Vendor_Request[1];
                uint8_t reg_len  = Vendor_Request[2];
                if (reg_len > 60) reg_len = 60; /* 防越界: 64 - 4 */
                for (uint8_t i = 0; i < reg_len; i++) {
                    uint8_t wr_addr = reg_addr + i;
                    /* 工程模式写保护：仅擦除/刷新命令寄存器需要工程模式密钥
                     * DATA 寄存器 (0x60~0x87) 允许无条件写入，因为它们只是参数缓冲区，
                     * NU17112 仅在检测到 0x50=0xA5 时才读取这些参数。
                     * 这允许 PC 在设置工程模式标志之前预写参数，避免时序竞争。 */
                    if (wr_addr == REG_ENG_ERASE_ALL_CMD) {
                        /* 擦除/刷新命令: 仅在工程模式下可写 */
                        if (i2c_buff[REG_WORK_MODE] == ENGINEERING_MODE_KEY)
                            i2c_buff[wr_addr] = Vendor_Request[3 + i];
                    }
                    else
                    {
                        i2c_buff[wr_addr] = Vendor_Request[3 + i];
                    }
                }
            }

            if (Vendor_Request[0] == 0x0B) {
                SysTick_DelayNticks((SystemCoreClock / 1000) * 100); // delay 100ms
                USBD_DeInit();
                USBD_Disconnect();
                /* Delay 1500ms to ensure the device is disconnected */
                for (int i = 0; i < 15; i++) {
										//printf("UART0 debug init OK\r\n");
                    SysTick_DelayNticks((SystemCoreClock / 1000) * 100); // delay 100ms
                }
                RCC->GPREG0 = 0x4A379CEB;
                NVIC_SystemReset();
                while (1);
            }

            __disable_irq();
            if (ep_in_busy == 0) {
                USBD_HW_Transmit(0x81, Vendor_Response, 64);
                ep_in_busy = 1;
            }
            __enable_irq();

            /* Waiting for the response to be sent to the host */
            while (ep_in_busy);

            __disable_irq();
            ep_out_evt = 0;
            USBD_HW_ReadyToReceive(0x01); /* Ready to get next OUT */
            __enable_irq();
        }
    }
}


void USB_IRQHandler(void) {
    uint8_t IntrUSB;
    uint8_t IntrIn;
    uint8_t IntrOut;

    IntrUSB = USB->INTRUSB;
    IntrIn  = USB->INTRIN;
    IntrOut = USB->INTROUT;

    Handle_USBD_INT(IntrUSB, IntrIn, IntrOut);
}

void I2C_IRQHandler(void) {
    uint8_t status;
    static uint8_t i2c_seg_addr = 0;
    status = I2C_GetBusStatus();
    static uint8_t next_is_segaddr  = 0;
    switch (status) {
        case 0x60: { /* Own SLA+W has been received; ACK has been returned */
            i2c_cnt = 0;
            next_is_segaddr = 1;
            I2C_Config(I2C_CON_AA);
        } break;
        case 0x80:  /* Previously addressed with own SLV address; DATA has been received; ACK has been returned */
            if(next_is_segaddr)
            {
                next_is_segaddr = 0;
                i2c_cnt = I2C_ReadData();
            }
            else
						{
                uint8_t wr_data = I2C_ReadData();
                uint8_t wr_addr = (uint8_t)i2c_cnt;

                /* 工程模式写保护：仅擦除/刷新命令寄存器需要工程模式密钥
                 * DATA 寄存器 (0x60~0x87) 允许无条件写入 (参数缓冲区)。 */
                if (wr_addr == REG_ENG_ERASE_ALL_CMD) {
                    /* 擦除/刷新命令: 仅在工程模式下可写 */
                    if (i2c_buff[REG_WORK_MODE] == ENGINEERING_MODE_KEY)
                        i2c_buff[wr_addr] = wr_data;
                    /* 非工程模式下丢弃写入 */
                }
                else
                {
                    i2c_buff[wr_addr] = wr_data;
                }
                i2c_cnt++;
            }
            I2C_Config(I2C_CON_AA);
         break;
        case 0xA8: { /* Own SLA+R has been received; ACK has been returned */
            I2C_WriteData(i2c_buff[(uint8_t)i2c_cnt]);
            i2c_cnt++;
            I2C_Config(I2C_CON_AA);
        } break;
        case 0xB8: { /* Data byte has been transmitted; ACK has been received */
            I2C_WriteData(i2c_buff[(uint8_t)i2c_cnt]);
            i2c_cnt++;
            I2C_Config(I2C_CON_AA);
        } break;
        case 0xC0: { /* Data byte has been transmitted; NOT ACK has been received */
            I2C_Config(I2C_CON_AA);
        } break;
        case 0xA0: { /* A STOP or repeated START has been received */
            I2C_Config(I2C_CON_AA);
        } break;
        default: {
            I2C_Config(I2C_CON_AA);
        } break;
    }
}



void mcu_stop_mode(void) 
{
		__disable_irq();
	
		NVIC_DisableIRQ(I2C_IRQn);
		NVIC_DisableIRQ(USB_IRQn);
	
		USBD_DeInit();
		USBD_Disconnect();
	
		EXTI_InitTypeDef EXTI_InitStructure;
		RCC_APBPeriphClockCmd(RCC_APBPeriph_GPIO | RCC_APBPeriph_EXTI, ENABLE);

		/* Enable and set EXTI1_0_IRQn Interrupt priority */
		NVIC_SetPriority(EXTI1_0_IRQn, 0);
		NVIC_EnableIRQ(EXTI1_0_IRQn);

		/* Configure PA0 as input pull-down mode */
		GPIO_Init(GPIOC, GPIO_Pin_0, GPIO_MODE_IN |GPIO_PUPD_UP);
		/* Initializes the EXTI configuration structure */
		EXTI_StructInit(&EXTI_InitStructure);

		/* Configure EXTI0 line */
		EXTI_InitStructure.EXTI_Line = EXTI_Line0;
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
		EXTI_Init(&EXTI_InitStructure);
		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource0);


    EXTI->PR = 0xFFFFFFFF;

    PWR->STOP_CR0 = 0xFFE00;
    PWR->STOP_CR1 = 0xFFE00;
    PWR->STOP_CR2 = 0x27;
    PWR->STOP_CR3 = 0x1E1;
    PWR->STOP_CR4 = 0x7FF;
    PWR->STOP_CR5 = 0x5;
    PWR->ANAKEY1  = 2;

    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    __WFI();
    SCB->SCR &= (~SCB_SCR_SLEEPDEEP_Msk);

		RCC->GPREG0 = 0x00;
		

		NVIC_SystemReset();
		while (1);
}

void EXTI1_0_IRQHandler(void)
{
	NVIC_DisableIRQ(EXTI1_0_IRQn);
  /* Check trigger EXTI line */
  if (EXTI_GetFlagStatus(EXTI_Line0) != RESET)
  {
    /* Clear EXTI line 0 pending bit */
    EXTI_ClearFlag(EXTI_Line0);
  }

  if (EXTI_GetFlagStatus(EXTI_Line1) != RESET)
  {
    EXTI_ClearFlag(EXTI_Line1);
  }

}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif
