#ifndef __CONFIG_H
#define __CONFIG_H

#define FW_VERSION 0x0100         // 0.1.00
#define FW_NAME    "mobile power" // 固件名称
#define FW_ADDRESS 0x08000200     // 固件地址
#define USAGE_PAGE 0xFF00         // USAGE PAGE
#define USAGE_ID   0x01           // USAGE ID

#define MANUFACTURER  "WestBerryTech" // 制造商
#define PRODUCT       "Mobile Power"  // 名称
#define VENDOR_ID     0xFFFF          // USB VID
#define PRODUCT_ID    0xFFFF          // USB PID
#define DEVICE_VER    0x0100          // 版本号
#define SERIAL_NUMBER "00000001"

/* -----------------------------------------------------------------------
 * 安全使用年限 (GB/T 35590 要求, 固定值, 不从 NU17112 读取)
 * ----------------------------------------------------------------------- */
#define SAFETY_SERVICE_YEARS    5     // 安全使用年限 5 年

/* -----------------------------------------------------------------------
 * 生产模式寄存器地址 (i2c_buff 地址, PC → WB7720 → NU17112 → Flash)
 * ----------------------------------------------------------------------- */
#define REG_PROD_MODE_FLAG      0x90  // 生产模式触发标志: PC 写 0xB5 触发, NU17112 写 0x00 清除
#define REG_PROD_WRITE_STATUS   0x91  // 写入状态回报: 0x01=进行中, 0x02=成功, 0xFF=失败 (NU17112→WB7720)
#define REG_PROD_MANUFACTURER   0x92  // 生产厂家 ASCII 20B → Flash ProductInfo_t.manufacturer_name
#define REG_PROD_MODEL          0xA6  // 产品型号 ASCII 20B → Flash ProductInfo_t.model_name
#define REG_PROD_BATTERY_MFR    0xBA  // 电池生产厂商 ASCII 20B → Flash ProductInfo_t.battery_mfr
#define REG_PROD_BATTERY_MODEL  0xCE  // 电池型号 ASCII 20B → Flash ProductInfo_t.battery_model
#define REG_PROD_PROD_DATE      0xE2  // 电池生产日期 ASCII 20B → Flash ProductInfo_t.battery_prod_date

/* -----------------------------------------------------------------------
 * 设备信息读回区 (NU17112 开机写入 i2c_buff, 供 WB7720 响应 CMD_READ_DEVICE_INFO)
 * 地址范围: 0x18~0x6B (待 NU17112 固件确认, 当前暂定)
 * ----------------------------------------------------------------------- */
#define REG_DEVINFO_MANUFACTURER    0x18  // 生产厂家 ASCII 20B (ProductInfo_t.manufacturer_name)
#define REG_DEVINFO_MODEL           0x2C  // 产品型号 ASCII 20B (ProductInfo_t.model_name)
#define REG_DEVINFO_BATTERY_MFR     0x40  // 电池生产厂商 ASCII 20B (ProductInfo_t.battery_mfr)
#define REG_DEVINFO_BATTERY_MODEL   0x54  // 电池型号 ASCII 20B (ProductInfo_t.battery_model)
#define REG_DEVINFO_PROD_DATE       0x68  // 电池生产日期 ASCII 20B (ProductInfo_t.battery_prod_date)

/* -----------------------------------------------------------------------
 * 工程模式寄存器地址
 * ----------------------------------------------------------------------- */
#define REG_WORK_MODE           0x50  // 工程模式标志: 0xA5=进入工程模式, NU17112 处理后写 0x00
#define REG_ENG_CURRENT_DATE    0x60  // 当前日期 [年Lo][年Hi][月][日]
#define REG_ENG_PRODUCTION_DATE 0x70  // 生产日期 [年Lo][年Hi][月][日]
#define REG_ENG_CYCLE_COUNT     0x80  // 循环次数 [次Lo][次Hi]

#endif /* __CONFIG_H */
