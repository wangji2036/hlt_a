#ifndef __BAT_RECORD_H__
#define __BAT_RECORD_H__

#include "g_data.h"
#include <stdbool.h>

/********************* Flash Address Definitions *********************/
#define LOG_PAGE_COUNT          2
#define MAX_RECORDS_PER_PAGE    24
#define FLASH_LOG_PAGE1         0x1400
#define FLASH_LOG_PAGE2         0x1200
#define FLASH_PAGE_SIZE         512

#define GET_ACTIVE_PAGE_ADDR(p) ((p) == 0 ? FLASH_LOG_PAGE1 : FLASH_LOG_PAGE2)

/********************* Version Management *********************/
#define MAGIC_VALUE_V5          0x42415430  /* 'BAT0' */
#define MAGIC_VALUE_V6          0x42415436  /* 'BAT6' */
#define MAGIC_VALUE_V7          0x42415437  /* 'BAT7' */
#define MAGIC_VALUE             MAGIC_VALUE_V7

/********************* Exception Types *********************/
#define EXCEPTION_TYPE_OVERVOLTAGE  0x01
#define EXCEPTION_TYPE_OVERTEMP     0x02
#define EXCEPTION_TYPE_UNDERTEMP    0x03

#if CONFIG_NEW_CCC_LOG_ENABLE

/********************* Window Tracker Structures (GB31241) *********************/

typedef struct {
    bool     triggered;
    uint16_t max_voltage;
    uint16_t total_voltage;
    TimeStamp_t max_timestamp;
    uint8_t  cell_num;
} OvWindowTracker_t;

typedef struct {
    bool     triggered;
    int16_t  max_temperature;
    TimeStamp_t max_timestamp;
    uint8_t  event_type;
    uint8_t  charge_state;
} TempWindowTracker_t;

/********************* Flash Page Layout (496B per page) *********************/

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint8_t  page_records_count;
    uint8_t  page_number;
    uint8_t  overflow_ptr;
    uint8_t  page_seq;
    uint32_t page_timestamp;
    BatteryExceptionRecord_t records[MAX_RECORDS_PER_PAGE];
    uint16_t checksum;
    uint16_t padding;
} FlashPageLayout_t;

/********************* Public API *********************/

void    battery_record_init(void);
void    battery_record_update_overvoltage(void);
void    battery_record_update_temperature(void);
void    battery_record_periodic_check(void);
uint8_t battery_record_read_exceptions(BatteryExceptionRecord_t *buf, uint8_t max_count);
void    battery_record_print_next_log(void);
bool    battery_record_erase_all(void);
void    battery_record_reset_tracking(void);
bool    battery_record_read_by_page_index(uint8_t page, uint8_t index, BatteryExceptionRecord_t *record);
uint8_t battery_record_get_page_count(uint8_t page);
uint16_t battery_record_get_overtemp_count(void);
uint16_t battery_record_get_overvolt_count(void);
bool    battery_record_is_tracking_active(void);
void    battery_record_sleep_check(void);

#endif /* CONFIG_NEW_CCC_LOG_ENABLE */
#endif /* __BAT_RECORD_H__ */
