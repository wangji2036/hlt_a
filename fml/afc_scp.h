#ifndef AFC_SCP_H_
#define AFC_SCP_H_

#include "typdef.h"


union scp_packet_t
{
	struct
	{
		uint8_t msg_len;
		uint8_t msg_0;
		uint8_t msg_1;
		uint8_t msg_2;
		uint8_t msg_3;
		uint8_t msg_4;
		uint8_t msg_5;
		uint8_t msg_6;
		uint8_t msg_7;
		uint8_t msg_8;
		uint8_t msg_9;
		uint8_t msg_10;
	} bytes;
	uint32_t words[3];
}  __attribute__ ((packed));

#define CMD_SINGLE_READ                     (0x0C)                  //单字节读
#define CMD_SINGLE_WRITE                    (0x0B)                  //单字节写
#define CMD_MULTI_READ                      (0x1C)                  //多字节读
#define CMD_MULTI_WRITE                     (0x1B)                  //多字节写


#define FCP_NACK                            (0x03)                  //nack
#define FCP_ACK                             (0x08)                  //ack
#define FCP_DEVICE_TYPE0                    (0x00)                  //5V/2A--9V/1.67A--12V/1.25A
#define FCP_DEVICE_TYPE1                    (0x01)                  //5V/2A--9V/2A--12V/2A
#define FCP_DEVICE_TYPE2                    (0x02)                  //5V/2A--9V/2A--12V/1.5A
#define FCP_DEVICE_TYPE                     FCP_DEVICE_TYPE1        //select device type
#define FCP_DISCRETE_VOUT_SUPPORT           (0x01)                  //discrete voltage mode support
#define FCP_DISCRETE_CAPABILITIES           (0x02)                  //diccrete voltage cnt
#define FCP_MAX_POWER                       (36)                    //12V/3A=24W
#define FCP_DISCRETE_VOUT0                  (50)                    //9V/2A
#define FCP_DISCRETE_VOUT1                  (90)                    //9V/2A
#define FCP_DISCRETE_VOUT2                  (120)                   //12V/2A
#define SCP_ADP_TYPE1                       (0xBC)                  //SCP充电器类型 B类标准SCP C=0x91 A=0x90
#define SCP_B_ADP_TYPE                      (0x10)                  //B类充电器类型:B类大电流支持电压和电流调节
#define SCP_VENDER_ID_H                     (0x00)                  //厂商ID高8位
#define SCP_VENDER_ID_L                     (0xB1)                  //厂商ID低8位
#define SCP_MODULE_ID_H                     (0x01)                  //B类充电器型号代码高8位
#define SCP_MODULE_ID_L                     (0x00)                  //B类充电器型号代码低8位
#define SCP_SERIAL_NUM_H                    (0x04)                  //B类充电器产品序列号-出厂年份(2015+0x01)
#define SCP_SERIAL_NUM_L                    (0x08)                  //B类充电器产品序列号-出厂周数
#define SCP_PROTOCOL_CHIP_ID                (0x01)                  //协议芯片CHIP ID
#define SCP_HW_VER                          (0x01)                  //B类充电器硬件版本
#define SCP_SW_VER_H                        (0x01)                  //SCP软件版本1.xx
#define SCP_SW_VER_L                        (0x30)                  //SCP软件版本1.30
#define SCP_ADP_B_TYPE1                     (0x03)                  //B类充电器型号
#define SCP_FACTORY_ID                      (0x00)                  //制造商ID
#define SCP_MAX_PWR                         (0x98)                  //MAX_PWR=0x28*10^1*0.1=40W
#define SCP_CNT_PWR                         (0x9E)                  //CNT_PWR=0x1E*10^1*0.1=30W
#define SCP_MIN_VOUT                        (0xB7)                  //MIN_VOUT=5500mV
#define SCP_MAX_VOUT                        (0xCA)                  //MAX_VOUT=10000mV
#define SCP_MIN_IOUT                        (0x5E)                  //MIN_IOUT=300mA
#define SCP_MAX_IOUT                        (0x94)                  //MAX_IOUT=2000mA
#define SCP_VSTEP                           (0x14)                  //VSTEP=20mV
#define SCP_ISTEP                           (0x64)                  //ISTEP=100mA
#define SCP_MAX_VERR                        (0x94)                  //MAX_VERR=0x14*10=200mV
#define SCP_MAX_IERR                        (0x19)                  //MAX_IERR=0x14*10=200mA
#define SCP_MAX_STTIME                      (0x32)                  //MAX_STTIME=0x32*1=50ms
#define SCP_MAX_RSPTIME                     (0x14)                  //MAX_RSPTIME=0x14*1=20ms





#define FCP_REG_DEVICE_TYPE                 (0x00)
#define FCP_REG_SPEC_VER                    (0x01)
#define FCP_REG_SLAVE_CTRL                  (0x02)
#define FCP_REG_SLAVE_STATE                 (0x03)
#define FCP_REG_ID_OUT0                     (0x04)
#define FCP_REG_CAPABILITIES                (0x20)
#define FCP_REG_DISCRETE_CAPABILITIES       (0x21)
#define FPC_REG_MAX_PWR                     (0x22)                  //max out power = MAX_PWR/2
#define FCP_REG_ADAPTER_STATUS              (0x28)
#define FCP_REG_VOUT_STATUS                 (0x29)                  //out voltage = VOUT_STATUS/10
#define FCP_REG_OUTPUT_CTRL                 (0x2B)                  //update vout from VOUT_CONFIG
#define FCP_REG_VOUT_CONFIG                 (0x2C)                  //50=5000mV
#define FCP_REG_DISCRETE_VOUT0              (0x30)
#define FCP_REG_DISCRETE_VOUT1              (0x31)
#define FCP_REG_DISCRETE_VOUT2              (0x32)
#define FCP_REG_DISCRETE_VOUT3              (0x33)
#define FCP_REG_DISCRETE_VOUT4              (0x34)
#define FCP_REG_DISCRETE_VOUT5              (0x35)
#define FCP_REG_DISCRETE_VOUT6              (0x36)
#define FCP_REG_DISCRETE_VOUT7              (0x37)
#define SCP_REG_ADP_TYPE0                   (0x7E)
#define SCP_REG_ADP_TYPE1                   (0x80)
#define SCP_REG_B_ADP_TYPE                  (0x81)
#define SCP_REG_VENDER_ID_H                 (0x82)
#define SCP_REG_VENDER_ID_L                 (0x83)
#define SCP_REG_MODULE_ID_H                 (0x84)
#define SCP_REG_MODULE_ID_L                 (0x85)
#define SCP_REG_SERIAL_NUM_H                (0x86)
#define SCP_REG_SERIAL_NUM_L                (0x87)
#define SCP_REG_CHIP_ID                     (0x88)
#define SCP_REG_HW_VER                      (0x89)
#define SCP_REG_FW_VER_H                    (0x8A)
#define SCP_REG_FW_VER_L                    (0x8B)
#define SCP_REG_B_ADP_TYPE1                 (0x8D)
#define SCP_REG_FACTORY_ID                  (0x8E)
#define SCP_REG_RESEVED0                    (0x8F)
#define SCP_REG_MAX_PWR                     (0x90)
#define SCP_REG_CNT_PWR                     (0x91)
#define SCP_REG_MIN_VOUT                    (0x92)
#define SCP_REG_MAX_VOUT                    (0x93)
#define SCP_REG_MIN_IOUT                    (0x94)
#define SCP_REG_MAX_IOUT                    (0x95)
#define SCP_REG_VSTEP                       (0x96)
#define SCP_REG_ISTEP                       (0x97)
#define SCP_REG_MAX_VERR                    (0x98)
#define SCP_REG_MAX_IERR                    (0x99)
#define SCP_REG_MAX_STTIME                  (0x9A)
#define SCP_REG_MAX_RSPTIME                 (0x9B)
#define SCP_REG_RESEVED1                    (0x9C)
#define SCP_REG_CTRL_BYTE0                  (0xA0)
#define SCP_REG_CTRL_BYTE1                  (0xA1)
#define SCP_REG_STATUS_BYTE0                (0xA2)
#define SCP_REG_STATUS_BYTE1                (0xA3)
#define SCP_REG_STATUS_BYTE2                (0xA4)
#define SCP_REG_SSTS                        (0xA5)
#define SCP_REG_INSIDE_TMP                  (0xA6)
#define SCP_REG_PORT_TMP                    (0xA7)
#define SCP_REG_READ_VOUT_H                 (0xA8)
#define SCP_REG_READ_VOUT_L                 (0xA9)
#define SCP_REG_READ_IOUT_H                 (0xAA)
#define SCP_REG_READ_IOUT_L                 (0xAB)
#define SCP_REG_DAC_VSET_H                  (0xAC)
#define SCP_REG_DAC_VSET_L                  (0xAD)
#define SCP_REG_DAC_ISET_H                  (0xAE)
#define SCP_REG_DAC_ISET_L                  (0xAF)
#define SCP_REG_VSET_BOUNDARY_H             (0xB0)
#define SCP_REG_VSET_BOUNDARY_L             (0xB1)
#define SCP_REG_ISET_BOUNDARY_H             (0xB2)
#define SCP_REG_ISET_BOUNDARY_L             (0xB3)
#define SCP_REG_MAX_VSET_OFFSET             (0xB4)
#define SCP_REG_MAX_ISET_OFFSET             (0xB5)
#define SCP_REG_VSET_H                      (0xB8)
#define SCP_REG_VSET_L                      (0xB9)
#define SCP_REG_ISET_H                      (0xBA)
#define SCP_REG_ISET_L                      (0xBB)
#define SCP_REG_VSET_OFFSET_H               (0xBC)
#define SCP_REG_VSET_OFFSET_L               (0xBD)
#define SCP_REG_ISET_OFFSET_H               (0xBE)
#define SCP_REG_ISET_OFFSET_L               (0xBF)
#define SCP_REG_SREAD_VOUT                  (0xC8)
#define SCP_REG_SREAD_IOUT                  (0xC9)
#define SCP_REG_VSSET                       (0xCA)
#define SCO_REG_ISSET                       (0xCB)
#define SCP_REG_STEP_VSET_OFFSET            (0xCC)
#define SCP_REG_STEP_ISET_OFFSET            (0xCD)
#define SCP_REG_SPEC_FUN1                   (0xCE)
#define SCP_REG_SPEC_FUN2                   (0xCF)
#define SCP_TEST_S_REG_SS                   (0xFF)

extern union scp_packet_t scp_packet;
void dpdm_src_afc_handle(void);
extern uint16_t scp_vout;
extern uint16_t scp_iout;

#endif
