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
 * 设备信息读回区 — 复用生产模式地址 (0x92~0xF5)
 * 正常模式: NU17112 读 Flash → 写 i2c_buff[0x92~0xF5] → WB7720 构建 Type 0x03
 * 生产模式: PC → i2c_buff[0x92~0xF5] → NU17112 读取写 Flash
 * 两种模式时序互斥，无需独立地址区。
 * ----------------------------------------------------------------------- */
/* DEVINFO 地址 = PROD 地址，见上方 REG_PROD_* 定义 */

/* -----------------------------------------------------------------------
 * CMD 命令码 (上位机 → 下位机, Vendor_Request[0])
 * ----------------------------------------------------------------------- */
#define CMD_READ_STATUS         0x01  // 请求遥测(Type 0x01) 或异常日志(Type 0x02), round-robin
#define CMD_READ_DEVICE_INFO    0x02  // 请求设备信息(Type 0x03), Vendor_Request[1]=sub_idx
#define CMD_REBOOT              0x0B  // USB 断开并重启 MCU
#define CMD_WRITE_REGISTER      0x0C  // 写 i2c_buff 寄存器

/* -----------------------------------------------------------------------
 * HID Report Type (下位机 → 上位机, report_buffer[4])
 * ----------------------------------------------------------------------- */
#define REPORT_TYPE_TELEMETRY       0x01  // 遥测数据
#define REPORT_TYPE_EXCEPTION_LOG   0x02  // 异常日志
#define REPORT_TYPE_DEVICE_INFO     0x03  // 设备信息 (3 子页, SubIdx 区分)

/* -----------------------------------------------------------------------
 * 异常日志滚动写入区 (i2c_buff 0x37~0x4D, NU17112 写入, WB7720 读取缓存)
 * 见接口规范 v1.3 §3.2b
 * ----------------------------------------------------------------------- */
#define REG_EXC_TOTAL_COUNT     0x37  // NU17112 有效异常记录总数 (0~5)
#define REG_EXC_CURRENT_IDX     0x38  // 本次写入的记录序号 (0~4, 循环)
#define REG_EXC_RESERVED        0x39  // 保留, 固定 0x00
#define REG_EXC_RECORD          0x3A  // 当前 BatteryExceptionRecord_t (20B, 0x3A~0x4D)
#define EXC_RECORD_SIZE         20    // 每条异常记录大小 (字节)
#define EXC_CACHE_MAX           5     // WB7720 本地最多缓存 5 条

/* -----------------------------------------------------------------------
 * 睡眠/唤醒命令寄存器 (从 0x44/0x45 迁移至 0x4E/0x4F, 避开异常日志区)
 * ----------------------------------------------------------------------- */
#define REG_SLEEP_CMD           0x4E  // 睡眠命令寄存器
#define REG_WAKE_CMD            0x4F  // 唤醒命令寄存器

/* -----------------------------------------------------------------------
 * 工程模式寄存器地址
 * ----------------------------------------------------------------------- */
#define REG_WORK_MODE           0x50  // 工程模式标志: 0xA5=进入工程模式, NU17112 处理后写 0x00
#define REG_ENG_CURRENT_DATE    0x60  // 当前日期 [年Lo][年Hi][月][日]
#define REG_ENG_PRODUCTION_DATE 0x70  // 生产日期 [年Lo][年Hi][月][日]
#define REG_ENG_CYCLE_COUNT     0x80  // 循环次数 [次Lo][次Hi]

#endif /* __CONFIG_H */
