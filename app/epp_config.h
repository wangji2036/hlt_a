#ifndef __EPP_CONFIG_H__
#define __EPP_CONFIG_H__

//启用合同数量检测

//启用EPP FOD 在线Calibration功能
#define EPP_FUNC_ONLINE_CALIB_ENABLE 0

//EPP Qi Version，在Nego阶段向Rx回报的Qi版本号
#define EPP_PROTOCOL_QI_VERSION 0x20U

//在EPP1.3及以上版本支持Data Streams和Authentication功能
#if (EPP_PROTOCOL_QI_VERSION < 0x13)
#define QI_DATASTREAM                               0 //< Enable/disable Data Streams features
#define QI_AUTHENTICATION                           0 //< Enable/ Disable Authentication application
#else
#define QI_DATASTREAM                               1 //< Enable/disable Data Streams features
#define QI_AUTHENTICATION                           1 //< Enable/ Disable Authentication application
#endif

//制造商ID
#define EPP_MANUFACTURER_CODE 0x005CU
#define EPP_MANUFACTURER_CODE_LSB (EPP_MANUFACTURER_CODE & 0xFF)
#define EPP_MANUFACTURER_CODE_MSB ((EPP_MANUFACTURER_CODE >> 8) & 0xFF)

// EPP Negotiable Load Power 
#define EPP_CAP_NEGOTIABLE_POWER 0x1EU
// EPP Potential Load Power
#define EPP_CAP_POTENTIAL_POWER 0x1EU
// Dup AR OB Buffer Size WPID NRS
#define EPP_CAP_DUP 0U
#if(QI_AUTHENTICATION == 1)
#define EPP_CAP_AR 1U       //authentication required
#else
#define EPP_CAP_AR 0U       //authentication not required
#endif
#define EPP_CAP_OB 0U       
#define EPP_CAP_BUFFER_SIZE 0x07U
#define EPP_CAP_WPID 0U
#define EPP_CAP_NRS 0U

// EPP Timeout
#define EPP_RP1_TIMEOUT 5050U // 5 seconds
#define EPP_RP2_TIMEOUT 21000U // 21 seconds
#define EPP_RP0_TIMEOUT 21000U

#define EPP_CEP_TIMEOUT 2050U // 1.6 seconds

// EPP Retry Count
#define EPP_RETRY_COUNT 3U

// EPP Retry Interval
#define EPP_RETRY_INTERVAL 1000U // 1 second


#define EPP_TX_MODULE_VERSION "1.0.0"
#define EPP_TX_MODULE_DESCRIPTION "This is a MPP EPP TX Module."



// Max buffer size of transport layer from PTx side
#define ADT_SIZE_CONFIG     					 0x07U
#define ADC_TYPE_END         					 0x00U      // ADC/end: Close outgoing data transport stream
#define ADC_TYPE_AUTH        					 0x02U      // ADC/auth: Open authentication data transport stream
#define ADC_TYPE_RST         					 0x05U      // ADC/rst: Reset all incoming and outgoing data transport streams
#define ADC_TYPE_PROP_0x10   					 0x10U      // ADC/prop: 0x10 is a test proprietary channel other could be added if needed

// DATA_STREAM_RESPONSE (DSR) Types
#define DSR_NAK       							 0x00U      // Data Stream Response is NACK
#define DSR_POLL      							 0x33U      // Data Stream Response is POLL
#define DSR_ND        							 0x55U      // Data Stream Response is Not-Defined
#define DSR_ACK       							 0xFFU      // Data Stream Response is ACK

#define QI_MSG_MAX_SIZE                             32
#define QI_PACKET_SIZE                 QI_MSG_MAX_SIZE      // Size of max Qi packet data is used
#define TRANSMIT_BUFFER_SIZE                     0x09U      // ADT max length is 7 bytes plus 1 byte header and 1 checksum byte



#endif // __EPP_CONFIG_H__